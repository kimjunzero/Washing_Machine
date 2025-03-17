/*
 * led.h
 *
 * Created: 2025-03-05 오전 10:21:21
 *  Author: microsoft
 */ 

#ifndef LED_H_
#define LED_H_
#define  F_CPU 16000000UL  // 16MHZ
#include <avr/io.h>
#include <util/delay.h>  // _delay_ms _delay_us
#define LED0_PIN    PB0  // 세탁 단계 LED
#define LED2_PIN    PB2  // 헹굼 단계 LED
#define LED3_PIN    PB3  // 탈수 단계 LED
#endif /* LED_H_ */