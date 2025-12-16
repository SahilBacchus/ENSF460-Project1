/* 
 * File:   IOs.h
 * Author: jibra 
 *
 * Created on October 3, 2025, 3:15 AM
 */

#ifndef IOS_H
#define IOS_H

#include <xc.h>

// event enum: 
typedef enum {
    EV_PB1,
    EV_PB2,
    EV_PB3,
    EV_PB12,
    EV_PB13,
    EV_PB23,
    EV_PB123
} ButtonEvent;

// Function declarations
void IOinit();
void IOcheck();
void disable_CN();
void enable_CN();
uint8_t handle_debounce(ButtonEvent ev, volatile uint8_t *event_flag); 




#endif // 