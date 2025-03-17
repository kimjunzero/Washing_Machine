#include "fnd.h"

// 포인터 배열에서 접근하기 위한 변수
extern volatile uint8_t timer_active;	// timer가 비활성화 되면 "StOP"을 출력하기 위함
extern volatile uint8_t motor_forward;	// 회전 방향에 맞게 출력하기 위함
extern volatile uint8_t wait_flag;
// Decimal 폰트 배열 정의:
uint8_t fnd_font[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xD8, 0x80, 0x98, 0x7F };
	
// 각 세그먼트 정방향 Circle 폰트
uint8_t fnd_font_forward_circle_0[] = { 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf6, 0xf4, 0xf0, 0x8c, 0xce };
uint8_t fnd_font_forward_circle_1[] = { 0xff, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf6, 0xf6, 0xf6, 0xf6, 0xc0, 0xC7 };
uint8_t fnd_font_forward_circle_2[] = { 0xff, 0xff, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0xce, 0x86 };
uint8_t fnd_font_forward_circle_3[] = { 0xff, 0xff, 0xff, 0xf7, 0xe7, 0xc7, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x92, 0x92 };

// 각 세그먼트 역방향 Circle 폰트
uint8_t fnd_font_reverse_circle_0[] = { 0xfb, 0xf9, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf0, 0x8c, 0xce };
uint8_t fnd_font_reverse_circle_1[] = { 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xf6, 0xf6, 0xc0, 0xc7 };
uint8_t fnd_font_reverse_circle_2[] = { 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xf6, 0xf6, 0xf6, 0xce, 0x86 };
uint8_t fnd_font_reverse_circle_3[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xde, 0xce, 0xc6, 0xc6, 0xc6, 0xc6, 0x92, 0x92 };

// 정방향 Circle 폰트 포인터 배열
uint8_t * fnd_font_forward_circle[] = {	fnd_font_forward_circle_0, fnd_font_forward_circle_1, fnd_font_forward_circle_2, fnd_font_forward_circle_3 };

// 역방향 Circle 폰트 포인터 배열
uint8_t * fnd_font_reverse_circle[] = { fnd_font_reverse_circle_0, fnd_font_reverse_circle_1, fnd_font_reverse_circle_2, fnd_font_reverse_circle_3 };

// Circle 폰트 포인터 배열 - 이중 포인터 배열로 통합
uint8_t ** fnd_font_circle[] = { fnd_font_forward_circle, fnd_font_reverse_circle };


void init_fnd(void)
{
	// PORTC의 모든 핀을 출력으로 설정
    FND_DATA_DDR = 0xFF; 
	 
	// 오른쪽 FND 디스플레이 - 분초시계
    FND_DIGIT_DDR |= (1 << FND_R_DIGIT_D1) | (1 << FND_R_DIGIT_D2) | (1 << FND_R_DIGIT_D3) | (1 << FND_R_DIGIT_D4);
    
    // 왼쪽 FND 디스플레이
    FND_DIGIT_DDR |= (1 << FND_L_DIGIT_D1) | (1 << FND_L_DIGIT_D2) | (1 << FND_L_DIGIT_D3) | (1 << FND_L_DIGIT_D4);
	
    FND_DATA_PORT = ~0x00;  // 모든 세그먼트 OFF (active-low 가정)
}

void fnd_display_0(uint32_t* seconds, uint32_t* half_seconds) 
{
    static int digit_select = 0;		// 현재 활성화된 자리를 기억 (0~3)
	volatile uint32_t idx = *half_seconds;	// 왼쪽 FND 출력 font index
	
	if (wait_flag == 0) {
		if(timer_active == 1){
			idx %= 12; // 0 ~ 11까지
		}
		else
			idx = 12;	// 대기 모드이면 STOP 출력
	}
	else {
		idx = 13;  // 선택 모드이면 SELT 출력
	}
    
    switch (digit_select)
    {
		// 오른쪽 FND 출력
        case 0:
            FND_DIGIT_PORT = 0x80;  // 우측부터 첫 번째 자리 활성화 (10000000)
            FND_DATA_PORT = fnd_font[*seconds % 10];  // 초의 일의 자리 (0~9)
            break;
            
        case 1:
            FND_DIGIT_PORT = 0x40;  // 우측부터 두 번째 자리 활성화 (01000000)
            FND_DATA_PORT = fnd_font[(*seconds / 10) % 6];  // 초의 십의 자리 (0~5)
            break;
            
        case 2:
            FND_DIGIT_PORT = 0x20;  // 우측부터 세 번째 자리 활성화 0x20(00100000) => PA7번 핀으로 변경
            // 추가 기능: 1초마다 점(특수 패턴)을 깜빡임
           // if ((*seconds) % 2)
           //     FND_DATA_PORT = fnd_font[10] & fnd_font[(*seconds / 60) % 10];
          //  else
                FND_DATA_PORT = fnd_font[(*seconds / 60) % 10];  // 분의 일의 자리 (0~9)
            break;
            
        case 3:
            FND_DIGIT_PORT = 0x10;  // 우측부터 네 번째 자리 활성화 (00010000)
            FND_DATA_PORT = fnd_font[(*seconds / 600) % 6];  // 분의 십의 자리 (0~5)
            break;
		
		// 왼쪽 FND 출력(애니메이션 용)
		case 4:
			FND_DIGIT_PORT = 0x08;  // 우측부터 첫 번째 자리
			FND_DATA_PORT = fnd_font_circle[motor_forward][0][idx];
			break;
		
		case 5:
			FND_DIGIT_PORT = 0x04;  // 우측부터 두 번째 자리
			FND_DATA_PORT = fnd_font_circle[motor_forward][1][idx];
			break;
		
		case 6:
			FND_DIGIT_PORT = 0x02;  // 우측부터 세 번째 자리
			FND_DATA_PORT = fnd_font_circle[motor_forward][2][idx];
			break;
		
		case 7:
			FND_DIGIT_PORT = 0x01;  // 우측부터 네 번째 자리
			FND_DATA_PORT = fnd_font_circle[motor_forward][3][idx];
			break;	
    }
    
    // 다음 자리로 이동 (순환 방식)
    digit_select = (digit_select + 1) % 8;
}