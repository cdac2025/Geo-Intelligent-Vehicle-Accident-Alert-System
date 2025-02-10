/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body with UART4 for GPS reception (baud rate 9600)
  *                   and UART5 for ESP32 transmission.
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ACCEL_THRESHOLD 100  // Define your threshold value here
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart4;  // For receiving GPS data from Arduino Uno
UART_HandleTypeDef huart5;  // For transmitting data to ESP32

/* USER CODE BEGIN PV */
uint8_t txbuffer[2];
uint8_t Rx_X;
uint8_t Rx_Y;
uint8_t Rx_Z;
char charbuffer[100];  // Increased buffer size to avoid overflow warnings
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART4_Init();  // For receiving GPS data from Arduino Uno (baud rate 9600)
  MX_UART5_Init();  // For transmitting data to ESP32

  /* USER CODE BEGIN 2 */
  /* Initial configuration of the sensor (SPI setup) */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET); // CS Pulled Low
  txbuffer[0] = 0x20; // Control Register 4
  txbuffer[1] = 0x37; // Configuration data
  HAL_SPI_Transmit(&hspi1, txbuffer, 2, 50);
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET); // CS Pulled High
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* --- Read Accelerometer Data via SPI --- */

      // Read the OUT_X register
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_RESET); // CS Pulled Low
      txbuffer[0] = 0x29 | 0x80; // Read operation (MSB = 1)
      HAL_SPI_Transmit(&hspi1, txbuffer, 1, 50);
      HAL_SPI_Receive(&hspi1, &Rx_X, 1, 50);
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_SET); // CS Pulled High

      // Read the OUT_Y register
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_RESET); // CS Pulled Low
      txbuffer[0] = 0x2B | 0x80; // Read operation (MSB = 1)
      HAL_SPI_Transmit(&hspi1, txbuffer, 1, 50);
      HAL_SPI_Receive(&hspi1, &Rx_Y, 1, 50);
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_SET); // CS Pulled High

      // Read the OUT_Z register
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_RESET); // CS Pulled Low
      txbuffer[0] = 0x2D | 0x80; // Read operation (MSB = 1)
      HAL_SPI_Transmit(&hspi1, txbuffer, 1, 50);
      HAL_SPI_Receive(&hspi1, &Rx_Z, 1, 50);
      HAL_GPIO_WritePin(GPIOE, SPI1_CS_Pin, GPIO_PIN_SET); // CS Pulled High

      /* --- Check Accelerometer Threshold --- */
      if (Rx_X > ACCEL_THRESHOLD || Rx_Y > ACCEL_THRESHOLD || Rx_Z > ACCEL_THRESHOLD)
      {
          // Accident detected!
          // Receive GPS location data from Arduino Uno via UART4.
          char gpsBuffer[100] = {0};
          // Using a blocking receive (timeout: 1000ms). Adjust as necessary.
          HAL_UART_Receive(&huart4, (uint8_t *)gpsBuffer, sizeof(gpsBuffer) - 1, 1000);

          // Prepare the accident message including the GPS location.
          sprintf(charbuffer, "Accident happened at %s\r\n", gpsBuffer);
          printf("Accident Happened %s\n", gpsBuffer);
          // Transmit the message to the ESP32 using UART5.
          HAL_UART_Transmit(&huart5, (uint8_t *)charbuffer, strlen(charbuffer), 100);
      }
      /*
      else
      {
          // No accident: send the current accelerometer data to ESP32.
          sprintf(charbuffer, "You are alive!\n");
          HAL_UART_Transmit(&huart5, (uint8_t *)charbuffer, strlen(charbuffer), 50);
      }
      */
      bzero(charbuffer, sizeof(charbuffer));

      /* --- LED Motion : Quadrant --- */
      if (Rx_X == 0 && Rx_Y == 0 && Rx_Z == 64)
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_SET);
          HAL_Delay(100);
      }
      if ((Rx_X > 0 && Rx_X <= 188))
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_SET);
          HAL_Delay(100);
      }
      if ((Rx_X > 188 && Rx_X <= 255))
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_RESET);
          HAL_Delay(100);
      }
      if ((Rx_Y > 0 && Rx_Y <= 188))
      {
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_RESET);
          HAL_Delay(100);
      }
      if ((Rx_Y > 188 && Rx_Y <= 255))
      {
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_SET);
          HAL_Delay(100);
      }

      /* --- LED Motion : Axis --- */
      if (Rx_X == 0 && Rx_Y == 0 && Rx_Z == 64)
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_SET);
      }
      if (Rx_X > 10)
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_RESET);
      }
      if (Rx_Y > 10)
      {
          HAL_GPIO_WritePin(GPIOD, LED2_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_SET);
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

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  /* SPI1 parameter configuration */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;           // Updated baud rate for UART4
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level for SPI1_CS */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level for LEDs */
  HAL_GPIO_WritePin(GPIOD, LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED2_Pin LED3_Pin LED4_Pin LED5_Pin */
  GPIO_InitStruct.Pin = LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
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
  /* User can add implementation to report the file name and line number */
}
#endif
