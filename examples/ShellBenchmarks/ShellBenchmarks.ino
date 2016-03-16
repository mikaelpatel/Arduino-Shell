/**
 * @file ShellBenchmarks.ino
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
 * This Arduino sketch measures the Shell library performance.
 */

#include <Shell.h>

// Shell 16 depth stack and 16 variables
Shell<16,16> shell(Serial);

// Define to benchmark scripts in data memory
#ifdef USE_SRAM_SCRIPTS
#undef SCRIPT
#define SCRIPT(x) x
#endif

#define BENCHMARK(script)				\
  do {							\
    uint32_t start0 = micros();				\
    uint32_t start;					\
    while ((start = micros()) == start0);		\
    shell.execute(SCRIPT(script));			\
    uint32_t stop = micros();				\
    us = stop - start - baseline;			\
    Serial.print(F(script));				\
    Serial.print(':');					\
    Serial.print(us);					\
    Serial.print(':');					\
    shell.print();					\
    Serial.flush();					\
  } while (0)

void setup()
{
  Serial.begin(57600);
  while (!Serial);
  Serial.println(F("ShellBenchmarks: started"));
  Serial.flush();

  uint32_t us, baseline = 0;
  BENCHMARK("");
  baseline = us;

  BENCHMARK(" ");
  BENCHMARK(",");
  BENCHMARK("T");
  BENCHMARK("F");
  BENCHMARK("0");
  BENCHMARK("1");
  BENCHMARK("-1");
  BENCHMARK("10");
  BENCHMARK("-10");
  BENCHMARK("100");
  BENCHMARK("-100");
  BENCHMARK("s");
  BENCHMARK("ss");
  BENCHMARK("r");
  BENCHMARK("u");
  BENCHMARK("o");
  BENCHMARK("oo");
  BENCHMARK("d");
  BENCHMARK("j");
  BENCHMARK("c");

  BENCHMARK("10000,1000,100,10,1");
  BENCHMARK("~");
  BENCHMARK("n");
  BENCHMARK("+");
  BENCHMARK("-");
  BENCHMARK("*");
  BENCHMARK("/");
  BENCHMARK("1+");
  BENCHMARK("1-");
  BENCHMARK("0<");
  BENCHMARK("0>");
  BENCHMARK("C");

  BENCHMARK("0@");
  BENCHMARK("0!");

  BENCHMARK("{}x");

  BENCHMARK("F{}i");
  BENCHMARK("T{}i");

  BENCHMARK("F{}{}e");
  BENCHMARK("T{}{}e");

  BENCHMARK("{F}w");
  BENCHMARK("1{1-q}w");
  BENCHMARK("10{1-q}w");
  BENCHMARK("100{1-q}w");
  BENCHMARK("1000{1-q}w");

  BENCHMARK("1,0{}l");
  BENCHMARK("1,1{}l");
  BENCHMARK("1,1{d}l");
  BENCHMARK("1,10{d}l");
  BENCHMARK("1,100{d}l");
  BENCHMARK("1,1000{d}l");

  BENCHMARK("1D");
  BENCHMARK("10D");
  BENCHMARK("M");
  BENCHMARK("13O");
  BENCHMARK("1,13W");
  BENCHMARK("13H");
  BENCHMARK("0,13W");
  BENCHMARK("13L");
  BENCHMARK("13R");
  BENCHMARK("13X");
  BENCHMARK("0A");
  BENCHMARK("100,3P");
  BENCHMARK("C");

  BENCHMARK("1,1000{d1,13W}l");
  BENCHMARK("1,1000{d13H}l");
  BENCHMARK("1,1000{d0,13W}l");
  BENCHMARK("1,1000{d13L}l");
  BENCHMARK("1,1000{d13Rd}l");
  BENCHMARK("1,1000{d13R~13W}l");
  BENCHMARK("1,1000{d13X}l");
  BENCHMARK("1,1000{dA0d}l");

  BENCHMARK("0f");

  shell.set(F("abs"), SCRIPT("u0<{n}i"));
  shell.set(F("min"), SCRIPT("oo>{s}id"));
  shell.set(F("max"), SCRIPT("oo<{s}id"));
  shell.set(F("fac"), SCRIPT("1,2r{*}l"));
  shell.set(F("5fac"), SCRIPT("5`fac:"));
  shell.set(F("x"), 0);
  shell.set(F("y"), 0);
  eeprom_busy_wait();

  BENCHMARK("`abs");
  BENCHMARK("`min");
  BENCHMARK("`max");
  BENCHMARK("`fac");
  BENCHMARK("`5fac");
  BENCHMARK("`x");
  BENCHMARK("`y");
  BENCHMARK("C");

  BENCHMARK("-10`abs@x");
  BENCHMARK("-10`abs:");
  BENCHMARK("-10,10`min:");
  BENCHMARK("-10,10`max:");
  BENCHMARK("5`fac:");
  BENCHMARK("`5fac:");

  BENCHMARK("`x@");
  BENCHMARK("`y!");
  BENCHMARK("`x,u@1+s!");
  BENCHMARK("`x@`y!");
}

void loop()
{
}
