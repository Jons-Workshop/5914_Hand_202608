/* USER CODE BEGIN Header */
/**	Project	-	5914_Hand_202607	(Formerly 5914_Bt_Remote_202512)
 *
 * On GitHub	-	https://github.com/Jons-Workshop/5914_Bt_Remote_202512.git *** PLEASE UPDATE ***
 *
 * 	Author	-	Jon Freeman  B Eng Hons MIET
 * 	16th July 2026
 *
 *	//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen
 *
 *	No eeprom or 'settings' here. Any required will be supplied over bluetooth from Bluetooth_HC_Loco_Interface_202508 unit
 *
 * https://www.waveshare.com/3.5inch-Capacitive-Touch-LCD.htm
 * https://www.buydisplay.com/download/ic/FT6236-FT6336-FT6436L-FT6436_Datasheet.pdf
 *
 * 3.5inch Capacitive Touch Display Module, 320×480 Resolution,
 * Embedded with ST7796S Display Driver and FT6336U Capacitive Touch Controller, IPS Display
 *
 * Note 3V3 and Vcc both to +3V3.
 * SPI "Transmit Only Master", using DMA for very fast display updates
 * July 12th 2026 SPI now *Full Duplex Master* DMA Tx+Rx to include use of SD card mounted on display. Looking at FAT file system
 *
 * July 2026 - Proper PCB design done at last. Powered by 3v7 lithium 18650 cell
 * New output to EN of regulator so that unit can switch itself off
 *
 *	***	How do HC and LC connect ?	***
 *
 *	Arbitrary decisions,
 *	Bluetooth modules are 'paired', we have no control over their 'connecting' process. Connection only proved by working comms.
 *	HC to issue "p/r" speculatively maybe 20 times per second whenever not connected. This is our 'ping'.
 *	LC receiving "p/r" to echo "p/r".
 *	HC receiving "p/r" increments 'received_pings', which means must be 'Connected'
 *
 *	LC/HC to assume no comms from HC/LC for > ?? ms means 'Disconnected'
 *
 *
 *
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include	<stdbool.h>
#include	"st7796s.h"		//	Display Driver IC
#include	"colours.h"
#include	"fonts.h"
#include	<string.h>
#include	<math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//#define FT6X36_ID    0xCDU	//	Wrong
#define FT6X36_ID      0x70		//	Found by testing all possible addresses, this was the sole responder to pull 'ACK' low
#define	PROPER_PCB	*****	SEE main.h	*****	for #defines for all sorts

#ifdef	PROPER_PCB
#define	TS_INT_IN_PIN	GPIO_PIN_15
#else
#define	TS_INT_IN_PIN	GPIO_PIN_0
#endif

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
uint32_t	touch_time	= 0 ;
uint32_t	exti_cnt = 0;
uint32_t	spi_tx_cplt_callback_cnt	= 0	;
bool		spi_tx_cplt = true;
bool		exti_flag = false;
bool		Bluetooth_connected_pres = false;
bool		Bluetooth_connected_prev = false;
bool	quarter_sec;
bool	ms10;

float	V_Loco_Batt = 0.0;
float	V_HC_Batt = 0.0;
float	I_Loco_Batt = 0.0;
float	Loco_Speed = 0.0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

void 	st7796_init(void)	;	//	Display Driver IC
void	spi_tx	(uint8_t * pData, uint16_t Size)	;
bool	DrawBar	(size_t Xstart, size_t Ystart, size_t Xend, size_t Yend, uint16_t Colour)	;
bool	DrawString	(size_t const x, size_t const y, const char * const str, sFONT* Font, uint16_t const BG_Col, uint16_t const FG_Col)	;

extern	void	ForeverLoop	()	;


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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  MX_FATFS_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_PWM_Start	(&htim1, TIM_CHANNEL_1);	//	pwm for LCD backlight
//  HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET);	//	Latch power ON
  st7796_init	();	//	Display Driver IC
  TIM1->CCR1 = 980;	//	display bril 0 - 999
//  display_test	();

  ForeverLoop	()	;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
//	  HAL_Delay	(200);
//	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//	  HAL_Delay	(200);
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV256;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10B17DB5;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
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
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 300 - 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000 - 1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SD_CS_Pin|LCD_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_DC_Pin|LCD_RST_Pin|TP_RST_Pin|CAN_SHDN_Pin
                          |CAN_STB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PWR_HOLD_Pin */
  GPIO_InitStruct.Pin = PWR_HOLD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_HOLD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_CS_Pin LCD_CS_Pin LCD_DC_Pin LCD_RST_Pin
                           TP_RST_Pin CAN_SHDN_Pin CAN_STB_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin|LCD_CS_Pin|LCD_DC_Pin|LCD_RST_Pin
                          |TP_RST_Pin|CAN_SHDN_Pin|CAN_STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TP_INI_IN_Pin */
  GPIO_InitStruct.Pin = TP_INI_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TP_INI_IN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*		bool	set_spi_prsc	(uint16_t	val)	;	//	New June 2026 ready for SD card stuff
 * 	Use val = 2, 4, 8, 16, 32, 64, 128 or 256
 * 	Returns false on input error
 */
bool	set_spi_prsc	(uint16_t	val)	{
	bool	rv = true	;
	uint16_t	tmp = SPI1->CR1;
	tmp &= 0xffc7;		//	zero bits 5, 4, 3
	switch	(val)	{	//	Note: These bits should not be changed when communication is ongoing.
	case	2:	//	done for case fPCLK / 2	40MHz
		break;
	case	4:
		tmp |= 0x0008;	//	done for case fPCLK / 4		20 MHz
		break;
	case	8:
		tmp |= 0x0010;	//	done for case fPCLK / 8		10 MHz
		break;
	case	16:
		tmp |= 0x0018;	//	done for case fPCLK / 16		5 MHz
		break;
	case	32:
		tmp |= 0x0020;	//	done for case fPCLK / 32		2.5 MHz
		break;
	case	64:
		tmp |= 0x0028;	//	done for case fPCLK / 64		1.25 MHz
		break;
	case	128:
		tmp |= 0x0030;	//	done for case fPCLK / 128		62.5	KHz
		break;
	case	256:
		tmp |= 0x0038;	//	done for case fPCLK / 256		31.25	KHz
		break;
	default:		//	val represents none of the above
		rv = false;
		break;
	}	//	End of switch
	if	(rv)
		SPI1->CR1 = tmp;	//	set new SPI prescaler on validated input
	return	(rv);
}


void	st7796s_byte_out	(uint8_t	val)	{	//	Display Driver IC
	spi_tx	(&val, 1);
}

void	st7796s_cmd	(uint8_t cmd)	{	//	Display Driver IC
	LCD_CMD;	//	Command/Data select
	st7796s_byte_out	(cmd);
	LCD_DATA;	//	Command/Data select
}

void	st7796s_data	(uint8_t cmd)	{	//	Display Driver IC
//	LCD_DATA;
	st7796s_byte_out	(cmd);
}

bool	st7796s_column_address_set	(size_t startx, size_t endx)	{
	if	((startx > 319) || (endx > 319) || (endx <= startx))
			return	(false);
	st7796s_cmd		(0x2a);
	st7796s_data	((uint8_t)(startx >> 8));
	st7796s_data	((uint8_t)startx);
	st7796s_data	((uint8_t)(endx >> 8));
	st7796s_data	((uint8_t)endx);
	return	(true);
}

bool	st7796s_row_address_set	(size_t starty, size_t endy)	{
	if	((starty > 479) || (endy > 479) || (endy <= starty))
			return	(false);
	st7796s_cmd		(0x2b);
	st7796s_data	((uint8_t)(starty >> 8));
	st7796s_data	((uint8_t)starty);
	st7796s_data	((uint8_t)(endy >> 8));
	st7796s_data	((uint8_t)endy);
	return	(true);
}


void	LCD_Clear	(uint16_t colour)	{
	DrawBar	(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, colour);
}

bool	st7796s_set_window	(size_t startx, size_t starty, size_t endx, size_t endy)	{
	if	(!st7796s_column_address_set(startx, endx))
		return	(false);
	if	(!st7796s_row_address_set(starty, endy))
		return	(false);
	st7796s_cmd	(0x2c);
	return	(true);
}

void st7796_rst(void)
//void st7796_rst(void)Exit Deep Standby Mode by pull down CSX to low (“0”) 6 times.
{
	for	(int r = 0; r <= 7; r++)	{
		LCD_CS_INACTIVE;
		HAL_Delay	(1);
		LCD_CS_ACTIVE;
		HAL_Delay	(1);
	}
    LCD_RESET_ACTIVE;
    HAL_Delay	(1);	//	Was 120
    LCD_RESET_INACTIVE;
    HAL_Delay	(1);	//	Was 100
}

void st7796_init(void)
{
    st7796_rst();	//	Leaves CS Active
//	   ST77XX_SWRESET,		//	#define ST77XX_SWRESET 0x01
//	   ST_CMD_DELAY, // Software reset#define ST_CMD_DELAY 0x80 // special signifier for command lists
//	   150,
    st7796s_cmd	(ST77XX_SWRESET);	//	It will be necessary to wait 5msec before sending new command following software reset.
    HAL_Delay	(6);				//	5 with a margin

//	   0xF0,	//	CSCON Command Set Control
//	   1, // Unlock manufacturer
//	   0xC3,	//	C3h enable command 2 part 1 (p239)
    st7796s_cmd		(0xf0);
    st7796s_data	(0xc3);

    st7796s_cmd		(0xf0);		//	   0xF0,	//	CSCON Command Set Control
    st7796s_data	(0x96);		//	No idea what this really does

    st7796s_cmd		(0xc5);		//	   0xC5,	//	c5h VCOM Control
    st7796s_data	(0x1c);		//	VCOM = 1.000 No idea what this really does

    st7796s_cmd		(ST77XX_MADCTL);	//	This command defines read/ write scanning direction of frame memory. But what does that mean?
    st7796s_data	(0x48);

    st7796s_cmd		(ST77XX_COLMOD);	//	0x3A Interface Pixel Format
    st7796s_data	(0x55);			//	set 16 bit per pixel

    st7796s_cmd		(0xb0);	//	IFMODE Interface Mode Control	SPI_EN is bit 7 of parameter
    st7796s_data	(0x80);	//	SPI_EN = “1”, DIN/SDA pin is used for 3/4 wire serial interface and DOUT pin is not used.

    st7796s_cmd		(0xb4);	//	DIC Display Inversion Control	Can't see this does anything at all. Left in just in case it does
    st7796s_data	(0x00);	//	0x00

    st7796s_cmd		(0xb6);	//	   0xB6,	//	DFC Display Function Control
    st7796s_data	(0x80);	//	these 3 params 0x80, 0x02, 0x3b are defaults from data sheet
    st7796s_data	(0x02);
    st7796s_data	(0x3b);

    st7796s_cmd		(0xb7);	//	EM Entry Mode Set
    st7796s_data	(0xc6);		//	c6 or 06, makes no difference. Can't see this does anything.

    st7796s_cmd		(0xf0);		//	 CSCON (F0h): Command Set Control
    st7796s_data	(0x69);		//D[7:0] = 69h disable command 2 part II
//	   0xF0,
//	   0x3C,	//D[7:0] = 3Ch disable command 2 part
    st7796s_cmd		(0xf0);
    st7796s_data	(0x3c);

//	   ST77XX_SLPOUT,	//#define ST77XX_SLPOUT 0x11
//	   ST_CMD_DELAY, // Exit sleep
//	   150,
    st7796s_cmd	(ST77XX_SLPOUT);

//    st7796s_cmd	(0x20);	//	INVOFF jf
    st7796s_cmd	(0x21);	//	INVON jf	Need this to get the colours right !
    st7796s_cmd	(ST77XX_DISPON);	//	#define ST77XX_DISPON 0x29
}


//void	display_test	()	{
//    HAL_Delay	(5);	//	Was 150
//    st7796s_cmd	(ST77XX_DISPON);	//	#define ST77XX_DISPON 0x29
//    HAL_Delay	(5);	//	Was 150
/*	DrawBar	(0, 0, 319, 479, MAGENTA);
	HAL_Delay	(250);
	DrawBar	(10, 10, 200, 350, YELLOW);
	HAL_Delay	(250);
	DrawBar	(50, 50, 160, 310, GREEN);
	HAL_Delay	(250);
	DrawBar	(70, 70, 140, 410, RED);
	HAL_Delay	(250);
	DrawBar	(90, 90, 120, 440, BLUE);
	HAL_Delay	(250);
	DrawBar	(0, 190, LCD_WIDTH - 1, 240, WHITE);
	HAL_Delay	(250);*/
//	DrawString	(size_t const x, size_t const y, const char * const str, sFONT* Font, size_t const BG_Col, size_t const FG_Col)	;
//	DrawString	(25, 25, "Hello World", &Arial_Narrow_Bold19x23, ST77XX_BLACK, ST77XX_CYAN)	;
//}


bool	DrawPixel	(uint32_t X, uint32_t Y, uint16_t Colour)	{
	uint16_t	buff[4];
	if	(!st7796s_set_window(X, Y, X+1, Y+1))
		return	(false);
	buff[0] = (Colour >> 8) | (Colour << 8);
	spi_tx	((uint8_t*)buff, 2);
	return	(true);
}

/*bool	DrawPixel2	(uint32_t X, uint32_t Y, uint16_t Colour)	{
	uint16_t	buff[4];
	if	(!st7796s_set_window(X, Y, X+1, Y+1))
		return	(false);
	buff[0] = buff[1] = buff[2] = buff[3] = (Colour >> 8) | (Colour << 8);
	spi_tx	((uint8_t*)buff, 8);
	return	(true);
}

bool	DrawPixel3	(uint32_t X, uint32_t Y, uint16_t Colour)	{
	uint16_t	buff[12];
	if	(!st7796s_set_window(X-1, Y-1, X+1, Y+1))
		return	(false);
//	buff[0] = (Colour >> 8) | (Colour << 8);
	for	(int i = 0; i < 10; i++)
		buff[i] = (Colour >> 8) | (Colour << 8);
//	spi_tx	((uint8_t*)buff, 2);
	spi_tx	((uint8_t*)buff, 18);
	return	(true);
}*/


#define	GBUFF_SIZE	160

bool	DrawBar	(size_t Xstart, size_t Ystart, size_t Xend, size_t Yend, uint16_t Colour)	{
	if	(!st7796s_set_window(Xstart, Ystart, Xend, Yend))
		return	(false);
	size_t	pixels_to_write = (1 + Xend - Xstart) * (1 + Yend - Ystart);
	uint16_t	colour_swap = ((Colour >> 8) & 0xff) | ((Colour << 8) & 0xff00);
	uint16_t	buff[GBUFF_SIZE];
//	LCD_DATA;
	for	(int i = 0; i < GBUFF_SIZE; i++)
		buff[i] = colour_swap;
	while	(pixels_to_write > GBUFF_SIZE)	{
		pixels_to_write -= GBUFF_SIZE;
		spi_tx	((uint8_t*)buff, GBUFF_SIZE * 2);
	}
	if	(pixels_to_write > 0)
		spi_tx	((uint8_t*)buff, (uint16_t)(pixels_to_write * 2));
	while	(!spi_tx_cplt)	{	}	;
	return	(true);
}


bool	fill_circle	(size_t X, size_t Y, size_t radius, uint16_t colour)	{
#define	MAX_RADIUS	150
#define	MIN_RADIUS	5
/*	if	(
			((X + radius) > LCD_WIDTH)
		||	((Y + radius) > LCD_HEIGHT)
		||	((radius > X))
		||	((radius > Y))
		||	(radius > MAX_RADIUS)
		||	(radius < MIN_RADIUS)
		)
		return	(false);
		*/
	double	sinx;
	double	cosx;
	double	drad = (double)radius;
	size_t	len;
	DrawBar	(X - radius, Y, X + radius, Y + 1, colour);
	for	(size_t i = 1; i < radius; i++)	{
		sinx = ((double)i / drad);
		cosx = sqrt(1.0 - (sinx * sinx));
		len = (size_t)(drad * cosx);
		DrawBar	(X - len, Y + i, X + len, Y + 1 + i, colour);
		DrawBar	(X - len, Y - i, X + len, Y + 1 - i, colour);
	}
	return	(true);
}



/*
 * void	DrawString	(const char * str, sFONT* Font)	{
 * Most efficient way to write text to display
 */
bool	DrawString	(size_t const x, size_t const y, const char * const str, sFONT* Font, uint16_t const BG_Col, uint16_t const FG_Col)	{
	size_t	const	font_bytes_per_row	= (size_t)((Font->Width / 8) + (Font->Width % 8 ? 1 : 0));
	size_t	const	font_bytes_per_char = (Font->Height * font_bytes_per_row);
	size_t	const	chars_to_show = strlen(str);
	uint16_t	dma_buff[LCD_WIDTH];		//	max width of display in pixels
	const uint8_t * 	bitmapptrs[28] ;		//	max string length, stores start addresses of character bitmaps

	if	((x + (chars_to_show * Font->Width)) > LCD_WIDTH)	{	//	trap out requests to print beyond screen limits
//		pc.write	("Gone off edge in JDrawString\r\n", 30);
		return	(false);
	}
	for	(size_t char_num = 0; char_num < chars_to_show;	char_num++)				{	//	for chars in string
		if	((str[char_num] < ' ') || (str[char_num] > (' ' + 96)))		{
//			pc.write	("Bad chars in JDrawString\r\n", 27);
			return	(false);	//	Weeds out invalid chars
		}
		bitmapptrs[char_num]  = Font->table + ((str[char_num] - ' ') * font_bytes_per_char);		//	made ptr to start of font bitmap for all str chars
	}

	st7796s_set_window	(x, y, x - 1 + (chars_to_show * Font->Width), y + Font->Height);
    uint16_t	todmaposition;
    uint8_t		tmp;
    uint16_t	const ColFGsw	= (FG_Col >> 8) | (FG_Col << 8);
    uint16_t	const ColBGsw	= (BG_Col >> 8) | (BG_Col << 8);	//	byte swapped

	for	(size_t height = 0;	height < font_bytes_per_char; height += font_bytes_per_row)	{//	for height of font
    	todmaposition = 0;
		for	(size_t char_num = 0; char_num < chars_to_show;	char_num++)				{	//	for chars in string
			for	(size_t line_bit = 0;	line_bit < Font->Width;	line_bit++)	{			//	for each pixel across width of character
				if	((line_bit & 0x07) == 0)	//	read new byte 1 in 8 times around
					tmp = *(bitmapptrs[char_num] + height + (line_bit >> 3));
				dma_buff[todmaposition++] = tmp & (0x80 >> (line_bit % 8)) ? ColFGsw : ColBGsw;	//	16  bit colour for one bit of char bitmap
			}		//	written one line of one char
		}			//	written one line for all chars in string
    	spi_tx	((uint8_t*)dma_buff, todmaposition * 2);
	}				//	endof for height of font
	while	(!spi_tx_cplt)	{	}	;
    return	(true);
}					//	endof DrawString


bool	touch_rx	(uint8_t * rxbuff, uint16_t len)	{
	uint8_t	values[2] = {0,0};
	HAL_StatusTypeDef	ret;	//	Used to test return results of HAL functions
    ret = HAL_I2C_Master_Transmit	(&hi2c1, FT6X36_ID, values, 1, 100);	//	Set Data Address to 0
    if	(ret != HAL_OK)	{
    	return	(false);	//	write 2 byte address followed by n data
    }
    HAL_Delay	(1);
	ret = HAL_I2C_Master_Receive	(&hi2c1, FT6X36_ID, rxbuff, len, 100);
    if	(ret != HAL_OK)	{
    	return	(false);	//	write 2 byte address followed by n data
    }
    return	(true);
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi)
{    // TX Done .. Do Something ...
	spi_tx_cplt = true;
	spi_tx_cplt_callback_cnt++;
}


void	spi_tx	(uint8_t * pData, uint16_t Size)	{
	while	(!spi_tx_cplt)	{
	}	;
	spi_tx_cplt = false;
	HAL_SPI_Transmit_DMA	(&hspi1, pData, Size);
}


//bool	spi_tx_free	()	{
//	return	(spi_tx_cplt);
//}


uint32_t	spi_callback_cnt	()	{
	return	(spi_tx_cplt_callback_cnt);
}

void	HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{	//	check for timers 2 and 6
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)	//	interrupt from touch screen. Rising edge trigger
{								//	GPIO_PIN_0 for Nucleo lash prototype, GPIO_PIN_15 on proper pcb. See #define PROPER_PCB above
  if(GPIO_Pin == TS_INT_IN_PIN) {	//	Interrupt is from Touch Screen
    exti_cnt++;
    exti_flag = true;
    touch_time = uwTick;		//	Record time of most recent touch
//  } else {	got interrupt from unknown source
  }
}


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
	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  HAL_Delay	(50);
	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  HAL_Delay	(50);
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
