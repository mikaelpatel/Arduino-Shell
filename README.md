# Arduino-Shell

This library provides a Forth/PostScript style shell for Arduino
sketches. Shell is a virtual stack machine with a byte token threaded
instruction set. The tokens, characters, are chosen so that it is
possible to write small scripts directly without a token compiler. As
Forth scripts are written in Reverse Polish Notation (RPN), and
executed in the given order.

![screenshot](https://dl.dropboxusercontent.com/u/993383/Cosa/screenshots/Screenshot%20from%202016-02-26%2015%3A15%3A48.png)

Shell has built-in instruction level trace to aid script debugging and
performance tuning. It prints the instruction cycle count, script
address, opcode (or full operation name), stack depth and
contents. Typical output in the Serial Monitor above.

Shell can be configured to trace full operation names or tokens.

The classical Arduino Blink sketch in the shell script language is
```
 13O{13H1000D13L1000DT}w
```
And with some extra spacing to make the operations easier to read.
```
 13 O { 13 H 1000 D 13 L 1000 D T } w
```
And with full instruction names and some formatting:
```
 13 output
 {
   13 high 1000 delay
   13 low  1000 delay
   true
 } while
```
A further compressed version (shorter):
```
 13 output
 {
   1000 13 over over high delay
   low delay
   true
 } while

 13O{1000,13ooHDLDT}w
```
And a slightly faster version:
```
 1000 13 dup output
 {
   over over high delay
   over over low delay
   true
 } while

 1000,13uO{ooHDooLDT}w
```
Scripts can be defined and executed from both Random Access Memory
(SRAM), Electrically Erasable Programmable Read-Only Memory (EEPROM),
and Program Memory (PROGMEM). The supporting data type `Script` and
macro for string literals `SCRIPT` can be used to define program
memory based scripts. All named scripts are persistent. Variables may
be written to EEPROM with the operation `z`. They are automatically
restored at startup together with defined scripts.

## Install

Download and unzip the Arduino-Shell library into your sketchbook
libraries directory. Rename from Arduino-Shell-master to Arduino-Shell.

The Shell library and examples should be found in the Arduino IDE
File>Examples menu.

## Instruction Set

Opcode | Parameters | Description | Forth
-------|:-----------|:------------|:-----
, | -- | no operation |
+ | x y -- x+y | addition | +
- | x y -- x-y | subtraction | -
* | x y -- x*y | multiplication | *
/ | x y -- x/y | division | /
% | x y -- x%y | modulo | MOD
# | x y -- x!=y | not equal |
= | x y -- x==y | equal | =
< | x y -- x<y | less than | <
> | x y -- x>y | greater than | >
~ | x -- ~x | bitwise not | NOT
& | x y -- x&y | bitwise and | AND
&#124; | x y -- x&#124;y | bitwise or | OR
^ | x y -- x^y | bitwise xor | XOR
@ | addr -- val | read variable | @
! | val addr -- | write variable | !
. | x -- | print number followed by one space | .
? | addr -- | print variable | ?
; | addr block -- | copy block and assign variable (function) |
: | addr -- | execute variable (function) |
\ | x1..xn n -- x1..xn | n > 0: mark stack frame with n-elements |
\ | x1..xn y1..ym n -- y1..ym | n < 0: remove stack frame with n-elements |
$ | n -- addr | address of n-element in frame |
a | -- bytes entries | allocated eeprom |
b | base -- | number print base | BASE
c | xn..x1 n -- | drop n stack elements |
d | x -- | drop | DROP
e | flag if-block else-block -- | execute block on flag | IF ELSE THEN
f | addr -- | forget variable |
g | xn..x1 n -- xn-1..x1 xn | rotate n-elements | ROLL
h | x y z -- (x*y)/z | scale | */
i | flag block -- | execute block if flag is true | IF THEN
j | xn..x1 -- xn..x1 n | stack depth | DEPTH
k | -- char | read character from input stream  | KEY
l | low high block( index -- ) -- | execute block from low to high | DO LOOP
m | -- | write new line to output stream | CR
n | x -- -x | negate | NEGATE
o | x y -- x y x | over | OVER
p | xn..x1 n -- xn..x1 xn | pick | PICK
q | x -- [x x] or 0 | duplicate if not zero | ?DUP
r | x y z --- y z x | rotate | ROT
s | x y -- y x | swap | SWAP
t | addr -- bool | write variable name to output stream | .NAME
u | x -- x x | duplicate | DUP
v | char -- | write character to output stream | EMIT
w | block( -- flag) -- | execute block while flag is true | BEGIN UNTIL
x | block -- | execute block | EXECUTE
y | -- | yield for multi-tasking scheduler |
z | addr -- | write variable to eeprom memory |
A | pin -- sample | analogRead(pin) |
C | xn..x1 -- | clear | ABORT
D | ms -- | delay |
E | period addr -- bool | check if timer variable has expired |
F | -- false | false | FALSE
H | pin -- | digitalWrite(pin, HIGH) |
I | pin -- | pinMode(pin, INPUT) |
K | -- [char true] or false | non-blocking read character from input stream | ?KEY
L | pin -- | digitalWrite(pin, LOW)  |
M | -- ms | millis() |
N | -- | no operation |
O | pin -- | pinMode(pin, OUTPUT) |
P | value pin -- | analogWrite(pin, value) |
R | pin --  bool | digitalRead(pin) |
S | -- | print stack contents | .S
T | -- true | true | TRUE
U | pin -- | pinMode(pin, INPUT_PULLUP) |
W | value pin -- | digitalWrite(pin, value) |
X | pin -- | digitalToggle(pin)  |
Z | -- | toggle trace mode |

## Special forms

The shell script language allows several special forms such as literal
values, blocks, control structures and argument frames.

### Boolean

Boolean values are true(-1) and false(0). Mapping to boolean may be
done with testing non-zero (0#). Defining true(-1) allows Boolean
values and relation operations to be directly combined with bitwise
operations.

The instructions to push the boolean values are _T_ and _F_.

### Literal Numbers

Integer numbers (decimal, binary and hexadecimal) may be used directly
in scripts. When the script is executed the value of the number is
pushed on the parameter stack. The statement:
```
 println((3 + (-5)) * 6);
```
may be written as the following script expression:
```
 3 -5 + 6 * . m
```
and compressed to:
```
 3,-5+6*.m
```
Binary literal numbers are prefixed with `0b`, and hexadecimal with
`0x` as in C.
```
 10 . 0b10 . 0x10 .
```

### Literal Characters

Quote (apostrophe) a character to push it on the parameter stack.
```
 'A .
```

### Variables

Variables are defined with `` `name ``. The operator will push the
address of the variable on the parameter stack. It may be accessed
using the operators fetch `@` and store `!`.
```
 42`x!
 `x@
```
The operator `?` can be used to print the value of a variable.
```
 `x?
```
It is short hand for:
```
 `x@.
```

### Blocks

Code blocks have the following form `{ code-block }`. When the script
is executed the address of the block is pushed on the parameter
stack. The block can be executed with the instruction _x_.
```
 { code-block } x
```
The operation `;` will copy a block to the heap and assign a
variable. Used in the form:
```
 `fun { code-block };
 `fun@x
```
The short hand for executing a function is `:`.
```
 `fun:
```
The instruction _f_ may be used to free the code block.
```
 `fun@f
```

### Control Structures

Control structures follow the same format at PostScript. They are also
in Reverse Polish Notation (RPN). The blocks are pushed on the
parameter stack before the control structure instruction. Below are
the control structures with full instruction names.
```
 bool { if-block } if
 bool { if-block } { else-block } ifelse

 low high { index loop-block } loop

 { while-block bool } while
```
The `loop-block` will recieve the current index on top of parameter
stack. The `while-block` should push a non-zero value to continue the
while loop otherwise zero (false) to terminate the loop.

The instructions are _i_ for `if`, _e_ for `ifelse`, _l_ for `loop`
and _w_ for `while`.

### Output Strings

Output strings have the following form `( output-string )`. When executed the
string within the parenthesis is written to the output stream. The
instruction _m_ will print a new-line (corresponds to forth cr).

### Stack Marker

A stack marker has the following form `[ code-block ]`. When executed the
number of stack elements generated by the code block is pushed on the
parameter stack.

### Frame Marker

A frame marker has the following form `n\ ... -n\` where _n_ is the
number of elements (parameters and locals) in the frame. Positive _n_
marks the frame and negative _n_ removes the frame stack elements
leaving any return values. Elements within the frame can be accessed
with `m$` where _m_ is the element index (1..n). The element address
is pushed on the parameter stack and the value may be accessed with
fetch `@` and store `!`.

Swap could be defined as:
```
 `swap {2\2$@1$@-2\};
```
which will mark a frame with two arguments, copy the second and then
the first argument, and last remove the frame, leaving the two return
values.

### Extended Instructions

Shell allows application extension with a virtual member function,
`trap()`. The function is called when the current instruction could not
be handled. The `trap()` function may parse any number of
instructions. Underscore `_` is used as the escape operation code.

## Example Scripts

### Blink

Turn board LED, pin 13, on/off with 1000 ms period.
```
 13 output
 {
   13 high 1000 delay
   13 low 1000 delay
   true
 } while
```
Script:
```
 13O{13H1000D13L1000DT}w
```

### Blink with digitalToggle

Toggle board LED, pin 13, on/off with 1000 ms period.
```
 13 output
 {
   13 digitalToggle 1000 delay
   true
 } while
```
Script:
```
 13O{13X1000DT}w
```

### Blink without delay

Turn board LED, pin 13, on/off without using delay. Use timer expired
instruction.
```
 13 output
 {
   1000 `timer ?expired
   { 13 digitalToggle } if
   true
 } while
```
Script:
```
 13O{1000`timer,E{13X}iT}w
```

### Blink controlled by on/off button

Turn board LED, pin 13, on/off with 1000 ms period if pin 2 is low.
```
 2 inputPullup
 13 output
 {
   2 digitalRead not
   {
     13 high 1000 delay
     13 low 1000 delay
   } if
   true
 } while
```
Script:
```
 2U13O{2R~{13H1000D13L1000D}iT}w
```

### Read Analog Pins

Read analog pins and print value in format "An = value".
```
 0 4 { ." A" dup . ." =" analogRead . cr } loop
```
Script:
```
 0,4{(A)u.(= )A.m}l
```
### Continously Read Analog Pins

Read analog pins (0..4) and print value continuously with 100 ms delay.
```
 {
   0 4
   {
     analogRead .
   } loop
   cr
   100 delay
   true
 } while
```
Script:
```
 {0,4{A.}lm100DT}w
```

### Termostat

Read analog pin 0, turn board LED on if value is within 100..200 else off.
```
 13 output
 {
   0 analogRead
   dup 100 < swap 200 > or not
   13 digitalWrite
   true
 } while
```
Script:
```
 13O{0Au100<s200>|~13WT}w
```

### Range check function

Check that a given parameter is within a range low to high.
````
 : within ( x low high -- bool )
   rot swap over swap > swap rot < or not ;

  10 5 100 within .
 -10 5 100 within .
 110 5 100 within .
```
Script:
```
 `within{rsos>sr<|~};
  10,5,100`within:.
 -10,5,100`within:.
 110,5,100`within:.
```

### Range check function with stack frame

Check that a given parameter is within a range low to high. Use a
stack frame for the three parameters.
````
 : within { x low high -- bool }
   x @ high @ > x @ low @ < or not ;

  10 5 100 within .
 -10 5 100 within .
 110 5 100 within .
```
Script:
```
 `within{3\1$@3$@>1$@2$@<|~-3\};
  10,5,100`within:.
 -10,5,100`within:.
 110,5,100`within:.
```

### Iterative Factorial (while)

Calculate factorial number of given parameter.
```
 : fac ( n -- n! )
   1 swap
   {
     dup 0>
       { swap over * swap 1- true }
       { drop false }
     ifelse
   } while ;

 5 fac .
```
Script:
```
 `fac{1s{u0>{so*s1-T}{dF}e}w};
 5`fac:.
```

### Iterative Factorial (loop)

Calculate factorial number of given parameter.
```
 : fac ( n -- n! | n > 0 )
   1 2 rot { * } loop ;

 5 fac .
```
Script:
```
 `fac{1,2r{*}l};
 5`fac:.
```

### Recursive Factorial

Calculate factorial number of given parameter.
```
 : fac ( n -- n! )
   dup 0>
     { dup 1- fac * }
     { drop 1 }
   ifelse ;

 5 fac .
```
Script:
```
 `fac{u0>{u1-`fac:*}{d1}e};
 5`fac:.
```

### Stack vector sum

Sum a vector of integers on stack. Use that stack marker to get number
of elements in vector.
```
 [ 1 2 3 4 5 6 7 8 ] 0 1 rot { drop + } loop
```
Script:
```
 [1,2,3,4,5,6,7,8]0,1r{d+}l
```

