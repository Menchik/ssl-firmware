/*
 * pid.h
 *
 *  Created on: Oct 1, 2024
 *      Author: allan
 */

#ifndef INC_PID_H_
#define INC_PID_H_

 /*
------------------------------------------------------------------------------
~ File   : pid.h
~ Author : Majid Derhambakhsh
~ Version: V1.0.0
~ Created: 02/11/2021 03:43:00 AM
~ Brief  :
~ Support:
		   E-Mail : Majid.do16@gmail.com (subject : Embedded Library Support)

		   Github : https://github.com/Majid-Derhambakhsh
------------------------------------------------------------------------------
~ Description:

~ Attention  :

~ Changes    :
------------------------------------------------------------------------------
*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Include ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */

#ifdef __CODEVISIONAVR__  /* Check compiler */

#pragma warn_unref_func- /* Disable 'unused function' warning */

/* ------------------------------------------------------------------ */

#elif defined(__GNUC__) && !defined(USE_HAL_DRIVER)  /* Check compiler */

#pragma GCC diagnostic ignored "-Wunused-function" /* Disable 'unused function' warning */

/* ------------------------------------------------------------------ */

#elif defined(USE_HAL_DRIVER)  /* Check driver */

	#include "main.h"

	/* --------------- Check Mainstream series --------------- */

	#ifdef STM32F0
		#include "stm32f0xx_hal.h"       /* Import HAL library */
	#elif defined(STM32F1)
		#include "stm32f1xx_hal.h"       /* Import HAL library */
	#elif defined(STM32F2)
		#include "stm32f2xx_hal.h"       /* Import HAL library */
	#elif defined(STM32F3)
		#include "stm32f3xx_hal.h"       /* Import HAL library */
	#elif defined(STM32F4)
		#include "stm32f4xx_hal.h"       /* Import HAL library */
	#elif defined(STM32F7)
		#include "stm32f7xx_hal.h"       /* Import HAL library */
	#elif defined(STM32G0)
		#include "stm32g0xx_hal.h"       /* Import HAL library */
	#elif defined(STM32G4)
		#include "stm32g4xx_hal.h"       /* Import HAL library */

	/* ------------ Check High Performance series ------------ */

	#elif defined(STM32H7)
		#include "stm32h7xx_hal.h"       /* Import HAL library */

	/* ------------ Check Ultra low power series ------------- */

	#elif defined(STM32L0)
		#include "stm32l0xx_hal.h"       /* Import HAL library */
	#elif defined(STM32L1)
		#include "stm32l1xx_hal.h"       /* Import HAL library */
	#elif defined(STM32L5)
		#include "stm32l5xx_hal.h"       /* Import HAL library */
	#elif defined(STM32L4)
		#include "stm32l4xx_hal.h"       /* Import HAL library */
	#elif defined(STM32H7)
		#include "stm32h7xx_hal.h"       /* Import HAL library */
	#else
	#endif /* STM32F1 */

	/* ------------------------------------------------------- */

	#if defined ( __ICCARM__ ) /* ICCARM Compiler */

	#pragma diag_suppress=Pe177   /* Disable 'unused function' warning */

	#elif defined   (  __GNUC__  ) /* GNU Compiler */


	#endif /* __ICCARM__ */

/* ------------------------------------------------------------------ */

#else                     /* Compiler not found */

#error Chip or Library not supported  /* Send error */

#endif /* __CODEVISIONAVR__ */

/* ------------------------------------------------------------------ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ------------------------ Library ------------------------ */
#define _PID_LIBRARY_VERSION    1.0.0

/* ------------------------ Public ------------------------- */
#define _PID_8BIT_PWM_MAX       UINT8_MAX
#define _PID_SAMPLE_TIME_MS_DEF 100

#ifndef _FALSE

	#define _FALSE 0

#endif

#ifndef _TRUE

	#define _TRUE 1

#endif

/* ---------------------- By compiler ---------------------- */
#ifndef GetTime

	/* ---------------------- By compiler ---------------------- */

	#ifdef __CODEVISIONAVR__  /* Check compiler */

		#define GetTime()   0

	/* ------------------------------------------------------------------ */

	#elif defined(__GNUC__) && !defined(USE_HAL_DRIVER)  /* Check compiler */

		#define GetTime()   0

	/* ------------------------------------------------------------------ */

	#elif defined(USE_HAL_DRIVER)  /* Check driver */

		#define GetTime()   HAL_GetTick()

	/* ------------------------------------------------------------------ */

	#else
	#endif /* __CODEVISIONAVR__ */
	/* ------------------------------------------------------------------ */

#endif

/* --------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* PID Mode */
typedef enum
{

	_PID_MODE_MANUAL    = 0,
	_PID_MODE_AUTOMATIC = 1

}PIDMode_TypeDef;

/* PID P On x */
typedef enum
{

	_PID_P_ON_M = 0, /* Proportional on Measurement */
	_PID_P_ON_E = 1

}PIDPON_TypeDef;

/* PID Control direction */
typedef enum
{

	_PID_CD_DIRECT  = 0,
	_PID_CD_REVERSE = 1

}PIDCD_TypeDef;

/* PID Structure */
typedef struct
{

	PIDPON_TypeDef  POnE;
	PIDMode_TypeDef InAuto;

	PIDPON_TypeDef  POn;
	PIDCD_TypeDef   ControllerDirection;

	uint32_t        LastTime;
	uint32_t        SampleTime;

	float          DispKp;
	float          DispKi;
	float          DispKd;

	float          Kp;
	float          Ki;
	float          Kd;

	float          *MyInput;
	float          *MyOutput;
	float          *MySetpoint;

	float          OutputSum;
	float          LastInput;

	float          OutMin;
	float          OutMax;

}PID_TypeDef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::::::: Init ::::::::::::: */
void PID_Init(PID_TypeDef *uPID);

void PID(PID_TypeDef *uPID, float *Input, float *Output, float *Setpoint, float Kp, float Ki, float Kd, PIDPON_TypeDef POn, PIDCD_TypeDef ControllerDirection);
void PID2(PID_TypeDef *uPID, float *Input, float *Output, float *Setpoint, float Kp, float Ki, float Kd, PIDCD_TypeDef ControllerDirection);

/* ::::::::::: Computing ::::::::::: */
uint8_t PID_Compute(PID_TypeDef *uPID);

/* ::::::::::: PID Mode :::::::::::: */
void            PID_SetMode(PID_TypeDef *uPID, PIDMode_TypeDef Mode);
PIDMode_TypeDef PID_GetMode(PID_TypeDef *uPID);

/* :::::::::: PID Limits ::::::::::: */
void PID_SetOutputLimits(PID_TypeDef *uPID, float Min, float Max);

/* :::::::::: PID Tunings :::::::::: */
void PID_SetTunings(PID_TypeDef *uPID, float Kp, float Ki, float Kd);
void PID_SetTunings2(PID_TypeDef *uPID, float Kp, float Ki, float Kd, PIDPON_TypeDef POn);

/* ::::::::: PID Direction ::::::::: */
void          PID_SetControllerDirection(PID_TypeDef *uPID, PIDCD_TypeDef Direction);
PIDCD_TypeDef PID_GetDirection(PID_TypeDef *uPID);

/* ::::::::: PID Sampling :::::::::: */
void PID_SetSampleTime(PID_TypeDef *uPID, int32_t NewSampleTime);

/* ::::::: Get Tunings Param ::::::: */
float PID_GetKp(PID_TypeDef *uPID);
float PID_GetKi(PID_TypeDef *uPID);
float PID_GetKd(PID_TypeDef *uPID);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#endif /* INC_PID_H_ */
