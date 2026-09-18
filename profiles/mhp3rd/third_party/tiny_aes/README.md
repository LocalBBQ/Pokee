# tiny-AES-c

`aes.c` and `aes.h` are unmodified copies of [tiny-AES-c](https://github.com/kokke/tiny-AES-c) at commit `23856752fbd139da0b8ca6e471a13d5bcc99a08d`, released into the public domain under the Unlicense (see `UNLICENSE`).

Only the installer uses it, through `host/install/executable_preparation.cpp`, for AES-128 in CBC mode. The build compiles `aes.c` as C++.
