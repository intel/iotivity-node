/* net.c -- CoAP network interface
 *
 * Copyright (C) 2010--2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif HAVE_SYS_UNISTD_H
#include <sys/unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef WITH_LWIP
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/timers.h>
#endif

#include "debug.h"
#include "mem.h"
#include "str.h"
#include "async.h"
#include "option.h"
#include "encode.h"
#include "block.h"
#include "net.h"

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)

#include <ocsocket.h>
#include <logger.h>
#if defined(WITH_DTLS)
#include "netdtls.h"
#endif /* WITH_DTLS */

#define MOD_NAME ("net.c")

time_t clock_offset;

static inline coap_queue_t *
coap_malloc_node() {
    return (coap_queue_t *)coap_malloc(sizeof(coap_queue_t));
}

static inline void
coap_free_node(coap_queue_t *node) {
    coap_free(node);
}
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_LWIP

#include <lwip/memp.h>

static void coap_retransmittimer_execute(void *arg);
static void coap_retransmittimer_restart(coap_context_t *ctx);

static inline coap_queue_t *
coap_malloc_node() {
    return (coap_queue_t *)memp_malloc(MEMP_COAP_NODE);
}

static inline void
coap_free_node(coap_queue_t *node) {
    memp_free(MEMP_COAP_NODE, node);
}

#endif /* WITH_LWIP */
#ifdef WITH_CONTIKI
# ifndef DEBUG
#  define DEBUG DEBUG_PRINT
# endif /* DEBUG */

#include "memb.h"
#include "net/uip-debug.h"

clock_time_t clock_offset;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])

void coap_resources_init();
void coap_pdu_resources_init();

unsigned char initialized = 0;
coap_context_t the_coap_context;

MEMB(node_storage, coap_queue_t, COAP_PDU_MAXCNT);

PROCESS(coap_retransmit_process, "message retransmit process");

static inline coap_queue_t *
coap_malloc_node() {
    return (coap_queue_t *)memb_alloc(&node_storage);
}

static inline void
coap_free_node(coap_queue_t *node) {
    memb_free(&node_storage, node);
}
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP

/** Callback to udp_recv when using lwIP. Gets called by lwIP on arriving
 * packages, places a reference in context->pending_package, and calls
 * coap_read to process the package. Thus, coap_read needs not be called in
 * lwIP main loops. (When modifying this for thread-like operation, ie. if you
 * remove the coap_read call from this, make sure that coap_read gets a chance
 * to run before this callback is entered the next time.)
 */
static void received_package(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    struct coap_context_t *context = (coap_context_t *)arg;

    LWIP_ASSERT("pending_package was not cleared.", context->pending_package == NULL);

    context->pending_package = p; /* we don't free it, coap_read has to do that */
    context->pending_address.addr = addr->addr; /* FIXME: this has to become address-type independent, probably there'll be an lwip function for that */
    context->pending_port = port;

    coap_read(context, -1); /* we want to read from unicast socket */
}

#endif /* WITH_LWIP */

unsigned int coap_adjust_basetime(coap_context_t *ctx, coap_tick_t now) {
    unsigned int result = 0;
    coap_tick_diff_t delta = now - ctx->sendqueue_basetime;

    if (ctx->sendqueue) {
        /* delta < 0 means that the new time stamp is before the old. */
        if (delta <= 0) {
            ctx->sendqueue->t -= delta;
        } else {
            /* This case is more complex: The time must be advanced forward,
             * thus possibly leading to timed out elements at the queue's
             * start. For every element that has timed out, its relative
             * time is set to zero and the result counter is increased. */

            coap_queue_t *q = ctx->sendqueue;
            coap_tick_t t = 0;
            while (q && (t + q->t < (coap_tick_t) delta)) {
                t += q->t;
                q->t = 0;
                result++;
                q = q->next;
            }

            /* finally adjust the first element that has not expired */
            if (q) {
                q->t = (coap_tick_t) delta - t;
            }
        }
    }

    /* adjust basetime */
    ctx->sendqueue_basetime += delta;

    return result;
}

int coap_insert_node(coap_queue_t **queue, coap_queue_t *node) {
    coap_queue_t *p, *q;
    if (!queue || !node)
        return 0;

    /* set queue head if empty */
    if (!*queue) {
        *queue = node;
        return 1;
    }

    /* replace queue head if PDU's time is less than head's time */
    q = *queue;
    if (node->t < q->t) {
        node->next = q;
        *queue = node;
        q->t -= node->t; /* make q->t relative to node->t */
        return 1;
    }

    /* search for right place to insert */
    do {
        node->t -= q->t; /* make node-> relative to q->t */
        p = q;
        q = q->next;
    } while (q && q->t <= node->t);

    /* insert new item */
    if (q) {
        q->t -= node->t; /* make q->t relative to node->t */
    }
    node->next = q;
    p->next = node;
    return 1;
}

int coap_delete_node(coap_queue_t *node) {
    if (!node)
        return 0;

    coap_delete_pdu(node->pdu);
    coap_free_node(node);

    return 1;
}

void coap_delete_all(coap_queue_t *queue) {
    if (!queue)
        return;

    coap_delete_all(queue->next);
    coap_delete_node(queue);
}

coap_queue_t *
coap_new_node() {
    coap_queue_t *node;
    node = coap_malloc_node();

    if (!node) {
#ifndef NDEBUG
        coap_log(LOG_WARNING, "coap_new_node: malloc\n");
#endif
        return NULL;
    }

    memset(node, 0, sizeof *node);
    return node;
}

coap_queue_t *
coap_peek_next(coap_context_t *context) {
    if (!context || !context->sendqueue)
        return NULL;

    return context->sendqueue;
}

coap_queue_t *
coap_pop_next(coap_context_t *context) {
    coap_queue_t *next;

    if (!context || !context->sendqueue)
        return NULL;

    next = context->sendqueue;
    context->sendqueue = context->sendqueue->next;
    if (context->sendqueue) {
        context->sendqueue->t += next->t;
    }
    next->next = NULL;
    return next;
}

#ifdef COAP_DEFAULT_WKC_HASHKEY
/** Checks if @p Key is equal to the pre-defined hash key for.well-known/core. */
#define is_wkc(Key)                         \
  (memcmp((Key), COAP_DEFAULT_WKC_HASHKEY, sizeof(coap_key_t)) == 0)
#else
/* Implements a singleton to store a hash key for the .wellknown/core
 * resources. */
int
is_wkc(coap_key_t k) {
    static coap_key_t wkc;
    static unsigned char _initialized = 0;
    if (!_initialized) {
        _initialized = coap_hash_path((unsigned char *)COAP_DEFAULT_URI_WELLKNOWN,
                sizeof(COAP_DEFAULT_URI_WELLKNOWN) - 1, wkc);
    }
    return memcmp(k, wkc, sizeof(coap_key_t)) == 0;
}
#endif

coap_context_t *
coap_new_context(uint8_t ipAddr[], uint16_t port) {

    OCDevAddr devAddr;
    coap_address_t* listen_addr;

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
    coap_context_t *c = (coap_context_t*)coap_malloc( sizeof( coap_context_t ) );
    //int reuse = 1;
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_LWIP
    coap_context_t *c = memp_malloc(MEMP_COAP_CONTEXT);
#endif /* WITH_LWIP */
#ifdef WITH_CONTIKI
    coap_context_t *c;

    if (initialized)
    return NULL;
#endif /* WITH_CONTIKI */

    OCBuildIPv4Address(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], port,
            &devAddr);

    listen_addr = (coap_address_t*) &devAddr;

    if (!listen_addr) {
        coap_free(c);
        coap_log(LOG_EMERG, "no listen address specified\n");
        return NULL;
    }

    coap_clock_init();
#ifdef WITH_LWIP
    prng_init(LWIP_RAND());
#else /* WITH_LWIP */
    prng_init((unsigned long)listen_addr ^ clock_offset);
#endif /* WITH_LWIP */

#ifndef WITH_CONTIKI
    if (!c) {
#ifndef NDEBUG
        coap_log(LOG_EMERG, "coap_init: malloc:\n");
#endif
        return NULL;
    }
#endif /* not WITH_CONTIKI */
#ifdef WITH_CONTIKI
    coap_resources_init();
    coap_pdu_resources_init();

    c = &the_coap_context;
    initialized = 1;
#endif /* WITH_CONTIKI */

    memset(c, 0, sizeof(coap_context_t));

    /* set well-known sockfd to uninitialize value  */
    c->sockfd_wellknown = -1;

    /* initialize message id */
    prng((unsigned char * )&c->message_id, sizeof(unsigned short));

    /* register the critical options that we know */
    coap_register_option(c, COAP_OPTION_IF_MATCH);
    coap_register_option(c, COAP_OPTION_URI_HOST);
    coap_register_option(c, COAP_OPTION_IF_NONE_MATCH);
    coap_register_option(c, COAP_OPTION_URI_PORT);
    coap_register_option(c, COAP_OPTION_URI_PATH);
    coap_register_option(c, COAP_OPTION_URI_QUERY);
    coap_register_option(c, COAP_OPTION_ACCEPT);
    coap_register_option(c, COAP_OPTION_PROXY_URI);
    coap_register_option(c, COAP_OPTION_PROXY_SCHEME);
    coap_register_option(c, COAP_OPTION_BLOCK2);
    coap_register_option(c, COAP_OPTION_BLOCK1);

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
    if (OCInitUDP((OCDevAddr *)listen_addr,
                    (int32_t *)&(c->sockfd), OC_SOCKET_REUSEADDR) != ERR_SUCCESS) {
        coap_free( c);
        return NULL;
    }

#if defined(WITH_DTLS)
    if (coap_dtls_init(c, ipAddr) != 0) {
        coap_free( c);
        return NULL;
    }
#else
    /* set dtls socket file descriptor to uninitialize value  */
    c->sockfd_dtls = -1;
#endif /* WITH_DTLS */
    return c;

#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
    c->conn = udp_new(NULL, 0, NULL);
    udp_bind(c->conn, listen_addr->port);

    process_start(&coap_retransmit_process, (char *)c);

    PROCESS_CONTEXT_BEGIN(&coap_retransmit_process);
#ifndef WITHOUT_OBSERVE
    etimer_set(&c->notify_timer, COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND);
#endif /* WITHOUT_OBSERVE */
    /* the retransmit timer must be initialized to some large value */
    etimer_set(&the_coap_context.retransmit_timer, 0xFFFF);
    PROCESS_CONTEXT_END(&coap_retransmit_process);
    return c;
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    c->pcb = udp_new();
    /* hard assert: this is not expected to fail dynamically */
    LWIP_ASSERT("Failed to allocate PCB for CoAP", c->pcb != NULL);

    udp_recv(c->pcb, received_package, (void*)c);
    udp_bind(c->pcb, &listen_addr->addr, listen_addr->port);

    c->timer_configured = 0;

    return c;
#endif
    return NULL;
}

void coap_free_context(coap_context_t *context) {
    if (!context)
        return;

    coap_delete_all(context->recvqueue);
    coap_delete_all(context->sendqueue);

#ifdef WITH_LWIP
    context->sendqueue = NULL;
    coap_retransmittimer_restart(context);
#endif

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
    /* coap_delete_list(context->subscriptions); */
    OCClose( context->sockfd );
    if (context->sockfd_wellknown != -1) {
        OCClose( context->sockfd_wellknown );
    }
#if defined(WITH_DTLS)
    coap_dtls_deinit( context );
#endif /* WITH_DTLS */
    coap_free( context );
#endif
#ifdef WITH_LWIP
    udp_remove(context->pcb);
    memp_free(MEMP_COAP_CONTEXT, context);
#endif
#ifdef WITH_CONTIKI
    memset(&the_coap_context, 0, sizeof(coap_context_t));
    initialized = 0;
#endif /* WITH_CONTIKI */
}

int coap_join_wellknown_group(coap_context_t *ctx,
        const coap_address_t *multicast_addr) {
#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
    if (OCInitUDPMulticast((OCDevAddr *)multicast_addr,
                    (int32_t *)&(ctx->sockfd_wellknown)) != ERR_SUCCESS) {
        return 1;
    }
    return 0;
#endif
    return 0;
}

int coap_option_check_critical(coap_context_t *ctx, coap_pdu_t *pdu,
        coap_opt_filter_t unknown) {

    coap_opt_iterator_t opt_iter;
    int ok = 1;

    coap_option_iterator_init(pdu, &opt_iter, COAP_OPT_ALL);

    while (coap_option_next(&opt_iter)) {

        /* The following condition makes use of the fact that
         * coap_option_getb() returns -1 if type exceeds the bit-vector
         * filter. As the vector is supposed to be large enough to hold
         * the largest known option, we know that everything beyond is
         * bad.
         */
        if ((opt_iter.type & 0x01)
                && coap_option_getb(ctx->known_options, opt_iter.type) < 1) {
            debug("unknown critical option %d\n", opt_iter.type);

            ok = 0;

            /* When opt_iter.type is beyond our known option range,
             * coap_option_setb() will return -1 and we are safe to leave
             * this loop. */
            if (coap_option_setb(unknown, opt_iter.type) == -1)
                break;
        }
    }

    return ok;
}

void coap_transaction_id(const coap_address_t *peer, const coap_pdu_t *pdu,
        coap_tid_t *id) {
    coap_key_t h;

    memset(h, 0, sizeof(coap_key_t));

    /* Compare the complete address structure in case of IPv4. For IPv6,
     * we need to look at the transport address only. */

#ifdef WITH_POSIX
    switch (peer->addr.sa.sa_family) {
        case AF_INET:
        coap_hash((const unsigned char *)&peer->addr.sa, peer->size, h);
        break;
        case AF_INET6:
        coap_hash((const unsigned char *)&peer->addr.sin6.sin6_port,
                sizeof(peer->addr.sin6.sin6_port), h);
        coap_hash((const unsigned char *)&peer->addr.sin6.sin6_addr,
                sizeof(peer->addr.sin6.sin6_addr), h);
        break;
        default:
        return;
    }
#endif

#ifdef WITH_ARDUINO
    coap_hash((const unsigned char *)peer->addr, peer->size, h);
#endif /* WITH_ARDUINO */

#if defined(WITH_LWIP) || defined(WITH_CONTIKI)
    /* FIXME: with lwip, we can do better */
    coap_hash((const unsigned char *)&peer->port, sizeof(peer->port), h);
    coap_hash((const unsigned char *)&peer->addr, sizeof(peer->addr), h);
#endif /* WITH_LWIP || WITH_CONTIKI */

    coap_hash((const unsigned char * )&pdu->hdr->id, sizeof(unsigned short), h);

    *id = ((h[0] << 8) | h[1]) ^ ((h[2] << 8) | h[3]);
}

coap_tid_t coap_send_ack(coap_context_t *context, const coap_address_t *dst,
        coap_pdu_t *request, coap_send_flags_t flag) {
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    if (request && request->hdr->type == COAP_MESSAGE_CON) {
        response = coap_pdu_init(COAP_MESSAGE_ACK, 0, request->hdr->id,
                sizeof(coap_pdu_t));
        if (response) {
            result = coap_send(context, dst, response, flag, NULL);
            coap_delete_pdu(response);
        }
    }
    return result;
}

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
/* releases space allocated by PDU if free_pdu is set */
int
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu) {

    int bytes_written = -1;

    if ( !context || !dst || !pdu )
    return bytes_written;

    bytes_written = OCSendTo( context->sockfd, (uint8_t*)(pdu->hdr), pdu->length, 0,
            (OCDevAddr*)dst);
    debug("bytes_written %d\n", (int)bytes_written);

    return bytes_written;
}
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
/* releases space allocated by PDU if free_pdu is set */
coap_tid_t
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu) {
    coap_tid_t id = COAP_INVALID_TID;

    if ( !context || !dst || !pdu )
    return id;

    /* FIXME: is there a way to check if send was successful? */
    uip_udp_packet_sendto(context->conn, pdu->hdr, pdu->length,
            &dst->addr, dst->port);

    coap_transaction_id(dst, pdu, &id);

    return id;
}
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
coap_tid_t
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu) {
    coap_tid_t id = COAP_INVALID_TID;
    struct pbuf *p;
    uint8_t err;
    char *data_backup;

    if ( !context || !dst || !pdu )
    {
        return id;
    }

    data_backup = pdu->data;

    /* FIXME: we can't check this here with the existing infrastructure, but we
     * should actually check that the pdu is not held by anyone but us. the
     * respective pbuf is already exclusively owned by the pdu. */

    p = pdu->pbuf;
    LWIP_ASSERT("The PDU header is not where it is expected", pdu->hdr == p->payload + sizeof(coap_pdu_t));

    err = pbuf_header(p, -sizeof(coap_pdu_t));
    if (err)
    {
        debug("coap_send_impl: pbuf_header failed\n");
        pbuf_free(p);
        return id;
    }

    coap_transaction_id(dst, pdu, &id);

    pbuf_realloc(p, pdu->length);

    udp_sendto(context->pcb, p,
            &dst->addr, dst->port);

    pbuf_header(p, -(ptrdiff_t)((uint8_t*)pdu - (uint8_t*)p->payload) - sizeof(coap_pdu_t)); /* FIXME hack around udp_sendto not restoring; see http://lists.gnu.org/archive/html/lwip-users/2013-06/msg00008.html. for udp over ip over ethernet, this was -42; as we're doing ppp too, this has to be calculated generically */

    err = pbuf_header(p, sizeof(coap_pdu_t));
    LWIP_ASSERT("Cannot undo pbuf_header", err == 0);

    /* restore destroyed pdu data */
    LWIP_ASSERT("PDU not restored", p->payload == pdu);
    pdu->max_size = p->tot_len - sizeof(coap_pdu_t); /* reduced after pbuf_realloc */
    pdu->hdr = p->payload + sizeof(coap_pdu_t);
    pdu->max_delta = 0; /* won't be used any more */
    pdu->length = pdu->max_size;
    pdu->data = data_backup;
    pdu->pbuf = p;

    return id;
}
#endif /* WITH_LWIP */

coap_tid_t coap_send_error(coap_context_t *context, coap_pdu_t *request,
        const coap_address_t *dst, unsigned char code, coap_opt_filter_t opts,
        coap_send_flags_t flag) {
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    assert(request);
    assert(dst);

    response = coap_new_error_response(request, code, opts);
    if (response) {
        result = coap_send(context, dst, response, flag, NULL);
        coap_delete_pdu(response);
    }

    return result;
}

coap_tid_t coap_send_message_type(coap_context_t *context,
        const coap_address_t *dst, coap_pdu_t *request,
        coap_send_flags_t flag, unsigned char type) {
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    if (request) {
        response = coap_pdu_init(type, 0, request->hdr->id, sizeof(coap_pdu_t));
        if (response) {
            result = coap_send(context, dst, response, flag, NULL);
            coap_delete_pdu(response);
        }
    }
    return result;
}

coap_tid_t coap_send(coap_context_t *context,
        const coap_address_t *dst, coap_pdu_t *pdu, coap_send_flags_t flag,
        uint8_t *cache_flag)
{
    coap_queue_t *node = NULL;
    coap_tick_t now;
    coap_tid_t tid;
    int bytesWritten;
    unsigned int r;

    if (!context)
        return COAP_INVALID_TID;
    if(!(flag & SEND_RETX)){
        coap_transaction_id(dst, pdu, &tid);
    }
    if((flag & SEND_NOW) || (flag & SEND_RETX))
    {
        goto sending;
    }

    node = coap_new_node();
    if (!node) {
        debug("coap_send: insufficient memory\n");
        return COAP_INVALID_TID;
    }

    prng((unsigned char * )&r, sizeof(r));
    /* add randomized RESPONSE_TIMEOUT to determine retransmission timeout */
    if(flag & SEND_NOW_CON) {
        node->timeout = COAP_DEFAULT_RESPONSE_TIMEOUT * COAP_TICKS_PER_SECOND
                + (COAP_DEFAULT_RESPONSE_TIMEOUT >> 1)
                * ((COAP_TICKS_PER_SECOND * (r & 0xFF)) >> 8);
    }
    else
    {
        node->timeout = MAX_MULTICAST_DELAY_SEC * ((COAP_TICKS_PER_SECOND * (r & 0xFF)) >> 8);
        node->delayedResNeeded = 1;
    }

    if (flag & SEND_SECURE_PORT) {
        node->secure = 1;
    }

    memcpy(&node->remote, dst, sizeof(coap_address_t));
    node->pdu = pdu;
    node->id = tid;

    /* Set timer for pdu retransmission. If this is the first element in
     * the retransmission queue, the base time is set to the current
     * time and the retransmission time is node->timeout. If there is
     * already an entry in the sendqueue, we must check if this node is
     * to be retransmitted earlier. Therefore, node->timeout is first
     * normalized to the base time and then inserted into the queue with
     * an adjusted relative time.
     */

    coap_ticks(&now);
    if (context->sendqueue == NULL)
    {
        node->t = node->timeout;
        context->sendqueue_basetime = now;
    }
    else
    {
        /* make node->t relative to context->sendqueue_basetime */
        node->t = (now - context->sendqueue_basetime) + node->timeout;
    }
    coap_insert_node(&context->sendqueue, node);

    #ifdef WITH_LWIP
        if (node == context->sendqueue)
            /* don't bother with timer stuff if there are earlier retransmits */
            coap_retransmittimer_restart(context);
    #endif

    #ifdef WITH_CONTIKI
    { /* (re-)initialize retransmission timer */
        coap_queue_t *nextpdu;

        nextpdu = coap_peek_next(context);
        assert(nextpdu); /* we have just inserted a node */

        /* must set timer within the context of the retransmit process */
        PROCESS_CONTEXT_BEGIN(&coap_retransmit_process);
        etimer_set(&context->retransmit_timer, nextpdu->t);
        PROCESS_CONTEXT_END(&coap_retransmit_process);
    }
    #endif /* WITH_CONTIKI */

    if(flag & SEND_NOW_CON)
    {
        goto sending;
    }
    return tid;

    sending:
        OC_LOG_V(DEBUG, MOD_NAME, PCF("sending 0x%x"), flag);
#if defined(WITH_DTLS)
        // A secure packet is first encrypted by DTLS library and then send
        // over the network.
        if (flag & SEND_SECURE_PORT) {
            bytesWritten = coap_dtls_encrypt(context, (OCDevAddr*)dst,
                            pdu, &node, tid, cache_flag);
        } else {
            bytesWritten = coap_send_impl(context, dst, pdu);
        }
#else
        bytesWritten = coap_send_impl(context, dst, pdu);
#endif /* WITH_DTLS */
        if(bytesWritten > 0) {
            return tid;
        }
        debug("coap_send_impl: error sending pdu\n");
        coap_free_node(node);
        return COAP_INVALID_TID;
}

coap_tid_t coap_retransmit(coap_context_t *context, coap_queue_t *node) {
    coap_tid_t tid = COAP_INVALID_TID;
    coap_send_flags_t flag;

    if (!context || !node)
        return COAP_INVALID_TID;

    /* re-initialize timeout when maximum number of retransmissions are not reached yet */
    if (node->retransmit_cnt < COAP_DEFAULT_MAX_RETRANSMIT) {
        node->retransmit_cnt++;
        node->t = node->timeout << node->retransmit_cnt;
        coap_insert_node(&context->sendqueue, node);
#ifdef WITH_LWIP
        if (node == context->sendqueue) /* don't bother with timer stuff if there are earlier retransmits */
        coap_retransmittimer_restart(context);
#endif

        debug("** retransmission #%d of transaction %d\n", node->retransmit_cnt,
            ntohs(node->pdu->hdr->id));
        flag = (coap_send_flags_t)(SEND_RETX | (node->secure ? SEND_SECURE_PORT : 0));
        tid = coap_send(context, (coap_address_t *)&(node->remote),node->pdu, flag, NULL);
        return (tid == COAP_INVALID_TID)? COAP_INVALID_TID : node->id;
    }

    /* no more retransmissions, remove node from system */

#ifndef WITH_CONTIKI
    debug("** removed transaction %d\n", ntohs(node->id));
#endif

    // deletion of node will happen in ocoap since we still need the info node has
    return COAP_INVALID_TID;
}

/**
 * Checks if @p opt fits into the message that ends with @p maxpos.
 * This function returns @c 1 on success, or @c 0 if the option @p opt
 * would exceed @p maxpos.
 */
static inline int check_opt_size(coap_opt_t *opt, unsigned char *maxpos) {
    if (opt && opt < maxpos) {
        if (((*opt & 0x0f) < 0x0f) || (opt + 1 < maxpos))
            return opt + COAP_OPT_SIZE(opt) < maxpos;
    }
    return 0;
}

int coap_read(coap_context_t *ctx, int sockfd) {
#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
    static char buf[COAP_MAX_PDU_SIZE];
#endif
#if defined(WITH_LWIP) || defined(WITH_CONTIKI)
    char *buf;
#endif
  char *pbuf = buf;
  coap_hdr_t *pdu;
  int bytes_read = -1;

  coap_address_t src, dst;
  coap_queue_t *node;
  unsigned char delayedResNeeded = 0;

#ifdef WITH_CONTIKI
    pbuf = uip_appdata;
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    LWIP_ASSERT("No package pending", ctx->pending_package != NULL);
    LWIP_ASSERT("Can only deal with contiguous PBUFs to read the initial details", ctx->pending_package->tot_len == ctx->pending_package->len);
    pbuf = ctx->pending_package->payload;
#endif /* WITH_LWIP */

    coap_address_init(&src);

#if defined(WITH_POSIX) || defined(WITH_ARDUINO)
  bytes_read = OCRecvFrom( sockfd, (uint8_t*)pbuf, sizeof(buf), 0,
              (OCDevAddr*)&src);

  // Set the delayed response flag for responding to multicast requests
  if (sockfd == ctx->sockfd_wellknown && bytes_read > 0) {
      delayedResNeeded = 1;
  }
#if defined(WITH_DTLS)
  // Perform the DTLS decryption if packet is coming on secure port
  if (sockfd == ctx->sockfd_dtls && bytes_read > 0) {
      if (coap_dtls_decrypt(ctx, (OCDevAddr*)&src, (uint8_t*)pbuf, bytes_read,
            (uint8_t**)&pbuf, &bytes_read) < 0) {
            bytes_read = -1;
      }
  }
#endif /* WITH_DTLS */

  pdu = (coap_hdr_t *) pbuf;
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
    if(uip_newdata()) {
        uip_ipaddr_copy(&src.addr, &UIP_IP_BUF->srcipaddr);
        src.port = UIP_UDP_BUF->srcport;
        uip_ipaddr_copy(&dst.addr, &UIP_IP_BUF->destipaddr);
        dst.port = UIP_UDP_BUF->destport;

        bytes_read = uip_datalen();
        ((char *)uip_appdata)[bytes_read] = 0;
        PRINTF("Server received %d bytes from [", (int)bytes_read);
        PRINT6ADDR(&src.addr);
        PRINTF("]:%d\n", uip_ntohs(src.port));
    }
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    /* FIXME: use lwip address operation functions */
    src.addr.addr = ctx->pending_address.addr;
    src.port = ctx->pending_port;
    bytes_read = ctx->pending_package->tot_len;
#endif /* WITH_LWIP */

    if (bytes_read < 0) {
        warn("coap_read: recvfrom\n");
        goto error_early;
    }

    if ((size_t) bytes_read < sizeof(coap_hdr_t)) {
        debug("coap_read: discarded invalid frame\n");
        goto error_early;
    }

    if (pdu->version != COAP_DEFAULT_VERSION) {
        debug("coap_read: unknown protocol version\n");
        goto error_early;
    }

    node = coap_new_node();
    if (!node)
        goto error_early;

#ifdef WITH_LWIP
    node->pdu = coap_pdu_from_pbuf(ctx->pending_package);
    ctx->pending_package = NULL;
#else
    node->pdu = coap_pdu_init(0, 0, 0, bytes_read);
#endif
    if (!node->pdu)
        goto error;

    coap_ticks(&node->t);
    memcpy(&node->local, &dst, sizeof(coap_address_t));
    memcpy(&node->remote, &src, sizeof(coap_address_t));

    if (!coap_pdu_parse((unsigned char *) pbuf, bytes_read, node->pdu)) {
        warn("discard malformed PDU");
        goto error;
    }

    //set the delayed response flag
    node->delayedResNeeded = delayedResNeeded;

    //set the secure flag on the received packet
#if defined(WITH_DTLS)
    node->secure = (sockfd == ctx->sockfd_dtls) ? 1 : 0;
#else
    node->secure = 0;
#endif /* WITH_DTLS */

    /* and add new node to receive queue */
    coap_transaction_id(&node->remote, node->pdu, &node->id);
    coap_insert_node(&ctx->recvqueue, node);

#ifndef NDEBUG
    if (LOG_DEBUG <= coap_get_log_level()) {
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 40
#endif
        unsigned char addr[INET6_ADDRSTRLEN + 8];

        if (coap_print_addr(&src, addr, INET6_ADDRSTRLEN + 8))
            debug("** received %d bytes from %s:\n", (int )bytes_read, addr);

        coap_show_pdu(node->pdu);
    }
#endif

    return bytes_read;

    error:
    /* FIXME: send back RST? */
    coap_delete_node(node);
    return bytes_read;
    error_early:
#ifdef WITH_LWIP
    /* even if there was an error, clean up */
    pbuf_free(ctx->pending_package);
    ctx->pending_package = NULL;
#endif
    return bytes_read;
}

int coap_remove_from_queue(coap_queue_t **queue, coap_tid_t id,
        coap_queue_t **node) {
    coap_queue_t *p, *q;

    if (!queue || !*queue)
        return 0;
    debug("*** looking for transaction %u == %u\n", id, (*queue)->id);

    /* replace queue head if PDU's time is less than head's time */
    if (id == (*queue)->id) { /* found transaction */
        *node = *queue;
        *queue = (*queue)->next;
        if (*queue) { /* adjust relative time of new queue head */
            (*queue)->t += (*node)->t;
        }
        (*node)->next = NULL;
        /* coap_delete_node( q ); */
        debug("*** removed transaction %u\n", id);
        return 1;
    }

    /* search transaction to remove (only first occurence will be removed) */
    q = *queue;
    do {
        p = q;
        q = q->next;
    } while (q && id != q->id);

    if (q) { /* found transaction */
        p->next = q->next;
        if (p->next) { /* must update relative time of p->next */
            p->next->t += q->t;
        }
        q->next = NULL;
        *node = q;
        /* coap_delete_node( q ); */
        debug("*** removed transaction %u\n", id);
        return 1;
    }

    return 0;

}

static inline int token_match(const unsigned char *a, size_t alen,
        const unsigned char *b, size_t blen) {
    return alen == blen && (alen == 0 || memcmp(a, b, alen) == 0);
}

void coap_cancel_all_messages(coap_context_t *context,
        const coap_address_t *dst, const unsigned char *token,
        size_t token_length) {
    /* cancel all messages in sendqueue that are for dst
     * and use the specified token */
    coap_queue_t *p, *q;

    debug("cancel_all_messages\n");
    while (context->sendqueue
            && coap_address_equals(dst, &context->sendqueue->remote)
            && token_match(token, token_length,
                    context->sendqueue->pdu->hdr->token,
                    context->sendqueue->pdu->hdr->token_length)) {
        q = context->sendqueue;
        context->sendqueue = q->next;
        debug("**** removed transaction %d\n", ntohs(q->pdu->hdr->id));
        coap_delete_node(q);
    }

    if (!context->sendqueue)
        return;

    p = context->sendqueue;
    q = p->next;

    /* when q is not NULL, it does not match (dst, token), so we can skip it */
    while (q) {
        if (coap_address_equals(dst, &q->remote)
                && token_match(token, token_length, q->pdu->hdr->token,
                        q->pdu->hdr->token_length)) {
            p->next = q->next;
            debug("**** removed transaction %d\n", ntohs(q->pdu->hdr->id));
            coap_delete_node(q);
            q = p->next;
        } else {
            p = q;
            q = q->next;
        }
    }
}

coap_queue_t *
coap_find_transaction(coap_queue_t *queue, coap_tid_t id) {
    while (queue && queue->id != id)
        queue = queue->next;

    return queue;
}

coap_pdu_t *
coap_new_error_response(coap_pdu_t *request, unsigned char code,
        coap_opt_filter_t opts) {
    coap_opt_iterator_t opt_iter;
    coap_pdu_t *response;
    size_t size = sizeof(coap_hdr_t) + request->hdr->token_length;
    int type;
    coap_opt_t *option;
    unsigned short opt_type = 0; /* used for calculating delta-storage */

#if COAP_ERROR_PHRASE_LENGTH > 0
  const char *phrase = coap_response_phrase(code);

    /* Need some more space for the error phrase and payload start marker */
    if (phrase)
        size += strlen(phrase) + 1;
#endif

    assert(request);

    /* cannot send ACK if original request was not confirmable */
    type = request->hdr->type == COAP_MESSAGE_CON ?
            COAP_MESSAGE_ACK : COAP_MESSAGE_NON;

    /* Estimate how much space we need for options to copy from
     * request. We always need the Token, for 4.02 the unknown critical
     * options must be included as well. */
    coap_option_clrb(opts, COAP_OPTION_CONTENT_TYPE); /* we do not want this */

    coap_option_iterator_init(request, &opt_iter, opts);

    /* Add size of each unknown critical option. As known critical
     options as well as elective options are not copied, the delta
     value might grow.
     */
    while ((option = coap_option_next(&opt_iter))) {
        unsigned short delta = opt_iter.type - opt_type;
        /* calculate space required to encode (opt_iter.type - opt_type) */
        if (delta < 13) {
            size++;
        } else if (delta < 269) {
            size += 2;
        } else {
            size += 3;
        }

        /* add coap_opt_length(option) and the number of additional bytes
         * required to encode the option length */

        size += coap_opt_length(option);
        switch (*option & 0x0f) {
        case 0x0e:
            size++;
            /* fall through */
            continue;
        case 0x0d:
            size++;
            break;
        default:
            ;
        }

        opt_type = opt_iter.type;
    }

    /* Now create the response and fill with options and payload data. */
    response = coap_pdu_init(type, code, request->hdr->id, size);
    if (response) {
        /* copy token */
        if (!coap_add_token(response, request->hdr->token_length,
                request->hdr->token)) {
            debug("cannot add token to error response\n");
            coap_delete_pdu(response);
            return NULL;
        }

        /* copy all options */
        coap_option_iterator_init(request, &opt_iter, opts);
        while ((option = coap_option_next(&opt_iter)))
            coap_add_option(response, opt_iter.type, COAP_OPT_LENGTH(option),
                    COAP_OPT_VALUE(option));

#if COAP_ERROR_PHRASE_LENGTH > 0
        /* note that diagnostic messages do not need a Content-Format option. */
        if (phrase)
            coap_add_data(response, strlen(phrase), (unsigned char *) phrase);
#endif
    }

    return response;
}


#define SZX_TO_BYTES(SZX) ((size_t)(1 << ((SZX) + 4)))

#define WANT_WKC(Pdu,Key)                   \
  (((Pdu)->hdr->code == COAP_REQUEST_GET) && is_wkc(Key))

/************************************************************************************************
 * Following code will be moved to newer handle_request in the future and kept for reference
 ************************************************************************************************/
#if 0
void
handle_request(coap_context_t *context, coap_queue_t *node) {
    coap_method_handler_t h = NULL;
    coap_pdu_t *response = NULL;
    coap_opt_filter_t opt_filter;
    coap_resource_t *resource;
    coap_key_t key;

    coap_option_filter_clear(opt_filter);

    /* try to find the resource from the request URI */
    coap_hash_request_uri(node->pdu, key);
    resource = coap_get_resource_from_key(context, key);

    if (!resource) {
        /* The resource was not found. Check if the request URI happens to
         * be the well-known URI. In that case, we generate a default
         * response, otherwise, we return 4.04 */

        switch(node->pdu->hdr->code) {

            case COAP_REQUEST_GET:
            if (is_wkc(key)) { /* GET request for .well-known/core */
                info("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);
                response = wellknown_response(context, node->pdu);

            } else { /* GET request for any another resource, return 4.04 */

                debug("GET for unknown resource 0x%02x%02x%02x%02x, return 4.04\n",
                        key[0], key[1], key[2], key[3]);
                response =
                coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(404),
                        opt_filter);
            }
            break;

            default: /* any other request type */

            debug("unhandled request for unknown resource 0x%02x%02x%02x%02x\r\n",
                    key[0], key[1], key[2], key[3]);
            if (!coap_is_mcast(&node->local))
            response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405),
                    opt_filter);
        }

        if (response && coap_send(context, &node->remote, response) == COAP_INVALID_TID) {
            warn("cannot send response for transaction %u\n", node->id);
        }
        coap_delete_pdu(response);

        return;
    }

    /* the resource was found, check if there is a registered handler */
    if ((size_t)node->pdu->hdr->code - 1 <
            sizeof(resource->handler)/sizeof(coap_method_handler_t))
    h = resource->handler[node->pdu->hdr->code - 1];

    if (h) {
        debug("call custom handler for resource 0x%02x%02x%02x%02x\n",
                key[0], key[1], key[2], key[3]);
        response = coap_pdu_init(node->pdu->hdr->type == COAP_MESSAGE_CON
                ? COAP_MESSAGE_ACK
                : COAP_MESSAGE_NON,
                0, node->pdu->hdr->id, COAP_MAX_PDU_SIZE);

        /* Implementation detail: coap_add_token() immediately returns 0
         if response == NULL */
        if (coap_add_token(response, node->pdu->hdr->token_length,
                        node->pdu->hdr->token)) {
            str token = {node->pdu->hdr->token_length, node->pdu->hdr->token};

            h(context, resource, &node->remote,
                    node->pdu, &token, response);
            if (response->hdr->type != COAP_MESSAGE_NON ||
                    (response->hdr->code >= 64
                            && !coap_is_mcast(&node->local))) {
                if (coap_send(context, &node->remote, response) == COAP_INVALID_TID) {
                    debug("cannot send response for message %d\n", node->pdu->hdr->id);
                }
            }

            coap_delete_pdu(response);
        } else {
            warn("cannot generate response\r\n");
        }
    } else {
        if (WANT_WKC(node->pdu, key)) {
            debug("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);
            response = wellknown_response(context, node->pdu);
        } else
        response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405),
                opt_filter);

        if (!response || (coap_send(context, &node->remote, response)
                        == COAP_INVALID_TID)) {
            debug("cannot send response for transaction %u\n", node->id);
        }
        coap_delete_pdu(response);
    }
}
#endif
static void handle_request(coap_context_t *context, coap_queue_t *rcvd) {
    /* Call application-specific reponse handler when available.  If
     * not, we must acknowledge confirmable messages. */
    if (context->request_handler) {
        context->request_handler(context, rcvd);
    } else {
        coap_send_flags_t flag = SEND_NOW;
        flag = (coap_send_flags_t)(flag | (rcvd->secure ? SEND_SECURE_PORT : 0));
        /* send ACK if rcvd is confirmable (i.e. a separate response) */
        coap_send_ack(context, &rcvd->remote, rcvd->pdu, flag);
    }
}

static void handle_response(coap_context_t *context, coap_queue_t *rcvd) {
    /* Call application-specific reponse handler when available.  If
     * not, we must acknowledge confirmable messages. */
    if (context->response_handler) {
        context->response_handler(context, rcvd);
    } else {
        coap_send_flags_t flag = SEND_NOW;
        flag = (coap_send_flags_t)(flag | (rcvd->secure ? SEND_SECURE_PORT : 0));
        /* send ACK if rcvd is confirmable (i.e. a separate response) */
        coap_send_ack(context, &rcvd->remote, rcvd->pdu, flag);
    }
}

static void handle_ack_rst(coap_context_t *context, uint8_t msgType, coap_queue_t *sent) {
    /* Call application-specific reponse handler when available.  If
     * not, we must acknowledge confirmable messages. */
    if (context->ack_rst_handler) {
        context->ack_rst_handler(context, msgType, sent);
    }
}

static inline int
#ifdef __GNUC__
handle_locally(coap_context_t *context __attribute__ ((unused)),
        coap_queue_t *node __attribute__ ((unused))) {
#else /* not a GCC */
    handle_locally(coap_context_t *context, coap_queue_t *node) {
#endif /* GCC */
        /* this function can be used to check if node->pdu is really for us */
        return 1;
    }

    void coap_dispatch(coap_context_t *context) {
        coap_queue_t *rcvd = NULL, *sent = NULL;
        coap_pdu_t *response;
        coap_opt_filter_t opt_filter;

        if (!context)
            return;

        memset(opt_filter, 0, sizeof(coap_opt_filter_t));

        while (context->recvqueue) {
            rcvd = context->recvqueue;

            /* remove node from recvqueue */
            context->recvqueue = context->recvqueue->next;
            rcvd->next = NULL;

            if (rcvd->pdu->hdr->version != COAP_DEFAULT_VERSION) {
                debug("dropped packet with unknown version %u\n",
                        rcvd->pdu->hdr->version);
                goto cleanup;
            }

            switch (rcvd->pdu->hdr->type) {
            case COAP_MESSAGE_ACK:
                /* find transaction in sendqueue to stop retransmission */
                if(coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent)){
                    handle_ack_rst(context, COAP_MESSAGE_ACK, sent);
                }

                //delete empty messages, this is ACK only message no piggybacked response
                if (rcvd->pdu->hdr->code == 0)
                    goto cleanup;
                break;

            case COAP_MESSAGE_NON: /* check for unknown critical options */
                if (coap_option_check_critical(context, rcvd->pdu, opt_filter)
                        == 0)
                    goto cleanup;
                break;

            case COAP_MESSAGE_CON: /* check for unknown critical options */
                if (coap_option_check_critical(context, rcvd->pdu, opt_filter)
                        == 0) {
                    /* FIXME: send response only if we have received a request. Otherwise,
                     * send RST. */
                    response = coap_new_error_response(rcvd->pdu,
                            COAP_RESPONSE_CODE(402), opt_filter);
                    if (!response)
                        warn("coap_dispatch: cannot create error reponse\n");
                    else {
                        coap_send_flags_t flag = SEND_NOW;
                        flag = (coap_send_flags_t)(flag | rcvd->secure ? SEND_SECURE_PORT : 0);
                        if (coap_send(context, &rcvd->remote, response, flag, NULL)
                                == COAP_INVALID_TID) {
                            warn("coap_dispatch: error sending reponse\n");
                        }
                        coap_delete_pdu(response);
                    }
                    goto cleanup;
                }
                break;

            case COAP_MESSAGE_RST:
                /* find transaction in sendqueue to stop retransmission */
                if(coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent)){
                    handle_ack_rst(context, COAP_MESSAGE_RST, sent);
                }
                goto cleanup;
                break;

            default:
                debug(
                        "TODO: Need to handle other message types in coap_dispatch");
            }

            /************************************************************************************************
             * Following code will be replaced at different parts of the stack
             ************************************************************************************************/
#if 0
            switch (rcvd->pdu->hdr->type) {
                case COAP_MESSAGE_ACK:
                /* find transaction in sendqueue to stop retransmission */
                coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);

                if (rcvd->pdu->hdr->code == 0)
                goto cleanup;

                /* FIXME: if sent code was >= 64 the message might have been a
                 * notification. Then, we must flag the observer to be alive
                 * by setting obs->fail_cnt = 0. */
                if (sent && COAP_RESPONSE_CLASS(sent->pdu->hdr->code) == 2) {
                    const str token = {sent->pdu->hdr->token_length,
                        sent->pdu->hdr->token};
                    coap_touch_observer(context, &sent->remote, &token);
                }
                break;

                case COAP_MESSAGE_RST:
                /* We have sent something the receiver disliked, so we remove
                 * not only the transaction but also the subscriptions we might
                 * have. */

                coap_log(LOG_ALERT, "got RST for message %u\n",
                        ntohs(rcvd->pdu->hdr->id));

                /* find transaction in sendqueue to stop retransmission */
                coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);

                if (sent)
                coap_handle_rst(context, sent);
                goto cleanup;

                case COAP_MESSAGE_NON: /* check for unknown critical options */
                if (coap_option_check_critical(context, rcvd->pdu, opt_filter)
                        == 0)
                goto cleanup;
                break;

                case COAP_MESSAGE_CON: /* check for unknown critical options */
                if (coap_option_check_critical(context, rcvd->pdu, opt_filter)
                        == 0) {

                    /* FIXME: send response only if we have received a request. Otherwise,
                     * send RST. */
                    response = coap_new_error_response(rcvd->pdu,
                            COAP_RESPONSE_CODE(402), opt_filter);

                    if (!response)
                    warn("coap_dispatch: cannot create error reponse\n");
                    else {
                        if (coap_send(context, &rcvd->remote,
                                        response) == COAP_INVALID_TID) {
                            warn("coap_dispatch: error sending reponse\n");
                        }
                        coap_delete_pdu(response);
                    }

                    goto cleanup;
                }
                break;
            }
#endif

            /* Pass message to upper layer if a specific handler was
             * registered for a request that should be handled locally. */
            if (handle_locally(context, rcvd)) {
                if (COAP_MESSAGE_IS_REQUEST(rcvd->pdu->hdr)){
                    handle_request(context, rcvd);
                }
                else if (COAP_MESSAGE_IS_RESPONSE(rcvd->pdu->hdr)){
                    handle_response(context, rcvd);
                }
                else {
                    coap_send_flags_t flag;
                    flag = (coap_send_flags_t)(SEND_NOW |
                            (rcvd->secure ? SEND_SECURE_PORT : 0));
                    debug("dropped message with invalid code\n");
                    coap_send_message_type(context, &rcvd->remote, rcvd->pdu,
                            flag, COAP_MESSAGE_RST);
                }
            }

            // we should not retry responses.....
            cleanup:
                coap_delete_node(sent);
                sent = NULL;
                coap_delete_node(rcvd);
                rcvd = NULL;
        }
    }

    int coap_can_exit(coap_context_t *context) {
        return !context
                || (context->recvqueue == NULL && context->sendqueue == NULL);
    }

#ifdef WITH_CONTIKI

    /*---------------------------------------------------------------------------*/
    /* CoAP message retransmission */
    /*---------------------------------------------------------------------------*/
    PROCESS_THREAD(coap_retransmit_process, ev, data)
    {
        coap_tick_t now;
        coap_queue_t *nextpdu;

        PROCESS_BEGIN();

        debug("Started retransmit process\r\n");

        while(1) {
            PROCESS_YIELD();
            if (ev == PROCESS_EVENT_TIMER) {
                if (etimer_expired(&the_coap_context.retransmit_timer)) {

                    nextpdu = coap_peek_next(&the_coap_context);

                    coap_ticks(&now);
                    while (nextpdu && nextpdu->t <= now) {
                        coap_retransmit(&the_coap_context, coap_pop_next(&the_coap_context));
                        nextpdu = coap_peek_next(&the_coap_context);
                    }

                    /* need to set timer to some value even if no nextpdu is available */
                    etimer_set(&the_coap_context.retransmit_timer,
                            nextpdu ? nextpdu->t - now : 0xFFFF);
                }
#ifndef WITHOUT_OBSERVE
                if (etimer_expired(&the_coap_context.notify_timer)) {
                    coap_check_notify(&the_coap_context);
                    etimer_reset(&the_coap_context.notify_timer);
                }
#endif /* WITHOUT_OBSERVE */
            }
        }

        PROCESS_END();
    }
    /*---------------------------------------------------------------------------*/

#endif /* WITH_CONTIKI */

#ifdef WITH_LWIP
    /* FIXME: retransmits that are not required any more due to incoming packages
     * do *not* get cleared at the moment, the wakeup when the transmission is due
     * is silently accepted. this is mainly due to the fact that the required
     * checks are similar in two places in the code (when receiving ACK and RST)
     * and that they cause more than one patch chunk, as it must be first checked
     * whether the sendqueue item to be dropped is the next one pending, and later
     * the restart function has to be called. nothing insurmountable, but it can
     * also be implemented when things have stabilized, and the performance
     * penality is minimal
     *
     * also, this completely ignores COAP_RESOURCE_CHECK_TIME.
     * */

    static void coap_retransmittimer_execute(void *arg)
    {
        coap_context_t *ctx = (coap_context_t*)arg;
        coap_tick_t now;
        coap_tick_t elapsed;
        coap_queue_t *nextinqueue;

        ctx->timer_configured = 0;

        coap_ticks(&now);

        elapsed = now - ctx->sendqueue_basetime; /* that's positive for sure, and unless we haven't been called for a complete wrapping cycle, did not wrap */

        nextinqueue = coap_peek_next(ctx);
        while (nextinqueue != NULL)
        {
            if (nextinqueue->t > elapsed) {
                nextinqueue->t -= elapsed;
                break;
            } else {
                elapsed -= nextinqueue->t;
                coap_retransmit(ctx, coap_pop_next(ctx));
                nextinqueue = coap_peek_next(ctx);
            }
        }

        ctx->sendqueue_basetime = now;

        coap_retransmittimer_restart(ctx);
    }

    static void coap_retransmittimer_restart(coap_context_t *ctx)
    {
        coap_tick_t now, elapsed, delay;

        if (ctx->timer_configured)
        {
            sys_untimeout(coap_retransmittimer_execute, (void*)ctx);
            ctx->timer_configured = 0;
        }
        if (ctx->sendqueue != NULL)
        {
            coap_ticks(&now);
            elapsed = now - ctx->sendqueue_basetime;
            if (ctx->sendqueue->t >= elapsed) {
                delay = ctx->sendqueue->t - elapsed;
            } else {
                /* a strange situation, but not completely impossible.
                 *
                 * this happens, for example, right after
                 * coap_retransmittimer_execute, when a retransmission
                 * was *just not yet* due, and the clock ticked before
                 * our coap_ticks was called.
                 *
                 * not trying to retransmit anything now, as it might
                 * cause uncontrollable recursion; let's just try again
                 * with the next main loop run.
                 * */
                delay = 0;
            }
            sys_timeout(delay, coap_retransmittimer_execute, (void*)ctx);
            ctx->timer_configured = 1;
        }
    }
#endif
