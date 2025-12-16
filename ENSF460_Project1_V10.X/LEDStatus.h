#ifndef CHECKSTATE_H
#define CHECKSTATE_H

#include <xc.h>


typedef enum {
    STATE_SET_TIME,
    STATE_COUNTING,
    STATE_PAUSED,
    STATE_ALARM,
    STATE_CLEAR
} timer_state;

// Function prototypes
void LEDStatus();

#endif