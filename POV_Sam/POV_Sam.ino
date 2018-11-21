//RadioShack POV Wand Arduino code
//by Amanda Ghassaei
//instructables.com/amandaghassaei
//April 2012

//for use with arduino uno or duemilanove with atmel328


/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

//in most of this code I have used the arduino portpin assignments to send data to pins, you can read more about how that works here: http://www.arduino.cc/en/Reference/PortManipulation
//I've also included (and commented) the standard arduino library commands that perform the same functions and the port commands in case you are interested


#include <avr/pgmspace.h>//need to store letter arrays in flash memory- or else we run out of space, more info here: http://arduino.cc/en/Reference/PROGMEM
#include <math.h>

/*******************************************************************************
  THIS NEXT SECTION IS WHAT YOU'LL WANT TO EDIT TO CREATE YOUR OWN MESSAGES
*******************************************************************************/
// setup
int sensorValue = 0;
volatile int delayScalar = 1;
String povtext = "MERRY BISHMAS";//PUT YOUR MESSAGE HERE!!- must be in all caps, spaces are fine, no punctuation
byte refreshrate = 1;//delay time for pixels to refresh in milliseconds- experiment with different values



//get length of string povtext
int dimtext = povtext.length();

//letterArray to make sure firmware is loaded correctly- each led should light up in order upon turning on
const boolean load[] = {
  1, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 1,
};


//incoming data storage
byte data = 0;//for portC

//variables
byte n; //variable for loops
byte t; //variable for loops
byte l; //variable for loops



//The letters of the alphabet- edit the look of these if you want
const boolean letterA[] PROGMEM = {
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
};

const boolean letterB[] PROGMEM = {
  0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
};

const boolean letterC[] PROGMEM = {
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
};

const boolean letterD[] PROGMEM = {
  0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
};

const boolean letterE[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
};

const boolean letterF[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

const boolean letterG[] PROGMEM = {
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
};

const boolean letterH[] PROGMEM = {
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
};

const boolean letterI[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
};

const boolean letterJ[] PROGMEM = {
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
};

const boolean letterK[] PROGMEM = {
  0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0,
};

const boolean letterL[] PROGMEM = {
  0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
};

const boolean letterM[] PROGMEM = {
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
};

const boolean letterN[] PROGMEM = {
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
};

const boolean letterO[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
};

const boolean letterP[] PROGMEM = {
  0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};

const boolean letterQ[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1,
};

const boolean letterR[] PROGMEM = {
  0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
};

const boolean letterS[] PROGMEM = {
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
};

const boolean letterT[] PROGMEM = {
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
};

const boolean letterU[] PROGMEM = {
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
};

const boolean letterV[] PROGMEM = {
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
};

const boolean letterW[] PROGMEM = {
  1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1,
  1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
};

const boolean letterX[] PROGMEM = {
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
};

const boolean letterY[] PROGMEM = {
  0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};

const boolean letterZ[] PROGMEM = {
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
};

// const char alphabet[] = {
//   'A','B','C','D','E','F'
}

void sendToWand(const boolean letterArray[]) { //function to get array data

  sensorValue = analogRead(A1);
  if (sensorValue > 100) {
    delayScalar = round(sensorValue / 10.0);
  } else {
    delayScalar = 5;
  }

  for (t = 0; t < 12; t++) { //for each time step

    for (l = 0; l < 8; l++) { //for next eight rows of data
      data = data << 1;//bitwise shift left
      data |= pgm_read_byte_near(letterArray + (l * 12 + t)); //add next value from dataset
    }

    //SET PINS:
    PORTD = data;
    delay(refreshrate * delayScalar);
    //clear data storage
    data = 0;
  }
}

void setup() {



  //port/pin assignments- set all pins to output- more info here: http://www.arduino.cc/en/Reference/PortManipulation
  DDRD = 0xFF;//port d- digital pins 0-7


  pinMode(A1, INPUT);
  //run intialization so we know device is working- leds should light up in order from top of wand to bottom
  for (byte j = 0; j < 20; j++) { //for each time step

    for (byte i = 0; i < 8; i++) { //for next eight rows of data
      data = data << 1;//bitwise shift left
      data |= load[(i * 20 + j)]; //add next value from dataset
    }
    PORTD = data;
    delay(100);
  }

  //clear data storage
  data = 0;
  //clear ports
  PORTD = data;

}

void loop() {

  //space at beginning of text
  PORTD = 0;
  delay(refreshrate * 3);

  for (n = 0; n < dimtext; n++) { //go through each character of povtext and call function sendToWand to display letter
    if (povtext.charAt(n) == 'A') {
      sendToWand(letterA);
    }
    else if (povtext.charAt(n) == 'B') {
      sendToWand(letterB);
    }
    else if (povtext.charAt(n) == 'C') {
      sendToWand(letterC);
    }
    else if (povtext.charAt(n) == 'D') {
      sendToWand(letterD);
    }
    else if (povtext.charAt(n) == 'E') {
      sendToWand(letterE);
    }
    else if (povtext.charAt(n) == 'F') {
      sendToWand(letterF);
    }
    else if (povtext.charAt(n) == 'G') {
      sendToWand(letterG);
    }
    else if (povtext.charAt(n) == 'H') {
      sendToWand(letterH);
    }
    else if (povtext.charAt(n) == 'I') {
      sendToWand(letterI);
    }
    else if (povtext.charAt(n) == 'J') {
      sendToWand(letterJ);
    }
    else if (povtext.charAt(n) == 'K') {
      sendToWand(letterK);
    }
    else if (povtext.charAt(n) == 'L') {
      sendToWand(letterL);
    }
    else if (povtext.charAt(n) == 'M') {
      sendToWand(letterM);
    }
    else if (povtext.charAt(n) == 'N') {
      sendToWand(letterN);
    }
    else if (povtext.charAt(n) == 'O') {
      sendToWand(letterO);
    }
    else if (povtext.charAt(n) == 'P') {
      sendToWand(letterP);
    }
    else if (povtext.charAt(n) == 'Q') {
      sendToWand(letterQ);
    }
    else if (povtext.charAt(n) == 'R') {
      sendToWand(letterR);
    }
    else if (povtext.charAt(n) == 'S') {
      sendToWand(letterS);
    }
    else if (povtext.charAt(n) == 'T') {
      sendToWand(letterT);
    }
    else if (povtext.charAt(n) == 'U') {
      sendToWand(letterU);
    }
    else if (povtext.charAt(n) == 'V') {
      sendToWand(letterV);
    }
    else if (povtext.charAt(n) == 'W') {
      sendToWand(letterW);
    }
    else if (povtext.charAt(n) == 'X') {
      sendToWand(letterX);
    }
    else if (povtext.charAt(n) == 'Y') {
      sendToWand(letterY);
    }
    else if (povtext.charAt(n) == 'Z') {
      sendToWand(letterZ);
    }
    else if (povtext.charAt(n) == ' ') {
      PORTD = 0;
      delay(refreshrate * 3); //off for 3 pixels
    }
    //space between each character
    PORTD = 0;
    delay(refreshrate);
  }

  //space at end of text
  PORTD = 0;
  delay(refreshrate * 3);

}

