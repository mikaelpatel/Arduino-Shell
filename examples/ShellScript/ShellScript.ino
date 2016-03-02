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
 *
 * Direct script strings: 8,296/431 bytes
 * Program memory strings: 8,346/393 bytes
 * Difference: +50/-38 bytes
 */

#include <Shell.h>

// Shell 16 depth stack and 16 variables
Shell<16,16> shell(Serial);

// Script buffer
#define USE_BUFFER
#if defined(USE_BUFFER)
const int BUF_MAX = 64;
char buf[BUF_MAX];
#define SCRIPT(s) strcpy_P(buf, PSTR(s))
#else
#define SCRIPT(s) (s)
#endif

void setup()
{
  memcpy_P(buf, F("\x00\x10\xFF\xCA\x00\x00\xA2\x7D"), 8);
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellScript: started"));

  // Turn on trace
  shell.trace(true);

  // : blinks ( n ms pin -- )
  //   dup output
  //   rot { dup high over delay dup low over delay } loop
  //   drop drop ;
  shell.execute(SCRIPT("{uOr{uHoDuLoD}ldd};\\blinks!"));

  // 5 1000 13 blinks
  shell.execute(SCRIPT("5,1000,13\\blinks:"));

  // Turn off trace
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
  shell.execute(SCRIPT("{oUuO{oR{1000}{200}eoHuDoLDT}w};\\monitor!"));

  // 2 13 monitor
  shell.execute(SCRIPT("2,13\\monitor:"));
}

void loop()
{
}
