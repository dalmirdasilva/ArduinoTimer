/*
 *  Interrupt and PWM utilities for 16 bit Timer3 on ATmega168/328
 */

#ifndef __ARDUINO_MODULE_TIMER_THREE_CPP__
#define __ARDUINO_MODULE_TIMER_THREE_CPP__

#include "TimerThree.h"
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef _AVR_IOM328P_H_

TimerThree Timer3;

ISR(TIMER3_OVF_vect) {

    // Interrupt service routine that wraps a user
    // defined function supplied by attachInterrupt
    Timer3.isrCallback();
}

void TimerThree::initialize(long microseconds) {

    // clear control register A
    TCCR3A = 0;

    // set mode as phase and frequency correct pwm, stop the timer
    TCCR3B = _BV(WGM13);
    setPeriod(microseconds);
}

void TimerThree::setPeriod(long microseconds) {

    // the counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2
    long cycles = (F_CPU * microseconds) / 2000000;
    if (cycles < TIMER_THREE_RESOLUTION) {

        // no prescale, full xtal
        clockSelectBits = _BV(CS10);
    } else if ((cycles >>= 3) < TIMER_THREE_RESOLUTION) {

        // prescale by /8
        clockSelectBits = _BV(CS11);
    } else if ((cycles >>= 3) < TIMER_THREE_RESOLUTION) {

        // prescale by /64
        clockSelectBits = _BV(CS11) | _BV(CS10);
    } else if ((cycles >>= 2) < TIMER_THREE_RESOLUTION) {

        // prescale by /256
        clockSelectBits = _BV(CS12);
    } else if ((cycles >>= 2) < TIMER_THREE_RESOLUTION) {

        // prescale by /1024
        clockSelectBits = _BV(CS12) | _BV(CS10);
    } else {

        // request was out of bounds, set as maximum
        cycles = TIMER_THREE_RESOLUTION - 1, clockSelectBits = _BV(CS12) | _BV(CS10);
    }
    ICR3 = pwmPeriod = cycles; // ICR1 is TOP in p & f correct pwm mode
    TCCR3B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
    TCCR3B |= clockSelectBits; // reset clock select register
}

void TimerThree::setPwmDuty(char pin, int duty) {
    unsigned long dutyCycle = pwmPeriod;
    dutyCycle *= duty;
    dutyCycle >>= 10;
    if (pin == 5) {
        OCR3A = dutyCycle;
    }
    if (pin == 2) {
        OCR3B = dutyCycle;
    }
    if (pin == 3) {
        OCR3C = dutyCycle;
    }
}

void TimerThree::pwm(char pin, int duty, long microseconds) {

    // expects duty cycle to be 10 bit (1024)
    if (microseconds > 0) {
        setPeriod(microseconds);
    }

    // sets data direction register for pwm output pin
    // activates the output pin
    if (pin == 5) {
        DDRE |= _BV(PORTE3);
        TCCR3A |= _BV(COM3A1);
    }
    if (pin == 2) {
        DDRE |= _BV(PORTE4);
        TCCR3A |= _BV(COM3B1);
    }
    if (pin == 3) {
        DDRE |= _BV(PORTE5);
        TCCR3A |= _BV(COM3C1);
    }
    setPwmDuty(pin, duty);
    start();
}

void TimerThree::disablePwm(char pin) {
    if (pin == 5) {
        TCCR3A &= ~_BV(COM3A1); // clear the bit that enables pwm on PE3
    }
    if (pin == 2) {
        TCCR3A &= ~_BV(COM3B1); // clear the bit that enables pwm on PE4
    }
    if (pin == 3) {
        TCCR3A &= ~_BV(COM3C1); // clear the bit that enables pwm on PE5
    }
}

void TimerThree::attachInterrupt(void (*isr)(), long microseconds) {
    if (microseconds > 0) {
        setPeriod(microseconds);
    }

    // register the user's callback with the real ISR
    isrCallback = isr;

    // sets the timer overflow interrupt enable bit
    TIMSK3 = _BV(TOIE1);
    sei();

    // ensures that interrupts are globally enabled
    start();
}

void TimerThree::detachInterrupt() {

    // clears the timer overflow interrupt enable bit
    TIMSK3 &= ~_BV(TOIE1);
}

void TimerThree::start() {
    TCCR3B |= clockSelectBits;
}

void TimerThree::stop() {

    // clears all clock selects bits
    TCCR3B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
}

void TimerThree::restart() {
    TCNT3 = 0;
}

#endif /* _AVR_IOM328P_H_ */
#endif /* __ARDUINO_MODULE_TIMER_THREE_CPP__ */
