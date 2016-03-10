/**
 * @file ShellBlink.ino
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
 * implement the classical blink sketch as a script.
 */

#include <Shell.h>

Shell<16,16> shell(Serial);

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellBlink: started"));

  // Use shell trace mode
  shell.trace(true);

  // : blink ( ms pin -- )
  //   dup output
  //   {
  //      dup high over delay
  //      dup low over delay
  //      true
  //    } while ;
  shell.set(F("blink"), SCRIPT("uO{uHoDuLoDT}w"));

  // 1000 13 blink
  shell.execute(SCRIPT("1000,13`blink:"));
}

void loop()
{
}
