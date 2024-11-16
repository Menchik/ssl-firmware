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

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef struct command {
	float vx;
	float vy;
	float vtheta;
	uint8_t kik_sig;
} command_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

#define PAYLOAD_SIZE 26

#define MOTOR_TIMER TIM1

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RADIO_IRQ_Pin GPIO_PIN_2
#define RADIO_IRQ_GPIO_Port GPIOA
#define RADIO_CSN_Pin GPIO_PIN_3
#define RADIO_CSN_GPIO_Port GPIOA
#define RADIO_CE_Pin GPIO_PIN_4
#define RADIO_CE_GPIO_Port GPIOA
#define RADIO_SCK_Pin GPIO_PIN_5
#define RADIO_SCK_GPIO_Port GPIOA
#define RADIO_MISO_Pin GPIO_PIN_6
#define RADIO_MISO_GPIO_Port GPIOA
#define RADIO_MOSI_Pin GPIO_PIN_7
#define RADIO_MOSI_GPIO_Port GPIOA
#define RELAY1_Pin GPIO_PIN_3
#define RELAY1_GPIO_Port GPIOB
#define RELAY2_Pin GPIO_PIN_4
#define RELAY2_GPIO_Port GPIOB
#define RELAY3_Pin GPIO_PIN_5
#define RELAY3_GPIO_Port GPIOB
#define RELAY4_Pin GPIO_PIN_9
#define RELAY4_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
