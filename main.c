#define F_CPU 8000000UL

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define NOTIF_LENGTH_MS 5
#define WARN_LOOPS  30

void init(void);

ISR (WDT_vect)
{
    /* The whole purpose of the WDT in this program is to wake up the chip at
     * regular intervals. The interrupt handler switches off the WDT, and the
     * main loop will switch it back on before powering down. */
    wdt_reset();
    MCUSR = 0;
    WDTCR |= (1<<WDCE) | (1<<WDE);
    WDTCR = 0;
}

int main(void)
{
    int door_state;
    int open_loops = 0;
    init();

    while (1) {
        /* Configure watchdog to wake up and then sleep for 1s to save power */
        {
            /* Disable interrupts to make sure the timed sequence happens */
            cli();
            
            /* Enable the watchdog */
            WDTCR |= (1<<WDE);

            /* Set timeout to 1s and enable interrupts */
            WDTCR = (1<<WDIE) | (1<<WDP2) | (1<<WDP1);

            /* Re-enable interrupts */
            sei();

            /* Now sleep. Also switch of ADC to save even more power */
            ADCSRA = 0;
            set_sleep_mode (SLEEP_MODE_PWR_DOWN);
            sleep_enable();
            sleep_cpu();
            
            /* We're awake. Disable the sleep flag just in case */
            sleep_disable();

        }

        /* Get door state */
        door_state = ((PINB & (1<<PB2))>>PB2);
        switch (door_state) {
            case 0:
                open_loops++;
                break;
            case 1:
                open_loops = 0;
                break;
        }

        /* Decide what to do based on loop counter */
        if (open_loops < WARN_LOOPS) {
            PORTB &= ~(1<<PB0);
            continue;
        }

        /* Beep */
        {
            TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);
            TCCR0B = (1<<CS01); /* 1/8 of clk */
            OCR0A = 128;
            _delay_ms(5);
            TCCR0A = 0;
        }
    }

    return 1;
}

void init(void) {
    /* Set PB0 as output */
    DDRB = 1<<PB0;

    /* Enable pull-up resistors for unused pins. Note PB2 has an external
     * pull-down resistor, PB5 is connected to Vcc, and PB0 is output. */
    PORTB = (1<<PB1)|(1<<PB3)|(1<<PB4);

    /* NOP is used for synchronization to allow PINB to be read from
     * accurately */
    _NOP();
}
