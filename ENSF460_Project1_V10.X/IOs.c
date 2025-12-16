/* 
 * File:   IOs.c
 * Author: 
 *
 * Created on September 22, 2025, 2:15 PM
 */

#include <xc.h>
#include "IOs.h"
#include "TimeDelay.h"
#include "UART2.h"
#include "LEDStatus.h"


extern volatile uint8_t PB1_event;
extern volatile uint8_t PB2_event;
extern volatile uint8_t PB3_event;

extern volatile uint8_t PB12_event;
extern volatile uint8_t PB13_event;
extern volatile uint8_t PB23_event;

extern volatile uint8_t PB123_event;

extern volatile uint8_t minutes;
extern volatile uint8_t seconds;

extern uint8_t no_buttons_event;
extern volatile timer_state state;



// defining input and output bits here to make it easier to code later
#define  PB1 PORTBbits.RB7
#define  PB2 PORTBbits.RB4
#define  PB3 PORTAbits.RA4

#define  LED1 LATBbits.LATB9
#define  LED2 LATAbits.LATA6


void IOinit(){
    //-- Initialization of pins --//

    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 1;
    
    TRISAbits.TRISA6 = 0;
    LATAbits.LATA6 = 1;

    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;
    CNEN2bits.CN23IE = 1;
    
    
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
}


void disable_CN(){
    CNEN2bits.CN23IE = 0;
    CNEN1bits.CN1IE = 0;
    CNEN1bits.CN0IE = 0;

}

void enable_CN(){
    CNEN2bits.CN23IE = 1;
    CNEN1bits.CN1IE = 1;
    CNEN1bits.CN0IE = 1;

}



uint8_t handle_debounce(ButtonEvent ev, volatile uint8_t *event_flag) {
    delay_ms(30);
   
    // Make sure the button intially pressed is still being pressed
    switch(ev) {
        case EV_PB123:
            if (!PB1 && !PB2 && !PB3) return 1;
            break;
        case EV_PB12:
            if (!PB1 && !PB2 && PB3) return 1;
            break;
        case EV_PB13:
            if (!PB1 && PB2 && !PB3) return 1;
            break;
        case EV_PB23:
            if (PB1 && !PB2 && !PB3) return 1;
            break;
        case EV_PB1:
            if (!PB1 && PB2 && PB3) return 1;
            break;
        case EV_PB2:
            if (PB1 && !PB2 && PB3) return 1;
            break;
        case EV_PB3:
            if (PB1 && PB2 && !PB3) return 1;
            break;

        default:
            break;
    }
    
    // clear if the pin is still not pressed after 30 ms --> it was debounce
    *event_flag = 0;
    return 0;
}


void IOcheck(){    
    delay_ms(50);
    
    // All 3 buttons are pressed - display group info
    if(PB123_event){
        // handle debounce
        if (!handle_debounce(EV_PB123, &PB123_event)) return; 
        
        Disp2String("\rENSF 460 L01 - Group 21     \r");
        PB123_event = 0;
    }
    
    // PB1 + PB2 - start/reset
    else if(PB12_event){
        // handle debounce
        if (!handle_debounce(EV_PB12, &PB12_event)) return; 
        
        // Check how long button(s) have been pressed for 
        //disable_CN();
        uint16_t hold_time = 0;
        while(!PB1 && !PB2 && PB3){
            if (!handle_debounce(EV_PB12, &PB12_event)) break; 
            delay_ms(100);
            hold_time += 130;
            //Disp2Dec(hold_time);
            if (hold_time >= 1500) break; 
        }        
        //enable_CN();
        
        // short press: <1.5 s
        if(hold_time < 1500){
            
//            // Pause timer if counting
//            if(state == STATE_COUNTING){
//                Disp2Time(minutes, seconds, STATE_PAUSED);
//                state = STATE_PAUSED;
//                Timer3_Stop();
//            }
            
            // start counting if time is set
            if(state == STATE_PAUSED || state == STATE_SET_TIME){
                if (minutes > 0 || seconds > 0) {
                        state = STATE_COUNTING;
                        Timer3_Start();
                        Disp2Time(minutes, seconds, state);
                    }
                else {
                    XmitUART2('\r', 1);
                    Disp2String("SET time first             \r");
                }
            }
        }
        
        // long press: >1.5 s
        else{
            state = STATE_SET_TIME;
            seconds = 0;
            minutes = 0;
            Disp2Time(minutes, seconds, STATE_CLEAR);
        }
        PB12_event = 0;
    }
    
    // PB1 + PB3 - Decrement seconds
    else if(PB13_event){
        if (!handle_debounce(EV_PB13, &PB13_event)) return; 
        uint8_t increments = 1;
        state = STATE_SET_TIME;
        
        while(!PB1 && PB2 && !PB3){
            if (!handle_debounce(EV_PB13, &PB13_event)) break; 
            delay_ms(300);
            seconds -= increments;
            if(seconds <= 0) seconds = 59;
            Disp2Time(minutes, seconds, state);
        }
        PB13_event = 0;    
    }
    
    // PB2 + PB3 - decrment minutes
    else if(PB23_event){
        if (!handle_debounce(EV_PB23, &PB23_event)) return; 
        uint8_t increments = 1;
        state = STATE_SET_TIME;
        
        while(PB1 && !PB2 && !PB3){
            if (!handle_debounce(EV_PB23, &PB23_event)) break; 
            delay_ms(300);
            minutes -= increments;
            if(minutes <= 0) minutes = 59;
            Disp2Time(minutes, seconds, state);
        }
        PB23_event = 0;    
    }    
    
    // PB1 - increment seconds 
    else if(PB1_event){
        if (!handle_debounce(EV_PB1, &PB1_event)) return; 
        uint16_t hold_time = 0;
        uint8_t increments = 1;
        state = STATE_SET_TIME;
        Timer3_Stop();

        while(!PB1 && PB2 && PB3){
            if (!handle_debounce(EV_PB1, &PB1_event)) break; 
            delay_ms(100);
            seconds += increments;
            if(seconds >= 60) seconds = 0;
            hold_time += 100;
            Disp2Time(minutes, seconds, state);
            
            // if helf for >1.5 seconds, increment by 5
            if(increments == 1 && hold_time >= 1500){
                seconds = (seconds/5)*5;
                increments = 5;
            }
            if (!PB2) break; 
        }
        PB1_event = 0;
    }
    
    // PB2 - increment minutes 
    else if(PB2_event){
        if (!handle_debounce(EV_PB2, &PB2_event)) return; 
        uint16_t hold_time = 0;
        uint8_t increments = 1;
        state = STATE_SET_TIME;
        Timer3_Stop();
        
        while(!PB2 && PB1 && PB3){
            if (!handle_debounce(EV_PB2, &PB2_event)) break;
            delay_ms(100);
            minutes += increments;
            if(minutes >= 60) minutes = 0;
            Disp2Time(minutes, seconds, state);
            hold_time += 120;
            
            // if held for >1.5 seconds, increment by 5
            if(increments == 1 && hold_time >= 1500){
                minutes = (minutes/5)*5;
                increments = 5;
            }
            if (!PB1) break; 
        }
        PB2_event = 0;
    }
    
    // PB3 - pause/resume/reset
    else if(PB3_event){
        if (!handle_debounce(EV_PB3, &PB3_event)) return; 
        uint16_t hold_time = 0;
        
        // Check how long button(s) have been pressed for 
        while(PB1 && PB2 && !PB3){
            if (!handle_debounce(EV_PB3, &PB3_event)) break; 
            delay_ms(30);
            hold_time += 60;
            if (hold_time >= 3000) break; 
        }
        
        // Short press(>3 s) - pause/ resume 
        if(hold_time < 3000){
            
            // if counting then pause
            if(state == STATE_COUNTING){
                state = STATE_PAUSED;
                Timer3_Stop();
                Disp2Time(minutes, seconds, state);

            }
            
            // if paused then resume
            else if(state == STATE_PAUSED){
            //else if(state == STATE_PAUSED || state == STATE_SET_TIME){
                if (minutes > 0 || seconds > 0) {
                        state = STATE_COUNTING;
                        Timer3_Start();
                        Disp2Time(minutes, seconds, state);
                    }
                else {
                    XmitUART2('\r', 1);
                    Disp2String("SET time first           \r");
                }
            }
            
        }
        
        // Long press(>3 s) - reset timer
        else if (hold_time >= 3000 && state != STATE_SET_TIME){  
            state = STATE_SET_TIME;
            seconds = 0;
            minutes = 0;
            Timer3_Stop(); 
            Disp2Time(minutes, seconds, STATE_CLEAR);
        }
        PB3_event = 0;
    }
     
}