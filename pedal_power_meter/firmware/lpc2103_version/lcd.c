/*
             Pedal Power Meter
     Copyright (C) Jorge Pinto aka Casainho, 2009.

  casainho [at] gmail [dot] com
      www.casainho.net

 Released under the GPL Licence, Version 3
*/

#include "lcd.h"
#include "lpc210x.h"

/* LCD pins connections:
 * -LCD ------------------------|-connected to-----
 *  1  - GND                    | GND
 *  2  - Vcc                    | Vcc ~= + 5 volts
 *  3  - Contrast               | 0,91V
 *  4  - _RS                    | P0.19
 *  5  - _R/W                   | P0.18
 *  6  - E                      | P0.17
 *  11 - DB4                    | P0.16
 *  12 - DB5                    | P0.15
 *  13 - DB6                    | P0.14
 *  14 - DB7                    | P0.13
 */

#define RS      (1 << 19)
#define RW      (1 << 18)
#define E       (1 << 17)
#define DB4     (1 << 16)
#define DB5     (1 << 15)
#define DB6     (1 << 14)
#define DB7     (1 << 13)

unsigned int data;
void delay_ms(unsigned long a)
{
	while (--a != 0)
		;
}

void e_pulse(void)
{
	delay_ms(LCD_CTRL_K_DLY); /* delay */
	IOSET = E; /* set E to high */
	delay_ms(LCD_CTRL_K_DLY); /* delay */
	IOCLR = E; /* set E to low */
}

void lcd_send_command (unsigned char byte)
{
	/* set RS port to 0 -> display set to comand mode */
	/* set RW port to 0 */
	IOCLR = (RW | RS);
	delay_ms(200*LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */

	IOCLR = (DB4 | DB5 | DB6 | DB7); /* clear D4-D7 */
	data = (unsigned int) (byte >> 4); /* get the 4 high bits */
	/* set data */
	if (data & 1)
		IOSET = DB4;
	if (data & 2)
		IOSET = DB5;
	if (data & 4)
		IOSET = DB6;
	if (data & 8)
		IOSET = DB7;

	delay_ms(200*LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */
	e_pulse(); /* high->low to E port (pulse) */

	IOCLR = (DB4 | DB5 | DB6 | DB7); /* clear D4-D7 */
	data = (unsigned int) (byte & 0x0f); /* Get the 4 low bits */
	/* set data */
	if (data & 1)
		IOSET = DB4;
	if (data & 2)
		IOSET = DB5;
	if (data & 4)
		IOSET = DB6;
	if (data & 8)
		IOSET = DB7;

	delay_ms(200*LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */
	e_pulse(); /* high->low to E port (pulse) */
}

void lcd_init (void)
{
	/* Define all lines as outputs */
	IODIR = (RS| RW | E | DB4 | DB5 | DB6 | DB7);

	/* clear RS, E, RW */
	IOCLR = (RS | E | RW);
	delay_ms(5000*LCD_CTRL_K_DLY); /* delay ~50ms */

	IOSET = (DB4 | DB5);
	e_pulse(); /* high->low to E port (pulse) */
	delay_ms(1000*LCD_CTRL_K_DLY); //delay ~10ms

	IOSET = (DB4 | DB5);
	e_pulse();
	delay_ms(1000*LCD_CTRL_K_DLY); /* delay ~10ms */

	IOSET = (DB4 | DB5);
	e_pulse();
	delay_ms(1000*LCD_CTRL_K_DLY); /* delay ~10ms */

	IOCLR = DB4;
	IOSET = DB5;
	e_pulse();

	lcd_send_command(DISP_ON);
	lcd_send_command(CLR_DISP);
}

void lcd_send_char (unsigned char byte)
{
	/* set RS port to 1 -> display set to data mode */
	IOSET = RS;
	IOCLR = RW; /* set RW port to 0 */
	delay_ms(200 * LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */

	IOCLR = (DB4| DB5 | DB6 | DB7); /* clear D4-D7 */
	data = (unsigned int) (byte >> 4); /* get the 4 high bits */
	/* set data */
	if (data & 1)
		IOSET = DB4;
	if (data & 2)
		IOSET = DB5;
	if (data & 4)
		IOSET = DB6;
	if (data & 8)
		IOSET = DB7;

	delay_ms(200*LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */
	e_pulse(); /* high->low to E port (pulse) */

	IOCLR = (DB4 | DB5 | DB6 | DB7); /* clear D4-D7 */
	data = (unsigned int) (byte & 0x0f); /* get the 4 low bits */
	/* set data */
	if (data & 1)
		IOSET = DB4;
	if (data & 2)
		IOSET = DB5;
	if (data & 4)
		IOSET = DB6;
	if (data & 8)
		IOSET = DB7;

	delay_ms(200*LCD_CTRL_K_DLY); /* delay for LCD char ~2ms */
	e_pulse(); /* high->low to E port (pulse) */
}

void lcd_send_int (long number, unsigned char number_of_digits)
{
	volatile unsigned char 	C[17],
                            Temp = 0,
							NumLen = 0,
							temp1;

	number_of_digits--;
	temp1 = number_of_digits;
	do {
		Temp++;
		C[Temp] = (char) (((long) number) % ((long) 10));

		/* Decide to fill with "space" if digit is a 0 or fill with "0" */
		if (C[Temp] == 0)
		{
			if ((number_of_digits == temp1) || (number != 0))
				C[Temp] = 0; /* Print a "0" on right side of numbers and on
				 last one  */

			else
				C[Temp] = 32; /* Space char */
		}

		number = number / 10;
	} while (number_of_digits--);

	NumLen = Temp;
	for (Temp = NumLen; Temp > 0; Temp--)
	{
	    if(C[Temp] >= 0 && C[Temp] < 10)
		{
		    lcd_send_char (C[Temp] + 48);
		}

	    else
	        lcd_send_char (32);
	}
}

void lcd_send_float (double number, unsigned char number_of_digits, \
		unsigned char number_of_floats)
{
	lcd_send_int ((long) number, number_of_digits);
	lcd_send_char ('.');

	number = (number - ((long) number));

	while (number_of_floats--)
	{
		number = number * 10;
		lcd_send_char (((long) number) + 48);
		number = (number - ((long) number));
	}
}

void lcd_send_string (unsigned char *string)
{
	while (*string != 0)
	{
		lcd_send_char (*string);
		string++;
	}
}
