/* orangutan_app1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/29/2015 11:16:31 PM
 *  Author: jwfregie
 */

#include <pololu/orangutan.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"

#define DelayIterations 10390
#define interupts 1

#define RED 'R'
#define YELLOW 'Y'
#define GREEN 'G'
#define ALL 'A'

#define PRINT 'P'
#define ZERO 'Z'
#define TOGGLE 'T'

volatile uint64_t Red_Counter = 0;
volatile uint64_t Yellow_counter = 0;
volatile uint64_t Green_Counter = 0;


volatile uint64_t Red_Frequency_Counter = 0;
volatile uint64_t Yellow_Frequency_counter = 0;


volatile uint32_t speed = 0;
	
volatile uint64_t Red_Toggle = 0;
volatile uint64_t Green_Toggle = 0;
volatile uint64_t Yellow_Toggle = 0;

volatile uint64_t Red_Frequency = 1000;
volatile uint64_t Green_Frequency = 100;
volatile uint64_t Yellow_Frequency = 10;

// # Define commonly used methods
#define Toggle_Green_LED() ( PORTD ^= 1<<PORTD5 );
#define Toggle_Red_LED() (PORTA ^= (1<<PORTA2));
#define Toggle_Yellow_LED() (PORTA ^= (1<<PORTA0));

volatile uint_fast16_t ii = 0;
#define Delay10ms {for (ii=0;ii<DelayIterations;ii++) ; }
	

char Command;
char ColorRequested;
char MS_String[5];
uint8_t byte_num = 0;

int main()
{
	
	DDRA |= (1<<PORTA0 | 1<<PORTA2);
	DDRD |= (1<<PORTD5);
	
	// Init LCD Screen
	lcd_init_printf();
	printf("Initializing");
	
	Toggle_Green_LED();
	Toggle_Red_LED();
	Toggle_Yellow_LED();

	for(int i = 0;i<80;i++) {Delay10ms;}

	Toggle_Green_LED();
	Toggle_Red_LED();
	Toggle_Yellow_LED();

	clear();

	// Enable Global Interrupts
	if (interupts) {

		//**********************************************************************************
		// 8 Bit Timer Setup
		// Timer 0 Counter Control Register 0 Part A
		// Set Waveform Generation mode 2 WGM02 = 0; WGM01 = 1; WGM00 = 0 = CTC Mode
		//**********************************************************************************
		TCCR0A |= (0 << COM0A1 | 0 << COM0A0 | 1 << WGM01);
		
		// Set the Prescaler to 256
		TCCR0B |= (0 << CS00 | 0 << CS01 | 1 << CS02 | 0 << WGM02);
		
		// Interrupt to fire at 1KHZ 20,000,000 Hz / 256 / 78 = 1001Hz ~= 1KHZ = 1000 fires @ 1000 times a second = 1 interrupt every ms
		OCR0A = 0x4E;
		
		// Turn on interrupts for Timer 0
		TIMSK0 = 0x02;		

		//**********************************************************************************
		// 16 Bit Timer Setup CTC Mode.
		//**********************************************************************************
		TCCR3A = 0x00;
	
		// Set the prescaler to 64
		TCCR3B |= (1 << WGM32 | 0 << CS32 | 1 << CS31 | 1 << CS30);
	
		// 20,000,000 Hz / 64 / 31250 = 10Hz; Fires 10 times a second
		OCR3A = 0x7A12;
		
		// Turn on Interrupts for Timer 3
		TIMSK3 = 0x2;
		
		//**********************************************************************************
		// 16Bit PWM Timer Setup
		//**********************************************************************************
		TCCR1A |= (1<<COM1A1 | 1 << WGM11 | 0 << WGM10);
		TCCR1B |= (1 << WGM13 | 1 << WGM12 | 1 << CS12 | 0 << CS11 | 1 << CS10);

		// Initalize this interrupt to 1 Hz = 1 Interrupt every second
		ICR1 = 19531*2;
		
		// Toggle the interrupt at half of the frequency speed to toggle the LED on and off.
		OCR1A = ICR1 / 2;
		
		// Enable Interrupt for the counter to work: Turn on both the match and the overflow interrupt, as an LED action is performed on both
		TIMSK1 = 0x3;

		sei();
	} else {
		
		Delay10ms;
		//determine10msdelay();
	}
	
	//**********************************************************************************
	// Enable Serial Ports
	//**********************************************************************************
	serial_set_baud_rate(USB_COMM, 9600);

	// Start receiving bytes in the ring buffer.
	serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));

	while(1) {
		
		if (Red_Toggle) {
			Red_Frequency_Counter++;
			
			if (Red_Frequency_Counter >= Red_Frequency) {
				Toggle_Red_LED();
				Red_Counter++;
				Red_Frequency_Counter = 0;
			}
			
			Red_Toggle = 0;
		}
		
		// USB_COMM is always in SERIAL_CHECK mode, so we need to call this
		// function often to make sure serial receptions and transmissions
		// occur.
		serial_check();

		// Deal with any new bytes received.
		serial_check_for_new_bytes_received();
	}
}

// 1 KHZ Interrupt
ISR(TIMER0_COMPA_vect) 
{
	Red_Toggle = 1;
}

// 10 HZ Interrupt
ISR(TIMER3_COMPA_vect) {
	sei();
	Yellow_Frequency_counter++;
	
	if (Yellow_Frequency_counter >= Yellow_Frequency) {
		Yellow_counter++;
		Yellow_Frequency_counter = 0;
		Toggle_Yellow_LED();
	}
				for (int i = 0; i < 51;i++) {
					Delay10ms;
				}
}

// 10 HZ Interrupt
ISR(TIMER1_COMPA_vect) {
	Green_Counter++;

}

// 10 HZ Interrupt
ISR(TIMER1_OVF_vect) {

	Green_Counter++;

}

//****************************************************************************************
// This function was used to determine the 10390 Delay Iterations time for the 10ms loop
//*****************************************************************************************
void determine10msdelay() {
	
	volatile uint_fast16_t i = 0;
	
	volatile int aftertime;
	
	volatile int time;
	
	time = get_ms();
	for (i; i < 65500; i++);
	aftertime = get_ms();
	
	printf("Time_Ms: %d", 10 * (65500 / (aftertime - time)));
}

void processByte(char byte)
{

	// Turn a lower case letter to upper case
	if (byte >= 0x61 && byte <= 0x7A) {
		
		byte = byte - 0x20;
	}

	// Set up the command
	if (byte == 'Z' || byte == 'P' || byte == 'T') {
		
		Command = byte;

	} else if (byte == 'R' || byte == 'G' || byte == 'Y' || byte =='A') {
		
		ColorRequested = byte;

	} else if (byte >= 0x30 && byte <= 0x39) {
		
		MS_String[byte_num] = byte;
		byte_num++;
	
	// Execute the established Command
	} else if (byte == 'Q') {

		uint16_t Target_ms_Blink = atoi(MS_String);
																clear();
																lcd_goto_xy(0,0);
																printf("HELLO?:  %d", Target_ms_Blink);
							for (int __i = 0; __i < 5; __i++) {
								MS_String[__i] == '\0';
							}
		byte_num = 0;
		
		if (Command == 'P') {

			switch (ColorRequested)
			{
				case'A':
					clear();
					lcd_goto_xy(0,0);
					printf("R:%u" "Y:%u", Red_Counter, Yellow_counter);
					lcd_goto_xy(0,1);
					printf("Y:%i G:%lu",Yellow_counter, Green_Counter);
					break;
				case 'R': 
					clear();
					lcd_goto_xy(0,0);
					printf("R:%lu", Red_Counter);
					break;
				case 'Y':
					clear();
					lcd_goto_xy(0,0);
					printf("Y:%lu", Yellow_counter);
					break;
					
				case 'G':
					clear();
					lcd_goto_xy(0,0);
					printf("G:%lu", Green_Counter);
					break;
			}
		} else if (Command == 'Z') {
			
			switch(ColorRequested) {
				case 'A':
				Red_Counter = 0;
				Yellow_counter = 0;
				Green_Counter = 0;
				break;
				
				case 'R':
				Red_Counter = 0;
				break;
				
				case 'Y':
				Yellow_counter = 0;
				break;
				
				case 'G':
				Green_Counter = 0;
				break;
			}
		} else if(Command == 'T') {
			
			switch(ColorRequested) {
						
				case 'A':
					Red_Frequency = Target_ms_Blink;
					Yellow_Frequency = Target_ms_Blink / 100;
						
					Green_Frequency = Target_ms_Blink;
						
					float tempFreq = Target_ms_Blink / 1000;
						
					// Limits of 1024 Prescaler = ICR1 = 1 = 19500Hz; ICR1 = 65555 = .3Hz
					// Ms is in KHZ, so 19.5 KHZ - .0003KHZ to miliseconds = 1 ms - 3000 ms means we are good to use this prescaler.
					// We can't input anything less than 1 ms into the system, and the fastest we can blink at is 19.5 KHZ, or roughly .05 ms. We can't blink any slower than once every 3 seconds.
					// 20,000,000/1024 = 19531
					ICR1 = (19531 * Green_Frequency / 1000) * 2;
						
					// We want to toggle half way to the top
					OCR1A = ICR1/2;
				break;
						
				case 'R':
					Red_Frequency = Target_ms_Blink;
				break;
						
				case 'Y':
					Yellow_Frequency = Target_ms_Blink / 100;
				break;
						
				case 'G':
					Green_Frequency = Target_ms_Blink;
					ICR1 = (19531 * Green_Frequency / 1000) * 2;
					// We want to toggle half way to the top
					OCR1A = ICR1/2;	
				break;
			}
		}
	}
	/*
	serial_wait_for_sending_to_finish();
	serial_send_buffer[0] = byte;
	serial_send(USB_COMM, serial_send_buffer, 1);
	*/
}

	