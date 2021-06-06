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

unsigned paddle1_position_y = 0xF1;
unsigned paddle2_position_y = 0xF1;
unsigned char ball_position_y = 0xFB;

unsigned char paddle1_left = 0xFD;
unsigned char paddle1_middle = 0xFB;
unsigned char paddle1_right = 0xF7;

unsigned char paddle2_left = 0xFD; 
unsigned char paddle2_middle = 0xFB;
unsigned char paddle2_right = 0xF7; 

unsigned char bottom = 0xEF;
unsigned char top = 0xFE;
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
			if (sensor_value < 450 && ((paddle2_position_y & 0x10) == 0x10)) { // Reset demo 
				paddle2_position_y = ((paddle2_position_y  << 1) | 0x01);
				paddle2_left = paddle2_left << 1;
				paddle2_middle = paddle2_middle << 1;
				paddle2_right = paddle2_right << 1;
			}else if (sensor_value > 650 && (paddle2_position_y != 0xF8)) { // Move LED to start of next row
				paddle2_position_y = ((paddle2_position_y  >> 1) | 0x80);
				paddle2_left = paddle2_left >> 1;
				paddle2_middle = paddle2_middle >> 1;
				paddle2_right = paddle2_right >> 1;
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
// 	static unsigned short sensor_value = 0x00;
	
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
			state = shift_button_wait;
			break;
	}	
	switch (state) {
		case shift_button_wait:	
			break;
		case shift_button_up:
			if(paddle1_position_y != 0xF8) {
				paddle1_position_y = ((paddle1_position_y>> 1) | 0x80);
				paddle1_left = paddle1_left >> 1;
				paddle1_middle = paddle1_middle >> 1;
				paddle1_right = paddle1_right >> 1;
			}
			
// 			PORTD = paddle1_position_y;
			break;
		case shift_button_down:
			if((paddle1_position_y & 0x10) == 0x10) {
				paddle1_position_y = ((paddle1_position_y<< 1) | 0x01);
				paddle1_left = paddle1_left << 1;
				paddle1_middle = paddle1_middle << 1;
				paddle1_right = paddle1_right << 1;
			}
// 			PORTD = paddle1_position_y;
			break;
			
		default:	
			break;
	}
	return state;
}

enum ball_physics{ball_moving_right_straight, ball_moving_right_up, ball_moving_right_down, ball_moving_left_up, ball_moving_left_down, ball_moving_left_straight};
int ball_physics_Tick(int state) {
	switch(state){
		case ball_moving_right_straight:
			if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_middle))){ //detects collision with middle of paddle
				state = ball_moving_left_straight;	
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xFE) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xEF) && ((~ball_position_y) == (~paddle2_right))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_down;
			}
			else{
				state = ball_moving_right_straight;	
			}
			break;
		case ball_moving_left_straight:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xFE) && ((ball_position_y) == (paddle2_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xEF) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_down;
			}
			else{
				state = ball_moving_left_straight;	
			}
			break;
		case ball_moving_left_down:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xFE) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xEF) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_down;
			}
			else if(ball_position_y == bottom){
				state = ball_moving_left_up;	
			}
			else{
				state = ball_moving_left_down;	
			}
			break;
		case ball_moving_left_up:
			if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xFE) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xEF) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_down;
			}
			else if(ball_position_y == top){
				state = ball_moving_left_down;	
			}
			else{
				state = ball_moving_left_up;	
			}
			break;
		case ball_moving_right_up:
			if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_middle))){ //detects collision with middle of paddle
				state = ball_moving_left_straight;	
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xFE) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xEF) && ((~ball_position_y) == (~paddle2_right))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_right))){
				state = ball_moving_left_down;
			}
			else if(ball_position_y == top){
				state = ball_moving_right_down;	
			}
			else{
				state = ball_moving_right_up;	
			}
			break;
		case ball_moving_right_down:
			if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_middle))){ //detects collision with middle of paddle
				state = ball_moving_left_straight;	
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xFE) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && (ball_position_y == 0xEF) && ((~ball_position_y) == (~paddle2_right))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_down;
			}
			else if(ball_position_y == bottom){
				state = ball_moving_right_up;	
			}
			else{
				state = ball_moving_right_down;	
			}
			break;
			
	}
	switch(state){
		case ball_moving_right_straight:
			ball_position_x >>= 1;
			break;
		case ball_moving_left_straight:
			ball_position_x <<= 1;
			break;
		case ball_moving_left_down:
			ball_position_x <<=1;
			ball_position_y = (ball_position_y << 1) | 0x01;
			break;
		case ball_moving_left_up:
			ball_position_x <<=1;
			ball_position_y = (ball_position_y >> 1) | 0x80;
			break;
		case ball_moving_right_down:
			ball_position_x >>=1;
			ball_position_y = (ball_position_y << 1) | 0x01;
			break;
		case ball_moving_right_up:
			ball_position_x >>=1;
			ball_position_y = (ball_position_y >> 1) | 0x80;
			break;
	}
	return state;	
}

enum display_states{change1, change2, change3};
int display(int state){
	switch (state) {
		case change1:	
			state = change2;
			break;
		case change2:	
			state = change3;
			break;
		case change3:	
			state = change1;
			break;	
		default:	
			state = change1;
			break;
	}	
	switch (state) {
		case change1:
// 			PORTC = 0xFF;
// 			PORTD = 0x00;
			PORTC = paddle1_position_x;
			PORTD = paddle1_position_y;
			break;
		case change2:
			PORTC = paddle2_position_x;
			PORTD = paddle2_position_y;
			break;
		case change3:
			PORTC = ball_position_x;
			PORTD = ball_position_y;
			break;
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
	
//     PORTC = 0x80;
//     PORTD = 0xF1;
    ADC_init();
    Set_A2D_Pin(0x01);
//     unsigned short sensor_value = 0;
    /* Insert your solution below */
    static task task1;
    static task task2;
    static task task3;
    static task task4;
	
    task *tasks[] = {&task1, &task2, &task3, &task4};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = 0;
    
    task1.state = start;
    task1.period = 200;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Joystick_Tick;
    
    task2.state = start;
    task2.period = 200;
    task2.elapsedTime = task2.period;
    task2.TickFct = &button_movement_Tick;
    
    task3.state = start;
    task3.period = 200;
    task3.elapsedTime = task3.period;
    task3.TickFct = &ball_physics_Tick;
	
    task4.state = start;
    task4.period = 1;
    task4.elapsedTime = task4.period;
    task4.TickFct = &display;
	
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
