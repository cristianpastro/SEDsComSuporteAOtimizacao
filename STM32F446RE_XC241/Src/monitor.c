/*
 * monitor.c
 *
 *  Created on: 20 de mai de 2019
 *      Author: cristian
 *
 *  Esta API realiza o envio das informações importantes para o
 *  console de monitoramento. As informações são recebidas de
 *  outros processos através da fila qMonitor. Optou-se por
 *  criar um processo exclusivo para este fim, pois caso desejar,
 *  pode-se alterar a forma de monitoramento editando-se apenas
 *  esta API.
 */

#include "CLI.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "tim.h"
#include "cmsis_os.h"
#include "usart.h"
#include "monitor.h"
#include "string.h"
//#include "stdio.h"
#include "printf-stdarg.h"
#include "math.h"
#include "SysControl.h"


xQueueHandle  qMonitor=NULL;
uint32_t ulHighFrequencyTimerTicks = 0;

uint32_t vGetTimerForRunTimeStats (void){
	return ulHighFrequencyTimerTicks;
}

void MonitorPutChar(char ucData){
		// Envia um caracter
		HAL_UART_Transmit_IT(&huart3, (uint8_t *)&ucData, 1);
		// Espera por uma interrupção da UART
		xSemaphoreTake(semtxUSART3, portMAX_DELAY);
}

void vConfigureTimerForRunTimeStats(void){
	MX_TIM6_Init();
}



void MonitorPutString(char *string, uint16_t size){
		/* Descobre o tamanho da string, caso não informado*/
		if (size == 0){
			uint8_t *tmp = (uint8_t *)string;

			while(*tmp++){
				size++;
			}
		}

		/* Transmite uma sequencia de dados, com fluxo controlado pela interrupção */
		HAL_UART_Transmit_IT(&huart3, (uint8_t *)string, size);

		/*Aguarda o fim da transmissão*/
		xSemaphoreTake(semtxUSART3, portMAX_DELAY);
}


void MonitorGetChar(UART_HandleTypeDef *huart, uint8_t *data, uint32_t timeout)
{
	osEvent event;

	if (huart->Instance == USART3)
	{
		event = osMessageGet(MonitorReceptEvent, timeout);
		if(event.status == osEventMessage){
			*data = (uint8_t)event.value.v;
		}
	}

}
char str[120];
/* USER CODE END Header_StartMonitorTask*/
void StartMonitorTask(void const * argument)
{

	qMonitor = xQueueCreate(20, sizeof(occurenceMonitor));
	occurenceMonitor recieve;

	Lucros procc1;
	procc1.pecasB2 = 0;
	procc1.pecasB3 = 0;
	procc1.pecasB2_est = 0;
	procc1.pecasB3_est = 0;

	uint8_t supervisoes=0;
	uint8_t otimizacoes=0;
	uint32_t tempo_supervisor=0;
	uint32_t tempo_otimizador = 0;

	uint16_t maior_otm=0, menor_otm=UINT16_MAX;
	uint16_t maior_supervisor=0, menor_supervisor=UINT16_MAX;

  for(;;)
  {
    xQueueReceive(qMonitor, &recieve, portMAX_DELAY);

    if (recieve.occurence == init_process){
    	sprintf (str, "O processo produtivo foi iniciado!\n\r");
    } else if (recieve.occurence ==  ins_piece){
    	sprintf (str, "%u Peças foram inseridas no buffer B1\n\r", (uint16_t)recieve.id);
    }
    else if (recieve.occurence == end_process){

    	sprintf (str, " ********************** Relatório Final **********************\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Peças processadas por B2:           %u\n\r", procc1.pecasB2);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Receita unitária de peças de B2:    R$ %u\n\r", (uint16_t)getReceitaB2());
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Receita total gerada por B2:        R$ %u\n\r", (uint16_t)(procc1.pecasB2 * getReceitaB2()));
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Peças processadas por B3:           %u\n\r", (uint16_t)procc1.pecasB3);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Receita unitária de peças de B3:    R$ %u\n\r", (uint16_t)getReceitaB3());
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Receita total gerada por B3:        R$ %u\n\r", (uint16_t)(procc1.pecasB3 * getReceitaB3()));
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "RECEITA TOTAL:                      R$ %u\n\r",(uint16_t)( procc1.pecasB2 * getReceitaB2() + procc1.pecasB3 * getReceitaB3()));
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Peças estragadas em B2:             %u\n\r", (uint16_t)procc1.pecasB2_est);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Peças estragadas em B3:             %u\n\r", (uint16_t)procc1.pecasB3_est);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Peças Totais no processo:           %u\n\r", (uint16_t)(procc1.pecasB2 + procc1.pecasB3 + procc1.pecasB2_est + procc1.pecasB3_est));
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Custo por peça:                    R$ %u\n\r", (uint16_t)getCustoPeca());
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "DESPESA TOTAL:                     R$ %u\n\r", (uint16_t)((procc1.pecasB2 + procc1.pecasB3 + procc1.pecasB2_est + procc1.pecasB3_est)*getCustoPeca()));
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Tempo do processo:                 %u segundos\n\r", (uint16_t)recieve.moment);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "\n\r");
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "-----------------------------------------\n\r");
    	MonitorPutString (str, strlen(str));
    	volatile float nn = (float)trunc(((double)((int32_t)(procc1.pecasB2 * getReceitaB2() + procc1.pecasB3 * getReceitaB3()) - (int32_t)((procc1.pecasB2 + procc1.pecasB3 + procc1.pecasB2_est + procc1.pecasB3_est)*getCustoPeca()))/(double)recieve.moment)*1000);
    	sprintf (str, "BENEFICIO TOTAL AO SISTEMA:        %u R$/s\n\r", (uint16_t)nn);
    	MonitorPutString (str, strlen(str));
    	MonitorPutString ("\n\r", 2);

    	sprintf (str, "Menor tempo de execução do Supervisor = %u\n\r", (uint16_t)menor_supervisor);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Maior tempo de execução do Supervisor = %u\n\r", (uint16_t)maior_supervisor);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Tempo total de execução do Supervisor = %d\n\r", (int)tempo_supervisor);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Número de execuções do Supervisor = %u\n\r", (uint16_t)supervisoes);
    	MonitorPutString (str, strlen(str));
    	sprintf (str, "Menor tempo de execução do Otimizador = %u\n\r", (uint16_t)menor_otm);
    	MonitorPutString (str, strlen(str));
   	  	sprintf (str, "Maior tempo de execução do Otimizador = %u\n\r", (uint16_t)maior_otm);
   	  	MonitorPutString (str, strlen(str));
    	sprintf (str, "Tempo total de execução do Otimizador = %d\n\r", (int)tempo_otimizador);
      	MonitorPutString (str, strlen(str));
      	sprintf (str, "Número de execuções do Otimizador = %u\n\r", (uint16_t)otimizacoes);


    }
    else if (recieve.occurence == B1_cai){
    	sprintf (str, "t=%6d - A peça %u foi inserida no buffer B1\n\r",(int)recieve.moment, (uint16_t)recieve.id);
    }
    else if (recieve.occurence == B2_retira){
    	sprintf (str, "t=%6d - A peça %d foi removida do buffer B2 - Houve uma receita de R$ %d \n\r",(int)recieve.moment, (int)recieve.id, (int)getReceitaB2());
    	procc1.pecasB2++;
    }
    else if (recieve.occurence == B3_retira){
    	sprintf (str, "t=%6d - A peça %u foi removida do buffer B3 - Houve uma receita de R$ %u \n\r",(int)recieve.moment, (uint16_t)recieve.id, (uint16_t)getReceitaB3());
    	procc1.pecasB3++;
    }
    else if (recieve.occurence == B2_estraga){
    	sprintf (str, "t=%6d - A peça %u foi removida do buffer B2, porém estava estragada - Deixou-se de obter receita com esta peça\n\r",(int)recieve.moment, (uint16_t)recieve.id);
    	procc1.pecasB2_est++;
    }
    else if (recieve.occurence == B3_estraga){
    	sprintf (str, "t=%6d - A peça %u foi removida do buffer B3, porém estava estragada - Deixou-se de obter receita com esta peça\n\r", (int)recieve.moment, (uint16_t)recieve.id);
    	procc1.pecasB3_est++;
    }
    else if (recieve.occurence == peca_inicio){
    	sprintf (str, "t=%6d - A Peça %u foi identificada no início do processo produtivo\n\r",(int)recieve.moment, (uint16_t)recieve.id);
    }
    else if (recieve.occurence == B2_cai){
    	sprintf (str, "t=%6d - A peça %u foi inserida no buffer B2\n\r",(int)recieve.moment, (uint16_t)recieve.id);
    }
    else if (recieve.occurence == B3_cai){
    	sprintf (str, "t=%6d - A peça %u foi inserida no buffer B3\n\r",(int)recieve.moment, (uint16_t)recieve.id);
    }else if (recieve.occurence == otm_time){
    	sprintf (str, "O processo de otimização demorou cerca de %u microssegundos para executar\n\r", (uint16_t)recieve.moment);
    	tempo_otimizador+= recieve.moment;
    	supervisoes++;
    	if (recieve.moment > maior_otm){
    		maior_otm = recieve.moment;
    	}
    	if (recieve.moment < menor_otm){
    		menor_otm = recieve.moment;
    	}
    }else if (recieve. occurence == sm_time){
    	sprintf (str, "O supervisor demorou cerca de %u microssegundos para executar\n\r", (int)recieve.moment);
    	tempo_supervisor+=recieve.moment;
    	otimizacoes++;
    	if (recieve.moment > maior_supervisor){
    		maior_supervisor = recieve.moment;
    	}
    	if (recieve.moment < menor_supervisor){
    		menor_supervisor = recieve.moment;
    	}

    } else {
    	sprintf (str, "Mensagem não reconhecida\n\r");
    }

    MonitorPutString (str, strlen(str));

  }

}
