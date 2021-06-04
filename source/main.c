/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <avr/interrupt.h>
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff(){
    TCCR1B = 0x00;
}
void TimerISR(){
    TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}

}

void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

void ADC_init(){
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE) ;
}
void Set_A2D_Pin(unsigned char pinNum) {
ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
// Allow channel to stabilize
static unsigned char i = 0;
for ( i=0; i<15; i++ ) { asm("nop"); } 
}

typedef struct task{
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);
} task;

unsigned char game_running = 0;
unsigned char solo = 0;
unsigned char duo = 0;
unsigned char paddle1_position_x = 0x80;
unsigned char paddle2_position_x = 0x01;
unsigned char ball_position_x = 0x04;
// unsigned char positionArray_x[3] = {paddle1_position_x,paddle2_position_x,ball_position_x};
unsigned char paddle1_position_y = 0xFE;
unsigned char paddle2_position_y = 0xFE;
unsigned char ball_position_y = 0xFB;
// unsigned char positionArray_y[3] = {paddle1_position_y,paddle2_position_y,ball_position_y};
// enum Demo_States {shift};
// int Demo_Tick(int state) {

// 	// Local Variables
// 	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
// 	static unsigned char row = 0xFE;  	// Row(s) displaying pattern. 
// 							// 0: display pattern on row
// 							// 1: do NOT display pattern on row
// 	// Transitions
// 	switch (state) {
// 		case shift:	
// 			break;
// 		default:	
// 			state = shift;
// 			break;
// 	}	
// 	// Actions
// 	switch (state) {
// 		case shift:	
// 			if (row == 0xEF && pattern == 0x01) { // Reset demo 
// 				pattern = 0x80;		    
// 				row = 0xFE;
// 			} else if (pattern == 0x01) { // Move LED to start of next row
// 				pattern = 0x80;
// 				row = (row << 1) | 0x01;
// 			} else { // Shift LED one spot to the right on current row
// 				pattern >>= 1;
// 			}
// 			break;
// 		default:
// 			break;
// 	}
// 	PORTC = pattern;	// Pattern to display
// 	PORTD = row;		// Row(s) displaying pattern	
// 	return state;
// }
enum menu_states {wait, solo_state, solo_state_wait, duo_state, duo_state_wait};
int menu(int state){
	if(game_running != 1){
		switch (state){
		case wait:	
			if((~PINB & 0x01) == 0x01){
				state = solo_state;	
			}
			else if((~PINB & 0x02) == 0x02){
				state = duo_state;
			}
			else{
				state = wait;	
			}
			break;
		case solo_state:
			if(~PINB == 0x00){
				state = solo_state_wait;
			}
			else{
				state = solo_state;	
			}
			break;
		case solo_state_wait:
			if(game_running == 0){
				state = wait;
			}
			else{
				state = solo_state_wait;	
			}
			break;
		case duo_state:
			if(~PINB == 0x00){
				state = duo_state_wait;
			}
			else{
				state = duo_state;	
			}
			break;
		case duo_state_wait:
			if(game_running == 0){
				state = wait;
			}
			else{
				state = duo_state_wait;	
			}
			break;
		default:	
			state = wait;
			break;
	}	
	switch (state) {
		case wait:
			duo = 0;
			solo = 0;
			game_running = 0;
			break;
		case solo_state:
			solo = 1;
			duo = 0;
			game_running = 1;
			break;
		case solo_state_wait:
			break;
		case duo_state:
			solo = 0;
			duo = 1;
			game_running = 1;
			break;
		case duo_state_wait:
			break;
		default:	
			break;
	}
	else{}
	return state;
}
enum Joystick_States {shift};
int Joystick_Tick(int state) {
	static unsigned short sensor_value = 0x00;
// 	if(solo == 1){
	switch (state) {
		case shift:	
			break;
		default:	
			state = shift;
			break;
	}	
	switch (state) {
		case shift:
			sensor_value = ADC;
			if (sensor_value < 450 && (paddle1_position_y  != 0xEF)) { // Reset demo 
				paddle1_position_y  = ((paddle1_position_y  << 1) | 0x01);
			}else if (sensor_value > 650 && (PORTD != 0xFE)) { // Move LED to start of next row
				paddle1_position_y  = ((paddle1_position_y  >> 1) | 0x80);
			} 
			else { // Shift LED one spot to the right on current row
			}
			break;
		default:
			break;
	}
// 	}
	return state;
}
enum button_movement {shift_button_wait, shift_button_up, shift_button_down};
int button_movement_Tick(int state) {
	static unsigned short sensor_value = 0x00;
	
	switch (state) {
		case shift_button_wait:	
			if(((~PINB & 0x01) == 0x01) & ((~PINB & 0x02) != 0x02)){
				state = shift_button_up;	
			}
			else if(((~PINB & 0x02) == 0x02) & ((~PINB & 0x01) != 0x01)){
				state = shift_button_down;	
			}
			else{
				state = shift_button_wait;
			}
			break;
		case shift_button_up:
			if(((~PINB & 0x01) == 0x01) & ((~PINB & 0x02) != 0x02)){
				state = shift_button_up;	
			}
			else{
				state = shift_button_wait;	
			}
			break;
		case shift_button_down:
			if(((~PINB & 0x02) == 0x02) & ((~PINB & 0x01) != 0x01)){
				state = shift_button_down;	
			}
			else{
				state = shift_button_wait;	
			}
			break;
		default:	
			state = shift_button;
			break;
	}	
	switch (state) {
		case shift_button_wait:	
			break;
		case shift_button_up:
			if(paddle1_position_y != 0xFE)) {
				paddle1_position_y = ((PORTD >> 1) | 0x80);
			}
			break;
		case shift_button_down:
			if(paddle1_position_y != 0xEF)) {
				paddle1_position_y = ((PORTD << 1) | 0x01);
			}
			break;
		default:	
			break;
	}
	return state;
}
enum display_states{change};
int display(state){
	static unsigned char i = 0;
	switch (state) {
		case change:	
			break;
		default:	
			state = shift;
			break;
	}	
	switch (state) {
		case change:	
// 			for(i = 0; i < 3; i++){
// 				PORTC = positionArray_x[i];
// 				PORTD = positionArray_y[i];
// 			}
			PORTC = paddle1_position_x;
			PORTD = paddle1_position_y;
			PORTC = paddle2_position_x;
			PORTD = paddle2_position_y;
			PORTC = ball_position_x;
			PORTD = ball_position_y;
		default:	
			break;
	}	
	return state;	
}
	
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0x00; PORTB = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
	
    PORTC = 0x80;
    PORTD = 0xFE;
    ADC_init();
    Set_A2D_Pin(0x01);
//     unsigned short sensor_value = 0;
    /* Insert your solution below */
    static task task1;
    static task task2;
    static task task3;
	
    task *tasks[] = {&task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = 0;
    
    task1.state = start;
    task1.period = 200;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Joystick_Tick;
    
    task2.state = start;
    task2.period = 200;
    task2.elapsedTime = task2.period;
    task2.TickFct = &button_movement_tick;
    
    task3.state = start;
    task3.period = 1;
    task3.elapsedTime = task3.period;
    task3.TickFct = &display;
	
    TimerSet(1);
    TimerOn();
    
    unsigned short x;
    while (1) {
          for(x = 0; x < numTasks; x++){
		    if(tasks[x]->elapsedTime == tasks[x]->period){
			    tasks[x]->state = tasks[x]->TickFct(tasks[x]->state);
			    tasks[x]->elapsedTime = 0;
		    }
		tasks[x]->elapsedTime += 1;
	    }
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
