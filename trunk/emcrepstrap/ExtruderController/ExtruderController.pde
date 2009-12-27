/*
 *  Extruder Firmware for Extruder Controller v2.x
 *
 *  Adopted by Jorge Pinto (casainho@gmail.com) for his own RepStrap, used with Linux EMC2 -- 2009.12.24
 *  Adopted by Sam Wong for his own RepStrap, used with Linux EMC2
 *
 *  License: GPLv2
 *
 *  Board documentation at: http://make.rrrf.org/ec-2.0
 *  Specification for the original communication protocol is located at: 
 *      http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  Original Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 *
 *********************************************************************************************************/

#ifndef __AVR_ATmega168__
#error Oops!  Make sure you have 'Arduino' selected from the boards menu.
#endif

#include <WProgram.h>
#include <stdint.h>
#include <avr/wdt.h>

#include "Configuration.h"
#include "Motor.h"
#include "Heater.h"
#include "Hardware.h"

char machine_on = 0;
unsigned long last_packet = 0;
unsigned char status[6];
static unsigned char coilPosition = 0;

/* This handles the Timer1 interrupt event.
 * On each event we must provide a new "step impulse" or stepper motor.
 * The stepper motor is a Nema 17 and is running at full steps: 200 steps per revolution.
 */
SIGNAL(TIMER1_COMPA_vect)
{  
  if (motor1.direction)
    coilPosition++;
  else
    coilPosition--;
    
  coilPosition &= 3;

  switch(coilPosition)
  {
    case 3:
    digitalWrite(H1D, 1);
    digitalWrite(H2D, 1);
    break;

    case 2:
    digitalWrite(H1D, 1);
    digitalWrite(H2D, 0);
    break;

    case 1:
    digitalWrite(H1D, 0);
    digitalWrite(H2D, 0);
    break;

    case 0:
    digitalWrite(H1D, 0);
    digitalWrite(H2D, 1);
    break;
    
  default:
    break;
  }

  analogWrite(H1E, 255);
  analogWrite(H2E, 255);
}

void setup()
{
  /* Disable Watchdog timer soon as possible... */
  wdt_disable();
  
  /* Init serial */
  Serial.begin(SERIAL_SPEED);
  
  /* Init hardware */
  heater1.init();
  motor1.init();
  turnOff();
  
  pinMode(DEBUG_PIN, OUTPUT); /* Init debug pin => debug LED */
  
  /* Enable the Watchdog timer with 250ms. After now, we must reset the timer before every 250ms... */
  wdt_enable(WDTO_250MS);
}

void loop()
{
    process_packets();
    heater1.manage();
    update_status();

    // If there wasn't any packet from host for a while, shut down the system
    if ((signed long) (millis() - last_packet) > 1000)
    {
        turnOff();
    }
    
    /* Reset the Watchdog timer.
     * If program do not reach here in every 250ms, MCU will reset (like if program hang in somewhere). */
    wdt_reset();
}

/*
 * Turn off the system.
 */
void turnOff()
{    
    machine_on = 0;

    motor1.turnOff();
    heater1.turnOff();
    update_status();
}

/*
 * Turn on the system.
 */
void turnOn()
{    
    machine_on = 1;

    motor1.turnOn();
    heater1.turnOn();
    update_status();
}

/*
 * Update the status.
 */
void update_status()
{
    memset(status, 0, 6);

    // [5] Heater on
    if (heater1.isHeaterOn())
	status[5] |= 1;
        
    if (machine_on)
	status[0] |= 2;
}

