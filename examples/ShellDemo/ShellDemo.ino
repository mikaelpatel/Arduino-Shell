/**
 * @file ShellDemo.ino
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
 * This Arduino sketch shows how to use the Shell library as
 * as an interactive shell.
 */

#include <Shell.h>

Shell<> shell(Serial);

const int BUF_MAX = 64;
char buf[BUF_MAX];
char* bp = buf;

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellDemo: started, use [Newline] mode"));

  // Program memory script: list words in dictionary
  // : words ( -- )
  //   0
  //   { dup 1+ swap .name
  //     over 8 mod 0= { cr } if
  //   } while
  //   8 mod 0<> { cr } if ;
  shell.set(F("words"), SCRIPT("0{u1+sto8%0={m}i}w8%0#{m}i"));

  // Program memory script: iterative factorial function
  // : fac ( n -- n! ) 1 2 rot { * } loop ;
  shell.set(F("fac"), SCRIPT("1,2r{*}l"));

  // Program memory script: blink given pin, given number of times
  // : blinks ( n ms pin -- )
  //   dup output
  //   rot 1 swap { drop dup high over delay dup low over delay } loop
  //   drop drop ;
  shell.set(F("blinks"), SCRIPT("uOr1s{duHoDuLoD}ldd"));

  // EEPROM script: blink led 10 times with 1000 ms
  // : demo ( -- ) 10 1000 13 blinks ;
  shell.execute("`demo{10,1000,13`blinks:};");

  // Execute program memory script: list words
  shell.execute(SCRIPT("`words:"));
  shell.trace(true);
}

void loop()
{
  // Read command line to execute
  if (shell.read(bp)) {
    shell.execute(buf);
    bp = buf;
  }
}
