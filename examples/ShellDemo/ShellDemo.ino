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

const int BUF_MAX = 64;
char buf[BUF_MAX];
char* bp = buf;

// Iterative factorial function
// : fac ( n -- n! ) 1 2 rot { * } loop ;
SCRIPT(fac, "1,2r{*}l");

// Fibonacci function
// : fib ( n -- fib(n) )
//   dup 1 > { dup 1- fib swap 2- fib + } if ;
SCRIPT(fib, "u1>{u1-`fib,s2-`fib+}i");

// Blink given pin, given number of times
// : blinks ( n ms pin -- )
//   dup output
//   rot 1 swap { drop dup high over delay dup low over delay } loop
//   drop drop ;
SCRIPT(blinks, "uOr1s{duHoDuLoD}ldd");

const script_t scripts[] PROGMEM = {
  SCRIPT_ENTRY(fac),
  SCRIPT_ENTRY(fib),
  SCRIPT_ENTRY(blinks),
  SCRIPT_NULL()
};

// #define USE_SHORT_OP_NAMES

#if defined(USE_SHORT_OP_NAMES)
Shell<32,32,false> shell(Serial, scripts);
#else
Shell<32,32> shell(Serial, scripts);
#endif

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellDemo: started, use [Newline] mode"));

  // Define a script in EEPROM
  // : demo ( -- ) 10 1000 13 blinks ;
  shell.execute(":demo{10,1000,13`blinks};");

  // Execute program memory script: list words
  shell.execute(F("Y"));
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
