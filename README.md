# Arduino-Shell

This library provides a forth style shell for Arduino
sketches. Shell uses a byte token threaded instruction set. The
tokens, characters, are chosen so that it is possible to write small
scripts directly. A token compiler is not required. As forth scripts
are in Reverse Polish Notation (RPN).

![screenshot](https://dl.dropboxusercontent.com/u/993383/Cosa/screenshots/Screenshot%20from%202016-02-26%2015%3A15%3A48.png)

Shell has trace built-in. It will display the execution of the
script with current script address, opcode, stack depth and
contents). Typical output in the Serial Monitor above.

The classical blink sketch in the shell script language is
```
 13O{13H1000D13L1000DT}w
```
And with some extra spacing to make the operations easier to read.
```
 13 O { 13 H 1000 D 13 L 1000 D T } w
```
And with full instruction names.
```
 13 Output { 13 High 1000 Delay 13 Low 1000 Delay True } while
```
This encodes the forth statement (with mock arduino functions).
```
 13 output
 begin
   13 high 1000 delay
   13 low  1000 delay
 repeat
```
A further compressed version (shorter):
```
 13O{1000,13ooHDLDT}w
```
And a faster version:
```
 1000,13dO{ooHDooLDT}w
```

## Install

Download and unzip the Arduino-Shell library into your sketchbook
libraries directory. Rename from Arduino-Shell-master to Arduino-Shell.

The Shell library and examples should be found in the Arduino IDE
File>Examples menu.

## Instruction Set

Opcode | Parameters | Description | Forth
--------|------------|------------|------
, | -- | no operation |
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
a | block1 len -- block2 | allocate block |
b | xn..x1 n -- | drop n stack elements |
c | xn..x1 -- | clear | ABORT
d | x -- | drop | DROP
e | flag if-block else-block -- | execute block on flag | IF ELSE THEN
f | block -- | free block |
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
t | -- | toggle trace mode |
u | x -- x x | duplicate | DUP
v | char -- | write character to output stream | EMIT
w | block( -- flag) -- | execute block while flag is true | BEGIN UNTIL
x | script/block -- | execute script/block | EXECUTE
y | -- | yield for multi-tasking scheduler |
z | -- | print stack contents | .S
A | pin -- sample | analogRead(pin) |
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
T | -- true | true | -1
U | pin -- | pinMode(pin, INPUT_PULLUP) |
W | value pin -- | digitalWrite(pin, value) |

## Special forms

The shell script language allows several special forms and instructions.

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

### Literal Characters

Quote (back-tick) a character to push it on the parameter stack.

### Variables

The Shell is contains a paramter stack and a variable table. The size
of the stack (STACK_MAX) and variable table (VAR_MAX) are given as
template parameters. The variable(0) is assigned and accessed below
using the operators `!` and `@`.
```
42,0!
0@
```
The instructions to read and write a variable use an address/index
within the variable block (0..VAR_MAX-1).

### Blocks

Code blocks have the following form `{ code-block }`. They begin with left
curley bracket and end with a right curley bracket. When the script is
executed the address of the block is pushed on the parameter stack.

The code block suffix `\` will push the length of the block in the
parameter stack. This can be used with _a_ to allocate and copy the
block to the heap. The instruction _f_ may be used to free the
code block.

A code block can be copied and assigned to a variable to create script
function.
```
{ code-block }\a0!
0@x
0@f
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
The instructions are _i_,_e_,_l_ and _w_.

### Output Strings

Output strings have the following form `( output-string )`. When executed the
string within the parenthesis is written to the output stream. The
instruction _m_ will print a new-line (corresponds to forth cr).

### Stack Marker

A stack marker has the following form `[ code-block ]`. When executed the
number of stack elements generated by the code block is pushed on the
parameter stack.

### Extended Instructions

Shell allows application extension with a virtual member function,
trap(). The function is called when the current instruction could not
be handled. The trap() function may parse any number of instructions.

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
