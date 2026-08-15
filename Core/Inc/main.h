/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32l4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define PWR_HOLD_Pin GPIO_PIN_4
#define PWR_HOLD_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_12
#define SD_CS_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_13
#define LCD_CS_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_14
#define LCD_DC_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_15
#define LCD_RST_GPIO_Port GPIOB
#define LCD_BL_Pin GPIO_PIN_8
#define LCD_BL_GPIO_Port GPIOA
#define TP_INI_IN_Pin GPIO_PIN_15
#define TP_INI_IN_GPIO_Port GPIOA
#define TP_INI_IN_EXTI_IRQn EXTI15_10_IRQn
#define TP_RST_Pin GPIO_PIN_3
#define TP_RST_GPIO_Port GPIOB
#define CAN_SHDN_Pin GPIO_PIN_4
#define CAN_SHDN_GPIO_Port GPIOB
#define CAN_STB_Pin GPIO_PIN_5
#define CAN_STB_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define	LCD_RESET_ACTIVE	(HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 	GPIO_PIN_RESET))	//	0
#define	LCD_RESET_INACTIVE	(HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 	GPIO_PIN_SET))		//	1
#define	LCD_CS_ACTIVE		(HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 	GPIO_PIN_RESET))	//	0
#define	LCD_CS_INACTIVE		(HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin,	GPIO_PIN_SET))		//	1
#define	LCD_CMD				(HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, 	GPIO_PIN_RESET))	//	0
#define	LCD_DATA			(HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, 	GPIO_PIN_SET))		//	1
#define	SD_CS_ACTIVE		(HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, 	GPIO_PIN_RESET))	//	0
#define	SD_CS_INACTIVE		(HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin,	GPIO_PIN_SET))		//	1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
