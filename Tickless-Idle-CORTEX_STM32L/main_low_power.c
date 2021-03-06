/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ST library functions. */
#include "stm32l1xx.h"
#include "discover_board.h"
#include "stm32l_discovery_lcd.h"

#define configQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define configQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

#define mainQUEUED_VALUE (100UL)

#define mainLED_TOGGLE_DELAY (10 / portTICK_PERIOD_MS)

static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);

static QueueHandle_t xQueue = NULL;

/* Block time used by the Tx task. */
TickType_t xSendBlockTime = (100UL / portTICK_PERIOD_MS);

/* The lower an upper limits of the block time.  An infinite block time is used
if xSendBlockTime is incremented past xMaxBlockTime. */
static const TickType_t xMaxBlockTime = (500L / portTICK_PERIOD_MS), xMinBlockTime = (100L / portTICK_PERIOD_MS);

/* The semaphore on which the Tx task blocks. */
static SemaphoreHandle_t xTxSemaphore = NULL;

void main_low_power(void)
{
	xTxSemaphore = xSemaphoreCreateBinary();

	xQueue = xQueueCreate(1, sizeof(unsigned long));

	xTaskCreate(prvQueueReceiveTask, "Rx", configMINIMAL_STACK_SIZE, NULL, configQUEUE_RECEIVE_TASK_PRIORITY, NULL);
	xTaskCreate(prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, configQUEUE_SEND_TASK_PRIORITY, NULL);

	vTaskStartScheduler();
}

static void prvQueueSendTask(void *pvParameters)
{
	const unsigned long ulValueToSend = mainQUEUED_VALUE;

	(void)pvParameters;

	while (1)
	{
		/* Enter the Blocked state to wait for the semaphore. */
		xSemaphoreTake(xTxSemaphore, xSendBlockTime);

		/* Send to the queue - causing the Tx task to flash its LED. */
		xQueueSend(xQueue, &ulValueToSend, 0);
	}
}

static void prvQueueReceiveTask(void *pvParameters)
{
	unsigned long ulReceivedValue;

	(void)pvParameters;

	while (1)
	{
		/* Wait until something arrives in the queue. */
		xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

		if (ulReceivedValue == mainQUEUED_VALUE)
		{
			GPIO_HIGH(LD_GPIO_PORT, LD_GREEN_GPIO_PIN);
			vTaskDelay(mainLED_TOGGLE_DELAY);
			GPIO_LOW(LD_GPIO_PORT, LD_GREEN_GPIO_PIN);
		}
	}
}

/* Handles interrupts generated by pressing a button. */
void EXTI0_IRQHandler(void)
{
	static const TickType_t xIncrement = 200UL / portTICK_PERIOD_MS;

	// For reseting
	if (xSendBlockTime == portMAX_DELAY)
	{
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

		/* Unblock the Tx task. */
		xSemaphoreGiveFromISR(xTxSemaphore, &xHigherPriorityTaskWoken);

		/* Start over with the 'short' block time */
		xSendBlockTime = xMinBlockTime;
	}
	else
	{
		xSendBlockTime += xIncrement;

		if (xSendBlockTime > xMaxBlockTime)
		{
			xSendBlockTime = portMAX_DELAY;
		}
	}

	EXTI_ClearITPendingBit(EXTI_Line0);
}