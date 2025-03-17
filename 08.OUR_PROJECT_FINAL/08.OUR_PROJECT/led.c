/*
 * led.c
 *
 * Created: 2025-03-05 오전 10:21:53
 *  Author: microsoft
 */ 

#include "led.h"
#define FUNC_NUMER 6

int led_main(void);   // 선언
void led_all_on(void);
void led_all_off(void);
void shift_left_ledon(void);
void shift_right_ledon(void);
void shift_left_keep_ledon(void);
void shift_right_keep_ledon(void);
void flower_on(void);
void flower_off(void);
void state_transition(void);
void make_pwm_led_control(void);

extern volatile int msec_count;

int state=0;
void (*fp[]) () = 
{
	shift_left_ledon,
	shift_right_ledon,
	shift_left_keep_ledon,
	shift_right_keep_ledon,
	flower_on,
	flower_off
};


int led_main(void)   // 정의 
{
	DDRA = 0xff;   // PORTA에 연결된 pin8개를 output mode로 

#if 1
	while(1)
	{
		fp[state]();
	}
#else 	
	while(1) 
	{
		shift_left_ledon();  // state 0
		shift_right_ledon(); // state 1
		shift_left_keep_ledon();  // 2 
		shift_right_keep_ledon(); // 3
		flower_on(); // 4
		flower_off(); // 5
	}	
#endif 
}

void flower_on(void)
{
#if 1
	static int i=0;

	if (msec_count >= 100)
	{
		msec_count=0;
		if (i >= 4)
		{
			i=PORTA=0x00;
			state_transition();   // state를 천이 to shift_right_keep_ledon
		}
		else
		{
			PORTA |= 0x10 << i | 0x08 >> i++;
		}
	}
#else   // org 
	PORTA=0;
	for (int i=0; i < 4; i++)
	{
		PORTA |= 0x10 << i | 0x08 >> i;
		_delay_ms(30);
	}
#endif 
}

void flower_off(void)
{
	unsigned char h=0xf0;
	unsigned char l=0x0f;
	
#if 1
static int i=0;

if (msec_count >= 30)
{
	msec_count=0;
	if (i >= 4)
	{
		i=PORTA=0x00;
		state_transition();   // state를 천이 to shift_right_keep_ledon
	}
	else
	{
		PORTA = ( ((h >> i) & 0xf0) | ((l << i) & 0x0f));
		i++;
	}
}
#else
	PORTA=0;
	for (int i=0; i < 4; i++)
	{
		PORTA = (h >> i) & 0xf0 | (l << i) & 0x0f;
		_delay_ms(30);
	}
	PORTA=0;
	_delay_ms(30);
#endif 
}

void shift_left_keep_ledon(void)
{
#if 1
static int i=0;

	if (msec_count >= 30)
	{
		msec_count=0;
		if (i >= 8)
		{
			i = PORTA = 0x00;
			state_transition();  // state를 천이 to shift_right_keep_ledon
		}
		else
		{
			PORTA =PORTA |= 0b00000001 << i++;
		}
	}
#else
	for (int i=0; i < 8; i++)
	{
		PORTA |= 0b00000001 << i;
		//               1 i=0;
		//        00000010 i=1;
		//        10000000 i=7;
		_delay_ms(30);
	}
#endif 	
}
void shift_right_keep_ledon(void)
{
#if 1
	static int i=0;

	if (msec_count >= 30)
	{
		msec_count=0;
		if (i >= 8)
		{
			i=PORTA=0x00;
			state_transition();   // state를 천이 to shift_right_keep_ledon
		}
		else
		{
			PORTA |= 0b10000000 >> i++;
		}
	}
#else 
   static int i=0;
	   
	PORTA=0;
	for (int i=0; i < 8; i++)
	{
		PORTA |= 0b10000000 >> i;
		//        10000000 i=0;
		//        01000000 i=1;
		_delay_ms(30);
	}	
#endif 
}
void shift_left_ledon(void)
{
#if 1 
   // (1)for문 제거  (2) _delay_ms(30) 제거
   static int i=0;
   
   if (msec_count >= 30)
   {
	   msec_count=0;
	   if (i >= 8)
	   {
		   i=PORTA=0;
		   state_transition();
	   }
	   else
	   {
		   PORTA = 0b00000001 << i++;  // (1) PORTA = 0b00000001 << i  (2) i++
	   }
   } 
#else   // org
	for (int i=0; i < 8; i++)
	{
		PORTA = 0b00000001 << i;
		//               1 i=0;
	    //        00000010 i=1;
	    //        10000000 i=7;
		_delay_ms(30);
	}
#endif 		
}

void shift_right_ledon(void)
{
#if 1
	static int i=0;

	if (msec_count >= 30)
	{
		msec_count=0;
		if (i >= 8)
		{
			i=PORTA=0x00;
			state_transition();   // state를 천이 to shift_left_keep_ledon
		}
		else
		{
			PORTA = 0b10000000 >> i++;
		}
	}
#else  // org
	PORTA=0;
	for (int i=0; i < 8; i++)
	{
		PORTA = 0b10000000 >> i;
		//        10000000 i=0;
		//        01000000 i=1;
		_delay_ms(30);
	}
#endif 
}

void led_all_on(void)
{
	PORTA = 0xff;
}

void led_all_off(void)
{
	PORTA = 0x00;
}

void state_transition(void)
{
	state++;
	state %= FUNC_NUMER;
}


void make_pwm_led_control(void) // main에서 호출 필요
{
	int dim=0; // LED 밝기 조정 변수
	int direction=1; // 1: 밝기 증가 모드, -1: 밝기 감소 모드
	
	DDRA=0xff;
	PORTA=0xff;			//led all on으로 시작
	//dark --> bright, bright --> dark로 움직이는 것
	
	while (1)
	{
		//led on pwm 함수 넘겨주는것, 그리고 함수 빙글빙글
		
		//함수를 불러줘야지
		led_on_pwm(dim);
		dim += direction;
		if (dim == 255) direction = -1; // 밝기를 뒤집어 준다
		if(dim == 0) direction = 1; //다시 밝게 한다. 최대까지 어두워지면 최대까지 밝게 하도록 조절
		
	}
}

//led pwm 함수 먼저
void led_on_pwm(int dim)  //call by value
{
	PORTA=0xff;
	
	for(int i=0; i<256; i++)
	{
		if (i > dim)
		PORTA=0; // 끄고, duty cycle이 max를 넘어가면 led all off
		//그렇지 않으면 delay 줌
		_delay_us(20);
				
	}
	
}