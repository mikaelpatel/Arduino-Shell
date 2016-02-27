/**
 * @file Shell.h
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
 */

#ifndef SHELL_H
#define SHELL_H

/**
 * Script Shell with stack machine instruction set. Instructions are
 * printable characters so that command lines and scripts can be
 * written directly.
 * @param[in] STACK_MAX max stack depth.
 * @param[in] VAR_MAX max number of variables.
 */
template<int STACK_MAX, int VAR_MAX>
class Shell {
public:
  /**
   * Construct a shell with given stream.
   * @param[in] ios input output stream.
   */
  Shell(Stream& ios) :
    m_sp(m_stack - 1),
    m_tos(0),
    m_trace(false),
    m_ios(ios)
  {
    memset(m_var, 0, sizeof(m_var));
  }

  /**
   * Set trace to given flag.
   * @param[in] flag trace true/false.
   */
  void trace(bool flag)
  {
    m_trace = flag;
  }

  /**
   * Get trace mode.
   * @return trace mode.
   */
  bool trace()
  {
    return (m_trace);
  }

  /**
   * Get depth of parameter stack.
   * @return number of elements on stack.
   */
  int depth() const
  {
    return (m_sp - m_stack + 1);
  }

  /**
   * Get top of stack.
   * @return top of stack.
   */
  int tos() const
  {
    return (m_tos);
  }

  /**
   * Set top of stack to given value.
   * @param[in] val to set.
   */
  void tos(int val)
  {
    m_tos = val;
  }

  /**
   * Pop top element from parameter stack.
   * @return top of stack.
   */
  int pop()
  {
    int res = m_tos;
    m_tos = (depth() > 0) ? *m_sp-- : 0;
    return (res);
  }

  /**
   * Push given value onto the parameter stack.
   * @param[in] val to push.
   */
  void push(int val)
  {
    if (depth() != STACK_MAX) *++m_sp = m_tos;
    m_tos = val;
  }

  /**
   * Push given value onto the parameter stack.
   * @param[in] val to push.
   */
  void push(const char* s)
  {
    push((int) s);
  }

  /**
   * Clear parameter stack.
   */
  void clear()
  {
    m_sp = m_stack - 1;
  }

  /**
   * Read variable.
   * @param[in] addr address.
   * @return value.
   */
  int read(int addr)
  {
    if (addr < 0 || addr >= VAR_MAX) return (0);
    return (m_var[addr]);
  }

  /**
   * Write variable.
   * @param[in] addr address.
   * @param[in] val value.
   */
  void write(int addr, int val)
  {
    if (addr < 0 || addr >= VAR_MAX) return;
    m_var[addr] = val;
  }

  /**
   * Write variable.
   * @param[in] addr address.
   * @param[in] s script.
   */
  void write(int addr, const char* s)
  {
    write(addr, (int) s);
  }

  /**
   * Print parameter stack contents.
   */
  void print()
  {
    int n = depth();
    m_ios.print(n);
    m_ios.print(':');
    if (n > 0) {
      int* tp = m_stack + 1;
      while (--n) {
	m_ios.print(' ');
	m_ios.print(*tp++);
      }
      m_ios.print(' ');
      m_ios.print(m_tos);
    }
    m_ios.println();
  }

  /**
   * Non-blocking read next character from shell stream. If available
   * add to buffer. If newline was read the buffer is null-terminated
   * and true is returned, otherwise false.
   * @param[in,out] bp buffer pointer.
   * @return true if line was read otherwise false.
   */
  bool read(char* &bp)
  {
    int c = m_ios.read();
    if (c < 0) return (false);
    *bp++ = c;
    if (c != '\n') return (false);
    *bp = 0;
    return (true);
  }

  /**
   * Execute given operation code (character). Return true if
   * successful otherwise false.
   * @param[in] op operation code.
   * @return bool.
   */
  bool execute(char op)
  {
    const char* script;
    int pin, addr, val, n;
    switch (op) {
    case ' ': // -- | no operation
    case ',':
      break;
    case '~': // x -- ~x | bitwise not
      tos(~tos());
      break;
    case '@': // addr -- val | read variable
      tos(read(tos()));
      break;
    case '!': // val addr -- | write variable
      addr = pop();
      write(addr, pop());
      break;
    case '#': // x y -- x!=y | not equal
      val = pop();
      tos(tos() != val ? -1 : 0);
      break;
    case '=': // x y -- x==y | equal
      val = pop();
      tos(tos() == val ? -1 : 0);
      break;
    case '<': // x y -- x<y | less than
      val = pop();
      tos(tos() < val ? -1 : 0);
      break;
    case '>': // x y -- x>y | greater than
      val = pop();
      tos(tos() > val ? -1 : 0);
      break;
    case '&': // x y -- x&y | bitwise and
      val = pop();
      tos(tos() & val);
      break;
    case '|': // x y -- x|y | bitwise or
      val = pop();
      tos(tos() | val);
      break;
    case '^': // x y -- x^y | bitwise xor
      val = pop();
      tos(tos() ^ val);
      break;
    case '+': // x y -- x+y | addition
      val = pop();
      tos(tos() + val);
      break;
    case '-': // x y -- x-y | subtraction
      val = pop();
      push(pop() - val);
      break;
    case '*': // x y -- x*y | multiplication
      val = pop();
      tos(tos() * val);
      break;
    case '/': // x y -- x/y | division
      val = pop();
      tos(tos() / val);
      break;
    case '%': // x y -- x%y | modulo
      val = pop();
      push(pop() % val);
      break;
    case '.': // x -- | print
      m_ios.print(pop());
      break;
    case 'c': // xn ... x1 -- | clear
      clear();
      break;
    case 'd': // x -- x x | duplicate
      push(tos());
      break;
    case 'e': // flag if-block else-block -- | execute block on flag
      val = *(m_sp - 1);
      if (val != 0) {
	pop();
	script = (const char*) pop();
      }
      else {
	script = (const char*) pop();
	pop();
      }
      pop();
      if (execute(script) != NULL) return (false);
      break;
    case 'i': // flag block -- | execute block if flag is true
      script = (const char*) pop();
      if (pop() && execute(script) != NULL) return (false);
      break;
    case 'k': // -- char or -1 | read from input stream
      push(m_ios.read());
      break;
    case 'l': // n block -- | execute block n-times
      script = (const char*) pop();
      n = pop();
      while (n--)
	if (execute(script) != NULL) return (false);
      break;
    case 'm': // -- | write new line to output stream
      m_ios.println();
      break;
    case 'n': // x -- -x | negate
      tos(-tos());
      break;
    case 'o': // x y -- x y x | over
      push(*m_sp);
      break;
    case 'p': // xn ... x1 n -- xn ... x1 xn | pick
      tos(*(m_sp - tos() + 1));
      break;
    case 'q': // x -- x x or 0 | duplicate if not zero
      if (tos()) push(tos());
      break;
    case 'r': // x y z --- y z x | rotate
      val = tos();
      tos(*(m_sp - 1));
      *(m_sp - 1) = *m_sp;
      *m_sp = val;
      break;
    case 's': // x y -- y x | swap
      val = tos();
      tos(*m_sp);
      *m_sp = val;
      break;
    case 't': // -- | toggle trace mode
      m_trace = !m_trace;
      break;
    case 'u': // x -- | drop
      pop();
      break;
    case 'v': // x -- | write character to output stream
      m_ios.write(pop());
      break;
    case 'w': // block( -- flag) -- | execute block while
      script = (const char*) pop();
      do {
	if (execute(script) != NULL) return (false);
      } while (pop());
      break;
    case 'x': // script -- | execute script
      script = (const char*) pop();
      if (execute(script) != NULL) return (false);
      break;
    case 'z': // -- | print stack contents
      print();
      break;
    case 'A': // pin -- sample | analogRead(pin)
      tos(analogRead(tos()));
      break;
    case 'D': // ms -- | delay()
      delay(pop());
      break;
    case 'F': // -- 0 | false
      push(0);
      break;
    case 'H': // pin -- | digitalWrite(pin, HIGH)
      digitalWrite(pop(), HIGH);
      break;
    case 'I': // pin -- | pinMode(pin, INPUT)
      pinMode(pop(), INPUT);
      break;
    case 'L': // pin -- | digitalWrite(pin, LOW)
      digitalWrite(pop(), LOW);
      break;
    case 'M': // -- ms | millis()
      push(millis());
      break;
    case 'N': // -- | no operation
      break;
    case 'O': // pin -- | pinMode(pin, OUTPUT)
      pinMode(pop(), OUTPUT);
      break;
    case 'P': // value pin -- | analogWrite(pin, value)
      pin = pop();
      analogWrite(pin, pop());
      break;
    case 'R': // pin -- value | digitalRead(pin)
      tos(digitalRead(tos()) ? -1 : 0);
      break;
    case 'T': // -- -1 | true
      push(-1);
      break;
    case 'U': // pin -- | pinMode(pin, INPUT_PULLUP)
      pinMode(pop(), INPUT_PULLUP);
      break;
    case 'W': // value pin -- | digitalWrite(pin, value)
      pin = pop();
      digitalWrite(pin, pop());
      break;
    default: // illegal operation code
      return (false);
    }
    return (true);
  }

  /**
   * Execute given script (null terminated sequence of operation
   * codes). Return NULL if successful otherwise script reference that
   * failed. Print error position in trace mode.
   * @param[in] s script.
   * @return bool.
   */
  const char* execute(const char* s)
  {
    const char* t = s;
    bool neg = false;
    char c;

    // Execute operation code in script
    while ((c = *s++) != 0) {

      // Check for negative numbers
      if (c == '-') {
	c = *s++;
	if (c < '0' || c > '9') {
	  c = '-';
	  s -= 1;
	}
	else {
	  neg = true;
	}
      }

      // Check for numbers
      if (c >= '0' && c <= '9') {
	int val = 0;
	do {
	  val = val * 10 + (c - '0');
	  c = *s++;
	} while (c >= '0' && c <= '9');
	if (neg) {
	  val = -val;
	  neg = false;
	}
	push(val);
	if (c == 0) break;
      }

      // Translate newline
      if (c == '\n') c = 'N';

      // Check for trace mode
      if (m_trace) {
	m_ios.print((int) s - 1);
	m_ios.print(':');
	m_ios.print(c);
	m_ios.print(':');
	print();
      }

      // Check for special forms; code blocks, and output strings
      char left = 0, right;
      if (c == '{') {
	left = '{';
	right = '}';
	push(s);
      }
      else if (c == '(') {
	left = '(';
	right = ')';
      }
      if (left) {
	int n = 1;
	while ((n != 0) && ((c = *s++) != 0)) {
	  if (c == left) n++;
	  else if (c == right) n--;
	  if (left == '(' && n > 0) m_ios.print(c);
	}
	if (c == 0) {
	  s -= 1;
	  c = left;
	  break;
	}
	continue;
      }

      // Execute operation code
      if (!execute(c)) break;
    }

    // Check for no errors
    if (c == 0 || c == '}') return (NULL);

    // Check for trace mode and error print
    if (m_trace) {
      m_ios.print(t);
      for (int i = 0, n = s - t - 1; i < n; i++)
	m_ios.print(' ');
      m_ios.println(F("^--?"));
    }

    // Return error position
    return (s - 1);
  }

protected:
  int m_stack[STACK_MAX];
  int m_var[VAR_MAX];
  int* m_sp;
  int m_tos;
  bool m_trace;
  Stream& m_ios;
};

#endif
