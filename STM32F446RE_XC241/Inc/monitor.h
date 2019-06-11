/**
  ******************************************************************************
  * File Name          : monitor.h
  * Description        : This file contains all the functions prototypes for
  *                      the monitor
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MONITOR_H
#define __MONITOR_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */
 void MonitorPutChar(char ucData);
 void MonitorPutString(char *string, uint16_t size);
 void MonitorGetChar(UART_HandleTypeDef *huart, uint8_t *data, uint32_t timeout);
 void StartMonitorTask(void const * argument);
/* USER CODE END Prototypes */

 /*Tipos de ocorrência que o Monitor pode trabalhar*/
 typedef enum{
	 ins_piece, otm_time, sm_time, init_process, end_process, B2_retira, B3_retira, B2_estraga, B3_estraga, peca_inicio,B1_cai, B2_cai, B3_cai, qEvensType_max
 }qOccurenceType;

 typedef struct occurenceMonitor{
	 qOccurenceType occurence;
	 uint32_t moment;
	 uint32_t id;
 }occurenceMonitor;

 typedef struct Lucros{
	 uint32_t tempo_processo;
	 uint8_t pecasB2;
	 uint8_t pecasB3;
	 uint8_t pecasB2_est;
	 uint8_t pecasB3_est;
 }Lucros;

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
