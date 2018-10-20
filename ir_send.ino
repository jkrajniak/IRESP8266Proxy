/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */


#include <IRremote.h>

int S = 3;

IRsend irsend;

void setup()
{
}

void loop() {
	irsend.sendRC5(0xc60, 12);
	delay(1000); //5 second delay between each signal burst
}
