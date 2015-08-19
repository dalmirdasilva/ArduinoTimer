/*
 *  Interrupt and PWM utilities for 16 bit Timer1 on ATmega168/328
 */

#ifndef __ARDUINO_MODULE_TIMER_ONE_CPP__
#define __ARDUINO_MODULE_TIMER_ONE_CPP__

#include "TimerOne.h"

TimerOne Timer1;

/**
 * Interrupt service routine that wraps a user
 * defined function supplied by attachInterrupt
 */
ISR(TIMER1_OVF_vect) {
    Timer1.isrCallback();
}

void TimerOne::initialize(long microseconds) {

    // Clear control register A
    TCCR1A = 0;

    // Set mode 8: phase and frequency correct pwm, stop the timer
    TCCR1B = _BV(WGM13);

    setPeriod(microseconds);
}

void TimerOne::setPeriod(long microseconds) {

    // The counter runs backwards after TOP,
    // interrupt is at BOTTOM so divide microseconds by 2
    long cycles = (F_CPU / 2000000) * microseconds;

    if (cycles < TIMER_ONE_RESOLUTION) {

        // No prescale, full xtal
        clockSelectBits = _BV(CS10);
    } else if ((cycles >>= 3) < TIMER_ONE_RESOLUTION) {

        // Prescale by / 8
        clockSelectBits = _BV(CS11);
    } else if ((cycles >>= 3) < TIMER_ONE_RESOLUTION) {

        // Prescale by / 64
        clockSelectBits = _BV(CS11) | _BV(CS10);
    } else if ((cycles >>= 2) < TIMER_ONE_RESOLUTION) {

        // Prescale by / 256
        clockSelectBits = _BV(CS12);
    } else if ((cycles >>= 2) < TIMER_ONE_RESOLUTION) {

        // Prescale by /1024
        clockSelectBits = _BV(CS12) | _BV(CS10);
    } else {

        // Request was out of bounds, set as maximum
        cycles = TIMER_ONE_RESOLUTION - 1, clockSelectBits = _BV(CS12)
                | _BV(CS10);
    }

    oldSREG = SREG;
    cli();

    // Disable interrupts for 16 bit register access
    // ICR1 is TOP in p & f correct pwm mode
    ICR1 = pwmPeriod = cycles;
    SREG = oldSREG;

    TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));

    // Reset clock select register, and starts the clock
    TCCR1B |= clockSelectBits;
}

void TimerOne::setPwmDuty(char pin, int duty) {
    unsigned long dutyCycle = pwmPeriod;

    dutyCycle *= duty;
    dutyCycle >>= 10;

    oldSREG = SREG;
    cli();
    if (pin == 1 || pin == 9) {
        OCR1A = dutyCycle;
    } else if (pin == 2 || pin == 10) {
        OCR1B = dutyCycle;
    }
    SREG = oldSREG;
}

void TimerOne::pwm(char pin, int duty, long microseconds) {

    if (microseconds > 0) {
        setPeriod(microseconds);
    }
    if (pin == 1 || pin == 9) {

        // Sets data direction register for pwm output pin
        DDRB |= _BV(PORTB1);

        // Activates the output pin
        TCCR1A |= _BV(COM1A1);
    } else if (pin == 2 || pin == 10) {
        DDRB |= _BV(PORTB2);
        TCCR1A |= _BV(COM1B1);
    }
    setPwmDuty(pin, duty);

    // Lex - make sure the clock is running.
    // We don't want to restart the count,
    // in case we are starting the second WGM
    // and the first one is in the middle of a cycle
    resume();

}

void TimerOne::disablePwm(char pin) {

    if (pin == 1 || pin == 9) {

        // Clear the bit that enables pwm on PB1
        TCCR1A &= ~_BV(COM1A1);
    } else if (pin == 2 || pin == 10) {

        // Clear the bit that enables pwm on PB2
        TCCR1A &= ~_BV(COM1B1);
    }
}

void TimerOne::attachInterrupt(void (*isr)(), long microseconds) {
    if (microseconds > 0) {
        setPeriod(microseconds);
    }

    // Register the user's callback with the real ISR
    isrCallback = isr;

    // Sets the timer overflow interrupt enable bit
    TIMSK1 = _BV(TOIE1);

    resume();
}

void TimerOne::detachInterrupt() {

    // Clears the timer overflow interrupt enable bit
    TIMSK1 &= ~_BV(TOIE1);
}

void TimerOne::resume() {
    TCCR1B |= clockSelectBits;
}

void TimerOne::start() {
    unsigned int tcnt1;
    TIMSK1 &= ~_BV(TOIE1);

    // Reset prescaler
    GTCCR |= _BV(PSRSYNC);

    oldSREG = SREG;
    cli();
    TCNT1 = 0;
    SREG = oldSREG;

    // Wait until timer moved on from zero - otherwise get a phantom interrupt
    do {
        oldSREG = SREG;
        cli();
        tcnt1 = TCNT1;
        SREG = oldSREG;
    } while (tcnt1 == 0);
}

void TimerOne::stop() {

    // Clears all clock selects bits
    TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
}

unsigned long TimerOne::read() {

    unsigned long tmp;
    unsigned int tcnt1;

    oldSREG = SREG;
    cli();
    tmp = TCNT1;
    SREG = oldSREG;

    char scale = 0;
    switch (clockSelectBits) {
        case 1:

            // No prescalse
            scale = 0;
            break;
        case 2:

            // x8 prescale
            scale = 3;
            break;
        case 3:

            // x64
            scale = 6;
            break;
        case 4:

            // x256
            scale = 8;
            break;
        case 5:

            // x1024
            scale = 10;
            break;
    }

    // Max delay here is ~1023 cycles.
    do {
        oldSREG = SREG;
        cli();
        tcnt1 = TCNT1;
        SREG = oldSREG;
    } while (tcnt1 == tmp);

    // If we are counting down add the top value to how far we have counted down.
    tmp = ((tcnt1 > tmp) ? (tmp) : (long) (ICR1 - tcnt1) + (long) ICR1 );

    // Add casts and reuse previous TCNT1
    return ((tmp * 1000L) / (F_CPU / 1000L)) << scale;
}

#endif /* __ARDUINO_MODULE_TIMER_ONE_CPP__ */
