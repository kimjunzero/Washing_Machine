﻿/*
 * uartultasonic.h
 *
 * Created: 2025-03-12 오후 2:49:40
 *  Author: microsoft
 */ 


#ifndef UARTULTASONIC_H_
#define UARTULTASONIC_H_
#define  F_CPU 16000000UL  // 16MHZ
#include <avr/io.h>
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h> // sei()등

#define TRIG 4
#define TRIG_PORT PORTG
#define TRIG_DDR  DDRG

#define ECHO 4
#define ECHO_PIN PINE // External INT 4
#define ECHO_DDR DDRE

 


#endif /* UARTULTASONIC_H_ */