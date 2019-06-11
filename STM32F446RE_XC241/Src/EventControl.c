/*
 * EventControl.c
 *
 *  Created on: 20 de mai de 2019
 *      Author: cristian
 *
 *  Esta API controla os eventos gerados automaticamente, como a
 *  remoção de peças dos buffers B2 e B3 quando atingirem o seu
 *  período de remoção. Esta API também verifica a peça removida
 *  e determina se a peça está estragada ou não.
 *
 */

#include "EventControl.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "SysControl.h"
#include "tim.h"
#include "monitor.h"
#include "string.h"
#include "StateMachine.h"

xSemaphoreHandle semEventControl;
extern xQueueHandle  qMonitor;

extern xQueueHandle qB1;
extern xQueueHandle qB2;
extern xQueueHandle qB3;

/* USER CODE BEGIN Header_StartEventsProcessTask */
/**
* @brief Function implementing the EventsProcessTa thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEventsProcessTask */
void StartEventsProcessTask(void const * argument)
{
	semEventControl = xSemaphoreCreateBinary();
	MX_TIM5_Init();   /*<!Timer que controla a remoção de peças*/
	MX_TIM2_Init();   /*<!Timer que controla o speaker*/

	occurenceMonitor send;
	piece removed;

  for(;;)
  {
    xSemaphoreTake(semEventControl, portMAX_DELAY);

	if (getSystemTime() % getPeriodoB2() == 0){
		/*Deve-se retirar uma peça de B2*/
		if (xQueueReceive(qB2, &removed, 0)){
			if (getSystemTime() - removed.tempo_insercao >= getTempoMaximo()){
				send.occurence = B2_estraga;
			}else{
				send.occurence = B2_retira;
			}
			send.moment = getSystemTime();
			send.id = removed.id_peca;

			xQueueSendToBack(qMonitor, &send, 1000);

			if (xQueueIsQueueEmptyFromISR(qB1) && xQueueIsQueueEmptyFromISR(qB2) && xQueueIsQueueEmptyFromISR(qB3)){
				send.moment = getSystemTime();
				send.id = removed.id_peca;
				send.occurence = end_process;

				xQueueSendToBack(qMonitor, &send, 1000);
			}

		}

	}
	if (getSystemTime() % getPeriodoB3() == 0){
		/*Deve-se retirar uma peça de B3*/
		if (xQueueReceive(qB3, &removed, 0)){
			if (getSystemTime() - removed.tempo_insercao >= getTempoMaximo()){
				send.occurence = B3_estraga;
			}else{
				send.occurence = B3_retira;
			}
			send.moment = getSystemTime();
			send.id = removed.id_peca;

			xQueueSendToBack(qMonitor, &send, 1000);

			if (xQueueIsQueueEmptyFromISR(qB1) && xQueueIsQueueEmptyFromISR(qB2) && xQueueIsQueueEmptyFromISR(qB3)){
				send.moment = getSystemTime();
				send.id = removed.id_peca;
				send.occurence = end_process;

				xQueueSendToBack(qMonitor, &send, 1000);
			}
		}
	}

  }
  /* USER CODE END StartEventsProcessTask */
}
