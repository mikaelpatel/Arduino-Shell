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
class Script;

/**
 * Create a shell script with given string in program memory.
 * @param[in] s script string.
 */
#define SCRIPT(s) ((const Script*) PSTR(s))

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
    m_dp((char*) sizeof(m_dp) + sizeof(m_entries) + (sizeof(dict_t) * VAR_MAX)),
    m_dict((dict_t*) sizeof(m_dp) + sizeof(m_entries)),
    m_entries(0),
    m_fp(m_stack + STACK_MAX),
    m_sp(m_stack + STACK_MAX),
    m_tos(0),
    m_marker(-1),
    m_trace(false),
    m_cycle(0),
    m_base(10),
    m_ios(ios)
  {
    // Restore state from eeprom
    uint8_t entries = eeprom_read_byte((const uint8_t*) sizeof(char*));
    char* dp = (char*) eeprom_read_word(0);

    // Check valid state before restore
    if (dp != (char*) 0xffff && entries < VAR_MAX) {
      m_entries = entries;
      m_dp = dp;
      for (uint8_t i = 0; i < entries; i++)
	m_var[i] = eeprom_read_word((uint16_t*) &m_dict[i].value);
    }
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
   * @param[in] value to set.
   */
  void tos(int value)
  {
    m_tos = value;
  }

  /**
   * Set top of stack to given value.
   * @param[in] s to set.
   */
  void tos(const char* s)
  {
    m_tos = (int) s;
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
   * @param[in] value to push.
   */
  void push(int value)
  {
    if (depth() != STACK_MAX) *--m_sp = m_tos;
    m_tos = value;
  }

  /**
   * Push given value onto the parameter stack.
   * @param[in] s string to push.
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
   * Print parameter stack contents in format: n: xn..x1
   */
  void print() const
  {
    int n = depth();
    m_ios.print(n);
    m_ios.print(':');
    if (n > 0) {
      const int* tp = m_stack + STACK_MAX - 1;
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
   * Read variable.
   * @param[in] addr address.
   * @return value.
   */
  int read(int addr) const
  {
    if (addr < 0 || addr >= (VAR_MAX + STACK_MAX)) return (0);
    return (m_var[addr]);
  }

  /**
   * Write variable.
   * @param[in] addr address.
   * @param[in] value to assign.
   */
  void write(int addr, int value)
  {
    if (addr < 0 || addr >= (VAR_MAX + STACK_MAX)) return;
    m_var[addr] = value;
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
   * Define a dictionary entry with given name and value.
   * @param[in] var name string (in program memory).
   * @param[in] value.
   * @return index or negative error code.
   */
  int set(const __FlashStringHelper* var, int value = 0)
  {
    char name[NAME_MAX];
    strcpy_P(name, (const char*) var);
    int i = lookup(name, strlen(name));
    if (i < 0) return (i);
    m_var[i] = value;
    return (i);
  }

  /**
   * Define a function with given name and script in data memory.
   * @param[in] var name string.
   * @param[in] script string in data memory.
   * @return index or negative error code.
   */
  int set(const __FlashStringHelper* var, const char* script)
  {
    return (set(var, (int) script));
  }

  /**
   * Define a function with given name and script in program memory.
   * @param[in] var name string.
   * @param[in] script in program memory.
   * @return index or negative error code.
   */
  int set(const __FlashStringHelper* var, const Script* script)
  {
    return (set(var, (int) m_progmem.as_addr((const char*) script)));
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
    m_cycle = 0;
    return (true);
  }

  /**
   * Execute given script (null terminated sequence of operation
   * codes). Return NULL if successful otherwise script reference that
   * failed. Prints error position in trace mode.
   * @param[in] script.
   * @return script reference or NULL.
   */
  const char* execute(const char* script)
  {
    Memory* mem = access(script);
    next_fn next = mem->get_next_fn();
    const char* ip = mem->as_local(script);
    bool neg = false;
    int base = 10;
    int* fp = m_fp;
    size_t len = 0;
    int w, n, addr, pin;
    const char* sp;
    char op;

    // Execute operation code in script
    while ((op = next(ip++)) != 0) {

      // Check for negative numbers
      if (op == '-') {
	op = next(ip);
	if (op < '0' || op > '9') {
	  op = '-';
	}
	else {
	  neg = true;
	  ip += 1;
	}
      }

      // Check for base prefix
      else if (op == '0') {
	op = next(ip++);
	if (op == 'x') base = 16;
	else if (op == 'b') base = 2;
	else ip -= 2;
	op = next(ip++);
      }

      // Check for literal numbers
      if (is_digit(op, base)) {
	int w = 0;
	do {
	  if (base == 16 && op >= 'a')
	    w = (w * base) + (op - 'a') + 10;
	  else
	    w = (w * base) + (op - '0');
	  op = next(ip++);
	} while (is_digit(op, base));
	if (neg) {
	  w = -w;
	  neg = false;
	}
	push(w);
	base = 10;
	if (op == 0) break;
      }

      // Translate newline (for trace mode)
      if (op == '\n') op = 'N';

      // Check for trace mode
      if (m_trace) {
	m_ios.print(++m_cycle);
	m_ios.print(':');
	m_ios.print(mem->prefix());
	m_ios.print(':');
	m_ios.print((int) ip - 1);
	m_ios.print(':');
	const class __FlashStringHelper* str = as_fstr(op);
	if (str == NULL)
	  m_ios.print(op);
	else
	  m_ios.print(str);
	if (op != '`') {
	  m_ios.print(':');
	  print();
	}
      }

      // Execute operation or parse special form
      char left = 0, right;
      switch (op) {
      /*
       * No operations.
       */
      case ' ': // -- | no operation
      case ',':
	continue;
      /*
       *   Arithmetic operators.
       */
      case 'n': // x -- -x | negate
	tos(-tos());
	continue;
      case '+': // x y -- x+y | addition
	w = pop();
	tos(tos() + w);
	continue;
      case '-': // x y -- x-y | subtraction
	w = pop();
	tos(tos() - w);
	continue;
      case '*': // x y -- x*y | multiplication
	w = pop();
	tos(tos() * w);
	continue;
      case '/': // x y -- x/y | division
	w = pop();
	tos(tos() / w);
	break;
      case '%': // x y -- x%y | modulo
	w = pop();
	tos(tos() % w);
	break;
      case 'h': // x y z -- x*y/z | scale
	w = pop();
	n = pop();
	tos(tos() * ((long) n) / w);
	break;
      /*
       *  Comparison/relational operators.
       */
      case 'F': // -- false | false
	push(0);
	continue;
      case 'T': // -- true | true
	push(-1);
	continue;
      case '=': // x y -- x==y | equal
	w = pop();
	tos(as_bool(tos() == w));
	continue;
      case '#': // x y -- x!=y | not equal
	w = pop();
	tos(as_bool(tos() != w));
	continue;
      case '<': // x y -- x<y | less than
	w = pop();
	tos(as_bool(tos() < w));
	continue;
      case '>': // x y -- x>y | greater than
	w = pop();
	tos(as_bool(tos() > w));
	continue;
      /*
       *  Bitwise/logical operators.
       */
      case '~': // x -- ~x | bitwise not
	tos(~tos());
	continue;
      case '&': // x y -- x&y | bitwise and
	w = pop();
	tos(tos() & w);
	continue;
      case '|': // x y -- x|y | bitwise or
	w = pop();
	tos(tos() | w);
	continue;
      case '^': // x y -- x^y | bitwise xor
	w = pop();
	tos(tos() ^ w);
	continue;
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
	continue;
      case 'g': // xn..x1 n -- xn-1..x1 xn | rotate n-elements
	n = tos();
	if (n > 0 && n < depth()) {
	  tos(m_sp[--n]);
	  for (; n > 0; n--)
	    m_sp[n] = m_sp[n - 1];
	  m_sp += 1;
	}
	else drop();
	continue;
      case 'j': // xn..x1 -- xn..x1 n | stack depth
	push(depth());
	continue;
      case 'o': // x y -- x y x | over
	push(*m_sp);
	continue;
      case 'p': // xn..x1 n -- xn..x1 xn | pick
	tos(*(m_sp + tos() - 1));
	continue;
      case 'r': // x y z --- y z x | rotate
	w = tos();
	tos(*(m_sp + 1));
	*(m_sp + 1) = *m_sp;
	*m_sp = w;
	continue;
      case 's': // x y -- y x | swap
	w = tos();
	tos(*m_sp);
	*m_sp = w;
	continue;
      case 'q': // x -- [x x] or 0 | duplicate if not zero
	if (tos() == 0) continue;
      case 'u': // x -- x x | duplicate
	push(tos());
	continue;
      /*
       * Memory access operations.
       */
      case '@': // addr -- value | read variable
	tos(read(tos()));
	continue;
      case '!': // value addr -- | write variable
	addr = pop();
	w = pop();
	write(addr, w);
	continue;
      case 'z': // addr -- | write variable to eeprom
	w = pop();
	if (w >= 0 && w < m_entries)
	  eeprom_write_word((uint16_t*) &m_dict[w].value, (uint16_t) m_var[w]);
	continue;
      case 'a': // -- bytes entries | allocated eeprom
	push((int) m_dp);
	push(m_entries);
	continue;
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
	continue;
      case '$': // n -- addr | address of n-element in frame
	n = tos();
	tos((m_fp - n) - m_var);
	continue;
      /*
       * Input/output operations.
       */
      case 'k': // -- char | blocking read from input stream
	while ((w = m_ios.read()) < 0) yield();
	push(w);
	continue;
      case 'b': // base -- | number print base
	m_base = pop();
	continue;
      case '?': // addr -- | print variable
	tos(read(tos()));
      case '.': // x -- | print number followed by one space
	w = pop();
	if (m_base == 2) m_ios.print(F("0b"));
	else if (m_base == 8) m_ios.print(F("0"));
	else if (m_base == 16) m_ios.print(F("0x"));
	m_ios.print(w, m_base > 0 ? m_base : -m_base);
	m_ios.print(' ');
	continue;
      case 'm': // -- | write new line to output stream
	m_ios.println();
	continue;
      case 't': // addr -- | write variable name output stream
	addr = tos();
	if (addr >= 0 && addr < m_entries) {
	  const uint8_t* np =
	    (const uint8_t*) eeprom_read_word((const uint16_t*) &m_dict[addr].name);
	  char c;
	  while ((c = eeprom_read_byte(np++)) != 0)
	    m_ios.print(c);
	  m_ios.print(' ');
	  tos(-1);
	}
	else tos(0);
	continue;
      case 'v': // char -- | write character to output stream
	w = pop();
	m_ios.write(w);
	continue;
      /*
       * Control structure operations.
       */
      case 'e': // flag if-block else-block -- | execute block on flag
	w = *(m_sp + 1);
	if (w != 0) {
	  drop();
	  sp = (const char*) pop();
	}
	else {
	  sp = (const char*) pop();
	  drop();
	}
	drop();
	if (execute(sp) != NULL) goto error;
	continue;
      case 'f': // addr -- | forget variable
	w = pop();
	if (w >= 0 && w < m_entries) {
	  m_entries = w;
	  m_dp = (char*) eeprom_read_word((const uint16_t*) &m_dict[w].name);
	  eeprom_update_block(&m_dp, 0, sizeof(m_dp));
	  eeprom_update_byte((uint8_t*) sizeof(m_dp), m_entries);
	}
	continue;
      case 'i': // flag block -- | execute block if flag is true
	sp = (const char*) pop();
	if (pop() && execute(sp) != NULL) goto error;
	continue;
      case 'l': // low high block( i -- ) -- | execute block from low to high
	{
	  sp = (const char*) pop();
	  int high = pop();
	  int low = pop();
	  for (int i = low; i <= high; i++) {
	    push(i);
	    if (execute(sp) != NULL) goto error;
	  }
	}
	continue;
      case 'w': // block( -- flag) -- | execute block while
	sp = (const char*) pop();
	do {
	  if (execute(sp) != NULL) goto error;
	} while (pop());
	continue;
      case 'x': // script -- | execute script
	sp = (const char*) pop();
	if (execute(sp) != NULL) goto error;
	continue;
      case 'y': // -- | yield
	yield();
	continue;
      /*
       * Script operations.
       */
      case '`': // -- addr | lookup or add variable
	{
	  char name[NAME_MAX];
	  size_t len = 0;
	  while (((op = next(ip++)) != 0) && isalnum(op))
	    name[len++] = op;
	  name[len] = 0;
	  if (len > 0) {
	    int i = lookup(name, len);
	    if (i != -1 && m_trace) {
	      m_ios.print(name);
	      m_ios.print(':');
	      print();
	    }
	    push(i);
	  }
	}
	ip = ip - 1;
	continue;
      case ':': // addr -- | execute function (variable)
	addr = pop();
	sp = (const char*) read(addr);
	if (sp == NULL) goto error;
	if (execute(sp) != NULL) goto error;
	continue;
      case ';': // addr block -- | copy block to variable
	{
	  const char* dest = m_eeprom.as_addr(m_dp);
	  const char* src = (const char*) pop();
	  int addr = pop();
	  if (addr >= 0 && addr < m_entries) {
	    eeprom_update_block(src, m_dp, len);
	    m_dp += len;
	    eeprom_update_byte((uint8_t*) m_dp, 0);
	    m_dp += 1;
	    eeprom_update_block(&m_dp, 0, sizeof(m_dp));
	    write(addr, dest);
	    eeprom_write_word((uint16_t*) &m_dict[addr].value, (uint16_t) dest);
	  }
	}
	continue;
      case '{': // -- block | start code block
	left = '{';
	right = '}';
	push(mem->as_addr(ip));
	break;
      case '}': // -- | end of block
	return (NULL);
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
      case '\'': // -- char | push character
	op = next(ip);
	if (op != 0) {
	  push(op);
	  ip += 1;
	}
	continue;
      /*
       * Arduino operations.
       */
      case 'A': // pin -- sample | analogRead(pin)
	pin = tos();
	tos(analogRead(pin));
	continue;
      case 'C': // xn..x1 -- | clear
	clear();
	continue;
      case 'D': // ms -- | delay()
	w = pop();
	delay((unsigned) w);
	continue;
      case 'E': // period addr -- bool | time-out
	addr = pop();
	w = read(addr);
	n = tos();
	if ((((unsigned) millis() & 0xffff) - ((unsigned) w)) >= ((unsigned) n)) {
	  tos(-1);
	  write(addr, millis());
	}
	else tos(0);
	continue;
      case 'H': // pin -- | digitalWrite(pin, HIGH)
	pin = pop();
	digitalWrite(pin, HIGH);
	continue;
      case 'I': // pin -- | pinMode(pin, INPUT)
	pin = pop();
	pinMode(pin, INPUT);
	continue;
      case 'K': // -- [char true] or false | non-blocking read from input stream
	w = m_ios.read();
	if (w < 0) {
	  push(0);
	} else {
	  push(w);
	  push(-1);
	}
	continue;
      case 'L': // pin -- | digitalWrite(pin, LOW)
	pin = pop();
	digitalWrite(pin, LOW);
	continue;
      case 'M': // -- ms | millis()
	push(millis());
	continue;
      case 'N': // -- | no operation
	continue;
      case 'O': // pin -- | pinMode(pin, OUTPUT)
	pin = pop();
	pinMode(pin, OUTPUT);
	continue;
      case 'P': // value pin -- | analogWrite(pin, value)
	pin = pop();
	w = pop();
	analogWrite(pin, w);
	continue;
      case 'R': // pin -- bool | digitalRead(pin)
	pin = tos();
	tos(as_bool(digitalRead(pin)));
	continue;
      case 'S': // -- | print stack contents
	print();
	continue;
      case 'U': // pin -- | pinMode(pin, INPUT_PULLUP)
	pin = pop();
	pinMode(pin, INPUT_PULLUP);
	continue;
      case 'W': // value pin -- | digitalWrite(pin, value)
	pin = pop();
	w = pop();
	digitalWrite(pin, w);
	continue;
      case 'X': // pin -- | digitalToggle(pin)
	pin = pop();
	digitalWrite(pin, !digitalRead(pin));
	continue;
      case 'Z': // -- | toggle trace mode
	m_trace = !m_trace;
	continue;
      case TRAP_OP_CODE:
	sp = trap(ip);
	if (sp == NULL) goto error;
	ip = sp;
	continue;
      default:
	goto error;
      }

      // Parse special parenphesis forms (allow nesting)
      if (left) {
	int n = 1;
	while ((n != 0) && ((op = next(ip++)) != 0)) {
	  if (op == left) n++;
	  else if (op == right) n--;
	  if (left == '(' && n > 0) m_ios.print(op);
	}
	if (op == '}') {
	  const char* src = (const char*) tos();
	  len = ip - src - 1;
	}
	if (op == 0) {
	  ip = ip - 1;
	  op = left;
	  break;
	}
      }
    }

    // Restore frame pointer
    m_fp = fp;

    // Check for no errors
    if (op == 0 || op == '}') return (NULL);

    // Check for trace mode and error print
  error:
    ip = ip - 1;
    if (m_trace && mem == &m_memory) {
      m_ios.print(script);
      if (script[strlen(script) - 1] != '\n') m_ios.println();
      for (int i = 0, n = ip - script; i < n; i++)
	m_ios.print(' ');
      m_ios.println(F("^--?"));
    }

    // Return error position
    return (ip);
  }

  /**
   * Execute given script in program memory (null terminated sequence
   * of operation codes). Return NULL if successful otherwise script
   * reference that failed. Prints error position in trace mode.
   * @param[in] script program memory based script.
   * @return script reference or NULL.
   */
  const char* execute(const Script* script)
  {
    return (execute(m_progmem.as_addr((const char*) script)));
  }

  /**
   * Execute script with extended operation code (character). Return
   * next script reference if successful otherwise NULL.
   * @param[in] ip script.
   * @return script reference or NULL.
   */
  virtual const char* trap(const char*)
  {
    return (NULL);
  }

protected:
  /** Max length of name. */
  static const size_t NAME_MAX = 16;

  /** Trap operation code prefix. */
  static const char TRAP_OP_CODE = '_';

  /**
   * Return next token from given source.
   * @param[in] src source pointer.
   * @return next token.
   */
  typedef char (*next_fn)(const char* src);

  /** Dictionary entry (in eeprom). */
  struct dict_t {
    const char* name;		//!< Name string (null terminated).
    int value;			//!< Value persistent.
  };

  char* m_dp;			//!< Dictionary pointer (in eeprom).
  dict_t* m_dict;		//!< Dictionary (in eeprom).
  uint8_t m_entries;		//!< Dictionary entries.
  int* m_fp;			//!< Frame pointer.
  int* m_sp;			//!< Stack pointer.
  int m_tos;			//!< Top of stack register.
  int m_marker;			//!< Stack marker.
  bool m_trace;			//!< Trace mode.
  unsigned m_cycle;		//!< Cycle counter.
  int m_base;			//!< Number print base.
  Stream& m_ios;		//!< Input/output Stream.
  int m_var[VAR_MAX];		//!< Variable table.
  int m_stack[STACK_MAX];	//!< Parameter stack.

  /**
   * Map given integer value to boolean (true(-1) and false(0)).
   * @param[in] val value.
   * @return bool.
   */
  int as_bool(int value)
  {
    return (value ? -1 : 0);
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
    case 'a': return (F("allocated"));
    case 'b': return (F("base"));
    case 'c': return (F("ndrop"));
    case 'd': return (F("drop"));
    case 'e': return (F("ifelse"));
    case 'f': return (F("forget"));
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
    case 't': return (F(".name"));
    case 'u': return (F("dup"));
    case 'v': return (F("emit"));
    case 'w': return (F("while"));
    case 'x': return (F("execute"));
    case 'z': return (F("zap"));
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
    case 'N': return (F(" "));
    case 'O': return (F("output"));
    case 'P': return (F("analogWrite"));
    case 'R': return (F("digitalRead"));
    case 'S': return (F(".s"));
    case 'T': return (F("true"));
    case 'U': return (F("inputPullup"));
    case 'W': return (F("digitalWrite"));
    case 'X': return (F("digitalToggle"));
    case 'Y': return (F("yield"));
    case 'Z': return (F("toggleTraceMode"));
    default:
      return (NULL);
    }
  }

  /**
   * Memory access class to allow scripts in various types of memory.
   * Maps to a linear address space; SRAM, EEPROM(16K), PROGMEM(32K).
   */
  class Memory {
  public:
    /**
     * Return prefix for address space.
     * @return string.
     */
    virtual const __FlashStringHelper* prefix()
    {
      return (F("RAM"));
    }

    /**
     * Return local script address mapped to linear address space.
     * @param src local address.
     * @return linear address.
     */
    virtual const char* as_addr(const char* src)
    {
      return (src);
    }

    /**
     * Return script address mapped to local address space.
     * @param src linear address.
     * @return local address.
     */
    virtual const char* as_local(const char* src)
    {
      return (src);
    }

    /**
     * Return read function for memory.
     * @return read function.
     */
    virtual next_fn get_next_fn()
    {
      return (next);
    }

    /**
     * Read byte from given local address.
     * @param src local address.
     * @return byte.
     */
    static char next(const char* src)
    {
      return (*src);
    };
  } m_memory;

  class ProgramMemory : public Memory {
  public:
    /**
     * Return prefix for address space.
     * @return string.
     */
    virtual const __FlashStringHelper* prefix()
    {
      return (F("PGM"));
    }

    /**
     * Return local script address mapped to linear address space.
     * @param src local address.
     * @return linear address.
     */
    virtual const char* as_addr(const char* src)
    {
      return ((const char*) -((int) src));
    }

    /**
     * Return script address mapped to local address space.
     * @param src linear address.
     * @return local address.
     */
    virtual const char* as_local(const char* src)
    {
      return ((const char*) -((int) src));
    }

    /**
     * Return read function for program memory.
     * @return read function.
     */
    virtual next_fn get_next_fn()
    {
      return (next);
    }

    /**
     * Read byte from given local address.
     * @param src local address.
     * @return byte.
     */
    static char next(const char* src)
    {
      return (pgm_read_byte(src));
    };
  } m_progmem;

  class EEPROM : public Memory {
  public:
    /**
     * Return prefix for address space.
     * @return string.
     */
    virtual const __FlashStringHelper* prefix()
    {
      return (F("EEM"));
    }

    /**
     * Return local script address mapped to linear address space.
     * @param src local address.
     * @return linear address.
     */
    virtual const char* as_addr(const char* src)
    {
      return (src + 0x4000);
    }

    /**
     * Return script address mapped to local address space.
     * @param src linear address.
     * @return local address.
     */
    virtual const char* as_local(const char* src)
    {
      return (src - 0x4000);
    }

    /**
     * Return read function for eeprom.
     * @return read function.
     */
    virtual next_fn get_next_fn()
    {
      return (next);
    }

    /**
     * Read byte from given local address.
     * @param src local address.
     * @return byte.
     */
    static char next(const char* src)
    {
      return (eeprom_read_byte((const uint8_t*) src));
    };
  } m_eeprom;

  /**
   * Return memory access handler for given script pointer (access factory).
   * @param[in] ip script pointer.
   * @return memory access.
   */
  Memory* access(const char* ip)
  {
    Memory* mem = &m_memory;
    if (((int) ip) < 0)
      mem = &m_progmem;
    else if (ip > (const char*) 0x4000)
      mem = &m_eeprom;
    return (mem);
  }

  /**
   * Lookup given name in dictionary. Return entry index or negative
   * error code.
   * @param[in] name string.
   * @param[in] len length of string.
   * @return entry index or negative error code.
   */
  int lookup(const char* name, size_t len)
  {
    int i = 0;

    // Lookup entry in dictionary
    for (; i < m_entries; i++) {
      const uint8_t* np =
	(const uint8_t*) eeprom_read_word((const uint16_t*) &m_dict[i].name);
      size_t j = 0;
      for (; j < len; j++)
	if (name[j] != (char) eeprom_read_byte(np++))
	  break;
      if (j == len && (eeprom_read_byte(np) == 0))
	return (i);
    }

    // Check if dictionary is full
    if (i == VAR_MAX) return (-1);

    // Add entry to dictionary
    eeprom_update_block(&m_dp, &m_dict[i].name, sizeof(m_dp));
    eeprom_update_block(name, m_dp, len);
    m_dp += len;
    eeprom_update_byte((uint8_t*) m_dp, 0);
    m_dp += 1;
    eeprom_update_block(&m_dp, 0, sizeof(m_dp));
    m_entries += 1;
    eeprom_update_byte((uint8_t*) sizeof(m_dp), m_entries);
    m_var[i] = 0;

    // Return entry index
    return (i);
  }
};

#endif
