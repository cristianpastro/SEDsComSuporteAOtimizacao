/**
  ******************************************************************************
  * File Name          : SysControl.h
  * Description        : This file contains all the functions prototypes for
  *                      the System General Control
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
#ifndef __SysControl_H
#define __SysControl_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "StateMachine.h"

typedef struct SystemParameters{
	uint8_t Speaker_Period;  /*<! Speker period (max 79 seconds)*/
	uint32_t custo_peca;     /*<! Custo unitário de cada peça*/
	uint32_t receita_B2;     /*<! Receita gerada por peças professadas por B2*/
	uint32_t receita_B3;     /*<! Receita gerada por peças professadas por B3*/
	uint32_t periodo_B2;     /*<! Período de retirada das peças do buffer B2*/
	uint32_t periodo_B3;     /*<! Período de retirada das peças do buffer B3*/
	uint8_t tempo_maximo;    /*<! Tempo máximo que uma peça pode ficar fora de B1*/
}SystemParameters;

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */
SystemParameters getSystemParameters();
void StartSystemControlTask(void const * argument);
void setSpeakerPeriod (uint8_t period);
void setCustoPeca (uint32_t custo_peca);
void setReceitaB2(uint32_t receita_B2);
void setReceitaB3(uint32_t receita_B3);
void setPeriodoB2 (uint32_t periodoB2);
void setPeriodoB3 (uint32_t periodoB3);
void setTempoMaximo (uint8_t tempo_maximo);
uint8_t getSpeakerPeriod (void);
uint32_t getCustoPeca (void);
uint32_t getReceitaB2(void);
uint32_t getReceitaB3(void);
uint32_t getPeriodoB2 (void);
uint32_t getPeriodoB3 (void);
uint8_t getTempoMaximo (void);
uint32_t getSystemTime(void);
void addSystemTime(void);
void addPieces(uint8_t n, xQueueHandle* q);
void changeMode(sm_t* sm, Mode_t new_mode);
void powerOnStateMachine(sm_t* sm);
void PowerOffStateMachine(sm_t* sm);
/* USER CODE END Prototypes */

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
