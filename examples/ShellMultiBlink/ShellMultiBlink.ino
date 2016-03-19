/**
 * @file ShellMultiBlink.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2016, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * @section Description
 * This Arduino sketch shows how to use the Shell library to
 * implement the classical blink without delay sketch as a script.
 * Toggle pin 13, 12 and 11 with different periods without delay.
 */

#include <Shell.h>

Shell<> shell(Serial);

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellMultiBlink: started"));

  // : multiblink ( -- )
  // 13 output 12 output 11 output
  // {
  //   500 :timer1 ?expired { 13 toggle } if
  //   300 :timer2 ?expired { 12 toggle } if
  //    10 :timer3 ?expired { 11 toggle } if
  //   true
  // } while;

  shell.set(F("multiblink"),
	    F("13O,12O,11O"
	      "{"
	      "  500:timer1,E{13X}i"
	      "  300:timer2,E{12X}i"
	      "  10:timer3,E{11X}i"
	      "  T"
	      "}w"));
  shell.execute(F("`multiblink"));
}

void loop()
{
}
