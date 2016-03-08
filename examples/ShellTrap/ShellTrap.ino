/**
 * @file ShellTrap.ino
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
 * This Arduino sketch shows how to use the Shell library
 * as an interactive shell and trap extended instructions.
 */

#include <Shell.h>

typedef Shell<16,16> BaseShell;
class ExtendedShell : public BaseShell {
public:
  ExtendedShell(Stream& ios) : BaseShell(ios)
  {}
  virtual const char* trap(const char* ip)
  {
    char op = *ip++;
    m_ios.print(F("trap::op="));
    m_ios.println(op);
    return (ip);
  }
};

ExtendedShell shell(Serial);

const int BUF_MAX = 64;
char buf[BUF_MAX];
char* bp = buf;

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellTrap: started, use [Newline] mode"));
  shell.trace(true);
}

void loop()
{
  if (shell.read(bp)) {
    shell.execute(buf);
    bp = buf;
  }
}
