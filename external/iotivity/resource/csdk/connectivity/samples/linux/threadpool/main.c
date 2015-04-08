#include <stdio.h>
#include <stdlib.h>

#include "uthreadpool.h"
#include "umutex.h"

u_thread_pool_t gThreadPoolHandle = NULL;

u_mutex gMutex = NULL;
u_cond gCond = NULL;

void task(void *data)
{
    printf("[TASK] Task is executing: data: %s\n", (char *) data);

    //Signal the condition that task has been completed
    printf("[TASK] Signaling the condition\n");
    u_cond_signal(gCond);
}

void testThreadPool(void)
{
    char *string = "Test glib thread pool";

    //Initialize the mutex
    printf("[testThreadPool] Initializing mutex\n");
    u_mutex_init();

    //Initialize the thread pool
    printf("[testThreadPool] Initializing thread pool\n");
    if (CA_STATUS_OK != u_thread_pool_init(2, &gThreadPoolHandle))
    {
        printf("thread_pool_init failed!\n");
        return;
    }

    //Create the mutex
    printf("[testThreadPool] Creating mutex\n");
    gMutex = u_mutex_new();
    if (NULL == gMutex)
    {
        printf("[testThreadPool] Failed to create mutex!\n");
        return;
    }

    //Create the condition
    printf("[testThreadPool] Creating Condition\n");
    gCond = u_cond_new();
    if (NULL == gCond)
    {
        printf("[testThreadPool] Failed to create condition!\n");

        u_mutex_free(gMutex);
        return;
    }

    //Lock the mutex
    printf("[testThreadPool] Locking the mutex\n");
    u_mutex_lock(gMutex);

    //Add task to thread pool
    printf("[testThreadPool] Adding the task to thread pool\n");
    if (CA_STATUS_OK != u_thread_pool_add_task(gThreadPoolHandle, task, (void *) string))
    {
        printf("[testThreadPool] thread_pool_add_task failed!\n");

        u_mutex_unlock(gMutex);
        u_mutex_free(gMutex);
        u_cond_free(gCond);
        return;
    }

    //Wait for the task to be executed
    printf("[testThreadPool] Waiting for the task to be completed\n");
    u_cond_wait(gCond, gMutex);

    //Unlock the mutex
    printf("[testThreadPool] Got the signal and unlock the mutex\n");
    u_mutex_unlock(gMutex);

    printf("[testThreadPool] Task is completed and terminating threadpool\n");
    u_mutex_free(gMutex);
    u_cond_free(gCond);
    u_thread_pool_free(gThreadPoolHandle);

    printf("Exiting from testThreadPool\n");
}

static void menu()
{
    printf(" =====================================================================\n");
    printf("|                 Welcome to Theadpool testing                        |\n");
    printf("|---------------------------------------------------------------------|\n");
    printf("|                           ** Options **                             |\n");
    printf("|  1 - Test Threadpool functionality                                  |\n");
    printf("|  0 - Terminate test                                                 |\n");
}

static void startTesting(void)
{
    while (1)
    {
        int choice = -1;
        scanf("%d", &choice);

        switch (choice)
        {
            case 0:
                printf("Terminating test.....\n");
                return;
            case 1:
                testThreadPool();
                break;
            default:
                printf("Invalid input...\n");
                menu();
                break;
        }
    }
}

int main()
{
    menu();
    startTesting();
    return 0;
}
