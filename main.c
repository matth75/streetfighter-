/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dac.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "lwip.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdio.h"

#include "HorombeRGB565.h"
#include "Scorvol.h"
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

/* USER CODE BEGIN PV */
uint32_t joystick_v;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef enum {
	bleu,
	rouge,
	noir,
	jaune
}ColorState;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	char text[60]={};
	static TS_StateTypeDef  TS_State;
	uint32_t potl,potr,joystick_h;
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	uint32_t pos_x = 200;
	uint32_t pos_y = 130;
	uint32_t pos_x_old = 200;
	uint32_t pos_y_old = 130;

	uint8_t carac;
	char  aie[] = "ouch\n\r";
	char ouch[] = "aiee\n\r";
	uint8_t bobo = 0;

	GPIO_PinState bp1=GPIO_PIN_RESET;
	GPIO_PinState bp2=GPIO_PIN_RESET;


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_UART7_Init();
  /* USER CODE BEGIN 2 */
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+ BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4);
  BSP_LCD_DisplayOn();
  BSP_LCD_SelectLayer(0);
//  BSP_LCD_Clear(LCD_COLOR_RED);
//  BSP_LCD_DrawBitmap(0,0,(uint8_t*)HorombeRGB565_bmp);
//  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);


  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  // carré qui se déplace

  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  	// actualise l'affichage
	  	BSP_LCD_SelectLayer(1);
	  	if ((pos_x_old!=pos_x)||(pos_y_old!=pos_y)){
	  		BSP_LCD_Clear(00);
	  		pos_x_old = pos_x;
	  		pos_y_old = pos_y;
	  	}

		BSP_LCD_FillRect((uint16_t) pos_x, (uint16_t) pos_y, 8, 8);

		// machine a etatsn
		bp1 = HAL_GPIO_ReadPin(BP1_GPIO_Port,BP1_Pin);
		bp2 = HAL_GPIO_ReadPin(BP2_GPIO_Port,BP2_Pin);

		if ((bp1 == GPIO_PIN_SET)&&(bp2 == GPIO_PIN_SET)){
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
		}
		if((bp1 == GPIO_PIN_SET)&&(bp2 == GPIO_PIN_RESET)){
			BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		}
		if((bp1 == GPIO_PIN_RESET)&&(bp2 == GPIO_PIN_SET)){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		}
		if((bp1 == GPIO_PIN_RESET)&&(bp2 == GPIO_PIN_RESET)){
			BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
		}


		BSP_LCD_SelectLayer(0);

	  	HAL_GPIO_WritePin(LED13_GPIO_Port,LED13_Pin,HAL_GPIO_ReadPin(BP1_GPIO_Port,BP1_Pin));
		HAL_GPIO_WritePin(LED14_GPIO_Port,LED14_Pin,HAL_GPIO_ReadPin(BP2_GPIO_Port,BP2_Pin));
		sprintf(text,"BP1 : %d",HAL_GPIO_ReadPin(BP1_GPIO_Port,BP1_Pin));
		BSP_LCD_DisplayStringAtLine(5,(uint8_t*) text);



		sConfig.Channel = ADC_CHANNEL_6;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		potr = HAL_ADC_GetValue(&hadc3);
		// display potr value
		sprintf(text,"potr value : %ld",potr);
		BSP_LCD_DisplayStringAtLine(6,(uint8_t*) text);
		// change LED blinking speed using potr
		htim3.Instance->ARR = 10000 - potr;

		sConfig.Channel = ADC_CHANNEL_7;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		potl = HAL_ADC_GetValue(&hadc3);

		htim3.Instance->PSC = 5000 - potl;

		sConfig.Channel = ADC_CHANNEL_8;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		joystick_v = HAL_ADC_GetValue(&hadc3);

		HAL_ADC_Start(&hadc1);
		while(HAL_ADC_PollForConversion(&hadc1, 100)!=HAL_OK);
		joystick_h = HAL_ADC_GetValue(&hadc1);


		sprintf(text,"POTL : %4u POTR : %4u joy_v : %4u joy_h : %4u \n\r",(uint16_t)potl,(uint16_t)potr,(uint16_t)joystick_v,(uint16_t)joystick_h);
		BSP_LCD_DisplayStringAtLine(9,(uint8_t*) text);

		// UART Display of joysticks
//		HAL_UART_Transmit(&huart1,(uint8_t*) text, 60, 100);
//		HAL_UART_Receive(&huart1,receive, 60, 10000);
//		BSP_LCD_DisplayStringAtLine(10,(uint8_t*) receive);


		HAL_UART_Receive(&huart1,&carac, 1, 1);
		if (carac == 'a'){
			HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,1);
		}
		if (carac == 'e'){
			HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,0);
		}



		if ((joystick_v > 3500)&&(pos_y>2)){
			pos_y -= 2;
		}
		if ((joystick_v < 1500)&&(pos_y<270)){
			pos_y += 2;
		}
		if ((joystick_h > 3500)&&(pos_x>2)){
			pos_x -= 2;
		}
		if ((joystick_h < 1500)&&(pos_x<478)){
			pos_x += 2;
		}



		// horloge temps réel
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		sprintf(text,"Date : %4u h: %4u mn : %4u sec", (uint8_t) sTime.Hours,(uint8_t) sTime.Minutes, (uint8_t) sTime.Seconds);
		BSP_LCD_DisplayStringAtLine(10,(uint8_t*) text);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

		BSP_TS_GetState(&TS_State);
		if(TS_State.touchDetected){
			//TODO : Il faudrait vérifier que le cercle reste entièrement sur l'écran...
		  BSP_LCD_FillCircle(TS_State.touchX[0],TS_State.touchY[0],4);
		  switch (bobo){
		  case 1:
			  HAL_UART_Transmit(&huart1,(uint8_t*) aie, 8, 1);
			  bobo = 0;
		  case 0:
			  HAL_UART_Transmit(&huart1,(uint8_t*) ouch, 8, 1);
			  bobo = 1;
		  }


	  	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	if (htim->Instance == TIM3){
		// toggle LED
		HAL_GPIO_TogglePin(LED11_GPIO_Port, LED11_Pin);
	}

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
