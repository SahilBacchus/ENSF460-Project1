#include "TimeDelay.h"
#include <xc.h>


extern volatile uint8_t T2_flag;

#define LED2 LATAbits.LATA6


void delay_ms(uint16_t time_ms){
    // Timer2 config
    T2CONbits.T32 = 0;      // Operate timer 2 as 16 bit timer
    T2CONbits.TCKPS = 3;    // Set prescaler to 1:256
    T2CONbits.TCS = 0;      // Use internal clock
    T2CONbits.TSIDL = 0;    // Operate in idle mode
    IPC1bits.T2IP = 5;      // Timer2 interrupt priority (7 is highest, 1 is lowest)
    IFS0bits.T2IF = 0;      // Clear Timer2 interrupt flag
    IEC0bits.T2IE = 1;      // Enable Timer2 interrupt
    
    PR2 = (uint16_t) time_ms * 125/128;  // based on our calculations (what it counts up to)
    TMR2 = 0;                            // what we count to
    T2CONbits.TON = 1;                   // turn timer2 on
    
    // Stay in idle until delay is over
    T2_flag = 0;
    while(T2_flag == 0){
        Idle();
    }
    
    T2CONbits.TON = 0;       // turn timer2 off
    
}


void Timer3_Init() {
    // Timer3 configuration
    T2CONbits.T32 = 0;        // Operate timer 3 as 16 bit timer
    T3CONbits.TCKPS = 3;      // Set prescaler to 1:256
    T3CONbits.TCS = 0;        // Use internal clock
    T3CONbits.TSIDL = 0;      // Operate in idle mode
    IPC2bits.T3IP = 4;        // Timer3 interrupt priority (7 is highest, 1 is lowest)
    IFS0bits.T3IF = 0;        // Clear Timer3 interrupt flag
    IEC0bits.T3IE = 1;        // Enable Timer3 interrupt
    
    PR3 = 976;                // based on our calculations  (what it counts up to)
    TMR3 = 0;                 // start counting from
}

void Timer3_Start() {
    TMR3 = 0;
    T3CONbits.TON = 1;        
}

void Timer3_Stop() {
    T3CONbits.TON = 0;        
}

void Timer3_SetPeriod(uint16_t period) {
    PR3 = period;
}

void delay_ms_LED2(uint16_t time_ms){
    // Timer1 config
    T1CONbits.TCKPS = 3;        // Set prescaler to 1:256
    T1CONbits.TCS = 0;          // Use internal clock
    T1CONbits.TSIDL = 0;        // Operate in idle mode
    IPC0bits.T1IP1 = 2;         // Timer1 interrupt priority (7 is highest, 1 is lowest)
    IFS0bits.T1IF = 0;          // Clear Timer1 interrupt flag
    IEC0bits.T1IE = 1;          // Enable Timer1 interrupt
    
    PR1 = (uint16_t) time_ms * 125/128;     // based on our calculations  (what it counts up to)
    TMR1 = 0;                               // start counting from
    
    T1CONbits.TON = 1;          // start timer1
    
    
}

void Stop_LED2_Blinking(void) {
    T1CONbits.TON = 0;  // Stop Timer1
    LED2 = 0;           // Ensure LED2 is off
}