single-file-libs
================

A collection of public domain single file libraries. This was inspired by Sean T. Barrett's
[stb](https://github.com/nothings/stb) libs.

| Libray               | Description                                           |
| -------------------- | ----------------------------------------------------- |
| **sgv_glmath.h**     | 3d matrix transforms like scaling, perspective, etc.  |

How to use
----------

Suppose your project has `src1.c`, `src2.c`, and `src3.c`, and you wish to use
`sgv_some_lib.h`. In one of the files, `#define SGV_LIB_IMPLEMENTATION` before
including the header file. In the other files, simply include the header as usual.

For instance, in `src2.c` and `src3.c`, include the lib as usual.

```
...
#include "sgv_some_lib.h"
```

In `src1.c`, define `SGV_LIB_IMPLEMENTATION` and then include the header.

```
#define SGV_LIB_IMPLEMENTATION
#include "sgv_some_lib.h"
```
