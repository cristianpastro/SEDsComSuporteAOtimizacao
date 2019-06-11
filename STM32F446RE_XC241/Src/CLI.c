/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : CLI.c

  * Description        : Code for CLI applications
  ******************************************************************************
  Esta API realiza o controle do CLI (Command-Line interface),
  identificando o comando e respondendo ao usuário. Depois de
  processado, o comando é enviado ao seu respectivo processo.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "CLI.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
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

/* USER CODE BEGIN Header_StartTerminalTask */


void CLIPutChar(char ucData){
		// Envia um caracter
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)&ucData,1);
		// Espera por uma interrupï¿½ï¿½o da UART
		xSemaphoreTake(semtxUSART2, portMAX_DELAY);
}



void CLIPutString(char *string, uint16_t size){
		/* Descobre o tamanho da string, caso não informado*/
		if (size == 0){
			uint8_t *tmp = (uint8_t *)string;

			while(*tmp++){
				size++;
			}
		}

		/* Transmite uma sequencia de dados, com fluxo controlado pela interrupção */
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)string, size);

		/*Aguarda o fim da transmissão*/
		xSemaphoreTake(semtxUSART2, portMAX_DELAY);
}


void CLIGetChar(UART_HandleTypeDef *huart, uint8_t *data, uint32_t timeout)
{
	osEvent event;

	if (huart->Instance == USART2)
	{
		event = osMessageGet(CLIReceptEvent, timeout);
		if(event.status == osEventMessage){
			*data = (uint8_t)event.value.v;
		}
	}

}

/**
* @brief Function implementing the TerminalTask thread. O CLI deve processar o
* o comando recebido e abrir o semáforo correto, passando os parâmetros
* correspondentes.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTerminalTask */
void StartTerminalTask(void const * argument)
{
	MX_USART2_UART_Init();
	semtxUSART2 = xSemaphoreCreateBinary();

	semtxUSART3 = xSemaphoreCreateBinary();

	osMessageQDef(CLIReceptEvent, 128, unsigned char);
	CLIReceptEvent = osMessageCreate(osMessageQ(CLIReceptEvent), NULL);

	osMessageQDef(MonitorReceptEvent, 128, unsigned char);
	MonitorReceptEvent = osMessageCreate(osMessageQ(MonitorReceptEvent), NULL);


	//char teste[256];

	 for(;;)
	{
		//CLIGetChar(&huart2, (uint8_t*)&teste[0], osWaitForever);

		CLIPutString ("Envio do CLI (USART2)\n", 22);

		vTaskDelay(1000);
	}

}
