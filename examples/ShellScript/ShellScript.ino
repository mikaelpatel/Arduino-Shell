/**
 * @file ShellScript.ino
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
 * execute scripts.
 */

#include <Shell.h>

// : blinks ( n ms pin -- )
//   dup output
//   rot 1 swap { drop dup high over delay dup low over delay } loop
//   drop drop ;
SCRIPT(blinks, "uOr1s{duHoDuLoD}ldd");

// : monitor ( buttonPin ledPin -- )
//   over inputPullup
//   dup output
//   {
//      over digitalRead
//      { 1000 } { 200 } ifElse
//      over high dup delay over low delay
//      true
//   } while ;
SCRIPT(monitor, "oUuO{oR{1000}{200}eoHuDoLDT}w");

// Script table
const script_t scripts[] PROGMEM = {
  SCRIPT_ENTRY(blinks),
  SCRIPT_ENTRY(monitor),
  { NULL, NULL }
};

// Shell 16 depth stack and 16 variables, and application script table
Shell<16,16> shell(Serial, scripts);

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellScript: started"));
  shell.trace(true);
  shell.execute(F("5,1000,13`blinks"));
  shell.execute(F("2,13`monitor"));
}

void loop()
{
}
