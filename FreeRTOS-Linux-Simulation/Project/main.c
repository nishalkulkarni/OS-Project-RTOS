#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h" /* RTOS firmware */
#include "task.h"     /* Task */
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#define NUM_OF_PHILOSOPHERS (5)
#define MAX_NUMBER_ALLOWED (NUM_OF_PHILOSOPHERS - 1)

SemaphoreHandle_t forks[NUM_OF_PHILOSOPHERS];
SemaphoreHandle_t entry_sem;
TaskHandle_t philosophers[NUM_OF_PHILOSOPHERS];

#define left(i) (i)
#define right(i) ((i + 1) % NUM_OF_PHILOSOPHERS)

void take_fork(int i)
{
    xSemaphoreTake(forks[left(i)], portMAX_DELAY);
    xSemaphoreTake(forks[right(i)], portMAX_DELAY);
    printf("Philosopher %d got the fork %d and %d\n", i, left(i), right(i));
}

void put_fork(int i)
{
    xSemaphoreGive(forks[left(i)]);
    xSemaphoreGive(forks[right(i)]);
    printf("Philosopher %d Gave up the fork %d and %d\n", i, left(i), right(i));
}

void philosophers_task(void *param)
{

    int i = *(int *)param;

    while (1)
    {

        xSemaphoreTake(entry_sem, portMAX_DELAY);

        take_fork(i);

        printf("Philosopher %d is eating\n", i);

        vTaskDelay(pdMS_TO_TICKS(1000));

        put_fork(i);

        xSemaphoreGive(entry_sem);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void dining_philosophers_main()
{

    int i;
    int param[NUM_OF_PHILOSOPHERS];

    // Create Five Semaphores for the five shared resources.
    // Which is the fork in this case.

    for (i = 0; i < NUM_OF_PHILOSOPHERS; i++)
    {
        forks[i] = xSemaphoreCreateMutex();
    }

    // This is the critical piece to avoid deadlock.
    // If one less philosopher is allowed to act then there will no deadlock.
    // As one philosopher will always get two forks and so it will go on.

    entry_sem = xSemaphoreCreateCounting(MAX_NUMBER_ALLOWED, MAX_NUMBER_ALLOWED);

    for (i = 0; i < NUM_OF_PHILOSOPHERS; i++)
    {
        param[i] = i;
        xTaskCreate(philosophers_task, "task", 30, &(param[i]), 2, NULL);
    }

    vTaskStartScheduler();
}

QueueHandle_t q = NULL;

void consumer_task(void *pvParameter)
{
    unsigned long counter;
    if (q == NULL)
    {
        printf("Queue is not ready");
        return;
    }
    while (1)
    {
        xQueueReceive(q, &counter, (TickType_t)(1000 / portTICK_PERIOD_MS));
        printf("value received on queue: %lu \n", counter);
        vTaskDelay(pdMS_TO_TICKS(500)); //wait for 500 ms
    }
}

void producer_task(void *pvParameter)
{
    unsigned long counter = 1;
    if (q == NULL)
    {
        printf("Queue is not ready \n");
        return;
    }
    while (1)
    {
        printf("value sent on queue: %lu \n", counter);
        xQueueSend(q, (void *)&counter, (TickType_t)0); // add the counter value to the queue
        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000)); //wait for a second
    }
}

void producer_consumer_main()
{
    q = xQueueCreate(20, sizeof(unsigned long));
    if (q != NULL)
    {
        printf("Queue is created\n");
        xTaskCreate(&producer_task, "producer_task", 2048, NULL, 5, NULL);
        printf("producer task  started\n");
        xTaskCreate(&consumer_task, "consumer_task", 2048, NULL, 5, NULL);
        printf("consumer task  started\n");
        vTaskStartScheduler();
    }
    else
    {
        printf("Queue creation failed");
    }
}

void vApplicationIdleHook(void);

int main(void)
{
    // Calling the dining philosopher function
    // dining_philosophers_main();

    // Calling the producer consumer function
    producer_consumer_main();

    return 0;
}

void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
    taskENTER_CRITICAL();
    {
        printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
        fflush(stdout);
    }
    taskEXIT_CRITICAL();
    exit(-1);
}

void vApplicationIdleHook(void)
{
    //	printf("Idle\r\n");
}
