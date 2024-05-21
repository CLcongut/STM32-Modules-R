/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define USART_REC_LEN 300
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern uint8_t USART1_RX_BUF[USART_REC_LEN];

typedef struct
{
    uint8_t rx_data[USART_REC_LEN];
    uint8_t rx_dn_flag;
} RX_DATA_STRUCT;

extern RX_DATA_STRUCT RX_DS;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DHT11_Pin GPIO_PIN_4
#define DHT11_GPIO_Port GPIOA
#define Key2_Pin GPIO_PIN_10
#define Key2_GPIO_Port GPIOB
#define Key1_Pin GPIO_PIN_11
#define Key1_GPIO_Port GPIOB
#define BLK_Pin GPIO_PIN_12
#define BLK_GPIO_Port GPIOB
#define CS_Pin GPIO_PIN_13
#define CS_GPIO_Port GPIOB
#define DC_Pin GPIO_PIN_14
#define DC_GPIO_Port GPIOB
#define RES_Pin GPIO_PIN_15
#define RES_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
