
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "NEC_Protocol.h"
/**
  * @addtogroup GPIO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
#define TIM1_PERIOD 

/* Private define ------------------------------------------------------------*/
/* Evalboard I/Os configuration */

#define IR_GPIO_PIN  (GPIO_PIN_4)
#define IR_GPIO_PORT (GPIOD)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ir_state_t ir_state =IR_BURST;
Ir_Status_TypeDef ir_status;
/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Configure TIM4 to generate an update interrupt each 1ms 
  * @param  None
  * @retval None
  */
void TIM1_Config(void)
{


  /* Time base configuration */
  TIM1_TimeBaseInit(31,TIM1_COUNTERMODE_UP, 65535,0);
  /* Clear TIM4 update flag */
  TIM1_ClearFlag(TIM1_FLAG_UPDATE);
  /* Enable update interrupt */
  //TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
  
  /* enable interrupts */
  //enable interrupts at main

  /* Enable TIM1 */
  TIM1_Cmd(ENABLE);
}

void IR_init(){
  
  GPIO_Init(IR_GPIO_PORT, IR_GPIO_PIN, GPIO_MODE_IN_FL_IT);
  
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);  
  TIM1_Config();
  
  return;
}

void IR_ClearFlag()
{
 ir_status.status = 0; 
}


void IR_Signal_IRQ()
{
  static uint16_t ir_period;
  static uint8_t recevie[4];
  static uint8_t i;
  ir_period = TIM1_GetCounter();
  
  
  switch (ir_state)
  {
    
  case IR_BURST:
    TIM1_SetCounter(0);   
    //GPIO_WriteReverse(GPIOB, GPIO_PIN_5);  
    
    if (ir_period < (TIME_BURST +TIME_TOLERANCE) & ir_period > (TIME_BURST - TIME_TOLERANCE ))  
      ir_state = IR_DATA; 
    break;
    
    
  case  IR_DATA:
    
    TIM1_SetCounter(0);
    
    if (i<32){
      
      if (ir_period < (TIME_HIGH +TIME_TOLERANCE) & ir_period > (TIME_HIGH - TIME_TOLERANCE )) 
      {
        recevie[i>>3] = recevie[i>>3] >> 1 ;
        recevie[i>>3] |= 0x80;
      }
      else if (ir_period < TIME_LOW+TIME_TOLERANCE & ir_period >TIME_LOW- TIME_TOLERANCE)
        recevie[i>>3] = recevie[i>>3] >> 1 ;      
      else{ 
        ir_state = IR_BURST; 
        i=0;
        break;
      }
      i++;
      break;
    }
    else{
      ir_state = IR_REPEAT; 
      i=0;
      ir_status.address = recevie[0];
      ir_status.command = recevie[2];
      ir_status.status = IR_RECEIVED;
      ir_status.hold_time = 1;
      TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    }
    break;
    
      case  IR_REPEAT:
           TIM1_SetCounter(0);
           if (ir_period < (TIME_REPEAT +TIME_TOLERANCE) ){
             if  (ir_status.hold_time < 255)
               ir_status.hold_time++;}
           else 
           {
             ir_status.hold_time=0;
             ir_state = IR_BURST;
           }
             
        
        break;
        
  }
  
  
  
  
  TIM1_ClearFlag(TIM1_FLAG_UPDATE);
  
}