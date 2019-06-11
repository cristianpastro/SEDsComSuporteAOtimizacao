/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void ConfigureSystemParameters(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Sp_Pin GPIO_PIN_0
#define Sp_GPIO_Port GPIOC
#define Sp_EXTI_IRQn EXTI0_IRQn
#define S1_Pin GPIO_PIN_1
#define S1_GPIO_Port GPIOC
#define S1_EXTI_IRQn EXTI1_IRQn
#define S2_Pin GPIO_PIN_2
#define S2_GPIO_Port GPIOC
#define S2_EXTI_IRQn EXTI2_IRQn
#define S3_Pin GPIO_PIN_3
#define S3_GPIO_Port GPIOC
#define S3_EXTI_IRQn EXTI3_IRQn
#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOA
#define A3_ret_Pin GPIO_PIN_10
#define A3_ret_GPIO_Port GPIOB
#define esteira_Pin GPIO_PIN_14
#define esteira_GPIO_Port GPIOB
#define speaker_Pin GPIO_PIN_15
#define speaker_GPIO_Port GPIOB
#define A1_Pin GPIO_PIN_3
#define A1_GPIO_Port GPIOB
#define A2_Pin GPIO_PIN_4
#define A2_GPIO_Port GPIOB
#define A3_go_Pin GPIO_PIN_5
#define A3_go_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
