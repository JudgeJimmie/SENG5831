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
	uint8_t red_speed = 1;
	uint8_t green_speed = 4;
	
	unsigned char receive_buffer_position = 0;
	char receive_buffer[32];
	
	uint8_t GreenRedToggle = 0;

int main()
{
	uint16_t counter_max = 5000000;
	uint8_t red_toggle = 0;
	uint8_t green_toggle = 0;
	uint16_t counter_green = 0;
	uint16_t counter_red = 0;
	
	// Set the baud rate to 9600 bits per second.  Each byte takes ten bit
	// times, so you can get at most 960 bytes per second at this speed.
	serial_set_baud_rate(USB_COMM, 9600);

	// Start receiving bytes in the ring buffer.
	serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));
	
	lcd_init_printf();
	clear();
	printf("Initializing");
	
	DDRD |= (1<<PORTD0);
	DDRD |= (1<<PORTD3);
	PORTD |= (1<<PORTD0);
	PORTD |= (1<<PORTD3);
	delay_ms(1000);
	PORTD ^= (1<<PORTD0);
	PORTD |= (1<<PORTD3);
	
	clear();
			printf("Red Speed = %d",red_speed);
			printf("\n");
			printf("Green Speed = %d",green_speed);
	while(1)
	{
		
		// USB_COMM is always in SERIAL_CHECK mode, so we need to call this
		// function often to make sure serial receptions and transmissions
		// occur.
		serial_check();

		// Deal with any new bytes received.
		check_for_new_bytes_received();
		
        unsigned char button_press = get_single_debounced_button_press(ANY_BUTTON);
		unsigned char button_release = get_single_debounced_button_release(ANY_BUTTON);

		if (button_press & TOP_BUTTON) {
			if (red_toggle) {
				red_toggle = 0;
			}
			else {
				red_toggle = 1;
				PORTD |= (1<<PORTD0);			
			}
		}
		if (button_press & BOTTOM_BUTTON) {
			if (green_toggle) {
				green_toggle = 0;
			}
			else {
				green_toggle = 1;
				PORTD |= (1<<PORTD3);
			}
		}
		
		if (button_press & MIDDLE_BUTTON){
			
			GreenRedToggle = !GreenRedToggle;
		}
		
		if (green_toggle && counter_green > counter_max) {
			green_led(TOGGLE);
			PORTD ^= (1<<PORTD3);
			counter_green = 0;
		}
		
		if (red_toggle && counter_red > counter_max) {
			red_led(TOGGLE);
			PORTD ^= (1<<PORTD0);	
			counter_red = 0;
		}
			
		if (green_toggle) {
			counter_green += green_speed;
		}
		
		if (red_toggle) {
			counter_red += red_speed;
		}
		
		if (!red_toggle) {
			counter_red = 0;
			PORTD &= ~(0x1 << PORTD0);
			red_led(0);
		}
		if (!green_toggle) {
			counter_green = 0;
			PORTD &= ~(0x1 << PORTD3);
			green_led(0);
		}
	}
}

void check_for_new_bytes_received()
{
	while(serial_get_received_bytes(USB_COMM) != receive_buffer_position)
	{
		// Process the new byte that has just been received.
		process_received_byte(receive_buffer[receive_buffer_position]);

		// Increment receive_buffer_position, but wrap around when it gets to
		// the end of the buffer.
		if (receive_buffer_position == sizeof(receive_buffer)-1)
		{
			receive_buffer_position = 0;
		}
		else
		{
			receive_buffer_position++;
		}
	}
}

void process_received_byte(char byte)
{
	switch(byte)
	{
		// If the character 'G' or 'g' is received, toggle the green LED.
		case '+':
			
			if (GreenRedToggle)
				red_speed++;
			else
				green_speed++;

			if (green_speed > 10) {
				green_speed = 1;
			}
			if (red_speed > 10) {
				red_speed = 1;
			}
			clear();
			printf("Red Speed = %d",red_speed);
			printf("\n");
			printf("Gr Speed = %d",green_speed);
			
			break;
			
		case '-':
		
					if (GreenRedToggle)
					red_speed--;
					else
					green_speed--;


			if (green_speed < 1) {
				green_speed = 10;
			}
			if (red_speed < 1) {
				red_speed = 10;
			}
			clear();
			printf("Red Speed = %d",red_speed);
			printf("\n");
			printf("Gr Speed = %d",green_speed);
			
			break;							
	}
}