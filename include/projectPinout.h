/******************************************************************************
 Note that the physical pin count starts at the reset button on long pin side 
 of the Huzzah32 development board.
 ******************************************************************************/
#ifndef _PROJECT_GPIO_PINS_H // Start of conditional preprocessor code that  
                             // only allows this library to be included once.
#define _PROJECT_GPIO_PINS_H // Preprocessor variable used by above check.
const int8_t enA1 = PIN_10_LBL_A5; // Enable Motor A. Physical pin 10.
const int8_t inA1 = PIN_6_LBL_A1; // Motor A In1 pin. Physical pin 6.
const int8_t inA2 = PIN_5_LBL_A0; // Motor A In2 pin. Physical pin 5.
const int8_t inB1 = PIN_19_LBL_14; // Motor B In1 pin. Physical pin 19.
const int8_t inB2 = PIN_20_LBL_32; // Motor B In2 pin. Physical pin 20.
const int8_t inC1 = PIN_21_LBL_15; // Motor C In1 pin. Physical pin 21.
const int8_t inC2 = PIN_22_LBL_33; // Motor C In2 pin. Physical pin 22.
const int8_t servoPin = PIN_23_LBL_27; // Servo control pin. Physical pin 23.

#endif // End of conditional preprocessor code