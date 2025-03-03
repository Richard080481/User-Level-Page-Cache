# Download SPDK

## Download SPDK Source Code
~~~{.sh}
git clone https://github.com/spdk/spdk
cd spdk
git submodule update --init
~~~

## Prerequisites

~~~{.sh}
./scripts/pkgdep.sh
~~~

## Build

Linux:

~~~{.sh}
./configure
make
~~~

## Unit Tests

~~~{.sh}
./test/unit/unittest.sh
~~~

You will see several error messages when running the unit tests, but they are
part of the test suite. The final message at the end of the script indicates
success or failure.

# Download FIO

## Download FIO Source Code

```bash
    git clone https://github.com/axboe/fio
    make
```

## Compiling SPDK

Run the SPDK configure script to enable fio
```bash
    cd spdk
    ./configure --with-fio=/path/to/fio/repo
    make
```
