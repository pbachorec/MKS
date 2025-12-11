/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include <rfm95.h>
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
RNG_HandleTypeDef hrng;

SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RNG_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint32_t get_precision_tick()
{
	__disable_irq();
	uint32_t precision_tick = __HAL_TIM_GET_COUNTER(&htim2);
	__enable_irq();
	return precision_tick;
}

static void precision_sleep_until(uint32_t target_ticks)
{
	while (get_precision_tick() < target_ticks) { /* do nothing */ }
}

static uint8_t random_int(uint8_t max)
{
	uint32_t rng;
	HAL_RNG_GenerateRandomNumber(&hrng, &rng);
	return (rng % (max + 1));
}
rfm95_handle_t rfm95_handle = {
		.spi_handle = &hspi3,
		.nss_port = RFM95_NSS_GPIO_Port,
		.nss_pin = RFM95_NSS_Pin,
		.device_address = {
				0x26, 0x0B, 0x04, 0x75
		},
		.application_session_key = {
				0x40, 0x0A, 0xE6, 0x65, 0x1A, 0xA8, 0x92, 0x92,	0x12, 0xC0, 0x08, 0xEC, 0xC2, 0xC1, 0x53, 0x72
		},
		.network_session_key = {
				0x80, 0x70, 0x52, 0xDA, 0x22, 0x5F, 0x54, 0xC3, 0xB8, 0xDC, 0xAD, 0xA1, 0x8A, 0xF2, 0x85, 0x34
		},
		.receive_mode = RFM95_RECEIVE_MODE_NONE,
		.precision_tick_frequency = 20000,


		.get_precision_tick = get_precision_tick,
		.precision_sleep_until = precision_sleep_until,
		.random_int = random_int
};
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == RFM95_DIO0_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO0);
	} else if (GPIO_Pin == RFM95_DIO1_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO1);
	} else if (GPIO_Pin == RFM95_DIO5_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO5);
	}
}
void send(uint8_t *buffer)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	rfm95_send_receive_cycle(&rfm95_handle, (uint8_t*)buffer, strlen(buffer));
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);

}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */



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
	MX_USART3_UART_Init();
	MX_RNG_Init();
	MX_SPI3_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim2);
	if(rfm95_init(&rfm95_handle))
	{
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);

	}else{
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);
	while (1); /*loop*/
	}


		static uint16_t lastTimer = 0;
		uint16_t counter = 0;
		static uint16_t buttonTimer=0;
		char msg[30];
		char buffer[30];
		/* USER CODE END 2 */

		/* Infinite loop */
		/* USER CODE BEGIN WHILE */
		while (1)
		{
			/* USER CODE END WHILE */

			/* USER CODE BEGIN 3 */

			if( HAL_GPIO_ReadPin(USER_Btn_GPIO_Port,USER_Btn_Pin)&&(HAL_GetTick() - buttonTimer) > 1000)
			{
				sprintf(buffer,"246781; press");
				send(buffer);
				buttonTimer=HAL_GetTick();
			}

			if ((HAL_GetTick() - lastTimer) > 30000)
			{
				counter++;

				sprintf(msg,"246781;%i",counter);
				memcpy(buffer, msg, sizeof(msg));

				send(buffer);
				lastTimer = HAL_GetTick();
			}


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

		/** Configure the main internal regulator output voltage
		 */
		__HAL_RCC_PWR_CLK_ENABLE();
		__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

		/** Initializes the RCC Oscillators according to the specified parameters
		 * in the RCC_OscInitTypeDef structure.
		 */
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		RCC_OscInitStruct.PLL.PLLM = 4;
		RCC_OscInitStruct.PLL.PLLN = 168;
		RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
		RCC_OscInitStruct.PLL.PLLQ = 7;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

		if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
		{
			Error_Handler();
		}
	}

	/**
	 * @brief RNG Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_RNG_Init(void)
	{

		/* USER CODE BEGIN RNG_Init 0 */

		/* USER CODE END RNG_Init 0 */

		/* USER CODE BEGIN RNG_Init 1 */

		/* USER CODE END RNG_Init 1 */
		hrng.Instance = RNG;
		if (HAL_RNG_Init(&hrng) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN RNG_Init 2 */

		/* USER CODE END RNG_Init 2 */

	}

	/**
	 * @brief SPI3 Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_SPI3_Init(void)
	{

		/* USER CODE BEGIN SPI3_Init 0 */

		/* USER CODE END SPI3_Init 0 */

		/* USER CODE BEGIN SPI3_Init 1 */

		/* USER CODE END SPI3_Init 1 */
		/* SPI3 parameter configuration*/
		hspi3.Instance = SPI3;
		hspi3.Init.Mode = SPI_MODE_MASTER;
		hspi3.Init.Direction = SPI_DIRECTION_2LINES;
		hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
		hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
		hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
		hspi3.Init.NSS = SPI_NSS_SOFT;
		hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
		hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
		hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
		hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		hspi3.Init.CRCPolynomial = 10;
		if (HAL_SPI_Init(&hspi3) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN SPI3_Init 2 */

		/* USER CODE END SPI3_Init 2 */

	}

	/**
	 * @brief TIM2 Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_TIM2_Init(void)
	{

		/* USER CODE BEGIN TIM2_Init 0 */

		/* USER CODE END TIM2_Init 0 */

		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		TIM_MasterConfigTypeDef sMasterConfig = {0};

		/* USER CODE BEGIN TIM2_Init 1 */

		/* USER CODE END TIM2_Init 1 */
		htim2.Instance = TIM2;
		htim2.Init.Prescaler = 4199;
		htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim2.Init.Period = 4294967295;
		htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
		{
			Error_Handler();
		}
		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
		{
			Error_Handler();
		}
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN TIM2_Init 2 */

		/* USER CODE END TIM2_Init 2 */

	}

	/**
	 * @brief USART3 Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_USART3_UART_Init(void)
	{

		/* USER CODE BEGIN USART3_Init 0 */

		/* USER CODE END USART3_Init 0 */

		/* USER CODE BEGIN USART3_Init 1 */

		/* USER CODE END USART3_Init 1 */
		huart3.Instance = USART3;
		huart3.Init.BaudRate = 115200;
		huart3.Init.WordLength = UART_WORDLENGTH_8B;
		huart3.Init.StopBits = UART_STOPBITS_1;
		huart3.Init.Parity = UART_PARITY_NONE;
		huart3.Init.Mode = UART_MODE_TX_RX;
		huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart3.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart3) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN USART3_Init 2 */

		/* USER CODE END USART3_Init 2 */

	}

	/**
	 * @brief GPIO Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_GPIO_Init(void)
	{
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		/* USER CODE BEGIN MX_GPIO_Init_1 */

		/* USER CODE END MX_GPIO_Init_1 */

		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOG, RFM95_NRST_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(RFM95_NSS_GPIO_Port, RFM95_NSS_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin : USER_Btn_Pin */
		GPIO_InitStruct.Pin = USER_Btn_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
		GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
		GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
		GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/*Configure GPIO pins : RFM95_DIO0_Pin RFM95_DIO5_Pin */
		GPIO_InitStruct.Pin = RFM95_DIO0_Pin|RFM95_DIO5_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/*Configure GPIO pin : RMII_TXD1_Pin */
		GPIO_InitStruct.Pin = RMII_TXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pins : RFM95_NRST_Pin USB_PowerSwitchOn_Pin */
		GPIO_InitStruct.Pin = RFM95_NRST_Pin|USB_PowerSwitchOn_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		/*Configure GPIO pin : RFM95_DIO1_Pin */
		GPIO_InitStruct.Pin = RFM95_DIO1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(RFM95_DIO1_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pin : USB_OverCurrent_Pin */
		GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
		GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/*Configure GPIO pin : USB_VBUS_Pin */
		GPIO_InitStruct.Pin = USB_VBUS_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pin : RFM95_NSS_Pin */
		GPIO_InitStruct.Pin = RFM95_NSS_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(RFM95_NSS_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
		GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		/* EXTI interrupt init*/
		HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);

		HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

		/* USER CODE BEGIN MX_GPIO_Init_2 */

		/* USER CODE END MX_GPIO_Init_2 */
	}

	/* USER CODE BEGIN 4 */

	/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
