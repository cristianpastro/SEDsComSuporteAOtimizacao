/*
 * StateMachine.c
 *
 *  Created on: 5 de abr de 2019
 *      Author: cristian
 *
 *  Esta API realiza o controle do Supervisor e do Otimizador.
 */

#include "StateMachine.h"
#include "stdlib.h"
#include "time.h"
#include "SysControl.h"
#include "cmsis_os.h"
#include <string.h>
#include "main.h"
#include "monitor.h"
#include "FreeRTOS.h"

sm_t BancoDeEnsaios_sm;
extern xQueueHandle  qMonitor;

xSemaphoreHandle semBlocking;     /*<! Semáforo que controla a máquina de estados*/

xSemaphoreHandle ChangeSmMutex;   /*<! Mutex que controla as modificações na máquina e estados*/
xSemaphoreHandle ChangeB1;        /*<! Mutex que controla as modificações no buffer B1*/

xSemaphoreHandle inOtm;           /*<! Põe o Supervisor pra dormir enquanto otimiza*/

extern Mode_t actual_mode;
xQueueHandle qB1;        /*<! Fila de peças no buffer B1*/
xQueueHandle qB2;        /*<! Fila de peças no buffer B2*/
xQueueHandle qB3;        /*<! Fila de peças no buffer B3*/

xTaskHandle  ComSup2Otm = NULL;          /*<! Estrutura mailbox para comunicar do Supervisor para o Otimizador */

piece esteira;           /*<! Peça contida na esteira*/

Event* optimizable_events[4];   /*<! Conjunto de eventos otimizáveis*/
uint8_t num_optimizable_events;

/*Protótipo das Interrupções*/
void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin);


/* ---------------------------------- FUNÇÕES EVENTOS -----------------------------------*/

void event_A1_GO (){
	HAL_GPIO_WritePin(A1_GPIO_Port, A1_Pin, GPIO_PIN_SET);
}

void event_A2_GO (){
	HAL_GPIO_WritePin(A2_GPIO_Port, A2_Pin, GPIO_PIN_SET);
}

void event_A3_GO (){
	HAL_GPIO_WritePin(A3_go_GPIO_Port, A3_go_Pin, GPIO_PIN_SET);
	vTaskDelay(1000);
	HAL_GPIO_WritePin(A3_go_GPIO_Port, A3_go_Pin, GPIO_PIN_RESET);
}

void event_A1_ret (){
	HAL_GPIO_WritePin(A1_GPIO_Port, A1_Pin, GPIO_PIN_RESET);
}

void event_A2_ret (){
	HAL_GPIO_WritePin(A2_GPIO_Port, A2_Pin, GPIO_PIN_RESET);
}

void event_A3_ret (){
	HAL_GPIO_WritePin(A3_ret_GPIO_Port, A3_ret_Pin, GPIO_PIN_SET);
	vTaskDelay(1000);
	HAL_GPIO_WritePin(A3_ret_GPIO_Port, A3_ret_Pin, GPIO_PIN_RESET);
}

void event_sp(){

	occurenceMonitor send;
	BaseType_t qResult;

	/*O elemento do início do processo produtivo necessariamente saiu de B1*/
	piece recieved;
	if (xSemaphoreTake(ChangeB1, portMAX_DELAY) == pdTRUE){
		qResult = xQueueReceive(qB1, &recieved, 0);

		xSemaphoreGive(ChangeB1);  /*!< Libera o Mutex*/
	}

	if (qResult == pdTRUE){
		recieved.tempo_insercao = getSystemTime();

		esteira = recieved; /*O elemento agora está na esteira*/

		send.occurence = peca_inicio;
		send.moment = getSystemTime();
		send.id = recieved.id_peca;

	}else{
		/*Não faz sentido, pois todas as peças que ingressam no processo
		produtivo devem vir de B1, e B1 está vazio!!! Sinaliza-se com um
		erro.*/
		send.occurence = qEvensType_max;
	}

	/*Envia para o monitor a mensagem para ser informada na na UART*/
	xQueueSendToBack(qMonitor, &send, 1000);

}

void event_s1(){

	occurenceMonitor send;

	send.occurence = B1_cai;
	send.moment = getSystemTime();
	send.id = esteira.id_peca;

	/*Envia para o monitor a mensagem para ser informada na na UART*/
	xQueueSendToBack(qMonitor, &send, 1000);

	/*Insere o elemento no buffer B1*/
	xQueueSendToBack(qB1, &esteira, 1000);

}

void event_s2(){

	occurenceMonitor send;

	send.occurence = B2_cai;
	send.moment = getSystemTime();
	send.id = esteira.id_peca;

	/*Envia para o monitor a mensagem para ser informada na na UART*/
	xQueueSendToBack(qMonitor, &send, 1000);

	/*Insere o elemento no buffer B1*/
	xQueueSendToBack(qB2, &esteira, 1000);

}

void event_s3(){

	occurenceMonitor send;

	send.occurence = B3_cai;
	send.moment = getSystemTime();
	send.id = esteira.id_peca;

	/*Envia para o monitor a mensagem para ser informada na na UART*/
	xQueueSendToBack(qMonitor, &send, 1000);

	/*Insere o elemento no buffer B1*/
	xQueueSendToBack(qB3, &esteira, 1000);

}


/**---------------------------------------------------------------------
  * @brief  Incializa uma máquina de estados para o controle do Banco de Ensaios
  * XC231. Esta função prepara a máquina para ser executada, porém não a executa.
  * @param sm: Ponteiro para a máquina de estados ao qual deseja-se inicializar.
  * @retval None
  */
void initStateMachine (sm_t* sm){

	sm->state = &sm->states_list[ST_INITIAL]; /*Aponta o estado atual para o estado inicial*/
	sm->occur_event = alphabet_max;   /*Quando nenhum evento ocorre na planta*/

	/*Definir para cada evento um callback (ou manter o callback nulo)*/
	sm->events_list[B1_in].event_id = B1_in;
	sm->events_list[B1_in].callback = (Callb_event_t)event_A1_GO;
	sm->events_list[B1_in].controllable = TRUE;

	sm->events_list[B2_in].event_id = B2_in;
	sm->events_list[B2_in].callback = (Callb_event_t)event_A2_GO;
	sm->events_list[B2_in].controllable = TRUE;

	sm->events_list[B3_in].event_id = B3_in;
	sm->events_list[B3_in].callback = (Callb_event_t)event_A3_GO;
	sm->events_list[B3_in].controllable = TRUE;

	sm->events_list[B1_out].event_id = B1_out;
	sm->events_list[B1_out].callback = (Callb_event_t)event_A1_ret;
	sm->events_list[B1_out].controllable = TRUE;

	sm->events_list[B2_out].event_id = B2_out;
	sm->events_list[B2_out].callback = (Callb_event_t)event_A2_ret;
	sm->events_list[B2_out].controllable = TRUE;

	sm->events_list[B3_out].event_id = B3_out;
	sm->events_list[B3_out].callback = (Callb_event_t)event_A3_ret;
	sm->events_list[B3_out].controllable = TRUE;

	sm->events_list[s1].event_id = s1;
	sm->events_list[s1].callback = (Callb_event_t)event_s1;
	sm->events_list[s1].controllable = FALSE;

	sm->events_list[s2].event_id = s2;
	sm->events_list[s2].callback = (Callb_event_t)event_s2;
	sm->events_list[s2].controllable = FALSE;

	sm->events_list[s3].event_id = s3;
	sm->events_list[s3].callback = (Callb_event_t)event_s3;
	sm->events_list[s3].controllable = FALSE;

	sm->events_list[sp].event_id = sp;
	sm->events_list[sp].callback = (Callb_event_t)event_sp;
	sm->events_list[sp].controllable = FALSE;


	/*Definir as transições de cada estado do autômato*/
	sm->states_list[ST_INITIAL].list_trans[0].dst = &sm->states_list[ST_1];
	sm->states_list[ST_INITIAL].list_trans[0].event = &sm->events_list[sp];
	sm->states_list[ST_INITIAL].list_trans[1].event = NULL;
	sm->states_list[ST_INITIAL].list_trans[1].event = NULL;
	sm->states_list[ST_INITIAL].marked = TRUE;

	sm->states_list[ST_1].list_trans[0].dst = &sm->states_list[ST_6];
	sm->states_list[ST_1].list_trans[0].event = &sm->events_list[B1_in];
	sm->states_list[ST_1].list_trans[1].dst = &sm->states_list[ST_4];
	sm->states_list[ST_1].list_trans[1].event = &sm->events_list[B2_in];
	sm->states_list[ST_1].list_trans[2].dst = &sm->states_list[ST_2];
	sm->states_list[ST_1].list_trans[2].event = &sm->events_list[B3_in];
	sm->states_list[ST_1].list_trans[3].event = NULL;
	sm->states_list[ST_1].list_trans[3].event = NULL;
	sm->states_list[ST_1].marked = FALSE;

	sm->states_list[ST_2].list_trans[0].dst = &sm->states_list[ST_3];
	sm->states_list[ST_2].list_trans[0].event = &sm->events_list[s3];
	sm->states_list[ST_2].list_trans[1].event = NULL;
	sm->states_list[ST_2].list_trans[1].event = NULL;
	sm->states_list[ST_2].marked = FALSE;

	sm->states_list[ST_3].list_trans[0].dst = &sm->states_list[ST_INITIAL];
	sm->states_list[ST_3].list_trans[0].event = &sm->events_list[B3_out];
	sm->states_list[ST_3].list_trans[1].event = NULL;
	sm->states_list[ST_3].list_trans[1].event = NULL;
	sm->states_list[ST_3].marked = FALSE;

	sm->states_list[ST_4].list_trans[0].dst = &sm->states_list[ST_5];
	sm->states_list[ST_4].list_trans[0].event = &sm->events_list[s2];
	sm->states_list[ST_4].list_trans[1].event = NULL;
	sm->states_list[ST_4].list_trans[1].event = NULL;
	sm->states_list[ST_4].marked = FALSE;

	sm->states_list[ST_5].list_trans[0].dst = &sm->states_list[ST_INITIAL];
	sm->states_list[ST_5].list_trans[0].event = &sm->events_list[B2_out];
	sm->states_list[ST_5].list_trans[1].event = NULL;
	sm->states_list[ST_5].list_trans[1].event = NULL;
	sm->states_list[ST_5].marked = FALSE;

	sm->states_list[ST_6].list_trans[0].dst = &sm->states_list[ST_7];
	sm->states_list[ST_6].list_trans[0].event = &sm->events_list[s1];
	sm->states_list[ST_6].list_trans[1].event = NULL;
	sm->states_list[ST_6].list_trans[1].event = NULL;
	sm->states_list[ST_6].marked = FALSE;

	sm->states_list[ST_7].list_trans[0].dst = &sm->states_list[ST_INITIAL];
	sm->states_list[ST_7].list_trans[0].event = &sm->events_list[B1_out];
	sm->states_list[ST_7].list_trans[1].event = NULL;
	sm->states_list[ST_7].list_trans[1].event = NULL;
	sm->states_list[ST_7].marked = FALSE;

}

/**---------------------------------------------------------------------
  * @brief  Realiza a execução do estado corrente da máquina de estados.
  * @param sm: Ponteiro para a máquina de estados ao qual deseja-se executar.
  * @note A máquina de estados sm deve neccessariamente já estar inicializada
  * com a função initStateMachine.
  * @retval None
  */
void execSM_classic(sm_t* sm){
	uint8_t i = 0;

	Callb_event_t exec_func = NULL;
	State* new_state = NULL;

	/*Adquire o mutex para que ninguém possa modificar a máquina de estados
	 * durante sua execução*/
	if (xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) == pdTRUE){

		/*Varre a lista de transições em busca de alguma que possa ocorrer*/
		while (new_state == NULL){

			/*Chegou no final vetor de eventos e não encontrou
			 * transição pra ocorrrer, deve-se esperar um evento da planta*/
			if (sm->state->list_trans[i].event == NULL){
				i=0;
				/*Libera o mutex e dorme*/
				xSemaphoreGive(ChangeSmMutex);
				xSemaphoreTake(semBlocking, portMAX_DELAY);
				/*Ao acordar, adquire o mutex novamente, pois ele foi liberado antes de dormir*/
				while(xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) != pdTRUE);
			}

			/*Se o evento for controlável, ocorre de toda forma
			 *Se o evento não for controlável, verifica se ele ocorreu na planta*/
			if (sm->state->list_trans[i].event->controllable == TRUE ||
				sm->state->list_trans[i].event->event_id == sm->occur_event){

				/*Controle clássico - A primeira transição encontrada vai ocorrer*/
				sm->occur_event = alphabet_max;
				exec_func = sm->state->list_trans[i].event->callback;
				new_state = sm->state->list_trans[i].dst;
			}

			i++;
		}
		xSemaphoreGive(ChangeSmMutex);
	}

	/*No controle clássico apenas executa-se a transição encontrada*/
	if (exec_func != NULL)
		exec_func();
	sm->state = new_state;

}

/**---------------------------------------------------------------------
  * @brief  Realiza a execução do estado corrente da máquina de estados
  * de forma otimizada.
  * @param sm: Ponteiro para a máquina de estados ao qual deseja-se executar
  * de forma otimizada.
  * @note A máquina de estados sm deve neccessariamente já estar inicializada
  * com a função initStateMachine.
  * @retval None
  */
void execSM_otm(sm_t* sm){

	Trans* enabled_trans[4];
	uint8_t numb_enabled_trans = 0;
	uint8_t i=0;

	/*Apaga resquísios de execuções antigas*/
	memset (enabled_trans, 0, 4*sizeof(Trans*));

	/*Adquire o mutex para que ninguém possa modificar a máquina de estados
	 * durante sua execução*/
	if (xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) == pdTRUE){

		/*Varre a lista de transições em busca de alguma que possa ocorrer*/
		while (!(sm->state->list_trans[i].event == NULL && numb_enabled_trans > 0)){

			/*Chegou no final vetor de eventos e não encontrou
			 * transição pra ocorrrer, deve-se esperar um evento da planta*/
			if (sm->state->list_trans[i].event == NULL){
				i=0;
				/*Libera o mutex e dorme*/
				xSemaphoreGive(ChangeSmMutex);
				xSemaphoreTake(semBlocking, portMAX_DELAY);
				/*Ao acordar, adquire o mutex novamente, pois ele foi liberado antes de dormir*/
				while(xSemaphoreTake(ChangeSmMutex, portMAX_DELAY) != pdTRUE);

			}

			if (sm->state->list_trans[i].event->controllable == TRUE ||
				sm->state->list_trans[i].event->event_id == sm->occur_event){
					/*Controle estendido - Cria uma lista de transições habilitadas*/
					enabled_trans[numb_enabled_trans++] = &(sm->state->list_trans[i]);
			}

			i++;
		}


		enabled_trans[numb_enabled_trans]=NULL; /*Para indicar o fim do vetor*/

		/*Notifica o Otimizador, acordando-o*/
		xTaskNotify(ComSup2Otm, (uint32_t)&enabled_trans[0], eSetValueWithOverwrite);
		/*Coloca o Supervisor pra Dormir*/
		xSemaphoreTake(inOtm, portMAX_DELAY);

		xSemaphoreGive(ChangeSmMutex);
	}

}

volatile uint32_t buf;
volatile float beneficio;
float calc_beneficio(SystemParameters* p, Trans* transa, uint32_t time){


	if (transa->event->event_id == B2_in){
		buf = (uint32_t)uxQueueMessagesWaiting(qB2);

		if ((((buf + 1)*p->periodo_B2) - (time% p->periodo_B2)) >= (uint32_t)p->tempo_maximo){
			beneficio = (-1.0*p->custo_peca)/(((buf + 1)*p->periodo_B2) - (time% p->periodo_B2));
		}
		else
			beneficio = (p->receita_B3 - p->custo_peca)/(float)(((buf + 1)*p->periodo_B3) - (time% p->periodo_B3));

	} else if (transa->event->event_id == B3_in){
		buf = (uint32_t)uxQueueMessagesWaiting(qB3);

		if ((((buf + 1)*p->periodo_B3) - (time% p->periodo_B3)) >= (uint32_t)p->tempo_maximo){
			beneficio = (-1.0*p->custo_peca)/(((buf + 1)*p->periodo_B3) - (time% p->periodo_B3));
		}
		else
			beneficio = (p->receita_B3 - p->custo_peca)/(float)(((buf + 1)*p->periodo_B3) - (time% p->periodo_B3));

	} else{

		beneficio = 0.0;

	}

	return beneficio;
}

void troca_trans (Trans** t1, Trans** t2){
	Trans* aux;
	aux = *t1;
	*t1 = *t2;
	*t2 = aux;
}

/**---------------------------------------------------------------------
  * @brief  Realiza o cálculo e os processos de otimização, ao fim, retorna
  * uma tupla de eventos para executar a ação e controle.
  * @param sm: Ponteiro para a máquina de estados ao qual deseja-se otimizar.
  * @param vec_trans: Vetor de ponteiros para transições otimizáveis habilitadas;
  * @param n_trans: Número de transições otimizáveis habilitadas;
  * @note A máquina de estados sm deve neccessariamente já estar inicializada
  * com a função initStateMachine.
  * @retval Tupla (vetor de ponteiros pra transições) ordenado de forma
  * cresente de benefícios ao sistema.
  */

void Otimiza(sm_t* sm, Trans** vec_trans, uint8_t n_trans){

	SystemParameters p = getSystemParameters();
	uint32_t temp = getSystemTime();

	if (calc_beneficio(&p, vec_trans[0], temp) < calc_beneficio(&p, vec_trans[1], temp)){
		troca_trans (&vec_trans[0], &vec_trans[1]);
	}

	if (calc_beneficio(&p, vec_trans[1], temp) < calc_beneficio(&p, vec_trans[2], temp)){
			troca_trans (&vec_trans[1], &vec_trans[2]);
	}

	if (calc_beneficio(&p, vec_trans[0], temp) < calc_beneficio(&p, vec_trans[1], temp)){
			troca_trans (&vec_trans[0], &vec_trans[1]);
	}

}

/*-------------------------------------- TAREFAS ---------------------------------------*/

/**---------------------------------------------------------------------
  * @brief  Tarefas para a máquina de estados. Executa os eventos controláveis sempre
  * quando possivel (quando habilitados). Quando não há evento controlável para ser
  * habilitado, dorme esperando a ocorrência de um evento não controlável.
  * @retval None
  */
void StartStateMachineTask(void const * argument){

	/*Inicializa a máquina de estados do banco de ensaios*/
	initStateMachine (&BancoDeEnsaios_sm);

	/*Este semáforo é fechado quando não há eventos possíveis para evoluir a máquina de
	 * estados e é aberto quando surgir um evento.*/
	semBlocking = xSemaphoreCreateBinary();

	inOtm = xSemaphoreCreateBinary();

	qB1 = xQueueCreate(BUFERS_SIZE, sizeof(piece));
	qB2 = xQueueCreate(BUFERS_SIZE, sizeof(piece));
	qB3 = xQueueCreate(BUFERS_SIZE, sizeof(piece));

	/*Cria o semáforo que controla o estado de ligado/desligado da máquina*/
	BancoDeEnsaios_sm.semPower = xSemaphoreCreateBinary();

	ChangeB1 = xSemaphoreCreateMutex();
	ChangeSmMutex = xSemaphoreCreateMutex();

	num_optimizable_events = 3;
	optimizable_events[0] = &BancoDeEnsaios_sm.events_list[B1_in];
	optimizable_events[1] = &BancoDeEnsaios_sm.events_list[B2_in];
	optimizable_events[2] = &BancoDeEnsaios_sm.events_list[B3_in];


	/*Espera indentificador da tarefa a ser notificada ser iniciado*/
	while (ComSup2Otm == NULL){
		vTaskDelay(10);
	}

	/*A máquina de estados inicia desligada*/
	xSemaphoreTake(BancoDeEnsaios_sm.semPower, portMAX_DELAY);

	while (qMonitor == NULL){
		vTaskDelay(10);
	}

	uint32_t ant_counter_value=0, curr_counter_value=vTaskGetCurrentCounterValue();
	volatile uint32_t curr_exec_time;

	while(1){

		/*Apenas para a tarefa sofrer preempção para atualizar o seu contador*/
		vTaskDelay(1);

		ant_counter_value = curr_counter_value;
		curr_counter_value = vTaskGetCurrentCounterValue();
		curr_exec_time = curr_counter_value - ant_counter_value;

		occurenceMonitor send;
		send.occurence = sm_time;
		send.moment = curr_exec_time * 10;

		/*Envia para o monitor a mensagem para ser informada na na UART*/

		xQueueSendToBack(qMonitor, &send, 1000);

		if (BancoDeEnsaios_sm.mode != ES)
			execSM_classic(&BancoDeEnsaios_sm);
		else
			execSM_otm(&BancoDeEnsaios_sm);

	}
}

/**---------------------------------------------------------------------
  * @brief Tarefa referente à otimização. Realiza três passos básicos. Primeiro a recepção
  * eventos habilitados pelo supervisor. Segundo o procedimento de Otimização em si. Terceiro,
  * o retorno do estado que deseja-se evoluir o Supervisor.
  * @note: O conjunto de eventos otimizáveis encontra-se no vetor "optimizable_events"
  * com tamanho "number_optm_events". Pode-se executar o processamento da otimização de forma
  * off-line, ou seja, processar em segundo plano enquanto a máquina de estados estiver parada,
  * se este for o caso, a região crítica que controla estas duas variáveis aqui no otimizador
  * deve ser rápida, de forma a não atrapalhar a evolução do Supervisor. Lembre-se, mesmo o
  * supervisor tento maior prioridade, em caso de regiões críticas usando mutexes pode haver
  * uma inversão de prioridades.
  * @retval None
  */
uint8_t i;

Trans** conjunto;           /*<! Recebido do Supervisor - Todas as transições habilitadas*/
uint8_t n_events;           /*<! Total de transições habilitadas*/
uint8_t n_opt_events;       /*<! Total de eventos otimizáveis habilitados*/
State *new_state;
Callb_event_t exec_func;

void StartOptimizerTask(void const * argument){

	/*Adquire o Habdler da Tarefa que será notificada*/
	ComSup2Otm = xTaskGetCurrentTaskHandle();

	xTaskNotifyStateClear(ComSup2Otm);

	while (qMonitor == NULL){
		vTaskDelay(10);
	}

	uint32_t ant_counter_value=0, curr_counter_value=vTaskGetCurrentCounterValue();
	volatile uint32_t curr_exec_time;

	while(1){

		/*Apenas para a tarefa sofrer preempção para atualizar o seu contador*/
		vTaskDelay(1);
		ant_counter_value = curr_counter_value;
		curr_counter_value = vTaskGetCurrentCounterValue();
		curr_exec_time = curr_counter_value - ant_counter_value;

		occurenceMonitor send;
		send.occurence = otm_time;
		send.moment = curr_exec_time * 10;

		/*Envia para o monitor a mensagem para ser informada na na UART*/
		if (curr_exec_time !=0)
			xQueueSendToBack(qMonitor, &send, 1000);

		/*Aguarda o supervisor enviar um conjunto de transições habilitadas,
		 * este conjunto contém os eventos, necessários para o processamento. Esta
		 * é uma função bloqueante, portanto o Otimizador dorme enquanto espera um
		 * conjunto.*/
		while (xTaskNotifyWait(0X00, UINT32_MAX, (uint32_t*)&conjunto, portMAX_DELAY) != pdTRUE);

		n_events = 0; n_opt_events = 0;

		/*Faz a intersecção*/
		while (conjunto[n_events] != NULL){
			for (i=0; i< num_optimizable_events; i++){
				if (conjunto[n_events]->event == optimizable_events[i]){
					n_opt_events++;
				}
			}
			n_events++;
		}

		/*Condição para realizar o processo de otimização*/
		if (n_opt_events !=0 && conjunto[1] != NULL){
			Otimiza(&BancoDeEnsaios_sm, conjunto, n_opt_events);
		}

		/*De posse da tupla otimizada, executa-se a tupla como se fosse
		 * controle clássico*/
		i=0;
		exec_func = NULL; new_state = NULL;
		while (new_state == NULL){

			if (conjunto[i] == NULL){
				/*Não faz sentido cair aqui*/
				asm("NOP");
			}

			/*Se o evento for conrolável, ocorre de toda forma
			 *Se o evento não for controlável, verifica se ele ocorreu na planta*/
			if (conjunto[i]->event->controllable == TRUE ||
					conjunto[i]->event->event_id == BancoDeEnsaios_sm.occur_event){
				/*A primeira transição encontrada vai ocorrer*/
				BancoDeEnsaios_sm.occur_event = alphabet_max;
				exec_func = conjunto[i]->event->callback;
				new_state = conjunto[i]->dst;
			}

			i++;
		}

		if (exec_func != NULL)
			exec_func();

		BancoDeEnsaios_sm.state = new_state;

		/*Acorda o Supervisor*/
		xSemaphoreGive(inOtm);

	}

}

/*----------------------------- INTERRUPÇÕES E CALLBACKS ------------------------------*/

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){

	/*Um evento ocorreu na planta*/
	if (GPIO_Pin == Sp_Pin)
		BancoDeEnsaios_sm.occur_event = sp;
	else if (GPIO_Pin == S1_Pin)
		BancoDeEnsaios_sm.occur_event = s1;
	else if(GPIO_Pin == S2_Pin)
		BancoDeEnsaios_sm.occur_event = s2;
	else if(GPIO_Pin == S3_Pin)
		BancoDeEnsaios_sm.occur_event = s3;


	/*Abre o semáforo que espera por um sensor*/
	xSemaphoreGiveFromISR(semBlocking, pdFALSE);

}
