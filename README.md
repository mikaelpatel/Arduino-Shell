# Arduino-Shell

This library provides a forth style shell for Arduino
sketches. The shell uses a byte token threaded instruction set. The
tokens, characters, are chosen so that it is possible to write small
scripts directly. A token compiler is not required. As forth scripts
are in Reversed Polish Notation (RPN).

![screenshot](https://dl.dropboxusercontent.com/u/993383/Cosa/screenshots/Screenshot%20from%202016-02-26%2015%3A15%3A48.png)

The shell has trace built-in. It will display the execution of the
script with current script address, opcode, stack depth and
contents). Typical output in the Serial Monitor above.

The classical blink sketch in the shell script language is
````
 13O{13H1000D13L1000DT}w
````
And with some extra spacing to make the operations easier to read.
````
 13 O { 13 H 1000 D 13 L 1000 D T } w
````
And with full instruction names.
````
 13 Output { 13 High 1000 Delay 13 Low 1000 Delay True } while
````
This encodes the forth statement (with mock arduino functions).
````
 13 output
 begin
   13 high 1000 delay
   13 low  1000 delay
 repeat
````
A further compressed version (shorter):
````
 13O{1000,13ooHDLDT}w
````
And a faster version:
````
 1000,13dO{ooHDooLDT}w
````

## Install

Download and unzip the Arduino-Shell library into your sketchbook
libraries directory. Rename from Arduino-Shell-master to Arduino-Shell.

The Shell library and examples should be found in the Arduino IDE
File>Examples menu.

## Instruction Set

Opcode | Parameters | Description
--------|------------|------------
\0 | -- | exit script
, | -- | no operation
( | -- | output string start
) | -- | output string end
{ | -- block | block start
} | -- | block end
~ | x -- ~x | bitwise not
@ | addr -- val | read variable
! | val addr -- | write variable
# | x y -- x!=y | not equal
= | x y -- x==y | equal
< | x y -- x<y | less than
> | x y -- x>y | greater than
& | x y -- x&y | bitwise and
^ | x y -- x^y | bitwise xor
+ | x y -- x+y | addition
- | x y -- x-y | subtraction
* | x y -- x*y | multiplication
/ | x y -- x/y | division
% | x y -- x%y | modulo
. | x -- | print
c | xn ... x1 -- | clear
d | x -- x x | duplicate
e | flag if-block else-block -- | execute block on flag
i | flag block -- | execute block if flag is true
k | -- char or -1 | read character from input stream
l | n block -- | execute block n-times
m | -- | write new line to output stream
n | x -- -x | negate
o | x y -- x y x | over
p | xn ... x1 n -- xn ... x1 xn | pick
q | x -- x x or 0 | duplicate if not zero
r | x y z --- y z x | rotate
s | x y -- y x | swap
t | -- | toggle trace mode
u | x -- | drop
v | x -- | write character to output stream
w | block( -- flag) -- | execute block while flag is true
x | script/block -- | execute script or block
z | -- | print stack contents
A | pin -- sample | analogRead(pin)
D | ms -- | delay
F | -- 0 | false
H | pin -- | digitalWrite(pin, HIGH)
I | pin -- | pinMode(pin, INPUT)
L | pin -- | digitalWrite(pin, LOW)
M | -- ms | millis()
N | -- | no operation
O | pin -- | pinMode(pin, OUTPUT)
P | value pin -- | analogWrite(pin, value)
R | pin --  value | digitalRead(pin)
T | -- -1 | true
U | pin -- | pinMode(pin, INPUT_PULLUP)
W | value pin -- | digitalWrite(pin, value)

## Special forms

The shell script language allows several special forms:

### Literal Numbers

Integer numbers may be used directly in scripts. When the script is
executed the value of the number is pushed on the parameter stack.

### Blocks

Code blocks have the following form "{...}". They begin with left
curley bracket and end with a right curley bracket. When the script is
executed the address of the block is pushed on the parameter stack.

### Control Structures

Control structures follow the same format at PostScript. They are also
Reversed Polish Notation. The block or blocks are push on the stack
before the control structure instruction. Below are the difference
control structure with full instruction names.

    bool { if-block } if
    bool { if-block } { else-block } ifelse

    n { loop-block } loop

    { while-block bool } while

The instructions are abbreviated i,e,l and w.

### Output Strings

Output strings have the following form "(string)". When executed the
string within the parenthesis is written to the output stream.

## Example Scripts

### Blink

Turn board LED, pin 13, on/off with 1000 ms period.

````
13 output
{
  13 high 1000 delay
  13 low 1000 delay
  true
} while
````
Script:
````
13O{13H1000D13L1000DT}w
````

### Termostat

Read analog pin 0, turn board LED on if value [100..200] else off.
````
13 output
{
  0 analogRead
  dup 100 < swap 200 > or not
  13 digitalWrite
  true
} while
````
Script:
````
13O{0Ad100<s200>|~13WT}w
````

### Blink with on/off button

Turn board LED, pin 13, on/off with 1000 ms period if pin 2 is low.
````
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
````
Script:
````
2U13O{2R~{13H1000D13L1000D}iT}w
````

### Iterative Factorial
````
: fac ( n -- n! )
  1 swap
  {
    dup 0>
      { swap over * swap 1- true }
      { drop false }
    ifelse
  } while ;
5 fac .
````
Script:
````
5,1s{d0>{so*s1-T}{uF}e}w.
````

### Range check [low..high]
````
: within ( x low high -- bool )
  rot swap over swap > swap rot < or not ;
10 5 100 within .
-10 5 100 within .
110 5 100 within .
````
Script:
````
10,5,100rsos>sr<|~.
-10,5,100rsos>sr<|~.
110,5,100rsos>sr<|~.
````




