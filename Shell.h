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
    m_sp(m_stack + STACK_MAX),
    m_tos(0),
    m_marker(-1),
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
    return (m_stack + STACK_MAX - m_sp);
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
   * Drop top element from parameter stack.
   */
  void drop()
  {
    m_tos = (depth() > 0) ? *m_sp++ : 0;
  }

  /**
   * Pop top element from parameter stack.
   * @return top of stack.
   */
  int pop()
  {
    int res = m_tos;
    m_tos = (depth() > 0) ? *m_sp++ : 0;
    return (res);
  }

  /**
   * Push given value onto the parameter stack.
   * @param[in] val to push.
   */
  void push(int val)
  {
    if (depth() != STACK_MAX) *--m_sp = m_tos;
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
    m_sp = m_stack + STACK_MAX;
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
      int* tp = m_stack + STACK_MAX - 1;
      while (--n) {
	m_ios.print(' ');
	m_ios.print(*--tp);
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
    case '~': // x -- ~x | bitwise not
      tos(~tos());
      break;
    case '@': // addr -- val | read variable
      tos(read(tos()));
      break;
    case '!': // val addr -- | write variable
      addr = pop();
      val = pop();
      write(addr, val);
      break;
    case '#': // x y -- x!=y | not equal
      val = pop();
      tos(as_bool(tos() != val));
      break;
    case '=': // x y -- x==y | equal
      val = pop();
      tos(as_bool(tos() == val));
      break;
    case '<': // x y -- x<y | less than
      val = pop();
      tos(as_bool(tos() < val));
      break;
    case '>': // x y -- x>y | greater than
      val = pop();
      tos(as_bool(tos() > val));
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
    case '.': // x -- | print number followed by one space
      m_ios.print(pop());
      m_ios.print(' ');
      break;
    case 'a': // block1 len -- block2 | allocate block
      {
	size_t len = (size_t) pop();
	const char* src = (const char*) pop();
	char* dest = (char*) malloc(len + 1);
	strlcpy(dest, src, len);
	push(dest);
      }
      break;
    case 'c': // xn..x1 -- | clear
      clear();
      break;
    case 'd': // x -- | drop
      drop();
      break;
    case 'e': // flag if-block else-block -- | execute block on flag
      val = *(m_sp + 1);
      if (val != 0) {
	drop();
	script = (const char*) pop();
      }
      else {
	script = (const char*) pop();
	drop();
      }
      drop();
      if (execute(script) != NULL) return (false);
      break;
    case 'f': // block -- | free block
      script = (const char*) pop();
      if (script != NULL) free((void*) script);
      break;
    case 'i': // flag block -- | execute block if flag is true
      script = (const char*) pop();
      if (pop() && execute(script) != NULL) return (false);
      break;
    case 'j': // xn..x1 -- xn..x1 n | stack depth
      push(depth());
      break;
    case 'k': // -- [char true] or false | non-blocking read from input stream
      val = m_ios.read();
      if (val < 0) {
	push(0);
      } else {
	push(val);
	push(-1);
      }
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
    case 'p': // xn..x1 n -- xn..x1 xn | pick
      tos(*(m_sp + tos() - 1));
      break;
    case 'q': // x -- [x x] or 0 | duplicate if not zero
      if (tos()) push(tos());
      break;
    case 'r': // x y z --- y z x | rotate
      val = tos();
      tos(*(m_sp + 1));
      *(m_sp + 1) = *m_sp;
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
    case 'u': // x -- x x | duplicate
      push(tos());
      break;
    case 'v': // char -- | write character to output stream
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
    case 'y': // -- | yield
      yield();
      break;
    case 'z': // -- | print stack contents
      print();
      break;
    case 'A': // pin -- sample | analogRead(pin)
      pin = tos();
      tos(analogRead(pin));
      break;
    case 'D': // ms -- | delay()
      delay(pop());
      break;
    case 'F': // -- false | false
      push(0);
      break;
    case 'H': // pin -- | digitalWrite(pin, HIGH)
      pin = pop();
      digitalWrite(pin, HIGH);
      break;
    case 'I': // pin -- | pinMode(pin, INPUT)
      pinMode(pop(), INPUT);
      break;
    case 'K': // -- char | blocking read from input stream
      while ((val = m_ios.read()) < 0) yield();
      push(val);
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
      pin = pop();
      pinMode(pin, OUTPUT);
      break;
    case 'P': // value pin -- | analogWrite(pin, value)
      pin = pop();
      analogWrite(pin, pop());
      break;
    case 'R': // pin -- bool | digitalRead(pin)
      pin = pop();
      tos(as_bool(digitalRead(pin)));
      break;
    case 'T': // -- true | true
      push(-1);
      break;
    case 'U': // pin -- | pinMode(pin, INPUT_PULLUP)
      pin = pop();
      pinMode(pin, INPUT_PULLUP);
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
   * failed. Prints error position in trace mode.
   * @param[in] s script.
   * @return bool.
   */
  const char* execute(const char* s)
  {
    const char* t = s;
    bool neg = false;
    int base = 10;
    char c;

    // Execute operation code in script
    while ((c = *s++) != 0) {

      // Check for negative numbers
      if (c == '-') {
	c = *s;
	if (c < '0' || c > '9') {
	  c = '-';
	}
	else {
	  neg = true;
	  s += 1;
	}
      }

      // Check for base
      else if (c == '0') {
	c = *s++;
	if (c == 'x') base = 16;
	else if (c == 'b') base = 2;
	else s -= 2;
	c = *s++;
      }

      // Check for literal numbers
      if (is_digit(c, base)) {
	int val = 0;
	do {
	  if (base == 16 && c >= 'a')
	    val = (val * base) + (c - 'a') + 10;
	  else
	    val = (val * base) + (c - '0');
	  c = *s++;
	} while (is_digit(c, base));
	if (neg) {
	  val = -val;
	  neg = false;
	}
	push(val);
	base = 10;
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

      // Check for special forms
      char left = 0, right;
      switch (c) {
      case ' ': // -- | no operation
      case ',':
	continue;
      case '\\': // block -- block len | copy script
	{
	  const char* script = (const char*) tos();
	  push(s - script - 1);
	}
	continue;
      case '\'': // -- char | push character
	c = *s;
	if (c != 0) {
	  push(c);
	  s += 1;
	}
	continue;
      case '{': // -- block | start code block
	left = '{';
	right = '}';
	push(s);
	break;
      case '(': // -- | start output string
	left = '(';
	right = ')';
	break;
      case '[': // -- | start stack marker
	if (m_marker == -1) {
	  m_marker = depth();
	  continue;
	}
	break;
      case ']': // xn..x1 -- n | end stack marker
	if (m_marker != -1) {
	  push(depth() - m_marker);
	  m_marker = -1;
	  continue;
	}
      default:
	;
      }

      // Parse special forms
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
      if (execute(c)) continue;

      // Check for trap operation code
      if (!trap(c)) break;
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

  /**
   * Execute given extened operation code (character). Return true if
   * successful otherwise false.
   * @param[in] op operation code.
   * @return bool.
   */
  virtual bool trap(char op)
  {
    return (op == 0);
  }


protected:
  int m_stack[STACK_MAX];
  int m_var[VAR_MAX];
  int* m_sp;
  int m_tos;
  int m_marker;
  bool m_trace;
  Stream& m_ios;

  /**
   * Map given integer value to boolean (true(-1) and false(0)).
   * @param[in] val value.
   * @return bool.
   */
  int as_bool(int val)
  {
    return (val ? -1 : 0);
  }

  /**
   * Check that the given character is a digit in the given base.
   * @param[in] c character.
   * @param[in] base must be 2, 8, 10 or 16.
   * @return bool.
   */
  bool is_digit(char c, int base)
  {
    if (base == 2)
      return (c >= '0' && c <= '1');
    if (base == 16 && c >= 'a')
      return (c >= 'a' && c <= 'f');
    return (c >= '0' && c <= '9');
  }

};

#endif
