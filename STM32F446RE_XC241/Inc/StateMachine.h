/*
 * StateMachine.h
 *
 *  Created on: 5 de abr de 2019
 *      Author: cristian
 */


#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
/* Private define ------------------------------------------------------------*/

typedef enum {
	FALSE=0, TRUE=!FALSE
}Bool;

/*Função de callback de eventos. São as ações possíveis de serem executadas
 * por eventos*/
typedef void (*Callb_event_t)(void);
typedef struct Trans Trans;
typedef struct State State;

/*Estados do supervisor*/
typedef enum{
  ST_INITIAL,
  ST_1,
  ST_2,
  ST_3,
  ST_4,
  ST_5,
  ST_6,
  ST_7,
  ST_END
}state_t;

typedef enum{
	B1_in,B2_in,B3_in, B1_out,B2_out,B3_out, s1, s2, s3, sp, alphabet_max
}alphabet;

typedef struct Event{
	alphabet event_id;       /*<! Identificador do evento*/
	Callb_event_t callback;  /*<! Função executada quando o evento ocorre*/
	Bool controllable;       /*<! Verdadeiro quando é um evento controlável*/
}Event;


typedef struct Trans{
	Event* event;           /*<! Ponteiro para o evento da transição*/
	State* dst;             /*<! Estado de destino*/
}Trans;

typedef struct State{
	Trans list_trans[4];   /*<! Lista de transições partindo deste estado*/
	Bool marked;           /*<! Verdadeiro se é um estado marcado*/
}State;

typedef enum{
	C1,C2,C3,C4,ES
}Mode_t;

/*Tipo State Machine*/
typedef struct StateMachine{
  State states_list[ST_END];             /*<! Lista de estados.*/
  Event events_list[alphabet_max];       /*<! Lista de eventos (alfabeto)*/
  State* state;                          /*<! Ponteiro para o estado atual*/
  alphabet occur_event;                  /*<! Evento ocorrido na planta*/
  Mode_t mode;                           /*<! Modo de operação da máquina de estados*/
  xSemaphoreHandle semPower;             /*<! Semáforo que liga e desliga a máquina de estados*/
}sm_t;

typedef struct piece{
	uint32_t id_peca;            /*<! Identificador para a peça*/
	uint32_t tempo_insercao;     /*<! Momento em que a peça foi inserida*/
}piece;


#define BUFERS_SIZE  15        /*<! Capacidade dos buffers*/
#define QTDE_B1_INIT 3         /*<! Quantidade de peças inicialmente em B1*/

/*Protótipo de Funções*/
void StateMachineTask(void const * argument);
void OtimizadorTask(void const * argument);
void callback_action(sm_t* sm);


#endif /* STATEMACHINE_H_ */
