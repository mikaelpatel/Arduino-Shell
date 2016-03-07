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

// Shell 16 depth stack and 16 variables
Shell<16,16> shell(Serial);

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellScript: started"));

  // : blinks ( n ms pin -- )
  //   dup output
  //   rot { dup high over delay dup low over delay } loop
  //   drop drop ;
  shell.execute(SCRIPT("`blinks{uOr{uHoDuLoD}ldd};"));

  // 5 1000 13 blinks
  shell.trace(true);
  shell.execute(SCRIPT("5,1000,13`blinks:"));
  shell.trace(false);

  // : monitor ( buttonPin ledPin -- )
  //   over inputPullup
  //   dup output
  //   {
  //      over digitalRead
  //      { 1000 } { 200 } ifElse
  //      over high dup delay over low delay
  //      true
  //   } while ;
  shell.execute(SCRIPT("`monitor{oUuO{oR{1000}{200}eoHuDoLDT}w};"));

  // 2 13 monitor
  shell.trace(true);
  shell.execute(SCRIPT("2,13`monitor:"));
  shell.trace(false);
}

void loop()
{
}
