# Arduino-Shell

This library provides a forth style shell for Arduino sketches. Shell
uses a byte token threaded instruction set. The tokens, characters,
are chosen so that it is possible to write small scripts directly. A
token compiler is not required. As forth scripts are in Reverse Polish
Notation (RPN).

![screenshot](https://dl.dropboxusercontent.com/u/993383/Cosa/screenshots/Screenshot%20from%202016-02-29%2011%3A00%3A13.png)

The shell has built-in instruction level trace. It prints the
instruction cycle count, script address, opcode, stack depth and
contents). Typical output in the Serial Monitor above.

The classical blink sketch in the shell script language is
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
 13O{1000,13ooHDLDT}w

 13 output
 {
   1000 13 over over high delay
   low delay
   true
 } while
```
And a slightly faster version:
```
 1000,13uO{ooHDooLDT}w

 1000 13 dup output
 {
   over over high delay
   over over low delay
   true
 } while
```

## Install

Download and unzip the Arduino-Shell library into your sketchbook
libraries directory. Rename from Arduino-Shell-master to Arduino-Shell.

The Shell library and examples should be found in the Arduino IDE
File>Examples menu.

## Instruction Set

Opcode | Parameters | Description | Forth
-------|:-----------|:------------|:-----
, | -- | no operation |
; | block1 -- block2 | allocate block |
~ | x -- ~x | bitwise not | NOT
@ | addr -- val | read variable | @
! | val addr -- | write variable | !
# | x y -- x!=y | not equal |
= | x y -- x==y | equal | =
< | x y -- x<y | less than | <
> | x y -- x>y | greater than | >
& | x y -- x&y | bitwise and | AND
&#124; | x y -- x&#124;y | bitwise or | OR
^ | x y -- x^y | bitwise xor | XOR
+ | x y -- x+y | addition | +
- | x y -- x-y | subtraction | -
* | x y -- x*y | multiplication | *
/ | x y -- x/y | division | /
% | x y -- x%y | modulo | MOD
. | x -- | print number followed by one space | .
$ | x1..xn n -- x1..xn | n > 0: mark stack frame with n-elements |
$ | x1..xn y1..ym n -- y1..ym | n < 0: remove stack frame with n-elements |
_ | n -- addr | address of n-element in frame |
b | xn..x1 n -- | drop n stack elements |
d | x -- | drop | DROP
e | flag if-block else-block -- | execute block on flag | IF ELSE THEN
f | block -- | free block |
g | xn..x1 n -- xn-1..x1 xn | rotate n-elements | ROLL
h | x y z -- (x*y)/z | scale | */
i | flag block -- | execute block if flag is true | IF THEN
j | xn..x1 -- xn..x1 n | stack depth | DEPTH
k | -- [char true] or false | non-blocking read character from input stream |
l | n block -- | execute block n-times | DO LOOP
m | -- | write new line to output stream | CR
n | x -- -x | negate | NEGATE
o | x y -- x y x | over | OVER
p | xn..x1 n -- xn..x1 xn | pick | PICK
q | x -- [x x] or 0 | duplicate if not zero | ?DUP
r | x y z --- y z x | rotate | ROT
s | x y -- y x | swap | SWAP
u | x -- x x | duplicate | DUP
v | char -- | write character to output stream | EMIT
w | block( -- flag) -- | execute block while flag is true | BEGIN UNTIL
x | block -- | execute block | EXECUTE
y | -- | yield for multi-tasking scheduler |
A | pin -- sample | analogRead(pin) |
C | xn..x1 -- | clear | ABORT
D | ms -- | delay |
F | -- false | false | 0
H | pin -- | digitalWrite(pin, HIGH) |
I | pin -- | pinMode(pin, INPUT) |
K | -- char | read character from input stream  | KEY
L | pin -- | digitalWrite(pin, LOW)  |
M | -- ms | millis() |
N | -- | no operation |
O | pin -- | pinMode(pin, OUTPUT) |
P | value pin -- | analogWrite(pin, value) |
R | pin --  bool | digitalRead(pin) |
S | -- | print stack contents | .S
T | -- true | true | -1
U | pin -- | pinMode(pin, INPUT_PULLUP) |
W | value pin -- | digitalWrite(pin, value) |
Z | -- | toggle trace mode |

## Special forms

The shell script language allows several special forms and
instructions for literal values and control structures.

### Boolean

Boolean values are true(-1) and false(0). Mapping to boolean may be
done with testing non-zero (0#). Boolean value can be directly
combined with bitwise operations.

The instructions to push the boolean values are _T_ and _F_.

### Literal Numbers

Integer numbers (decimal, binary and hexadecimal) may be used directly
in scripts. When the script is executed the value of the number is
pushed on the parameter stack. The statement;
```
print((3 + (-5)) * 6)
```
may be written as the following script expression:
```
3 -5 + 6 * .
```
and compressed to:
```
3,-5+6*.
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

Variables are defined with `\name`. The operator will return the
address of the variable. It may be accessed using the operators `!`
and `@`.
```
42\x!
\x@
```

### Blocks

Code blocks have the following form `{ code-block }`. They begin with left
curley bracket and end with a right curley bracket. When the script is
executed the address of the block is pushed on the parameter
stack. The block can be executed with the instruction _x_.
```
{ code-block }x
```
The code block suffix `;` will copy the block to the heap. This can be
used to create a named function by assigning the block to a variable.
```
{ code-block };\fun!
\fun@x
\fun@f
```
The instruction _f_ may be used to free the code block.
```
\fun@f
```

### Control Structures

Control structures follow the same format at PostScript. They are also
Reverse Polish Notation (RPN). The block(s) is/are pushed on the stack
before the control structure instruction. Below are the control
structures with full instruction names.
```
bool { if-block } if
bool { if-block } { else-block } ifelse

n { loop-block } loop

{ while-block bool } while
```
The instructions are _i_, _e_, _l_ and _w_.

### Output Strings

Output strings have the following form `( output-string )`. When executed the
string within the parenthesis is written to the output stream. The
instruction _m_ will print a new-line (corresponds to forth cr).

### Stack Marker

A stack marker has the following form `[ code-block ]`. When executed the
number of stack elements generated by the code block is pushed on the
parameter stack.

### Frame Marker

A frame marker has the following form `n$ ... -n$` where _n_ is the
number of parameters in the frame. Positive _n_ marks the frame and
negative _n_ removes the frame stack elements leaving any return
values. Elements within the frame can be accessed with `m_` where _m_
is the element index (1..n). The element address is pushed on the
parameter stack.

Swap could be defined as:
```
2$2_@1_@-2$
```
which will mark a frame with two arguments, copy the second and then
the first argument, and last remove the frame, leaving the two values.

### Extended Instructions

Shell allows application extension with a virtual member function,
trap(). The function is called when the current instruction could not
be handled. The trap() function may parse any number of
instructions. The backtick (aka grave accent) is reserved as an escape
operation code.

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

### Read Analog Pins

Read analog pins and print value in format "An = value".
```
0 5 { ." A" dup . ." =" dup analogRead . cr 1+ } loop drop
```
Script:
```
0,5{(A)u.(= )uA.m1+}ld
```
### Continously Read Analog Pins

Read analog pins and print value continuously with 1000 ms delay.
```
{
  5 dup
  {
    dup analogRead . 1-
  } loop
  cr drop
  1000 delay
  true
} while
```
Script:
```
{5u{uA.1-}lmd1000DT}w
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

### Blink with on/off button

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

### Iterative Factorial

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
{1s{u0>{so*s1-T}{dF}e}w}\a0!
5,0@x.
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
{rsos>sr<|~}\a0!
10,5,100,0@x.
-10,5,100,0@x.
110,5,100,0@x.
```

### Stack vector sum

Sum a vector of integers on stack. Use that stack marker to get number
of elements in vector.
```
[ 1 2 3 ] 0 swap { + } loop
```
Script:
```
[1,2,3]0s{+}l
```
