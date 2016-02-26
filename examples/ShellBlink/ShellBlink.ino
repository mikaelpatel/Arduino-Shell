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

class NullDeviceClass : public Stream {
public:
  virtual size_t write(uint8_t) { return (1); }
  virtual int available() { return (0); }
  virtual int read() { return (-1); }
  virtual int peek() { return (-1); }
  virtual void flush() {}
} NullDevice;

Shell<16,16> shell(NullDevice);

void setup()
{
  // : blink ( ms pin -- )
  //   dup output
  //   {
  //      dup high over delay
  //      dup low over delay
  //      true
  //    } while ;
  const char* blink = "dO{dHoDdLoDT}w";
  shell.push(1000);
  shell.push(13);
  shell.execute(blink);
}

void loop()
{
}
