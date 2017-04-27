# André LaMothe's "Teach Yourself Game Programming in 21 Days"
Here's my trek through this great book from 1996.

## Setup
I used Borland's Turbo C v2.01 to compile the examples. I also needed to set the memory model to 'Large' so it could see the far pointers.

André used Microsoft C/C++ 7.0; several things have to be changed for this to work in TC:

| MSCC7        | TC2         |
| ------------ | ----------- |
| `<memory.h>` | `<mem.h>`   |
| `_ffree`     | `farefree`  |
| `_fmalloc`   | `farmalloc` |
| `_fmemset`   | `memset`    |
| `_inp`       | `inp`       |
| `_int86`     | `int86`     |
| `_outp`      | `outp`      |

Also, TC doesn't support `// single line comments`, so it's `/* olde schoole time */`.

## TODO
* Chapter 3, Exercise 5: Try to make `Plot_Pixel_Fast()` even faster!
* Chapter 3, Exercise 6: Using the keyboard as an input device, try to mkae a crude drawing program that allows the user to change colors and move a pen on the screen.
