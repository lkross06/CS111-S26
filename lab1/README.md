## UID: 506372478

## Pipe Up

Manually implements the pipe (|) operator (e.g. `./pipe a b c ...` is identical to `a | b | c ...`).

## Building

From the project directory, run `make` to build the project, creating the executable `pipe`.

```
$ make
cc -std=c17 -Wpedantic -Wall -O2 -pipe -fno-plt -c -o pipe.o pipe.c
cc -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now  pipe.o -o pipe
```

## Running

From the project directory, run `./pipe <args>` to execute the program.

```
$ ls
Makefile  pipe  pipe.c  pipe.o  README.md  test_lab1.py
$ ./pipe ls
Makefile  pipe  pipe.c  pipe.o  README.md  test_lab1.py
```
```
$ ls | cat | wc
      6       6      51
$ ./pipe ls cat wc
      6       6      51
```

## Cleaning up

From the project directory, run `make clean` to clean up the directory by removing the executable and dependencies.

```
$ make clean
rm -f pipe.o pipe
```