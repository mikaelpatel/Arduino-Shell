/**
 * @file ShellAnalogPins.ino
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
 * execute scripts; generates output for Serial Plotter.
 */

#include <Shell.h>

Shell<> shell(Serial);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Periodically (100 ms) read analog pins (0..4) and write value.
  // {
  //   0 4 { analogRead . } loop cr
  //   100 delay true
  // } while

  shell.execute(SCRIPT("{0,4{A.}lm100DT}w"));
}

void loop()
{
}
