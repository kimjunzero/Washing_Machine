/*
 * uartultasonic.c
 *
 * Created: 2025-03-12 오후 2:49:54
 *  Author: microsoft
 */ 

#include "ultrasonic.h"
#include <stdio.h>
extern volatile int ultrasonic_check_timer;

void init_ultrasonic();
void trigger_ultrasonic();
void distance_ultrasonic();

volatile int ultrasonic_dis = 0; 
volatile char scm[50];

//P278 표 12-3
// PE4 : 외부 INT4 초음파 센서의 상승, 하강 엣지 둘다 INT가 ISR(INT4_vect)로 들어 온다.
// 결국 2번(상승: 1, 하강: 1) 들어 온다.
ISR(INT4_vect)
{
	// 1 상승에지
	if(ECHO_PIN & 1 << ECHO)
	{
		TCNT1 = 0;
	}
	else // 2 하강에지
	{
		// ECHO핀에 들어온 펄스 갯수를 us 환산
		ultrasonic_dis = 1000000.0 * TCNT1 * 1024 / F_CPU;
		// 예) TCINT에 10이 들어 있다고 가정 하자
		// 15.625KHZ의 주기 64us이다.
		// 0.000064sec(64us) * 10 = 0.00064sec(64us)
		// 640us / 58us(1cm이동 하는데 소요 시간) ==> 11cm이다.
		// --- 1cm : 58us
		sprintf(scm, "dis: %dcm\n", ultrasonic_dis / 58); // cm환산
	}
	
	// 2 하강에지
}
void init_ultrasonic(void)
{
	TRIG_DDR |= 1 << TRIG;
	ECHO_DDR &= !(1 << TRIG); // input mode ECHO_DDR &= 0b11110111;
	// P289 표 12-6
	// 0 1 : 상승엣지(rising edge)와 하강(falling edge) 둘다 INT를 띄우도록 요청.
	// 하드웨어가 소프트웨어에게 인터럽트하라고 콜을 보내는거다
	EICRB |= 0 << ISC41 | 1 << ISC40;// 1로 만들어주는건 OR연산 // INT4를 사용
	// 16bit timer1번을 설정을 해서 사용 65535(max) : 0xffff
	// 16MHz를 1024로 분주 16000000Hz/1024 --> 15625Hz --> 15.625KHz
	// 1주기 T(주기) = 1/f 1/15625 ==> 0.000064sec ==> 64us
	TCCR1B |= 1 << CS12 | 1<< CS10; // 1024로 분주. 타이머 0번 8비트, 타이머 1,3은 16비트 타이머
	// P318 표 14-1
	EIMSK |= 1 << INT4; // EXTERNAL INT 4(ECHO 핀)
}

// high신호를 줘서 초음파 신호를 주기 위함. delay를 통해 안전하게
void trigger_ultrasonic()
{
	TRIG_PORT &= ~(1 << TRIG); // low
	_delay_us(1); // 1usec
	TRIG_PORT |= 1 << TRIG; // high
	_delay_us(15); // 규격에는 10us인데 여유를 둬서 15us
	TRIG_PORT &= ~(1 << TRIG); // low
}

void distance_ultrasonic()
{
	if(ultrasonic_check_timer >= 1000) // 1초
	{
		ultrasonic_check_timer = 0;
		printf("%s", scm);
		trigger_ultrasonic();
	} 
}