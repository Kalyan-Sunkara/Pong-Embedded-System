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
#include "stdlib.h"
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
unsigned char pause = 1;
unsigned char paddle1_position_x = 0x80;
unsigned char paddle2_position_x = 0x01;
unsigned char ball_position_x = 0x04;

unsigned paddle1_position_y = 0xF1;
unsigned paddle2_position_y = 0xF1;
unsigned char ball_position_y = 0xFB;
unsigned long int speed = 175;

unsigned char paddle1_left = 0xFD;
unsigned char paddle1_middle = 0xFB;
unsigned char paddle1_right = 0xF7;

unsigned char paddle2_left = 0xFD; 
unsigned char paddle2_middle = 0xFB;
unsigned char paddle2_right = 0xF7; 

unsigned char bottom = 0xEF;
unsigned char top = 0xFE;

int player1_scores_point = 0;
int player2_scores_point = 0;
unsigned char player1_score = 0x00;
unsigned char player2_score = 0x00;

int go_right = 0;
int win_display_flag = 0;
// int goal = 0;

enum menu_states {wait1, solo_ingame, duo_ingame};
int menu(int state){
		switch (state){
		case wait1:	
			if((~PINB & 0x01) == 0x01){
				state = solo_ingame;	
			}
			else if((~PINB & 0x02) == 0x02){
				state = duo_ingame;
			}
			else{
				state = wait1;	
			}
			break;
		case solo_ingame:
			if(game_running == 0){
				state = wait1;	
			}
			else{
				state = solo_ingame;	
			}
			break;
		case duo_ingame:
			if(game_running == 0){
				state = wait1;	
			}
			else{
				state = duo_ingame;	
			}
			break;
		default:	
			state = wait1;
			break;
		}
	switch (state) {
		case wait1:
			duo = 0;
			solo = 0;
			game_running = 0;
			PORTB = ((PORTB & 0x0F) | 0x10);
			break;
		case solo_ingame:
			solo = 1;
			duo = 0;
			game_running = 1;
			PORTB = ((PORTB & 0x0F) | 0x20);
			break;
		case duo_ingame:
			solo = 0;
			duo = 1;
			game_running = 1;
			PORTB = ((PORTB & 0x0F) | 0x40);
			break;
		default:	
			break;
	}
	return state;
}
enum Joystick_States {wait_for_game3, shift};
int Joystick_Tick(int state) {
	static unsigned short sensor_value = 0x00;
// 	if(solo == 1){
	switch (state) {
		case wait_for_game3:
			if((pause == 1) || (duo == 0)){
				state = wait_for_game3;	
			}
			else{
				state = shift;	
			}
			break;
		case shift:
			if(pause == 1){
				state = wait_for_game3;	
			}
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
				paddle2_left = ((paddle2_left << 1) | 0x01);
				paddle2_middle = ((paddle2_middle << 1) | 0x01);
				paddle2_right = ((paddle2_right << 1) | 0x01);
			}else if (sensor_value > 650 && (paddle2_position_y != 0xF8)) { // Move LED to start of next row
				paddle2_position_y = ((paddle2_position_y  >> 1) | 0x80);
				paddle2_left = ((paddle2_left >> 1) | 0x80);
				paddle2_middle = ((paddle2_middle >> 1) | 0x80);
				paddle2_right = ((paddle2_right >> 1) | 0x80);
			} 
			else { // Shift LED one spot to the right on current row
			}
			break;
		case wait_for_game3:
			break;
		default:
			break;
	}
// 	}
	return state;
}
enum button_movement {wait_for_game2, shift_button_wait, shift_button_up, shift_button_down};
int button_movement_Tick(int state) {
// 	static unsigned short sensor_value = 0x00;
	switch (state) {
		case wait_for_game2:
			if(pause == 1){
				state = wait_for_game2;	
			}
			else{
				state = shift_button_wait;	
			}
			break;
		case shift_button_wait:	
			if(pause == 1){
				state = wait_for_game2;	
			}
			else if(((~PINB & 0x01) == 0x01) & ((~PINB & 0x02) != 0x02)){
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
			if(pause == 1){
				state = wait_for_game2;
			}
			else if(((~PINB & 0x01) == 0x01) & ((~PINB & 0x02) != 0x02)){
				state = shift_button_up;	
			}
			else{
				state = shift_button_wait;	
			}
			break;
		case shift_button_down:
			if(pause == 1){
				state = wait_for_game2;
			}
			else if(((~PINB & 0x02) == 0x02) & ((~PINB & 0x01) != 0x01)){
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
		case wait_for_game2:
			break;
		case shift_button_wait:	
			break;
		case shift_button_up:
			if(paddle1_position_y != 0xF8) {
				paddle1_position_y = ((paddle1_position_y>> 1) | 0x80);
				paddle1_left = ((paddle1_left >> 1) | 0x80);
				paddle1_middle = ((paddle1_middle >> 1) | 0x80);
				paddle1_right = ((paddle1_right >> 1) | 0x80);
			}
			break;
		case shift_button_down:
			if((paddle1_position_y & 0x10) == 0x10) {
				paddle1_position_y = ((paddle1_position_y<< 1) | 0x01);
				paddle1_left = ((paddle1_left << 1) | 0x01);
				paddle1_middle = ((paddle1_middle << 1) | 0x01);
				paddle1_right = ((paddle1_right << 1) | 0x01);
			}
			break;
			
		default:	
			break;
	}
	return state;
}

enum ball_physics{wait_for_game, score1, score2, ball_moving_right_straight, ball_moving_right_up, ball_moving_right_down, ball_moving_left_up, ball_moving_left_down, ball_moving_left_straight, spin_left_up, spin_left_down, spin_right_up, spin_right_down};
int ball_physics_Tick(int state) {
	static unsigned int spin_right_up_flag = 0;
	static unsigned int spin_left_up_flag = 0;
	static unsigned int spin_left_down_flag = 0;
	static unsigned int spin_right_down_flag = 0;
	switch(state){
		case wait_for_game:
			if(pause == 1){
				state = wait_for_game;	
			}
			else if(go_right == 1){
				state = ball_moving_right_straight;
			}
			else{
				state = ball_moving_left_straight;	
			}
			break;
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
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_right)){
				state = spin_left_up;	
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_left)){
				state = spin_left_down;	
			}
			else if(ball_position_x == 0x01){
				state = score1;	
			}
			else{
				state = ball_moving_right_straight;	
			}
			break;
		case ball_moving_left_straight:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == 0xFE) && ((ball_position_y) == (paddle1_left))){
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
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_right)){
				state = spin_right_up;	
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_left)){
				state = spin_right_down;	
			}
			else if(ball_position_x == 0x80){
				state = score2;	
			}
			else{
				state = ball_moving_left_straight;	
			}
			break;
		case ball_moving_left_down:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == top) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == bottom) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_right)){
				state = spin_right_up;	
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_left)){
				state = spin_right_down;	
			}
			else if(ball_position_x == 0x80){
				state = score2;	
			}
			else if(ball_position_y == bottom){
				state = ball_moving_left_up;	
			}
			else{
				state = ball_moving_left_down;	
			}
			break;
		case ball_moving_left_up:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == top) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == bottom) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_right)){
				state = spin_right_up;	
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_left)){
				state = spin_right_down;	
			}
			else if(ball_position_x == 0x80){
				state = score2;	
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
			else if((ball_position_x == 0x02) && (ball_position_y == top) && ((ball_position_y) == (paddle2_left))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && (ball_position_y == bottom) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_right)){
				state = spin_left_up;	
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_left)){
				state = spin_left_down;	
			}
			else if(ball_position_x == 0x01){
				state = score1;	
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
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_right)){
				state = spin_left_up;	
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_left)){
				state = spin_left_down;	
			}
			else if(ball_position_x == 0x01){
				state = score1;	
			}
			else if(ball_position_y == bottom){
				state = ball_moving_right_up;	
			}
			else{
				state = ball_moving_right_down;	
			}
			break;
		case spin_right_up:
			if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_middle))){ //detects collision with middle of paddle
				state = ball_moving_left_straight;	
			}
			else if((ball_position_x == 0x02) && (ball_position_y == top) && ((ball_position_y) == (paddle2_left))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x02) && ((~ball_position_y) == (~paddle2_left))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && (ball_position_y == bottom) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_up;
			}
			else if((ball_position_x == 0x02) && ((ball_position_y) == (paddle2_right))){
				state = ball_moving_left_down;
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_left)){
				state = spin_left_down;	
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_right)){
				state = spin_left_up;	
			}
			else if(ball_position_x == 0x01){
				state = score1;	
			}
			else if(ball_position_y == top){
				state = ball_moving_right_down;	
			}
			else{
				state = spin_right_up;	
			}
			break;
		case spin_right_down:
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
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_left)){
				state = spin_left_down;	
			}
			else if((ball_position_x == 0x01) && (ball_position_y == paddle2_right)){
				state = spin_left_up;	
			}
			else if(ball_position_x == 0x01){
				state = score1;	
			}
			else if(ball_position_y == bottom){
				state = ball_moving_right_up;	
			}
			else{
				state = spin_right_down;	
			}
			break;
		case spin_left_down:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == top) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == bottom) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_right))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_left)){
				state = spin_right_down;	
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_right)){
				state = spin_right_up;	
			}
			else if(ball_position_x == 0x80){
				state = score2;	
			}
			else if(ball_position_y == bottom){
				state = ball_moving_left_up;	
			}
			else{
				state = spin_left_down;	
			}
			break;
		case spin_left_up:
			if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_middle))){ //detects collision with middle of paddle
				state = ball_moving_right_straight;	
			}
			else if((ball_position_x == 0x40) && (ball_position_y == top) && ((ball_position_y) == (paddle1_left))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x40) && ((~ball_position_y) == (~paddle1_left))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && (ball_position_y == bottom) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_up;
			}
			else if((ball_position_x == 0x40) && ((ball_position_y) == (paddle1_right))){
				state = ball_moving_right_down;
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_left)){
				state = spin_right_down;	
			}
			else if((ball_position_x == 0x80) && (ball_position_y == paddle1_right)){
				state = spin_right_up;	
			}
			else if(ball_position_x == 0x80){
				state = score2;	
			}
			else if(ball_position_y == top){
				state = ball_moving_left_down;	
			}
			else{
				state = ball_moving_left_up;	
			}
			break;
		case score1:
			state = wait_for_game;
			break;
		case score2:
			state = wait_for_game;
			break;
		default:
			state = wait_for_game;
			break;		
	}
	switch(state){
		case wait_for_game:
			break;
		case ball_moving_right_straight:
			speed = 200;
			ball_position_x >>= 1;
			break;
		case ball_moving_left_straight:
			speed = 200;
			ball_position_x <<= 1;
			break;
		case ball_moving_left_down:
			speed = 165;
			ball_position_x <<=1;
			ball_position_y = (ball_position_y << 1) | 0x01;
			break;
		case ball_moving_left_up:
			speed = 165;
			ball_position_x <<=1;
			ball_position_y = (ball_position_y >> 1) | 0x80;
			break;
		case ball_moving_right_down:
			speed = 165;
			ball_position_x >>=1;
			ball_position_y = (ball_position_y << 1) | 0x01;
			break;
		case ball_moving_right_up:
			speed = 165;
			ball_position_x >>=1;
			ball_position_y = (ball_position_y >> 1) | 0x80;
			break;
		case spin_left_up:
			speed = 190;
			if(spin_left_up_flag == 0){
				ball_position_x <<=1;
				spin_left_up_flag = 1;
			}
			else{
				ball_position_x <<= 1;
				ball_position_y = (ball_position_y >> 1) | 0x80;
				spin_left_up_flag = 0;
			}
			break;
		case spin_left_down:
			speed = 190;
			if(spin_left_down_flag == 0){
				ball_position_x <<= 1;
				spin_left_down_flag = 1;
			}
			else{
				ball_position_x <<= 1;
				ball_position_y = (ball_position_y << 1) | 0x01;
				spin_left_down_flag = 0;
			}
			break;
		case spin_right_down:
			speed = 190;
			if(spin_right_down_flag == 0){
				ball_position_x >>=1;
				spin_right_down_flag = 1;
			}
			else{
				ball_position_x >>=1;
				ball_position_y = (ball_position_y << 1) | 0x01;
				spin_right_down_flag = 0;
			}
			break;
		case spin_right_up:
			speed = 190;
			if(spin_right_up_flag == 0){
				ball_position_x >>=1;
				spin_right_up_flag = 1;
			}
			else{
				ball_position_x >>=1;
				ball_position_y = (ball_position_y >> 1) | 0x80;
				spin_right_up_flag = 0;
			}
			break;
		case score1:
			player1_scores_point = 1;
			break;
		case score2:
			player2_scores_point = 1;
			break;
	}
	return state;	
}
enum game_states{pre, hold, play, win, win_hold};
int game_SM(int state){
	switch(state){
		case pre:
			if(game_running == 0){
				state = pre;	
			}
			else{
				state = hold;	
			}
			break;
		case hold:
			if((~PINB & 0x04) == 0x04){
				state = play;	
			}
			else{
				state = hold;	
			}
			break;
		case play:
			if(player1_scores_point == 1){
				player1_score = ((player1_score << 1)| 0x01);
				if(player1_score == 0x07){
					state = win;	
				}
				else{
					go_right = 1;
					state = hold;	
				}
			}
			else if(player2_scores_point == 1){
				player2_score = ((player2_score << 1)| 0x01);
				if(player2_score == 0x07){
					state = win;	
				}
				else{
					go_right = 0;
					state = hold;	
				}	
			}
			else{
				state = play;
			}
			break;
		case win:
			state = win_hold;
			break;
		case win_hold:
		 	if(game_running == 0){
				state = pre;	
			}
			else{
				state = win_hold;
			}
			break;		
	}
	switch(state){
		case pre:
			player1_score = 0x00;
			player2_score = 0x00;
			break;
		case hold:
			paddle1_position_x = 0x80;
			paddle2_position_x = 0x01;
			ball_position_x = 0x08;
			paddle1_position_y = 0xF1;
			paddle2_position_y = 0xF1;
			ball_position_y = 0xFB;
			paddle1_left = 0xFD;
			paddle1_middle = 0xFB;
			paddle1_right = 0xF7;
			paddle2_left = 0xFD; 
			paddle2_middle = 0xFB;
			paddle2_right = 0xF7; 
			player1_scores_point = 0;
			player2_scores_point = 0;
// 			goal = 0;
			pause = 1;
			break;
		case play:
			pause = 0;
			break;
		case win:
// 			game_running = 0;
			player1_scores_point = 0;
			player2_scores_point = 0;
			pause = 1;
			win_display_flag = 1;
			break;	
		case win_hold:
			paddle1_position_x = 0x80;
			paddle2_position_x = 0x01;
			ball_position_x = 0x08;
			paddle1_position_y = 0xF1;
			paddle2_position_y = 0xF1;
			ball_position_y = 0xFB;
			paddle1_left = 0xFD;
			paddle1_middle = 0xFB;
			paddle1_right = 0xF7;
			paddle2_left = 0xFD; 
			paddle2_middle = 0xFB;
			paddle2_right = 0xF7; 
			break;
	}
	return state;
}
enum display_states{menu_display, win_display, change1, change2, change3};
int display(int state){
	switch (state) {
		case menu_display:
			if(game_running == 0){
				state = menu_display;	
			}
			else if(win_display_flag == 1){
				state = win_display;
			}
			else{
				state = change1;	
			}
			break;
		case win_display:
			if(game_running == 0){
				state = menu_display;	
			}
			else if(win_display_flag == 1){
				state = win_display;
			}
			else{
				state = change1;	
			}
			break;
		case change1:
			if(game_running == 0){
				state = menu_display;	
			}
			else if(win_display_flag == 1){
				state = win_display;
			}
			else{
				state = change2;
			}
			break;
		case change2:	
			if(game_running == 0){
				state = menu_display;	
			}
			else if(win_display_flag == 1){
				state = win_display;
			}
			else{
				state = change3;
			}
			break;
		case change3:	
			if(game_running == 0){
				state = menu_display;	
			}
			else if(win_display_flag == 1){
				state = win_display;
			}
			else{
				state = change1;
			}
			break;	
		default:	
			state = menu_display;
			break;
	}	
	switch (state) {
		case menu_display:
			PORTC = 0xFF;
			PORTD = 0x00;
			break;
		case win_display:
			PORTC = 0xAA;
			PORTD = 0x00;
			break;
		case change1:
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
enum reset_states{wait_reset, reset};
int reset_SM(int state){
	switch(state){
		case wait_reset:
			if((~PINB & 0x08) == 0x08){
				state = reset;	
			}
			else{
				state = wait_reset;	
			}
			break;
		case reset:
			state = wait_reset;
			break;
		default:
			state = wait_reset;
	}
	switch(state){
		case wait_reset:
			break;
		case reset:
			game_running = 0;
			pause = 1;
			player1_score = 0x00;
			player2_score = 0x00;
			player1_scores_point = 0;
			player2_scores_point = 0;
			win_display_flag = 0;
			break;	
		default:
			break;
	}
	return state;
}
enum LED_STATES{score_check};
int LED_SM(int state){
	switch(state){
		case score_check:
			state = score_check;
			break;
		default:
			state = score_check;
	}
	switch(state){
		case score_check:
			if(game_running == 1){
				PORTA = (PORTA | (player1_score << 2));
				PORTA = (PORTA | (player2_score << 5));	
			}
			else{
				PORTA = (PORTA & 0x00);	
			}
			break;
		default:
			break;
	}
	return state;	
}
enum AI_states{wait_for_game4, follow, stay};
int AI(int state){
	switch(state){
		case wait_for_game4:
			if((pause == 1) || (solo == 0)){
				state = wait_for_game4;	
			}
			else{
				state = stay;
			}
			break;
		case follow:
			if((pause == 1) || (solo == 0)){
				state = wait_for_game4;	
			}
			else if((rand() % 2) == 1){
				state = stay;	
			}
			else{
				state = follow;	
			}
			break;
		case stay:
			if((pause == 1) || (solo == 0)){
				state = wait_for_game4;	
			}
			else if((rand() % 2) == 0){
				state = follow;	
			}
			else{
				state = stay;	
			}
			break;
		default:
			break;	
	}
	switch(state){
		case wait_for_game4:
			break;
		case follow:
			if (ball_position_y > paddle2_left) { // Reset demo 
				paddle2_position_y = ((paddle2_position_y  >> 1) | 0x80);
				paddle2_left = ((paddle2_left >> 1) | 0x80);
				paddle2_middle = ((paddle2_middle >> 1) | 0x80);
				paddle2_right = ((paddle2_right >> 1) | 0x80);
			}else if (ball_position_y < paddle2_right) { // Move LED to start of next row
				paddle2_position_y = ((paddle2_position_y  << 1) | 0x01);
				paddle2_left = ((paddle2_left << 1) | 0x01);
				paddle2_middle = ((paddle2_middle << 1) | 0x01);
				paddle2_right = ((paddle2_right << 1) | 0x01);
			}
			else{}
			break;
		case stay:
			break;
		default:
			break;
	}
	return state;	
}
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xFC; PORTA = 0x03;
    DDRB = 0xF0; PORTB = 0x0F;
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
    static task task5;
    static task task6;
    static task task7;
    static task task8;
    static task task9;
	
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8, &task9};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = 0;
    
	
    task1.state = start;
    task1.period = 200;
    task1.elapsedTime = task1.period;
    task1.TickFct= &menu;
	
    task2.state = start;
    task2.period = 150;
    task2.elapsedTime = task2.period;
    task2.TickFct = &game_SM;
	
    task3.state = start;
    task3.period = 1;
    task3.elapsedTime = task3.period;
    task3.TickFct = &display;
 
    task4.state = start;
    task4.period = 175;
    task4.elapsedTime = task4.period;
    task4.TickFct = &ball_physics_Tick;
	
    task5.state = start;
    task5.period = 150;
    task5.elapsedTime = task5.period;
    task5.TickFct = &button_movement_Tick;
    
	
    task6.state = start;
    task6.period = 150;
    task6.elapsedTime = task6.period;
    task6.TickFct = &Joystick_Tick;
    
    
    task7.state = start;
    task7.period = 100;
    task7.elapsedTime = task7.period;
    task7.TickFct = &LED_SM;

    task8.state = start;
    task8.period = 100;
    task8.elapsedTime = task8.period;
    task8.TickFct = &reset_SM;
    
    task9.state = start;
    task9.period = 150;
    task9.elapsedTime = task9.period;
    task9.TickFct = &AI;
	
    TimerSet(1);
    TimerOn();
    
    unsigned short x;
    while (1) {
          for(x = 0; x < numTasks; x++){
		    if(x == 3){
			 tasks[3]->period = speed;   
		    }
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
