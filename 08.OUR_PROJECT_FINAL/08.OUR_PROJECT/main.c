#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "button.h"
#include "fnd.h"
#include "pwm.h"
#include "def.h"
#include "led.h"

#define WAIT_MODE     0 // 대기 모드
#define TIME_SET_MODE 1 // 시간 설정 모드
#define WASHING_MODE  2 // 세탁 모드

// LED 핀 정의
#define BUZZER_PIN  PB5  // 피에조 핀

// 전역 변수: 시간 측정 및 디스플레이 제어에 사용 
// (volatile로 선언하여 ISR과 공유)
volatile uint8_t timer_active = 0;					// 1 : 활성 <-> 0 : 정지
volatile uint8_t motor_active = 0;					// 1 : 활성 <-> 0 : 정지
volatile uint8_t motor_forward = 1;					// 1 : forward <-> 0: reverse

volatile uint32_t milliseconds = 0;					// 밀리초 카운터 (타이머 ISR에서 증가)
volatile uint32_t milliseconds_1 = 0;	
volatile uint32_t displayUpdateCounter = 0;			// 디스플레이 갱신 주기 제어 변수

volatile uint32_t seconds = 0;						// 초 단위 카운터 (milliseconds에서 1초마다 증가)
volatile uint32_t half_seconds = 0;					// 0.5초 단위 카운터 (milliseconds에서 1초마다 증가)
volatile uint32_t speed = 5;						// 모터 속도를 왼쪽 FND(Circle)에 출력하기 위한 변수

// 포인터를 사용해 디스플레이 함수에 시간 데이터를 전달
volatile uint32_t* f_sec = &seconds;
volatile uint32_t* f_hsec = &half_seconds;
uint32_t wash_times[3] = {10, 10, 10}; // [세탁 시간, 헹굼 시간, 탈수 시간]
uint8_t wash_stage = 0;  // 현재 진행 중인 단계 (0: 세탁, 1: 헹굼, 2: 탈수)
uint8_t wash_running = 0; // 세탁 진행 중 여부
uint8_t time_setting_stage = 0; // 0: 세탁, 1: 헹굼, 2: 탈수
uint8_t wait_flag = 0; // 초기상태 모드 0이면 대기모드 1이면 시간설정 모드.
uint8_t mode = 0; // 각 모드를 담기위한 변수

void init_all(void);
void motor_right_left(void);
void update_led_display(uint8_t stage); // 세탁, 헹굼, 탈수 모드에 따라 LED 표시
extern void UART0_print_buffer(void);

void motor_on(void); // 모터 on 시키기 위한 초기화 설정
void motor_dwon(void); // 모터 off 시키기 위한 초기화 설정

// UART 관련 함수 선언
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern volatile uint8_t rx_buff[80];   // uart0로 부터 들어온 문자를 저장 하는 버퍼(변수)
extern volatile uint8_t rx_msg_received;
extern volatile uint8_t print_flag;
extern char print_buffer[];
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE); // for printf

// piezo 관련 함수 선언
extern void Music_Player(int *tone, int *Beats);
extern void init_speaker(void);
extern void Beep(int repeat);
extern const int my_song[]; // 종료 음악 음계
const int my_song_Beats[];  // 종료 음악 비트

// Timer0 오버플로우 인터럽트 벡터
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 6;					// 타이머 카운터 재설정
	if (timer_active == 1)
	{
		milliseconds++;					// 밀리초 카운터 증가
		milliseconds_1 += (speed % 6);	// motor speed에 따라 증가량 변동
	}
	displayUpdateCounter++;				// 디스플레이 갱신 신호
}

// 1초마다 실행되는 타이머 인터럽트 (시간 감소)
ISR(TIMER1_COMPA_vect)
{
	if (timer_active && wash_running && seconds > 0) 
	{
		seconds--;  // 1초 감소
		print_flag = 1;
		
		if (seconds == 0) // 초가 0이면 모터 종료
		{
			motor_down();
			print_flag = 1;
		}
	}
}

int main(void)
{
    // 모든 주변장치 초기화
    init_all();
 
    while (1)
    {
        pc_command_processing(); // UART 처리 함수
		UART0_print_buffer();    // COMPARTMASTER에 출력
		
		if (timer_active)
        {
            if (milliseconds > 999)
            {
                milliseconds = 0;
                if (seconds > 0)
                {
                    seconds--;
                    fnd_display_0(&seconds, f_hsec);
                }
                else
                {
                    wash_stage++;  // 세탁 단계 진행 (세탁 → 헹굼 → 탈수)
                    if (wash_stage < 3)
                    {
                        seconds = wash_times[wash_stage]; // 각 모드의 설정 시간을 sceonds에 담는다.
                        motor_active = 1;
                        OCR3C = 250;
                        
                        motor_right_left(); // 모터 방향 설정
                        
                        fnd_display_0(&seconds, f_hsec);
                        update_led_display(wash_stage);
                    }
                    else
                    {
                        half_seconds = 0; 
                        motor_down();
                        mode = WAIT_MODE;
                        update_led_display(255);
						
						Music_Player(my_song, my_song_Beats);  //세탁 종료 노래
                    }
                }
            }
        }
        
        // 0.5초 Count (탈수 모드에서는 속도를 2배 빠르게)
        if ((wash_stage == 2 && milliseconds_1 > 249) || (wash_stage != 2 && milliseconds_1 > 499))
        {
            milliseconds_1 = 0;
            half_seconds++;
        }

        // 7-Segment Display 갱신
        if (displayUpdateCounter > 1) 
        {
            displayUpdateCounter = 0;
            
            // 출력 함수 호출
            fnd_display_0(f_sec, f_hsec);
        }

        // 0번 버튼 입력
        if (get_button(BUTTON0, BUTTON0PIN))
        {
            wait_flag = !wait_flag; // 0이면 대기모드 1이면 시간 설정모드 (FND에 표시되기 위함)
			switch (mode)
            {
                case WAIT_MODE:
					mode = TIME_SET_MODE;  // 대기 모드 → 시간 설정 모드 전환
					timer_active = 0;  // 타이머 정지
					motor_active = 0;  // 모터 정지
					OCR3C = 0;         // 모터 PWM 0으로 설정
					break;

                case TIME_SET_MODE:
					mode = WAIT_MODE;  // 시간 설정 모드 → 대기 모드 전환
					timer_active = 0;  // 타이머 정지
					motor_active = 0;  // 모터 정지
					OCR3C = 0;         // 모터 PWM 0으로 설정
					break;

                case WASHING_MODE: // 세탁 모드였다가 0번 누르면 대기모드로 전환
					timer_active = !timer_active;  
					motor_active = !motor_active;
					OCR3C = (motor_active) ? 250 : 0;
					break;
            }
        }

        // 1번 버튼 입력
        else if (get_button(BUTTON1, BUTTON1PIN)) 
        {
            Beep(1);  // 버튼 처리 후 부저 작동
            if(mode == TIME_SET_MODE) // 시간 설정 모드일때 10씩 시간 증가
            {
			      if (seconds < 99) 
                  {
                       seconds += 10;
                       fnd_display_0(&seconds, f_hsec);
                  }
            }
        }

        // 2번 버튼 입력
        else if (get_button(BUTTON2, BUTTON2PIN))
        {
            Beep(1);  // 버튼 처리 후 부저 작동
			if(mode == TIME_SET_MODE)
            {
                if (seconds > 0) // 시간 설정 모드일때 시간 감소
                {
                    seconds-=10;
                    // FND에 업데이트된 값 표시
                    fnd_display_0(&seconds, f_hsec);
                }
            }
            
            if(mode == WAIT_MODE)
            {
                mode == WASHING_MODE;
				wash_stage = 0; // 세탁부터 시작
                wash_running = 1; // 세탁 진행 활성화
                seconds = wash_times[wash_stage]; // 현재 단계의 시간 설정
				
                motor_on();
                fnd_display_0(&seconds, f_hsec); 
            }
        }
        
        // 3번 버튼 입력 
        else if (get_button(BUTTON3, BUTTON3PIN)) // 세탁 시간 저장 버튼
        {
            Beep(1);  // 버튼 처리 후 부저 작동
			if (mode == TIME_SET_MODE) // 시간 설정 모드일 때만 동작
            {
                wash_times[time_setting_stage] = seconds;
                
                fnd_display_0(&seconds, f_hsec);
				time_setting_stage = (time_setting_stage + 1) % 3;
            }
            else if (mode == WAIT_MODE) // 대기 모드일때 시간 초기화.
            {
                 memset(wash_times, 0, sizeof(wash_times));
				 seconds = 0; 
                 motor_active = 0;         
                 motor_forward = 0;
				 OCR3C = 0;
                 fnd_display_0(&seconds, f_hsec); 
            }
        }
    }

    return 0;
}

// 타이머 카운트 0으로 초기화
void init_timer_0(void)
{
	// 초기 카운터값 설정
	TCNT0 = 6;
	
	// 분주비 256 : CS02 = 1, CS01 = 0, CS00 = 0
	TCCR0 |= (1 << CS02) | (0 << CS01) | (0 << CS00);
	
	// Timer0 오버플로우 인터럽트 허용
	TIMSK |= (1 << TOIE0);
}

// 모든 주변장치 초기화
void init_all(void)
{
	init_button();
	init_fnd();
	init_timer_0();
	init_timer3();
	init_L298N();
	init_speaker();	//부저 출력
	init_uart0();
	init_led();
	stdout = &OUTPUT;
	
	// 전역 인터럽트 허용
	sei();
}

// led(0~2) 초기화
void init_led(void)
{
	DDRB |= (1 << LED0_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN);     // LED 핀을 출력으로 설정
	PORTB &= ~((1 << LED0_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN)); // 초기 상태: 모든 LED OFF
}

// 현재 세탁 단계에 맞춰 LED 표시
void update_led_display(uint8_t stage)
{
	PORTB &= ~((1 << LED0_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN)); // 모든 LED OFF

	switch (stage)
	{
		case 0: PORTB |= (1 << LED0_PIN); break; // 세탁 LED0 ON
		case 1: PORTB |= (1 << LED2_PIN); break; // 헹굼 LED1 ON
		case 2: PORTB |= (1 << LED3_PIN); break; // 탈수 LED2 ON
		default: break; // 그 외는 모든 LED OFF 유지
	}
}

// 정회전, 역회전 설정 함수
void motor_right_left(void)
{
	PORTF &= ~((1 << 6) | (1 << 7)); // IN1, IN2 초기화 (모터 정지)

	if (wash_stage == 0 || wash_stage == 2)  // 세탁(정회전), 탈수(정회전)
	{
		PORTF |= (1 << 6); //  정회전
		motor_forward = 1;
	}
	else if (wash_stage == 1)  // 헹굼 모드 (사용자가 설정한 방향 반영)
	{
		PORTF |= (1 << 7);  // 역회전
		motor_forward = 0;
	}
}

// 모터_ON 하기 위한 초기화 함수
void motor_on()
{
	wash_running = 1;
	timer_active = 1;
	motor_active = 1;
	OCR3C = 250;
}

// 모터_OFF 하기 위한 초기화 함수
void motor_down()
{
	wash_running = 0;
	timer_active = 0;
	motor_active = 0;
	OCR3C = 0;
}