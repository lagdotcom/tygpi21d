# André LaMothe's "Teach Yourself Game Programming in 21 Days"
Here's my trek through this great book from 1996.

## Setup
I used Borland's Turbo C v2.01 to compile the examples. I also needed to set the memory model to 'Large' so it could see the far pointers.

André used Microsoft C/C++ 7.0; several things have to be changed for this to work in TC:

| MSCC7        | TC2       |
| ------------ | --------- |
| `<memory.h>` | `<mem.h>` |
| `_int86`     | `int86`   |
| `_inp`       | `inp`     |
| `_outp`      | `outp`    |
| `_fmemset`   | `memset`  |

Also, TC doesn't support `// single line comments`, so it's `/* olde schoole time */`.
