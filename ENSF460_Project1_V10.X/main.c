/*
 * File:   main.c
 * Author: sahil
 *
 * Created on September 29, 2025, 3:51 PM
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>
#include "clkChange.h"
#include "UART2.h"
#include "IOs.h"
#include "TimeDelay.h"
#include "LEDStatus.h"


volatile uint8_t PB1_event;
volatile uint8_t PB2_event;
volatile uint8_t PB3_event;

volatile uint8_t PB12_event;


volatile uint8_t PB13_event;
volatile uint8_t PB23_event;

volatile uint8_t PB123_event;


volatile uint8_t minutes = 0;
volatile uint8_t seconds = 0;

volatile uint8_t no_buttons_event = 0;

volatile timer_state state;

volatile uint8_t T2_flag;
volatile uint8_t display_flag;


volatile uint8_t Prev_PB1_status;
volatile uint8_t Prev_PB2_status;
volatile uint8_t Prev_PB3_status;

volatile uint8_t time_complete;







/**
 * You might find it useful to add your own #defines to improve readability here
 */


// defining input and output bits here to make it easier to code later
#define  PB1 PORTBbits.RB7
#define  PB2 PORTBbits.RB4
#define  PB3 PORTAbits.RA4

#define  LED1 LATBbits.LATB9
#define  LED2 LATAbits.LATA6



int main(void) {
    
    /** This is usually where you would add run-once code
     * e.g., peripheral initialization. For the first labs
     * you might be fine just having it here. For more complex
     * projects, you might consider having one or more initialize() functions
     */
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500);
    
    //T3CON config
    Timer3_Init();

    /* Let's set up some I/O */
    IOinit();
    
    /* Let's clear some flags */
    PB1_event = 0;
    PB2_event = 0;
    PB3_event = 0;
    
    PB12_event = 0;
    PB13_event = 0;
    PB23_event = 0;
    
    PB123_event = 0;

    Prev_PB1_status = 1;
    Prev_PB2_status = 1;
    Prev_PB3_status = 1;
    
    T2_flag = 0;

    no_buttons_event = 0;
    
    state = STATE_SET_TIME;
    
    LED1 = 0;
    LED2 = 0;
    

    
    /* Let's set up our UART */    
    InitUART2();
  
    Disp2String("SET 00m : 00s                   \r");    
    
    
    while(1) {
        Idle(); // Send CPU into idle --> will stay there until woken
        
        //==== Run main program loop after any interrupts ===//
        
        // check state of the input/outputs and put us into the corresponding state
        IOcheck();
        
        // Making sure we always check for when the timer is complete
        if(time_complete && state == STATE_COUNTING) {
            time_complete = 0;
            state = STATE_ALARM;
            display_flag = 1; 
        }
        
        // Display the current time
        if(display_flag){
            display_flag = 0;
            Disp2Time(minutes, seconds, state);
        }
        
        // This checks the current state and decides what to do with the LED
        LEDStatus();
    }
    
    return 0;
}



void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T1IF = 0;
    
    LED2 = !LED2;
}


// Timer 2 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T2IF = 0;
    T2CONbits.TON = 0;
    T2_flag = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    IFS0bits.T3IF = 0;
    
    if (state == STATE_COUNTING) {
        display_flag = 1;
        
        // Decrement time (happens every second)
        if (seconds == 0) {
            if (minutes == 0) { // time is up
                time_complete = 1;
                T3CONbits.TON = 0; // stop timer 
            } 
            else {
                minutes--;
                seconds = 59;
            }
        } 
        else {
            seconds--;
        }
        
        // Make LED1 blink -> 1 second on, 1 second off
        LED1 = !LED1;

    }
}

uint8_t fall = 0;
uint8_t prev_fall = 0;

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //Don't forget to clear the CN interrupt flag!
    
    // find which button(s) have been pressed and set corresponding event
    if (IFS1bits.CNIF == 1){
        fall = (!PB1) + (!PB2) + (!PB3);
        if(fall > prev_fall){
            if(!PB1 && !PB2 && !PB3){
                PB123_event = 1;
            }
            else if(!PB1 && !PB2){
                PB12_event = 1;
            } 
            else if(!PB1 && !PB3){
                PB13_event = 1;
            }
            else if(!PB2 && !PB3){
                PB23_event = 1;
            }           
            else if(!PB1 && Prev_PB1_status){
                PB1_event = 1;
            }            
            else if(!PB2 && Prev_PB2_status){
                PB2_event = 1;
            }     
            else if(!PB3 && Prev_PB3_status){
                PB3_event = 1;
            }                
        }
        
        Prev_PB1_status = PB1;
        Prev_PB2_status = PB2;
        Prev_PB3_status = PB3;
        prev_fall = fall;
    }
    
    IFS1bits.CNIF = 0;

    
}