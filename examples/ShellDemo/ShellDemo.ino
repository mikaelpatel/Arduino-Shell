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
 * This Arduino sketch shows how to use the Shell library.
 * 1. Execute a script.
 * 2. Interactive mode with commands on Serial.
 */

#include <Shell.h>

// Shell with 16 integer depth stack and serial output stream
Shell<16> shell(Serial);

// Configure; demo variants
#define SCRIPT_DEMO
#define INTERACTIVE_DEMO

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellDemo: started"));

#if defined(SCRIPT_DEMO)
  Serial.println(F("script mode..."));

  // : fun ( pin -- )
  //   1000 swap over over high delay low delay ;
  const char* fun = "1000sooHDLD";

  // : fie ( pin fn -- )
  //   over output over over execute over over execute execute ;
  const char* fie = "oOooxooxx";

  // 13 ' fun fie
  shell.trace(true);
  shell.push(13);
  shell.push(fun);
  shell.execute(fie);

  // : fum ( pin -- )
  //   dup output
  //   1000 swap
  //   10 { ( ms pin -- ms pin )
  //     over over high delay
  //     over over low delay
  //   }
  //   loop
  //   drop drop ;
  const char* fum = "dO1000s,10{ooHDooLD}luu";

  // 13 fum
  shell.push(13);
  shell.execute(fum);
#endif
#if defined(INTERACTIVE_DEMO)
  Serial.println(F("interactive mode..."));
#endif
}

void loop()
{
#if defined(INTERACTIVE_DEMO)
  const int BUF_MAX = 64;
  static char buf[BUF_MAX];
  static char* bp = buf;
  if (shell.read(bp)) {
    shell.execute(buf);
    bp = buf;
  }
#endif
}
