/**
******************************************************************************
* @file    main.c
* @author  HW Wong
* @version  V2.2.0
* @date     5-Mar-2015
* @brief   
******************************************************************************
NEC Infrared Protocol
******************************************************************************
*/ 

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "NEC_Protocol.h"
#include "main.h"
#include "stdio.h"
/**
* @addtogroup GPIO_Toggle
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Evalboard I/Os configuration */



/* Private macro -------------------------------------------------------------*/
#define  KEY1_ON !GPIO_ReadInputPin(KEY1_PORT,KEY1_PIN)
#define  KEY1_OFF GPIO_ReadInputPin(KEY1_PORT,KEY1_PIN)
#define  KEY2_ON !GPIO_ReadInputPin(KEY2_PORT,KEY2_PIN)
#define  KEY2_OFF GPIO_ReadInputPin(KEY2_PORT,KEY2_PIN)
#define LED_ON() GPIO_WriteLow(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS)
#define LED_OFF() GPIO_WriteHigh(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS)

/* Private variables ---------------------------------------------------------*/
__IO int16_t level = 0;
struct commands IrCommand;
__IO TestStatus OperationStatus;
bool rampToggle;
__IO uint16_t levelMemory = UPPER_LIMIT;
__IO uint16_t  calUpperLimit;
/* Private function prototypes -----------------------------------------------*/
void DelayLoop(uint16_t nCount);
void Delay (uint16_t nCount);
void Delayus(void);
void Delayms(unsigned int time);
static void FLASH_Config(void);
void rampUpDown(uint16_t value);
uint16_t waitKey2(uint16_t timeout);
void stepRamp(int16_t step);
void CalFreq();
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
* @brief  Main program.
* @param  None
* @retval None
*/



void main(void)
{
  
  /* Initialize I/Os in Output Mode */
  GPIO_Init(TRIAC_PORT, (GPIO_Pin_TypeDef)TRIAC_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);  
  GPIO_Init(ZERO_CROSS_PORT, ZERO_CROSS_PIN, GPIO_MODE_IN_PU_IT);
  
  /* Configure the Fcpu to DIV1*/
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
  
  /* Configure the HSI prescaler to the optimal value */
  CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
  
  /* Configure the system clock to use HSI clock source and to run at 16Mhz */
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);
  
  
  
  
  
  GPIO_Init(KEY1_PORT, KEY1_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(KEY2_PORT, KEY2_PIN, GPIO_MODE_IN_FL_NO_IT);
  
  /* Initialize LED port */
  GPIO_Init(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS, GPIO_MODE_OUT_OD_HIZ_FAST);
  
  /* Initialize ext. interrput pin for zero cross deccation  */
  EXTI_SetExtIntSensitivity(ZERO_EXTI_PORT, EXTI_SENSITIVITY_RISE_ONLY);
  //  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_ONLY);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_RISE_ONLY); 
  
  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128,125);
  
  /* Set Time2 PWM 100Hz */
  TIM2_DeInit();
  TIM2_TimeBaseInit(TIM2_PRESCALER_8,20000);
  TIM2_OC1Init(TIM2_OCMODE_PWM1,TIM2_OUTPUTSTATE_ENABLE,0,TIM2_OCPOLARITY_HIGH);
  TIM2_ARRPreloadConfig(ENABLE);
  TIM2_Cmd(ENABLE);
  
  Delayms(1);
  
  /*Measure AC period */
  CalFreq();
  
  
  
  FLASH_Config();
  /* load ir command */
  for (uint8_t i=0 ;i < sizeof(IrCommand);i++){
    ((uint8_t*)&IrCommand)[i] = FLASH_ReadByte(0x4000 + i);
  }
  
  IR_init();
  GPIO_WriteHigh(LED_GPIO_PORT,LED_GPIO_PINS);
  
  
  enableInterrupts();
  
  
  uint8_t i=0 ;
  
  while (level < calUpperLimit - 5){
    level += 5;
    Delayms(1);
  }
  level = calUpperLimit;
  
  while (1)
  {
    uint8_t buf[10];
    uint8_t ans;
    
    
    /* Toggles LEDs */
    //    ans = getchar();
    //    printf("%d",ans);
    //    
    //    if (ans==8){
    //      printf("end\n\r");
    //      i=0;
    ////      level =atoi(buf);
    //    }else   {
    //    buf[i]=ans;
    //    i++;}
    
    
    if(KEY1_ON)
    {
      Delayms(1200);
      struct commands IrTemp;
      if( KEY1_ON){
        uint8_t time;
        for (i=0; i< sizeof(IrTemp)/2;i++){
          time = 100;
          while(ir_status.status!=IR_RECEIVED){
            Delayms(100);
            GPIO_WriteReverse(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS);
            time--;
            if (!time) goto STOP_TEACHER; 
          }
          ((uint16_t*)&IrTemp)[i] = ir_status.address <<8 | ir_status.command;
          LED_ON();
          Delayms(1200);
          IR_ClearFlag(); 
        }
        
        for (i=0 ;i < sizeof(IrTemp);i++){
          FLASH_ProgramByte(0x4000+i,((uint8_t*)&IrTemp)[i]);
          /* Wait until End of high voltage flag is set*/
          while (!FLASH_GetFlagStatus(FLASH_FLAG_EOP));
        }
        IrCommand = IrTemp;
      STOP_TEACHER:
        LED_OFF();
      }
    }
    
    
    
    /* Button Task */
    if(KEY2_ON){
      uint16_t t = waitKey2(240);
      
      if (t >240){
        if (level == LOWER_LIMIT) rampToggle = 1;
        else if  (level == calUpperLimit) rampToggle =0 ;
        while(KEY2_ON)
        {
          Delayms(1);
          stepRamp((rampToggle) ? LEVEL_STEP : -LEVEL_STEP);
        }
        rampToggle = !rampToggle;
      }else if (t > 20){
        if (level > LOWER_LIMIT)  
          rampUpDown(LOWER_LIMIT);
        else 
          rampUpDown(calUpperLimit);
      }
      
    }
    
    
    
    /* IR Task */
    if(ir_status.status==IR_RECEIVED)
    {
      uint16_t ir = (uint16_t)(ir_status.address << 8 | ir_status.command);
      //      if( ir ==  IrCommand.OnOff){
      //        if (level > LOWER_LIMIT)  
      //          rampUpDown(LOWER_LIMIT);
      //        else 
      //          rampUpDown(levelMemory);
      //      }else if(ir == IrCommand.increase){
      //        while(ir_status.hold_time){
      //          stepRamp(LEVEL_STEP); 
      //          Delayms(1);}
      //      } else if(ir ==IrCommand.decrease){
      //        while(ir_status.hold_time){
      //          stepRamp(-LEVEL_STEP);   
      //          Delayms(1);}
      //      } else if ( ir == IrCommand.off){
      //        rampUpDown(LOWER_LIMIT);
      //      }
      
      if(ir == IrCommand.increase){
        Delayms(500);
        if (ir_status.hold_time){
          while(ir_status.hold_time){
            stepRamp(LEVEL_STEP); 
            Delayms(1);}
        }else{
          if (level ==  calUpperLimit || level ==LOWER_LIMIT)
            rampUpDown(levelMemory);
          else if (level > LOWER_LIMIT)
            rampUpDown(calUpperLimit);
        }
      }
      
      if(ir == IrCommand.decrease){
        Delayms(500);
        if (ir_status.hold_time){
          while(ir_status.hold_time){
            stepRamp(-LEVEL_STEP); 
            Delayms(1);}
        }else{
          rampUpDown(LOWER_LIMIT);
        }
      }
      
      if ( ir == IrCommand.off){
        rampUpDown(LOWER_LIMIT);
      }
      
      if ( ir == IrCommand.OnOff)
      { if (level ==  calUpperLimit || level ==LOWER_LIMIT)
        rampUpDown(levelMemory);
      else if (level > LOWER_LIMIT)
        rampUpDown(calUpperLimit);
      }
      
      IR_ClearFlag(); 
    }
    
    
    
  }
  
  
}

void stepRamp(int16_t step){
  if(step > 0){
    level = (level >= calUpperLimit - step ) ? calUpperLimit : level + step;
  }else 
    level = (level <= LOWER_LIMIT - step ) ? LOWER_LIMIT : level + step;
    
    if (level == calUpperLimit) 
    {
      
      while(level > calUpperLimit/1.5)
      {
        level -=20;
        Delayms(1);
      }
      
      while(level < calUpperLimit - step )
      {
        level +=20;
        Delayms(1);
      }
      level = calUpperLimit;
      while(ir_status.hold_time || KEY2_ON);
    }
    
    if (level!=LOWER_LIMIT && level!=calUpperLimit) levelMemory = level;
}


void rampUpDown(uint16_t value){
  
  if (level < value){
    if (calUpperLimit == value && level == LOWER_LIMIT)    {
      CalFreq();
      value = calUpperLimit;  //cal freq. and apply to value
    }
    
    while(level<value){
      if (level < value - RAMP_STEP)
        level += RAMP_STEP;
      else 
        level = value;
      Delayms(1);
    }
  }else{
    while(level > value){
      if (level > value + RAMP_STEP)
        level -= RAMP_STEP;
      else 
        level = value;
      Delayms(1);
    }     
  } 
}

uint16_t waitKey2(uint16_t timeout){
  Delayms(5); // delay 2ms 
  uint16_t t=5;
  while(timeout--){
    Delayms(1);
    t++;
    if(KEY2_OFF)
      break;
  }
  return t;
}

void CalFreq(){
  uint16_t AcPeriod=0;
  
  while( AcPeriod < 19500 ||  AcPeriod > 20500 ){
    while(!GPIO_ReadInputPin(ZERO_CROSS_PORT,ZERO_CROSS_PIN)); // wait for AC low
    while(GPIO_ReadInputPin(ZERO_CROSS_PORT,ZERO_CROSS_PIN)); // wait for AC falling high
    TIM2_SetCounter(0);                                            // reset counter
    while(!GPIO_ReadInputPin(ZERO_CROSS_PORT,ZERO_CROSS_PIN)); // wait for AC low
    while(GPIO_ReadInputPin(ZERO_CROSS_PORT,ZERO_CROSS_PIN)); // wait for AC falling high
    AcPeriod =  TIM2_GetCounter();
  }
  
  TIM2_SetAutoreload(AcPeriod );
  calUpperLimit =(uint16_t)( AcPeriod *0.80);
  levelMemory = calUpperLimit;
}


void Delayms(unsigned int time)
{
  TIM4_SetCounter(0);
  TIM4_Cmd(ENABLE);
  while(time--){
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    while(!TIM4_GetFlagStatus(TIM4_FLAG_UPDATE));
  }
  TIM4_Cmd(DISABLE);
}

/**
* @brief Delay
* @param nCount
* @retval None
*/
void Delay(uint16_t nCount)
{
  /* Decrement nCount value */
  while (nCount != 0)
  {
    nCount--;
  }
}

void DelayLoop(uint16_t nCount){
  while(nCount != 0){
    Delay(0xffff);
    nCount--;
  }
}

int putchar (int c)
{
  /* Write a character to the UART1 */
  UART1_SendData8(c);
  /* Loop until the end of transmission */
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  
  return (c);
}

int getchar (void)
{
#ifdef _COSMIC_
  char c = 0;
#else
  int c = 0;
#endif
  /* Loop until the Read data register flag is SET */
  while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);
  c = UART1_ReceiveData8();
  return (c);
}

/**
* @brief  Configure the FLASH for block programming
* @param  None
* @retval None
*/
void FLASH_Config(void)
{
  /* Define flash programming Time*/
  FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
  
  FLASH_Unlock(FLASH_MEMTYPE_PROG);
  /* Wait until Flash Program area unlocked flag is set*/
  while (FLASH_GetFlagStatus(FLASH_FLAG_PUL) == RESET)
  {}
  
  /* Unlock flash data eeprom memory */
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  /* Wait until Data EEPROM area unlocked flag is set*/
  while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
  {}
}


#ifdef USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
