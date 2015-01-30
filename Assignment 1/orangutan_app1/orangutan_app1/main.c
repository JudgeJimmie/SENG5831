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

int main()
{
	lcd_init_printf();
	clear();
	printf("Waiting...");
	while(1)
	{
        unsigned char button = button_is_pressed(ANY_BUTTON);

		if (button & TOP_BUTTON)
            red_led(255);
		else
			red_led(0);
		if (button & BOTTOM_BUTTON)
			green_led(255);
        else
			green_led(0);
		delay_ms(250);
	}
}
