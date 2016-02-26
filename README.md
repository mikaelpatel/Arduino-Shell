# Arduino-Shell

This library provides a forth style shell for Arduino
sketches. The shell uses a byte token threaded instruction set. The
tokens, characters, are chosen so that it is possible to write small
scripts directly. A token compiler is not required.

The classical blink sketch in the shell script language is
````
 13O{13H1000D13L1000DT}w
````
And with some extra spacing to make the operations easier to read.
````
 13 O { 13 H 1000 D 13 L 1000 D T } w
````
This encodes the forth statement (with mock arduino functions).
````
 13 pinModeOutput
 begin
   13 digitalWriteHigh
   1000 delay
   13 digitalWriteLow
   1000 delay
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
, | -- | no operation
; | -- | no operation
~ | x -- ~x | one's complement
@ | addr -- val | read variable
! | val addr -- | write variable
# | x y -- x!=y | not equal
= | x y -- x==y | equal
< | x y -- x<y | less than
> | x y -- x>y | greater than
& | x y -- x&y | and
^ | x y -- x^y | xor
+ | x y -- x+y | add
- | x y -- x-y | subtract
* | x y -- x*y | multiply
/ | x y -- x/y | divide
% | x y -- x%y | remainder
. | x -- | print
c | xn ... x1 -- | clear
d | x -- x x | duplicate
e | flag if-block else-block -- | execute block on flag
i | flag block -- | execute block if flag is true
l | n block(i -- ) -- | execute block n-times
n | x -- -x | negate
o | x y -- x y x | over
p | xn ... x1 n -- xn ... x1 xn | pick
r | x y z --- y z x | rotate
s | x y -- y x | swap
t | -- | toggle trace mode
u | x -- | drop
w | block( -- flag) -- | execute block while flag is true
x | script -- | execute
z | -- | print stack contents
A | pin -- sample | analogRead(pin)
D | ms -- | delay
F | -- 0 | false
H | pin -- | digitalWrite(pin, HIGH)
I | pin -- | pinMode(pin, INPUT)
U | pin -- | pinMode(pin, INPUT_PULLUP)
L | pin -- | digitalWrite(pin, LOW)
M | -- ms | millis()
N | -- | no operation
O | pin -- | pinMode(pin, OUTPUT)
P | value pin -- | analogWrite(pin, value)
R | pin --  value | digitalRead(pin)
T | -- -1 | true
W | value pin -- | digitalWrite(pin, value)

## Special forms

The shell script language allows the following special forms. First,
integer numbers may be used directly in scripts. When the script is
executed the value of the number is pushed on the parameter stack.

Second, blocks "{...}". They begin with left curley bracket and end
with a right curley bracket. When the script is executed the address
of the block is pushed on the parameter stack and can be used with
operation code; execute "x", loop "l", while "w", if-true "i", and
if-else "e". Blocks may be nested.
