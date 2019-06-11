/*
 * SysControl.c
 *
 *  Created on: 20 de mai de 2019
 *      Author: cristian
 *
 *  Esta API realiza o controle dos parâmetros do sistema, como
 *  modo de operação (C1, C2, C3, C4 e ES), receitas e despesas
 *  de cada peça, peças do buffer B1, etc.
 */

#include "SysControl.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "StateMachine.h"
#include "monitor.h"

SystemParameters parameters;
volatile uint32_t system_time = 0;
extern xSemaphoreHandle semEventControl;
xSemaphoreHandle semSystemControl;
Mode_t actual_mode;

extern xQueueHandle  qMonitor;
extern xSemaphoreHandle ChangeSmMutex;
extern xSemaphoreHandle ChangeB1;
extern sm_t BancoDeEnsaios_sm;
extern xQueueHandle qB1;

Bool powerMode;


SystemParameters getSystemParameters(){
	return parameters;
}

void setSpeakerPeriod (uint8_t period){
	if (period < 80)
		parameters.Speaker_Period = period;
}

void setCustoPeca (uint32_t custo_peca){
	parameters.custo_peca = custo_peca;
}

void setReceitaB2(uint32_t receita_B2){
	parameters.receita_B2 = receita_B2;
}

void setReceitaB3(uint32_t receita_B3){
	parameters.receita_B3 = receita_B3;
}

void setPeriodoB2 (uint32_t periodoB2){
	parameters.periodo_B2 = periodoB2;
}

void setPeriodoB3 (uint32_t periodoB3){
	parameters.periodo_B3 = periodoB3;
}

void setTempoMaximo (uint8_t tempo_maximo){
	parameters.tempo_maximo = tempo_maximo;
}

uint8_t getSpeakerPeriod (void){
	return parameters.Speaker_Period;
}

uint32_t getCustoPeca (void){
	return parameters.custo_peca;
}

uint32_t getReceitaB2(void){
	return parameters.receita_B2;
}

uint32_t getReceitaB3(void){
	return parameters.receita_B3;
}

uint32_t getPeriodoB2 (void){
	return parameters.periodo_B2;
}

uint32_t getPeriodoB3 (void){
	return parameters.periodo_B3;
}

uint8_t getTempoMaximo (void){
	return parameters.tempo_maximo;
}

uint32_t getSystemTime(void){
	return system_time;
}

void addSystemTime(void){
	system_time++;
	BaseType_t pxHigherPriority = pdFALSE;
	xSemaphoreGiveFromISR(semEventControl, &pxHigherPriority);
	if (pxHigherPriority == pdTRUE){
		portYIELD();
	}
}

/* @brief Insere "n" peças no buffer "q". As peças serão identificadas
 * de 1 a n.
 * @param n: Número de peças que deseja-se inserir no buffer "q".
 * @param q: Buffer ao qual deseja-se inserir as peças.
*  @retval None
*  @note: só é seguro chamar esta função quando a máquina de estados NÃO
*  estiver em execução
*/
void addPieces(uint8_t n, xQueueHandle* q){
	uint8_t i;
	piece p;

	if (xSemaphoreTake(ChangeB1, portMAX_DELAY) == pdTRUE){

		for (i=0; i< n; i++){
			p.id_peca = i+1;
			p.tempo_insercao = 0;
			xQueueSendToBack(*q, &p, 1000);
		}

		xSemaphoreGive(ChangeB1);  /*!< Libera o Mutex*/
	}

	occurenceMonitor init;
	init.occurence = ins_piece;
	init.id = n;

	while (qMonitor == NULL){
		osDelay(10);
	}
	xQueueSendToBack(qMonitor, &init, 1000);

}

/* @brief Altera o modo de operação da máquina de estados
 * @param sm: Máquina de estados ao qual deseja-se alterar o modo;
 * @param new_mode: Modo ao qual deseja-se operar;
*  @retval None
*  @note: só é seguro chamar esta função quando a máquina de estados NÃO
*  estiver em execução
*/
void changeMode(sm_t* sm, Mode_t new_mode){
	if (xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) == pdTRUE){
		sm->mode = new_mode;

		if (new_mode == C1){
			sm->states_list[ST_1].list_trans[1].dst = &sm->states_list[ST_6];
			sm->states_list[ST_1].list_trans[1].event = &sm->events_list[B1_in];
			sm->states_list[ST_1].list_trans[0].dst = &sm->states_list[ST_4];
			sm->states_list[ST_1].list_trans[0].event = &sm->events_list[B2_in];
			sm->states_list[ST_1].list_trans[2].dst = &sm->states_list[ST_2];
			sm->states_list[ST_1].list_trans[2].event = &sm->events_list[B3_in];
			sm->states_list[ST_1].list_trans[3].event = NULL;
			sm->states_list[ST_1].list_trans[3].event = NULL;
			sm->states_list[ST_1].marked = FALSE;
		}else if (new_mode == C2){
			sm->states_list[ST_1].list_trans[1].dst = &sm->states_list[ST_6];
			sm->states_list[ST_1].list_trans[1].event = &sm->events_list[B1_in];
			sm->states_list[ST_1].list_trans[2].dst = &sm->states_list[ST_4];
			sm->states_list[ST_1].list_trans[2].event = &sm->events_list[B2_in];
			sm->states_list[ST_1].list_trans[0].dst = &sm->states_list[ST_2];
			sm->states_list[ST_1].list_trans[0].event = &sm->events_list[B3_in];
			sm->states_list[ST_1].list_trans[3].event = NULL;
			sm->states_list[ST_1].list_trans[3].event = NULL;
			sm->states_list[ST_1].marked = FALSE;
		}
	}

	xSemaphoreGive(ChangeSmMutex);  /*!< Libera o Mutex*/

}

void powerOnStateMachine(sm_t* sm){
	if (xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) == pdTRUE){
		if (sm->semPower !=NULL){
				xSemaphoreGive(sm->semPower);
			}
	}

	xSemaphoreGive(ChangeSmMutex);  /*!< Libera o Mutex*/

	occurenceMonitor init;
	init.occurence = init_process;
	xQueueSendToBack(qMonitor, &init, 1000);
}

void PowerOffStateMachine(sm_t* sm){

	if (xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) == pdTRUE){
		if (sm->semPower != NULL){
			xSemaphoreTake(sm->semPower, portMAX_DELAY);
		}
	}
	xSemaphoreGive(ChangeSmMutex);  /*!< Libera o Mutex*/

}

/* USER CODE BEGIN Header_StartSystemControlTask */
/**
* @brief Function implementing the SystemControlTa thread. O processador de eventos
* deve receber um evento do CLI ou do Event Controller, processá-lo e realizar
* a ação correspondente.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSystemControlTask */
void StartSystemControlTask(void const * argument)
{

	semSystemControl = xSemaphoreCreateBinary();

  for(;;)
  {
	changeMode(&BancoDeEnsaios_sm, ES);
	addPieces (QTDE_B1_INIT, &qB1);
	powerOnStateMachine(&BancoDeEnsaios_sm);


    xSemaphoreTake(semSystemControl, portMAX_DELAY);


  }
  /* USER CODE END StartSystemControlTask */
}
