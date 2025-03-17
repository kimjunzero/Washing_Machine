/* fnd.h - 7-세그먼트 디스플레이 설정 및 함수 선언
*
* 이 헤더 파일은 7-세그먼트 디스플레이의 데이터 포트, 디지털(자리) 제어 포트,
* 그리고 각 디스플레이 모드를 초기화하고 업데이트하는 함수들의 프로토타입을 정의합니다.
*/

#define F_CPU   16000000UL  // CPU 동작 주파수: 16MHz
#include <avr/io.h>
#include <util/delay.h>

// 7-세그먼트 디스플레이 세그먼트 데이터 포트 및 데이터 방향 레지스터
#define FND_DATA_PORT       PORTC
#define FND_DATA_DDR        DDRC


// 7-세그먼트 디스플레이 자리(디지털) 제어 포트 및 데이터 방향 레지스터
#define FND_DIGIT_PORT      PORTA
#define FND_DIGIT_DDR       DDRA


// 4자리 디스플레이의 각 자리 제어 핀 (PORTB의 핀 번호)
// 오른쪽 디스플레이
#define FND_R_DIGIT_D1		4
#define FND_R_DIGIT_D2		5 
#define FND_R_DIGIT_D3		6
#define FND_R_DIGIT_D4		7

// 왼쪽 디스플레이
#define FND_L_DIGIT_D1		0
#define FND_L_DIGIT_D2		1
#define FND_L_DIGIT_D3		2
#define FND_L_DIGIT_D4		3

// 함수 프로토타입: 디스플레이 초기화 및 업데이트 함수
void init_fnd(void);
void fnd_display_0(uint32_t* seconds, uint32_t* half_seconds);

