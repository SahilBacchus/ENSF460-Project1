/* 
 * File:   LEDStatus.c
 * Author: x
 *
 * Created on October 24, 2025, 9:22 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "LEDStatus.h"
#include "UART2.h"
#include "TimeDelay.h"
/*
 * 
 */

extern volatile timer_state state;


#define LED1 LATBbits.LATB9
#define  LED2 LATAbits.LATA6


void LEDStatus(){
    switch(state) {
        case STATE_SET_TIME:
            LED1 = 0;
            Stop_LED2_Blinking();
            break;
            
        case STATE_COUNTING:
            Stop_LED2_Blinking();
            break;
            
        case STATE_PAUSED:
            LED1 = 0;
            Stop_LED2_Blinking();
            break;
            
        case STATE_ALARM:
            LED1 = 1;  // Keep LED1 solid on
            delay_ms_LED2(50);
            break;
            
        default:
            LED1 = 0;
            Stop_LED2_Blinking();
            break;
    }
}

