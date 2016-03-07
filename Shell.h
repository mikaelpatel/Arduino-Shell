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

/** Script strings in program memory */
typedef const class __FlashStringHelper* Script;

/**
 * Create a shell script with given string in program memory.
 * @param[in] s script string.
 */
#define SCRIPT(s) (Script) F(s)

/**
 * Script Shell with stack machine instruction set. Instructions are
 * printable characters so that command lines and scripts can be
 * written directly.
 * @param[in] STACK_MAX max stack depth (default 16).
 * @param[in] VAR_MAX max number of variables (default 32).
 * @param[in] FULL_OP_NAMES trace with operation name (default true).
 */
template<int STACK_MAX = 16, int VAR_MAX = 32, bool FULL_OP_NAMES = true>
class Shell {
public:
  /**
   * Construct a shell with given stream.
   * @param[in] ios input output stream.
   */
  Shell(Stream& ios) :
    m_dp(0),
    m_fp(m_stack + STACK_MAX),
    m_sp(m_stack + STACK_MAX),
    m_tos(0),
    m_marker(-1),
    m_trace(false),
    m_cycle(0),
    m_ios(ios)
  {
    memset(m_dict, 0, sizeof(m_dict));
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
  bool trace() const
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
   * Set top of stack to given value.
   * @param[in] val to set.
   */
  void tos(const char* val)
  {
    m_tos = (int) val;
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
    if (addr < 0 || addr >= (VAR_MAX + STACK_MAX)) return (0);
    return (m_var[addr]);
  }

  /**
   * Write variable.
   * @param[in] addr address.
   * @param[in] val value.
   */
  void write(int addr, int val)
  {
    if (addr < 0 || addr >= (VAR_MAX + STACK_MAX)) return;
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
   * Print parameter stack contents in format: n: xn..x1
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
    /*
     *   Arithmetic operators.
     */
    case 'n': // x -- -x | negate
      tos(-tos());
      break;
    case '+': // x y -- x+y | addition
      val = pop();
      tos(tos() + val);
      break;
    case '-': // x y -- x-y | subtraction
      val = pop();
      tos(tos() - val);
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
      tos(tos() % val);
      break;
    case 'h': // x y z -- x*y/z | scale
      val = pop();
      n = pop();
      tos(tos() * ((long) n) / val);
      break;
    /*
     *  Comparison/relational operators.
     */
    case 'F': // -- false | false
      push(0);
      break;
    case 'T': // -- true | true
      push(-1);
      break;
    case '=': // x y -- x==y | equal
      val = pop();
      tos(as_bool(tos() == val));
      break;
    case '#': // x y -- x!=y | not equal
      val = pop();
      tos(as_bool(tos() != val));
      break;
    case '<': // x y -- x<y | less than
      val = pop();
      tos(as_bool(tos() < val));
      break;
    case '>': // x y -- x>y | greater than
      val = pop();
      tos(as_bool(tos() > val));
      break;
    /*
     *  Bitwise/logical operators.
     */
    case '~': // x -- ~x | bitwise not
      tos(~tos());
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
    /*
     * Stack operations.
     */
    case 'c': // xn..x1 n -- | drop n stack elements
      n = tos();
      if (n > 0 && n < depth()) {
	m_sp += n;
      }
    case 'd': // x -- | drop
      drop();
      break;
    case 'g': // xn..x1 n -- xn-1..x1 xn | rotate n-elements
      n = tos();
      if (n > 0 && n < depth()) {
	tos(m_sp[--n]);
	for (; n > 0; n--)
	  m_sp[n] = m_sp[n - 1];
	m_sp += 1;
      }
      else drop();
      break;
    case 'j': // xn..x1 -- xn..x1 n | stack depth
      push(depth());
      break;
    case 'o': // x y -- x y x | over
      push(*m_sp);
      break;
    case 'p': // xn..x1 n -- xn..x1 xn | pick
      tos(*(m_sp + tos() - 1));
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
    case 'q': // x -- [x x] or 0 | duplicate if not zero
      if (tos() == 0) break;
    case 'u': // x -- x x | duplicate
      push(tos());
      break;
    /*
     * Memory access operations.
     */
    case '@': // addr -- val | read variable
      tos(read(tos()));
      break;
    case '!': // val addr -- | write variable
      addr = pop();
      val = pop();
      write(addr, val);
      break;
    /*
     * Script/control structure operations.
     */
    case ':': // addr -- | execute function (variable)
      addr = pop();
      script = (const char*) read(addr);
      if (script == NULL) return (false);
      if (execute(script) != NULL) return (false);
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
    case 'l': // n block -- | execute block n-times
      script = (const char*) pop();
      n = pop();
      while (n--)
	if (execute(script) != NULL) return (false);
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
    /*
     * Stack frame operations.
     */
    case '\\':
      n = pop();
      // x1..xn n -- x1..xn | mark n-element stack frame
      if (n > 0) {
	m_fp = m_sp + n - 1;
      }
      // x1..xn y1..ym n -- y1..ym | resolve n-element stack frame
      else {
	n = m_fp - m_sp + n;
	if (n >= 0) {
	  while (n--) *--m_fp = m_sp[n];
	  m_sp = m_fp;
	}
	else {
	  m_sp = m_fp;
	  drop();
	}
      }
      break;
    case '$': // n -- addr | address of n-element in frame
      n = tos();
      tos((m_fp - n) - m_var);
      break;
    /*
     * Input/output operations.
     */
    case 'k': // -- char | blocking read from input stream
      while ((val = m_ios.read()) < 0) yield();
      push(val);
      break;
    case '?': // addr -- | print variable
      tos(read(tos()));
    case '.': // x -- | print number followed by one space
      val = pop();
      m_ios.print(val);
      m_ios.print(' ');
      break;
    case 'm': // -- | write new line to output stream
      m_ios.println();
      break;
    case 'v': // char -- | write character to output stream
      val = pop();
      m_ios.write(val);
      break;
    /*
     * Arduino operations.
     */
    case 'A': // pin -- sample | analogRead(pin)
      pin = tos();
      tos(analogRead(pin));
      break;
    case 'C': // xn..x1 -- | clear
      clear();
      break;
    case 'D': // ms -- | delay()
      val = pop();
      delay((unsigned) val);
      break;
    case 'E': // period addr -- bool | time-out
      addr = pop();
      val = read(addr);
      n = tos();
      if ((((unsigned) millis() & 0xffff) - ((unsigned) val)) >= ((unsigned) n)) {
	tos(-1);
	write(addr, millis());
      }
      else tos(0);
      break;
    case 'H': // pin -- | digitalWrite(pin, HIGH)
      pin = pop();
      digitalWrite(pin, HIGH);
      break;
    case 'I': // pin -- | pinMode(pin, INPUT)
      pin = pop();
      pinMode(pin, INPUT);
      break;
    case 'K': // -- [char true] or false | non-blocking read from input stream
      val = m_ios.read();
      if (val < 0) {
	push(0);
      } else {
	push(val);
	push(-1);
      }
      break;
    case 'L': // pin -- | digitalWrite(pin, LOW)
      pin = pop();
      digitalWrite(pin, LOW);
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
      val = pop();
      analogWrite(pin, val);
      break;
    case 'R': // pin -- bool | digitalRead(pin)
      pin = tos();
      tos(as_bool(digitalRead(pin)));
      break;
    case 'S': // -- | print stack contents
      print();
      break;
    case 'U': // pin -- | pinMode(pin, INPUT_PULLUP)
      pin = pop();
      pinMode(pin, INPUT_PULLUP);
      break;
    case 'W': // value pin -- | digitalWrite(pin, value)
      pin = pop();
      val = pop();
      digitalWrite(pin, val);
      break;
    case 'X': // pin -- | digitalToggle(pin)
      pin = pop();
      digitalWrite(pin, !digitalRead(pin));
      break;
    case 'Z': // -- | toggle trace mode
      m_trace = !m_trace;
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
   * @return script reference or NULL.
   */
  const char* execute(const char* s)
  {
    bool progmem = false;
    const char* t = s;
    bool neg = false;
    int base = 10;
    int* fp = m_fp;
    size_t len = 0;
    char c;

    // Check for program memory address
    if (((int) s) < 0) {
      progmem = true;
      s = (const char*) -((int) s);
    }

    // Execute operation code in script
    while ((c = read(s++, progmem)) != 0) {

      // Check for negative numbers
      if (c == '-') {
	c = read(s, progmem);
	if (c < '0' || c > '9') {
	  c = '-';
	}
	else {
	  neg = true;
	  s += 1;
	}
      }

      // Check for base prefix
      else if (c == '0') {
	c = read(s++, progmem);
	if (c == 'x') base = 16;
	else if (c == 'b') base = 2;
	else s -= 2;
	c = read(s++, progmem);
      }

      // Check for literal numbers
      if (is_digit(c, base)) {
	int val = 0;
	do {
	  if (base == 16 && c >= 'a')
	    val = (val * base) + (c - 'a') + 10;
	  else
	    val = (val * base) + (c - '0');
	  c = read(s++, progmem);
	} while (is_digit(c, base));
	if (neg) {
	  val = -val;
	  neg = false;
	}
	push(val);
	base = 10;
	if (c == 0) break;
      }

      // Translate newline (for trace mode)
      if (c == '\n') c = 'N';

      // Check for trace mode
      if (m_trace) {
	const class __FlashStringHelper* str = as_fstr(c);
	m_ios.print(++m_cycle);
	m_ios.print(':');
	m_ios.print((int) s - 1);
	m_ios.print(':');
	if (str == NULL)
	  m_ios.print(c);
	else
	  m_ios.print(str);
	if (c != '`') {
	  m_ios.print(':');
	  print();
	}
      }

      // Check for special forms
      char left = 0, right;
      switch (c) {
      case ' ': // -- | no operation
      case ',':
	continue;
      case '}': // -- | end of block
	return (NULL);
      case ';': // addr block -- | copy block to variable
	{
	  const char* src = (const char*) pop();
	  int addr = pop();
	  if (progmem) {
	    write(addr, src);
	  }
	  else {
	    char* dest = (char*) malloc(len + 1);
	    if (dest == NULL) break;
	    strlcpy(dest, src, len);
	    write(addr, dest);
	  }
	}
	continue;
      case '`': // -- addr | lookup or add variable
	{
	  const char* name = s;
	  size_t len = 0;
	  int i;
	  while (((c = read(s++, progmem)) != 0) && isalnum(c)) len++;
	  if (len > 0) {
	    for (i = 0; i != m_dp; i++)
	      if (progmem) {
		if (!strncmp_P(m_dict[i], name, len))
		  break;
	      }
	      else {
		if (!strncmp(m_dict[i], name, len))
		  break;
	      }
	    if (i == m_dp) {
	      char* dest = (char*) malloc(len + 1);
	      if (dest != NULL) {
		if (progmem)
		  strlcpy_P(dest, name, len + 1);
		else
		  strlcpy(dest, name, len + 1);
		m_dict[i] = dest;
		m_dp += 1;
	      }
	      else
		i = -1;
	    }
	    if (i != -1 && m_trace) {
	      m_ios.print(m_dict[i]);
	      m_ios.print(':');
	      print();
	    }
	    push(i);
	  }
	}
	s = s - 1;
	continue;
      case '\'': // -- char | push character
	c = read(s, progmem);
	if (c != 0) {
	  push(c);
	  s += 1;
	}
	continue;
      case '{': // -- block | start code block
	left = '{';
	right = '}';
	if (progmem)
	  push(-((int) s));
	else
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

      // Parse special parenphesis forms (allow nesting)
      if (left) {
	int n = 1;
	while ((n != 0) && ((c = read(s++, progmem)) != 0)) {
	  if (c == left) n++;
	  else if (c == right) n--;
	  if (left == '(' && n > 0) m_ios.print(c);
	}
	if (c == '}') {
	  const char* src = (const char*) tos();
	  len = s - src;
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
      if (c != TRAP_CHAR) break;
      const char* p = trap(s);
      if (p == NULL) break;
      s = p;
    }

    // Restore frame pointer
    m_fp = fp;

    // Check for no errors
    if (c != '}') m_cycle = 0;
    if (c == 0 || c == '}') return (NULL);

    // Check for trace mode and error print
    if (m_trace) {
      size_t len = strlen(t) - 1;
      m_ios.print(t);
      if (t[len] != '\n') m_ios.println();
      for (int i = 0, n = s - t; i < n; i++)
	m_ios.print(' ');
      m_ios.println(F("^--?"));
    }

    // Return error position
    return (s);
  }

  /**
   * Execute given script in program memory (null terminated sequence
   * of operation codes). Return NULL if successful otherwise script
   * reference that failed. Prints error position in trace mode.
   * @param[in] s program memory based script.
   * @return script reference or NULL.
   */
  const char* execute(Script s)
  {
    return (execute((const char*) (-(int) s)));
  }

  /**
   * Execute script with extended operation code (character). Return
   * next script reference if successful otherwise NULL.
   * @param[in] s script.
   * @return script reference or NULL.
   */
  virtual const char* trap(const char*)
  {
    return (NULL);
  }

protected:
  static const char TRAP_CHAR = '_';
  int m_dp;			//!< Next free dictionary entry.
  int* m_fp;			//!< Frame pointer.
  int* m_sp;			//!< Stack pointer.
  int m_tos;			//!< Top of stack register.
  int m_marker;			//!< Stack marker.
  bool m_trace;			//!< Trace mode.
  unsigned m_cycle;		//!< Cycle counter.
  Stream& m_ios;		//!< Input/output Stream.
  char* m_dict[VAR_MAX];	//!< Dictionary.
  int m_var[VAR_MAX];		//!< Variable table.
  int m_stack[STACK_MAX];	//!< Parameter stack.

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
   * @param[in] base must be bin, dec or hex.
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

  /**
   * Return full operation code name.
   * @param[in] op operation code (character).
   * @return program memory string or NULL.
   */
  const class __FlashStringHelper* as_fstr(char op)
  {
    if (!FULL_OP_NAMES) return (NULL);
    switch (op) {
    case 'c': return (F("ndrop"));
    case 'd': return (F("drop"));
    case 'e': return (F("ifelse"));
    case 'f': return (F("free"));
    case 'g': return (F("roll"));
    case 'h': return (F("*/"));
    case 'i': return (F("if"));
    case 'j': return (F("depth"));
    case 'k': return (F("key"));
    case 'l': return (F("loop"));
    case 'm': return (F("cr"));
    case 'n': return (F("negate"));
    case 'o': return (F("over"));
    case 'p': return (F("pick"));
    case 'q': return (F("?dup"));
    case 'r': return (F("rot"));
    case 's': return (F("swap"));
    case 'u': return (F("dup"));
    case 'v': return (F("emit"));
    case 'w': return (F("while"));
    case 'x': return (F("execute"));
    case 'y': return (F("yield"));
    case 'A': return (F("analogRead"));
    case 'C': return (F("clear"));
    case 'D': return (F("delay"));
    case 'E': return (F("?expired"));
    case 'F': return (F("false"));
    case 'H': return (F("high"));
    case 'I': return (F("input"));
    case 'K': return (F("?key"));
    case 'L': return (F("low"));
    case 'M': return (F("millis"));
    case 'N': return (F(""));
    case 'O': return (F("output"));
    case 'P': return (F("analogWrite"));
    case 'R': return (F("digitalRead"));
    case 'S': return (F(".s"));
    case 'T': return (F("true"));
    case 'U': return (F("inputPullup"));
    case 'W': return (F("digitalWrite"));
    case 'X': return (F("digitalToggle"));
    case 'Z': return (F("toggleTraceMode"));
    default:
      return (NULL);
    }
  }

  /**
   * Read next operation code from random access or program memory.
   * @param[in] s script pointer.
   * @param[in] progmem program memory flag.
   * @return operation code.
   */
  char read(const char* s, bool progmem)
  {
    if (progmem)
      return (pgm_read_byte(s));
    else
      return (*s);
  }

};

#endif
