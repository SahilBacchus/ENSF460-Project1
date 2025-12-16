#ifndef TIMEDELAY_H
#define TIMEDELAY_H

#include <xc.h>

// Function prototypes
void Timer3_Init();
void Timer3_Start();
void Timer3_Stop();
void Timer3_SetPeriod(uint16_t period);
void delay_ms(uint16_t time_ms);
void delay_ms_LED2(uint16_t tms);
void Stop_LED2_Blinking(void);



// Timer3 period constants for different blink rates
#define TIMER3_500MS 1953    
#define TIMER3_1S    3906     
#define TIMER3_4S   15625


#endif