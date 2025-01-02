/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "main_regulator.h"  // regulator
#include "bme280_add.h"  // czujnik temperatury
#include "bme280_defs.h"
#include "GFX.h"  // wyswietlacz
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	PWM_OFF = 0,  // PWM jest wylaczony (przy wysokiej temperaturze)
	PWM_ON = 1    // PWM jest wlaczony
} PwmState ;

typedef enum {
	REGULATOR_OFF = 0,  // regulator jest wylaczony (przy starcie)
	REGULATOR_ON = 1    // regulator jest wlaczony (klikniecie USER_Btn)
} RegulatorState ;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MSGLEN 6
#define MAXTEMPERATURE 60.0  // czujnik (-40, 80)
#define REGULATOR_DT 0.1 // w sekundach 72MHz/(7200*1000) -> 10Hz
#define outputMax 10.0
#define outputMin 0.0
#define SetValMax 80
#define SetValMin 0

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
PID_TypeDef pid;           // Struktura regulatora PID
uint8_t setpoint_tem = 30; // setpoint (-128 to 127)
uint8_t prep_setpoint_tem = 30;
float temperature = 99.9;  // wartosc mierzona
double regulator_output = 0.0;  // Wyjscie regulatora PID
double u_sat = 0;  // wyjscie po saturacji
uint16_t pwm_output = 0.0;    // Wyjscie regulatora PID pwm

PwmState pwm_state = PWM_ON;  // poczatkowo PWM wlaczone
RegulatorState regulator_state = REGULATOR_OFF;  // poczatkowo regulator wylaczony
int8_t oledScreen = 0;  // 0-temperatura, 1-Set Value, 2-PWM signal [%]
uint8_t rx_data[MSGLEN];
uint32_t enkoder_count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Oled_screen_update(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Oled_screen_update(void){
	char temp_str[20];
	switch (oledScreen) {
		case 0:  // temperatura
	  	sprintf(temp_str, "%.1f\321C", temperature);
			break;
		case 1:  // Set Value
			if(setpoint_tem != prep_setpoint_tem){
	  		if(setpoint_tem<10){
		  		sprintf(temp_str, "SV %d\321C", (int8_t)prep_setpoint_tem);
	  		}
	  		else if(setpoint_tem<=100){
		  		sprintf(temp_str, "SV%d\321C", (int8_t)prep_setpoint_tem);
	  		}
			}
			else if(setpoint_tem<10){
	  		sprintf(temp_str, "SV %d\321C", (int8_t)setpoint_tem);
			}
			else if(setpoint_tem<=100){
	  		sprintf(temp_str, "SV%d\321C", (int8_t)setpoint_tem);
			}
			break;
		case 2:  // PWM signal [%]
			if(pwm_output/10<10){
				sprintf(temp_str, "U\317\317\317%d\320", pwm_output/10);
			}
			else if(pwm_output/10>=100){
				sprintf(temp_str, "U\317%d\320", pwm_output/10);
			}
			else{
				sprintf(temp_str, "U\317\317%d\320", pwm_output/10);
			}
			break;
		default:  // temperatura
	  	sprintf(temp_str, "%hhn", rx_data);
			break;
	}
	if(pwm_state == PWM_OFF){
		strcpy(temp_str, "ERRORR");  // przegrzanie
	}

	if(setpoint_tem != prep_setpoint_tem && oledScreen == 1){
		GFX_draw_fill_rect(0, 0, 128, 32, WHITE);
		GFX_draw_string(5, 0, (unsigned char *)temp_str, BLACK, WHITE, 3, 3);
		SSD1306_display_repaint();
	}
	else{
		GFX_draw_fill_rect(0, 0, 128, 32, BLACK);
		GFX_draw_string(5, 0, (unsigned char *)temp_str, WHITE, BLACK, 3, 3);
		SSD1306_display_repaint();
	}
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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C2_Init();
  MX_TIM3_Init();
  MX_TIM7_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  // Inicjalizacja PID
  //PI_Init(&pid, 1.2, 0.004, 0.0, outputMin, outputMax, REGULATOR_DT);  // PI Kp, Ki, Kd, Min, Max, dt
  PID_Init(&pid, 1.4, 0.02, 8.0, outputMin, outputMax, REGULATOR_DT);  // PID Kp, Ki, Kd, Min, Max, dt

  // Timers Interrupt
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);
  HAL_TIM_Base_Start_IT(&htim7);

  // UART Interrupt
  HAL_UART_Receive_IT(&huart3, rx_data, 6);

  // Inicjalizacja czujnika BME280
  if(BME280_init() != BME280_OK) {
	  printf("Error: Temperature sensor initialization failed!\n");
  }

  // Inicjalizacja wyswietlacza OLED
  if (SSD1306_init() != TRUE) {
    printf("Error: OLED initialization failed!\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	// Real-time regulator
	if(htim->Instance == TIM7){
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);  // LED3

    // 1. Aktualizacja temperatury
		temperature = BME280_read_temperature();  // measurements with constant Ts

    // 2. Obliczanie wyjscia regulatora
		if(regulator_state){
			//regulator_output = PI_Compute(&pid, setpoint_tem, temperature);  // anty wind-up
			regulator_output = PID_Compute(&pid, setpoint_tem, temperature);  // anty wind-up
			u_sat = saturation(regulator_output, outputMin, outputMax);
			pwm_output = (uint16_t)(u_sat*100); // 0...1000

      // 3. Wyłączenie PWM w przypadku przegrzania
			if(temperature >= MAXTEMPERATURE){  // restartowanie tylko poprzez USER_restart B2
				pwm_state = PWM_OFF;  // ustawia sygnal sterujacy na 0
			}

      // 4. Aktualizacja PWM
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_output * pwm_state);
		}

    // 5. Aktualizacja ekranu
		Oled_screen_update();

		printf("%.2f, %.2f, %u\r\n", temperature, u_sat, pwm_output);
	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  //LED2
  if(GPIO_Pin == USER_Btn_Pin){
	  // przelaczanie wyswietlacza USER_Btn
	  // 0-temperatura, 1-Set Value, 2-PWM signal [%]
	  if(oledScreen>=0 && oledScreen<2){
	  	oledScreen++;
	  }
	  else{
	  	oledScreen = 0;
	  }
  }
  // zatwierdzenie wartosci zadanej i zalaczenie regulatora
  if(GPIO_Pin == ButtonYrSave_Pin){
  	regulator_state = REGULATOR_ON;
  	setpoint_tem = prep_setpoint_tem;
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM4){
		uint16_t previous_encoder_count = 0;
		previous_encoder_count = enkoder_count;
		enkoder_count = __HAL_TIM_GET_COUNTER(&htim4);

    // Obliczenie różnicy (delta) z uwzględnieniem przepełnienia
    int16_t delta = (int16_t)(enkoder_count - previous_encoder_count);

    // Aktualizacja wartości zadanej w zależności od delta
    if (delta > 0) {
    		prep_setpoint_tem += 1; // Ruch do przodu zwiększa wartość
    } else if (delta < 0) {
    		prep_setpoint_tem -= 1; // Ruch do tyłu zmniejsza wartość
    }

    // Opcjonalnie: Ograniczenie zakresu setpoint_tem
    if (prep_setpoint_tem > SetValMax) {
    		prep_setpoint_tem = SetValMax;
    } else if (prep_setpoint_tem < SetValMin) {
    		prep_setpoint_tem = SetValMin;
    }
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	// ustawianie wartosci zadanej temperatury yr (0-80C)
	if(huart->Instance == USART3){
		int value = atoi((char*)rx_data);  // Konwersja ASCII na liczbę
		if(value>=SetValMin && value<=SetValMax){
		  setpoint_tem = value;
		}
		memset(rx_data, 0, sizeof(rx_data));
		HAL_UART_Receive_IT(&huart3, rx_data, MSGLEN);
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
