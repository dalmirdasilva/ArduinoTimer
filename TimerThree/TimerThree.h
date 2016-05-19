/*
 *  Interrupt and PWM utilities for 16 bit Timer3 on ATmega168/328
 */

#ifndef __ARDUINO_MODULE_TIMER_THREE_H__
#define __ARDUINO_MODULE_TIMER_THREE_H__

#include <avr/io.h>
#ifndef _AVR_IOM328P_H_

// Timer3 is 16 bit
#define TIMER_THREE_RESOLUTION 65536

class TimerThree {
public:

    unsigned int pwmPeriod;
    unsigned char clockSelectBits;

    /**
     * Initializes the timer one at 1000000 - 1Hz
     *
     * @param microseconds          The initial period.
     */
    void initialize(long microseconds = 1000000);

    /**
     * Starts the timer.
     */
    void start();

    /**
     * Stops the timer.
     */
    void stop();

    /**
     * Resumes the timer.
     */
    void resume();

    /**
     * Reads the timer.
     *
     * Returns the value of the timer in microseconds.
     */
    unsigned long read();

    /**
     * Sets the pwm to the given pin.
     *
     * @param pin           The pwm pin.
     * @param duty          Expects duty cycle to be 10 bit (1024)
     * @param microseconds  The Period.
     */
    void pwm(char pin, int duty, long microseconds = -1);

    /**
     * Disable pwm for the given pin.
     *
     * @param pin           The pin to disable pwm.
     */
    void disablePwm(char pin);

    /**
     * Attaches an interrupt.
     *
     * @param isr           The function pointer.
     * @param microseconds  The Period.
     */
    void attachInterrupt(void (*isr)(), long microseconds = -1);

    /**
     * Detach the attached interrupt.
     */
    void detachInterrupt();

    /**
     * Sets the period.
     *
     * @param microseconds  The period size.
     */
    void setPeriod(long microseconds);

    /**
     * Sets the duty cicle.
     *
     * @param pin           The pin.
     * @param duty          The duty cicle.
     */
    void setPwmDuty(char pin, int duty);

    /**
     * The pointer to the function.
     */
    void (*isrCallback)();

    void restart();
};

extern TimerThree Timer3;

#endif /* _AVR_IOM328P_H_ */
#endif /* __ARDUINO_MODULE_TIMER_THREE_H__ */
