#include "pwm.h"

/*
	16bit Timer3 활용
	PWM 출력 신호 (3EA)
	==================
	PE3 : OC3A
	PE4 : OC3B (INT4 => 초음파 echo 펄스 external INT로 사용중)
	PE5 : OC3C => 모터 연결
	BTN0 : start / stop
	BTN1 : speed up (OC3C : 20씩 증가, max = 250)
	BTN2 : speed down (OC3C : 20씩 감소, min = 60)
*/

void init_timer3(void)
{
	DDRE |= 1 << 3 | 1 << 5;		//  현재 4번핀은 초음파 echo
	
	// 16bit Timer3의 모드 5 : 8비트 고속 PWM (p.348 표15-6)
	TCCR3A |= 1 << WGM30;
	TCCR3B |= 1 << WGM32;
	
	// 비반전 모드
	// 사용자가 설정한 PWM값에 일치시 OC3C의 출력핀이 LOW로 바뀌고 BOTTOM에 HIGH로 바뀐다.
	// p.350 표15-7
	TCCR3A |= 1 << COM3C1;
	
	// 분주비 : 64; 16000000 / 64 => 2500000Hz (250KHz)
	// T = 1/f = 1/2500000Hz => 0.000004sec(= 4us)
	// 2500000Hz에서 256개의 펄스를 count하면 소요 시간 = 1.02ms
	//				 127개							  = 0.5ms
	// p.318 표 14-1
	TCCR3B |= 1 << CS31 | 1 << CS30;	// 분주비 : 64
	OCR3C = 0;	// OCR(Output Compare Register) : PWM 값
}

// PF6 : IN1 (L298N)
// PF7 : IN2 (L298N)
// PF6(IN1), PF7(IN2)
//    0          0   : 정지
//    1          0   : 정회전
//    0          1   : 역회전
//    1          1   : STOP
void init_L298N(void)
{
	DDRF |= 1 << 6 | 1 << 7;		// 출력 모드로 설정
	PORTF &= ~(1 << 6 | 1 << 7);
	PORTF |= 1 << 6;				// 정회전
	OCR3C = 0;					// 회전하는 상태로 시작
}
