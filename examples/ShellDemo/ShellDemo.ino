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
  Serial.println(F("Type: `demo:"));

  // void blinks(int n, unsigned ms, int pin)
  // {
  //   pinMode(pin, OUTPUT);
  //   for (int i = 0; i < n; i++) {
  //     digitalWrite(pin, HIGH);
  //     delay(ms);
  //     digitalWrite(pin, LOW);
  //     delay(ms);
  //   }
  // }

  // : blinks ( n ms pin -- )
  //   dup output
  //   rot 1 swap { drop dup high over delay dup low over delay } loop
  //   drop drop ;

  shell.set(F("blinks"), SCRIPT("uOr1s{duHoDuLoD}ldd"));
  shell.execute("`demo{10,1000,13`blinks:};");
  shell.trace(true);
}

void loop()
{
  if (shell.read(bp)) {
    shell.execute(buf);
    bp = buf;
  }
}
