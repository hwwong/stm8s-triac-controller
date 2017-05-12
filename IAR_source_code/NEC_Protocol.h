// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
#


#ifndef LIBNECDECODER_H
#define LIBNECDECODER_H


// Uncomment this to enable extended NEC protocol support.
//#define PROTOCOL_NEC_EXTENDED

//  Burst, 9ms + cap typ
#define TIME_BURST 6750
#define TIME_HIGH  1125
#define TIME_LOW   560
#define TIME_REPEAT 55000

// ir signal time tolerance
#define TIME_TOLERANCE 150

// Definition for state machine 
typedef enum  { IR_BURST, IR_DATA, IR_REPEAT }ir_state_t;

// Definition for status bits
#define IR_WAIT 0
#define IR_RECEIVED 1 


// Timer Overflows till keyhold flag is cleared
#define IR_HOLD_OVF 5

// Struct definition
typedef struct ir_struct
{
#ifdef PROTOCOL_NEC_EXTENDED
uint8_t address_l;
uint8_t address_h;
#else
uint8_t address;
#endif
uint8_t command;
uint8_t status;
uint8_t hold_time;
  } Ir_Status_TypeDef;

// Global status structure


  extern ir_state_t ir_state; 
  extern Ir_Status_TypeDef ir_status;

// Functions
void IR_init();
void IR_ClearFlag();
#endif