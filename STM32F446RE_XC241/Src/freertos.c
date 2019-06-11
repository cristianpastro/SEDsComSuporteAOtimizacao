/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "CLI.h"
#include "EventControl.h"
#include "SysControl.h"
#include "StateMachine.h"
#include "monitor.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId blinkLedTaskHandle;
osThreadId StateMachineTasHandle;
osThreadId OptimizerTaskHandle;
osThreadId TerminalTaskHandle;
osThreadId SystemControlTaHandle;
osThreadId EventsProcessTaHandle;
osThreadId MonitorHandle;
osThreadId osThreadDef;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartblinkLedTask(void const * argument);
void StartStateMachineTask(void const * argument);
void StartOptimizerTask(void const * argument);
void StartTerminalTask(void const * argument);
void StartSystemControlTask(void const * argument);
void StartEventsProcessTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of blinkLedTask */
  osThreadDef(blinkLedTask, StartblinkLedTask, osPriorityRealtime, 0, 128);
  blinkLedTaskHandle = osThreadCreate(osThread(blinkLedTask), NULL);

  /* definition and creation of StateMachineTas */
  osThreadDef(StateMachineTas, StartStateMachineTask, osPriorityHigh, 0, 128);
  StateMachineTasHandle = osThreadCreate(osThread(StateMachineTas), NULL);

  /* definition and creation of OptimizerTask */
  osThreadDef(OptimizerTask, StartOptimizerTask, osPriorityHigh, 0, 128);
  OptimizerTaskHandle = osThreadCreate(osThread(OptimizerTask), NULL);

  /* definition and creation of TerminalTask */
  osThreadDef(TerminalTask, StartTerminalTask, osPriorityAboveNormal, 0, 128);
  TerminalTaskHandle = osThreadCreate(osThread(TerminalTask), NULL);

  /* definition and creation of SystemControlTa */
  osThreadDef(SystemControlTa, StartSystemControlTask, osPriorityAboveNormal, 0, 128);
  SystemControlTaHandle = osThreadCreate(osThread(SystemControlTa), NULL);

  /* definition and creation of EventsProcessTa */
  osThreadDef(EventsProcessTa, StartEventsProcessTask, osPriorityNormal, 0, 128);
  EventsProcessTaHandle = osThreadCreate(osThread(EventsProcessTa), NULL);

  /* definition and creation of MonitorTask */
  osThreadDef(MonitorTask, StartMonitorTask, osPriorityNormal, 0, 128);
  MonitorHandle = osThreadCreate(osThread(MonitorTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}


/**
* @brief Esta função implementa a tarefa blinkLedTask. Esta tarefa simplesmente
* pisca um led com um período de 1 segundo, a fim de verificar o funcionamento
* do sistema. Como esta é a tarefa mais prioritária do sistema, caso o LED pare
* de piscar, significa que houve alguma falha grave.
* @param argument: Not used
* @retval None
*/
void StartblinkLedTask(void const * argument)
{

  for(;;)
  {
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    vTaskDelay(1000);
  }
  /* USER CODE END StartblinkLedTask */
}



/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
