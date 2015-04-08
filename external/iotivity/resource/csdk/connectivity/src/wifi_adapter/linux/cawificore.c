#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "cawificore.h"
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "logger.h"

#define TAG PCF("CA")

#define CA_MAX_BUFFER_SIZE 512  // Max length of buffer
#define CA_UNICAST_PORT 5283 // The port on which to listen for incoming data
int32_t unicast_socket;
pthread_t pthread_unicast_server;
pthread_t pthread_unicast_client;
pthread_mutex_t mutex_unicast;
pthread_cond_t sync_cond_unicast;
int32_t unicast_send_thread_flag = 0; // 0: run, 1: stop

char* target = NULL;
void* list = NULL;

#define CA_MULTICAST_ADDR "224.0.1.187"
#define CA_MULTICAST_PORT 5683

int32_t multicast_send_socket;
struct sockaddr_in multicast_send_interface_addr;

pthread_t pthread_multicast_server;
pthread_t pthread_multicast_client;

int32_t multicast_receive_socket;
struct sockaddr_in multicast_receive_interface_addr;

void* data_list = NULL;

pthread_mutex_t mutex_multicast;
pthread_cond_t sync_cond_multicast;
int32_t multicast_send_thread_flag = 0; // 0: run, 1: stop

CAPacketReceiveCallback gPacketReceiveCallback = NULL;

void CAWiFiInitialize()
{
    OIC_LOG(DEBUG, TAG, "CAWiFiInitialize");

    pthread_mutex_init(&mutex_unicast, NULL);

    pthread_mutex_init(&mutex_multicast, NULL);

    // [UDP Server]
    struct sockaddr_in si_me;

    // create a UDP socket
    if ((unicast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        OIC_LOG_V(DEBUG, TAG, "CAWiFiInit, creating socket failed");

        return;
    }

    OIC_LOG_V(DEBUG, TAG, "CAWiFiInit, socket created");

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(CA_UNICAST_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    if (bind(unicast_socket, (struct sockaddr*) &si_me, sizeof(si_me)) == -1)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, binding socket failed");

        return;
    }

    OIC_LOG(DEBUG, TAG, "CAWiFiInit, socket binded");

    // [multicast sender]
    uint32_t multiTTL = 1;

    // 1. Set up a typical UDP socket
    multicast_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicast_send_socket < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, Socket error");

        return;
    }

    memset(&multicast_send_interface_addr, 0, sizeof(multicast_send_interface_addr));
    multicast_send_interface_addr.sin_family = AF_INET;
    multicast_send_interface_addr.sin_addr.s_addr = inet_addr(CA_MULTICAST_ADDR);
    multicast_send_interface_addr.sin_port = htons(CA_MULTICAST_PORT);

    // [multicast receiver]
    // 1. Create a typical UDP socket and set Non-blocking for reading
    multicast_receive_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicast_receive_socket < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, Socket error");

        return;
    }

    // 2. Allow multiple sockets to use the same port number
    int32_t ret_val = setsockopt(multicast_receive_socket, SOL_SOCKET, SO_REUSEADDR, &multiTTL,
            sizeof(multiTTL));
    if (ret_val < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, Failed to set REUSEADDR");
    }

    // 3. Set up the interface
    memset(&multicast_receive_interface_addr, 0, sizeof(multicast_receive_interface_addr));
    multicast_receive_interface_addr.sin_family = AF_INET;
    multicast_receive_interface_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    multicast_receive_interface_addr.sin_port = htons(CA_MULTICAST_PORT);

    // 4. Bind to the interface
    ret_val = bind(multicast_receive_socket, (struct sockaddr *) &multicast_receive_interface_addr,
            sizeof(multicast_receive_interface_addr));
    if (ret_val < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, Failed to bind socket");

        return;
    }

    // 5. Join the multicast group
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(CA_MULTICAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret_val = setsockopt(multicast_receive_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
            sizeof(mreq));
    if (ret_val < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiInit, Failed to join multicast group");

        return;
    }
}

void CAWiFiTerminate()
{
    OIC_LOG(DEBUG, TAG, "CAWiFiTerminate");

    pthread_mutex_destroy(&mutex_unicast);

    pthread_mutex_destroy(&mutex_multicast);

    close(unicast_socket);

    close(multicast_send_socket);

    close(multicast_receive_socket);
}

int32_t CAWiFiSendUnicastMessage(const char* address, const char* data, int lengh)
{
    CASendUnicastMessage(address, data);

    return 0;
}

int32_t CAWiFiSendMulticastMessage(const char* m_address, const char* data)
{
    CASendMulticastMessage(data);

    return 0;
}

int32_t CAWiFiStartUnicastServer(const char* address, int port)
{
    OIC_LOG_V(DEBUG, TAG, "CAWiFiStartUnicastServer(%s, %d)", address, port);

    int32_t result = pthread_create(&pthread_unicast_server, NULL, (void *) &CAUnicastReceiveThread,
            (void *) NULL);
    if (result < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiStartUnicastServer, creating unicast_receive_thread failed");

        return -1;
    }

    result = pthread_create(&pthread_unicast_client, NULL, (void *) &CAUnicastSendThread,
            (void *) NULL);
    if (result < 0)
    {
        OIC_LOG(DEBUG, TAG, "CAWiFiStartUnicastServer, creating unicast_send_thread failed");

        return -1;
    }

    OIC_LOG(DEBUG, TAG, "CAWiFiStartUnicastServer, receive & send thread created");

    return 0;
}

int32_t CAWiFiStartMulticastServer(const char* m_address, int port)
{
    OIC_LOG_V(DEBUG, TAG, "CAWiFiStartMulticastServer(%s, %d)", m_address, port);

    int32_t result = pthread_create(&pthread_multicast_server, NULL,
            (void *) &CAMulticastReceiveThread, (void *) NULL);
    if (result < 0)
    {

        return -1;
    }

    result = pthread_create(&pthread_multicast_client, NULL, (void *) &CAMulticastSendThread,
            (void *) NULL);
    if (result < 0)
    {
        OIC_LOG(DEBUG, TAG, "creating receive_thread failed");

        return -1;
    }

    return 0;
}

int32_t CAWiFiStopUnicastServer(int32_t server_id)
{

    CAStopUnicastSendThread();

    return 0;
}

int32_t CAWiFiStopMulticastServer(int32_t server_id)
{

    CAStopMulticastSendThread();

    return 0;
}

void CAWiFiSetCallback(CAPacketReceiveCallback callback)
{
    gPacketReceiveCallback = callback;
}

void* CAUnicastReceiveThread(void* data)
{
    OIC_LOG(DEBUG, TAG, "CAUnicastReceiveThread");

    char buf[CA_MAX_BUFFER_SIZE];
    int32_t recv_len;

    struct sockaddr_in si_other;
    int32_t slen = sizeof(si_other);

    // keep listening for data
    while (1)
    {
        OIC_LOG(DEBUG, TAG, "CAUnicastReceiveThread, Waiting for data...");
        fflush(stdout);

        memset(buf, 0, sizeof(char) * CA_MAX_BUFFER_SIZE);

        // try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(unicast_socket, buf, CA_MAX_BUFFER_SIZE, 0,
                (struct sockaddr *) &si_other, &slen)) == -1)
        {
            OIC_LOG(DEBUG, TAG, "CAUnicastReceiveThread, recv_len() error");
            continue;
        }

        // print details of the client/peer and the data received
        OIC_LOG_V(DEBUG, TAG, "CAUnicastReceiveThread, Received packet from %s:%d",
                inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		OIC_LOG_V(DEBUG, TAG, "CAUnicastReceiveThread, Data: %s", buf);

        if (gPacketReceiveCallback != NULL)
        {
            gPacketReceiveCallback(inet_ntoa(si_other.sin_addr), buf);
        }
    }

    return (void*) 0;
}

void* CAUnicastSendThread(void* data)
{
    OIC_LOG(DEBUG, TAG, "CAUnicastSendThread");

    while (!unicast_send_thread_flag)
    {
        pthread_mutex_lock(&mutex_unicast);

        pthread_cond_wait(&sync_cond_unicast, &mutex_unicast);

        pthread_mutex_unlock(&mutex_unicast);

        if (unicast_send_thread_flag)
            return (void*) 0;

        CASendUnicastMessageImpl(target, list, strlen(list));
    }

    return (void*) 0;
}

void CASendUnicastMessage(char* address, void* data)
{
    target = address;
    list = data;
    unicast_send_thread_flag = 0;

    pthread_cond_signal(&sync_cond_unicast);
}

int32_t CASendUnicastMessageImpl(const char* address, const char* data, int32_t lengh)
{
    OIC_LOG_V(DEBUG, TAG, "CASendUnicastMessageImpl, address: %s, data: %s", address, data);

    // [UDP Client]
    struct sockaddr_in si_other;
    int32_t slen = sizeof(si_other);

    memset((char *) &si_other, 0, sizeof(si_other));

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(CA_UNICAST_PORT);
    if (inet_aton(address, &si_other.sin_addr) == 0)
    {
        OIC_LOG(DEBUG, TAG, "CASendUnicastMessageImpl, inet_aton, error...");
        return 0;
    }

    OIC_LOG_V(DEBUG, TAG, "CASendUnicastMessageImpl, sendto, to: %s, data: %s", address, data);
    if (sendto(unicast_socket, data, strlen(data), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
        OIC_LOG(DEBUG, TAG, "CASendUnicastMessageImpl, sendto, error...");

        return 0;
    }

    return 0;
}

void CAStopUnicastSendThread()
{
    unicast_send_thread_flag = 1;

    pthread_cond_signal(&sync_cond_unicast);
}

void* CAMulticastReceiveThread(void* data)
{
    OIC_LOG(DEBUG, TAG, "CAMulticastReceiveThread");

    // 6. Read from the socket and print out a message when one is received
    char msgbuf[CA_MAX_BUFFER_SIZE];

    struct sockaddr_in client;
    int32_t addrlen = sizeof(client);

    OIC_LOG(DEBUG, TAG, "CAMulticastReceiveThread, waiting for input...");

    while (1)
    {
        int32_t recv_bytes = recvfrom(multicast_receive_socket, msgbuf, CA_MAX_BUFFER_SIZE, 0,
                (struct sockaddr *) &client, (socklen_t *) &addrlen);
        if (recv_bytes < 0)
        {
            if (errno != EAGAIN)
            {
                OIC_LOG(DEBUG, TAG, "CAMulticastReceiveThread, error recvfrom");

                return (void*) 0;
            }

            continue;
        }

        msgbuf[recv_bytes] = 0;

        OIC_LOG_V(DEBUG, TAG, "Received msg: %s, size: %d", msgbuf, recv_bytes);

        char* sender = inet_ntoa(client.sin_addr);
        char local[INET_ADDRSTRLEN];
        CAGetLocalAddress(local);
        if (strcmp(sender, local) == 0)
        {
            OIC_LOG_V(DEBUG, TAG, "skip the local request (via multicast)");
        }
        else
        {
            if (gPacketReceiveCallback != NULL)
            {
                gPacketReceiveCallback(inet_ntoa(client.sin_addr), msgbuf);
            }
        }

    }

    return (void*) 0;
}

void* CAMulticastSendThread(void* data)
{
    OIC_LOG(DEBUG, TAG, "CAMulticastSendThread");

    while (!multicast_send_thread_flag)
    {
        pthread_mutex_lock(&mutex_multicast);

        pthread_cond_wait(&sync_cond_multicast, &mutex_multicast);

        pthread_mutex_unlock(&mutex_multicast);

        if (multicast_send_thread_flag)
            return (void*) 0;

        CASendMulticastMessageImpl(data_list);
    }

    return (void*) 0;
}

void CASendMulticastMessage(void* data)
{
    data_list = data;
    multicast_send_thread_flag = 0;

    pthread_cond_signal(&sync_cond_multicast);
}

int32_t CASendMulticastMessageImpl(const char* msg)
{
    OIC_LOG_V(DEBUG, TAG, "CASendMulticastMessageImpl, sendto, data: %s", msg);

    int32_t result = sendto(multicast_send_socket, msg, strlen(msg), 0,
            (struct sockaddr *) &multicast_send_interface_addr,
            sizeof(multicast_send_interface_addr));
    if (result < 0)
    {
        OIC_LOG(DEBUG, TAG, "CASendMulticastMessageImpl, sending message error...");

        return -1;
    }

    return 0;
}

void CAStopMulticastSendThread()
{
    multicast_send_thread_flag = 1;

    pthread_cond_signal(&sync_cond_multicast);
}

void CAGetLocalAddress(char* addressBuffer)
{
    //char addressBuffer[INET_ADDRSTRLEN];
    memset(addressBuffer, 0, INET_ADDRSTRLEN);

    struct ifaddrs* ifAddrStruct = NULL;
    struct ifaddrs* ifa = NULL;
    void* tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET)
        { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;

            memset(addressBuffer, 0, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if (strcmp(addressBuffer, "127.0.0.1") == 0)
                continue;
        }
    }

    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
}

