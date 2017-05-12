/**
  ******************************************************************************
  * @file    EXTI_InterruptPriority\main.h
  * @author  MCD Application Team
  * @version V2.0.1
  * @date    18-November-2011
  * @brief   This file contains the external declaration for main.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */   

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Private typedef -----------------------------------------------------------*/
typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;

struct commands
{
uint16_t increase;
uint16_t decrease ;
uint16_t OnOff;
uint16_t off;
};
    
/* Private define ------------------------------------------------------------*/
#define BLOCK_OPERATION    0    /* block 0 in data eeprom memory: address is 0x4000 */

/* Evalboard I/Os configuration */
#define TRIAC_FIRE_WIDTH 500
#define UPPER_LIMIT 16500
#define LOWER_LIMIT 0
#define LEVEL_STEP  1
#define RAMP_STEP   2

#define ZERO_EXTI_PORT     (EXTI_PORT_GPIOA)
#define ZERO_CROSS_PORT    (GPIOA)
#define ZERO_CROSS_PIN     (GPIO_PIN_3)

#define TRIAC_PORT         (GPIOC)
#define TRIAC_PIN          (GPIO_PIN_5)

#define LED_GPIO_PORT           (GPIOA)
#define LED_GPIO_PINS            (GPIO_PIN_2)

#define KEY1_PORT          (GPIOC)
#define KEY1_PIN           (GPIO_PIN_7)
#define KEY2_PORT          (GPIOC)
#define KEY2_PIN           (GPIO_PIN_6)

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

#endif /* __MAIN_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
