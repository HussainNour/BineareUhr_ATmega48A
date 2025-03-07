#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#define F_CPU 10000000UL
#include <util/delay.h>


// Define EEPROM addresses for storing data
#define EEPROM_SECONDS_ADDR 0
#define EEPROM_MINUTES_ADDR 2
#define EEPROM_HOURS_ADDR   4
#define EEPROM_BRIGHTNESS_ADDR 6
//#define EEPROM_INIT_FLAG_ADDR 8 // EEPROM-Adresse für das Initialisierungsflag

volatile uint16_t hours ;
volatile uint16_t minutes ;
volatile uint16_t seconds ;
volatile int8_t brightness = 3;
volatile uint8_t powerSaveMode = 0;
volatile uint8_t powerDownMode = 0;
volatile uint8_t sec_Mode = 0;



void initLEDs();

void initTaste();

void init_PWM();
void enable_LEDs();
void disable_LEDs();

void initTimer();

void saveToEEPROM();
void loadFromEEPROM();


ISR(TIMER0_COMPA_vect) {
	enable_LEDs();
}

ISR(TIMER0_COMPB_vect) {
	disable_LEDs();
}

ISR(PCINT0_vect) {
	_delay_ms(10);
	
	if ((PINB & 0b00000111) == 0b00000110 ||
	(PINB & 0b00000111) == 0b00000101 ||
	(PINB & 0b00000111) == 0b00000011){
		// Gruppe 1: Tastenkombinationen, die in Gruppe 1 gehören
		// Führen Sie hier die Aktionen für Tastenkombinationen in Gruppe 1 aus
		if (!(PINB & (1 << PB0)) && !powerSaveMode && !powerDownMode) {
			_delay_ms(20); // Debouncing
			while (!(PINB & (1 << PB0))); // Wait for button release
			brightness = brightness - 3;
			init_PWM();
		}

		if (!(PINB & (1 << PB1)) && !powerDownMode) {
			_delay_ms(20); // Debouncing
			while (!(PINB & (1 << PB1))); // Wait for button release
			powerSaveMode = !powerSaveMode;
		}

		if (!(PINB & (1 << PB2))) {
			_delay_ms(20); // Debouncing
			while (!(PINB & (1 << PB2))); // Wait for button release
			powerDownMode = !powerDownMode;
			if (powerDownMode) {
				saveToEEPROM();
			}
		}
	}
	else {
		// Gruppe 2: Tastenkombinationen, die in Gruppe 2 gehören
		// Führen Sie hier die Aktionen für Tastenkombinationen in Gruppe 2 aus
		if (!(PINB & ((1 << PB0) | (1 << PB1)))) {
			_delay_ms(20); // Debouncing
			while (!(PINB & ((1 << PB0) | (1 << PB1)))); // Wait for button release
			sec_Mode = !sec_Mode;
			init_PWM();
		}

		if (!(PINB & ((1 << PB1) | (1 << PB2)))) {
			_delay_ms(20); // Debouncing
			while (!(PINB & ((1 << PB1) | (1 << PB2)))); // Wait for button release
			++minutes;
			if(minutes==60){
				minutes=0;
			}
			//init_PWM();
		}

		if (!(PINB & ((1 << PB0) | (1 << PB2)))) {
			_delay_ms(20); // Debouncing
			while (!(PINB & ((1 << PB0) | (1 << PB2)))); // Wait for button release
			++hours;
			if(hours==24){
				hours=0;
			}
			//init_PWM();
		}
	}
}











ISR(TIMER2_COMPA_vect) {
	seconds++;
	if (seconds == 60) {
		seconds = 0;
		minutes++;
		if (minutes == 60) {
			minutes = 0;
			hours++;
			if (hours == 24) {
				hours = 0;
			}
		}
	}
	PORTD ^= (1 << PD7); // Toggle LED
}

int main(void) {
	// Aktualisiere die Werte im EEPROM
	loadFromEEPROM();
	
	initLEDs();
	initTaste();
	
	init_PWM();
	
	initTimer();
	
	sei(); // Enable global interrupts
	
	while (1) {
		if (powerDownMode) {
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sleep_cpu(); // Enter Power-Down Mode
		}
		else if (powerSaveMode) {
			set_sleep_mode(SLEEP_MODE_PWR_SAVE);
			sleep_enable();
			sleep_cpu(); // Enter Power-Save Mode
		}
		else {
			sleep_disable();
			init_PWM();
		}
	}

	
	return 0;
}




void init_PWM() {
	TIMSK0 |= (1 << OCIE0B) | (1 << OCIE0A);
	TCCR0B  |= (1 << CS00) | (1 << WGM01);
	
	if (brightness > 10) {
		brightness = 10; // Begrenze die Helligkeit auf maximal 10
		} else if (brightness < -3) {
		brightness = 3; // Begrenze die Helligkeit auf minimal 0
	}
	
	// Begrenze die Helligkeit auf den Bereich von -3 bis 3
	if (brightness > 3) {
		brightness = 3;
		} else if (brightness < -3) {
		brightness = 3;
	}

	// Berechnung der PWM-Duty-Cycle-Werte basierend auf Helligkeitswert
	OCR0A = 126 + (50 * brightness); // Lineare Interpolation für OCR0A
	OCR0B = 126 - (50 * brightness); // OCR0B ist das Komplement von OCR0A

}

void enable_LEDs() {
	
	PORTC = hours;
	
	if(sec_Mode){
		PORTD = seconds;
	}
	else{
		PORTD= minutes;
	}
}


void disable_LEDs() {
	PORTD = 0;
	PORTC = 0;
}

void initTimer() {
	ASSR |= (1 << AS2);
	TCCR2A |= (1 << WGM21); // CTC mode
	TCCR2B |= (1 << CS22) | (1 << CS20); // Prescaler 128, start timer
	OCR2A = 255; // Set compare value for ~1-second interrupt
	TIMSK2 |= (1 << OCIE2A); // Enable compare match interrupt
}


void initTaste(){
	// Initialisiere Taster
	DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2)); // Set PB0, PB1, and PB2 as inputs for buttons
	PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2); // Enable internal pull-up resistors for buttons
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2); // Aktiviere PCINT0, PCINT1 und PCINT2 für Pin Change Interrupt für PB0, PB1, PB2
	PCICR |= (1 << PCIE0); // Aktiviere PCIE0 für PCINT0, PCINT1 und PCINT2
	
}
void initLEDs(){
	// Initialisiere Portrichtungen
	DDRD |= 0x3F; // Set PD0-PD5 as outputs for minutes
	DDRC |= 0x1F; // Set PC0-PC4 as outputs for hours
	PORTD &= 0x3F;
	PORTC &= 0x1F;
	DDRD |= (1 << PD7); // Set PD7 as output for LED
	PORTD &= (1 << PD7);
	
}

void saveToEEPROM() {
	eeprom_update_word((uint16_t*)EEPROM_SECONDS_ADDR, seconds);
	eeprom_update_word((uint16_t*)EEPROM_MINUTES_ADDR, minutes);
	eeprom_update_word((uint16_t*)EEPROM_HOURS_ADDR, hours);
	//	eeprom_update_byte((int8_t*)EEPROM_BRIGHTNESS_ADDR, brightness);
}


void loadFromEEPROM() {
	seconds = eeprom_read_word((uint16_t*)EEPROM_SECONDS_ADDR) %60;
	minutes = eeprom_read_word((uint16_t*)EEPROM_MINUTES_ADDR)%60;
	hours = eeprom_read_word((uint16_t*)EEPROM_HOURS_ADDR)%24;
	//	brightness = eeprom_read_byte((int8_t*)EEPROM_BRIGHTNESS_ADDR);
}


