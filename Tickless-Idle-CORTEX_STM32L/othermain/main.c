/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* ST library functions. */
#include "stm32l1xx.h"
#include "discover_board.h"

static void prvSetupHardware(void);

extern void main_low_power(void);

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);

int main(void)
{
	prvSetupHardware();
	main_low_power();
	return 0;
}

static void prvSetupHardware(void)
{
	/* GPIO, EXTI and NVIC Init structure declaration */
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	void SystemCoreClockUpdate(void);

	/* System function that updates the SystemCoreClock variable. */
	SystemCoreClockUpdate();

	/* Essential on STM32 Cortex-M devices. */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Systick is fed from HCLK/8. */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

	/* Set MSI clock range to ~4.194MHz. */
	RCC_MSIRangeConfig(RCC_MSIRange_6);

	/* Enable the GPIOs clocks. */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH, ENABLE);

	/* Enable comparator clocks. */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_COMP, ENABLE);

	/* Enable SYSCFG clocks. */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Set internal voltage regulator to 1.5V. */
	PWR_VoltageScalingConfig(PWR_VoltageScaling_Range2);

	/* Wait Until the Voltage Regulator is ready. */
	while (PWR_GetFlagStatus(PWR_FLAG_VOS) != RESET)
		;

	/* Configure User Button pin as input */
	GPIO_InitStructure.GPIO_Pin = USERBUTTON_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(USERBUTTON_GPIO_PORT, &GPIO_InitStructure);

	/* Select User Button pin as input source for EXTI Line */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	/* Configure EXT1 Line 0 in interrupt mode trigged on Rising edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; /* PA0 for User button AND IDD_WakeUP */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the LED_pin as output push-pull for LD3 & LD4 usage */
	GPIO_InitStructure.GPIO_Pin = LD_GREEN_GPIO_PIN | LD_BLUE_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LD_GPIO_PORT, &GPIO_InitStructure);

	/* Force a low level on LEDs */
	GPIO_LOW(LD_GPIO_PORT, LD_GREEN_GPIO_PIN);
	GPIO_LOW(LD_GPIO_PORT, LD_BLUE_GPIO_PIN);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void vApplicationIdleHook(void) {}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void vApplicationTickHook(void) {}

void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	volatile unsigned long ulSetToNonZeroInDebuggerToContinue = 0;

	(void)ulLine;
	(void)pcFileName;

	taskENTER_CRITICAL();
	{
		while (ulSetToNonZeroInDebuggerToContinue == 0)
		{
			__asm volatile("NOP");
			__asm volatile("NOP");
		}
	}
	taskEXIT_CRITICAL();
}
