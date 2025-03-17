#include "uart0.h"
#include "led.h"
#include <string.h> // strcpy strcmp strcmp...

#define COMMAND_NUMBER 10
#define COMMAND_LENGTH 20
#define PRINT_BUFFER_SIZE 128

volatile uint8_t rx_msg_received = 0;
extern volatile uint32_t seconds;  // 남은 세탁 시간 (버튼으로 설정된 값 사용)
extern volatile uint32_t wash_times[3]; // 버튼으로 설정한 값 저장
volatile char print_buffer[PRINT_BUFFER_SIZE];
volatile uint8_t print_flag = 0; // print_flag = 1이면 출력
void UART0_print_buffer(void);

ISR(USART0_RX_vect)
{
   volatile uint8_t rx_data;
   volatile static int i=0;
   
   rx_data = UDR0;  // uart0의 H/W register(UDR0)로 부터 1byte를 읽어 들인다. 
                    // rx_data = UDR0;를 실행하면 UDR0의 내용이 빈다.(empty)
   if (rx_data == '\n')
   {
      rx_buff[rear++][i] = '\0';
      rear %= COMMAND_NUMBER; // rear : 0 ~ 9
      i = 0; // 다음 string을 저장하기 위한 1차원 index값을 0으로
      // !!!! rx_buff queue full check 하는 logic 추가 
   }
   else
   {
      rx_buff[rear][i++] = rx_data;
      // COMMAND_LENGTH를 check하는 logic 추가
   }
}
/*
  1. 전송속도: 9600bps : 총글자수 :9600/10bit ==> 960자 
     ( 1글자를 송.수신 하는데 소요 시간 : 약 1ms)
  2. 비동기 방식(start/stop부호 비트로 데이터를 구분)
  3. RX(수신) : 인터럽트 방식으로 구현 
*/

void init_uart0(void)
{
   // 1. 9600bps로 설정
   UBRR0H = 0x00;
   UBRR0L = 207;  // 9600bps P219 표9-9
   // 2. 2배속 통신  표9-1
   UCSR0A |= 1 << U2X0;  // 2배속 통신 
   UCSR0C |= 0x06;   // 비동기/data8bits/none parity
   // P215 표9-1
   // RXEN0 : UART0로 부터 수신이 가능 하도록 
   // TXEN0 : UART0로 부터 송신이 가능 하도록 
   // RXCIE0 : UART0로 부터 1byte가 들어오면(stop bit가 들어 오면)) rx interrupt를 발생 시켜라
   UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
   
}

// UART0로 1byte를 전송 하는 함수 (polling방식)
void UART0_transmit(uint8_t data)
{
   // 데이터 전송 중이면 전송이 끝날떄 까지 기다린다. 
   while ( !(UCSR0A & 1 << UDRE0))
      ;   // no operation
   UDR0 = data;  // data를 H/W전송 register에 쏜다. 
}

// printf 출력 및 초기화
void UART0_print_buffer(void)
{
	if (print_flag) {
		printf("%s", print_buffer);
		print_flag = 0;
	}
}

void pc_command_processing(void)
{
	if (front != rear) {
		snprintf(print_buffer, PRINT_BUFFER_SIZE, "Received command: %s\n", rx_buff[front]);
		print_flag = 1;

		if (strncmp(rx_buff[front], "start", 5) == 0) {
			motor_on(); 
			if (seconds == 0) {
				seconds = wash_times[0];
			}

			motor_on();

			snprintf(print_buffer, PRINT_BUFFER_SIZE, "<Washing resumed> time = %d sec\n", seconds);
			
			print_flag = 1;
			UART0_print_buffer();  
		}

		else if (strncmp(rx_buff[front], "stop", 4) == 0) {
			motor_down();

			snprintf(print_buffer, PRINT_BUFFER_SIZE, "<Washing paused> time remaining: %d sec\n", seconds);
			print_flag = 1;
		}

		front = (front + 1) % COMMAND_NUMBER;
	}
}