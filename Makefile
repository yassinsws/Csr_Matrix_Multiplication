.PHONY: all clean

CC:=gcc
CFLAGS+=-no-pie -Wall -Wextra -Wpedantic -O3

all: main

main: main.c csrlib.c matr_mult_csr.S matr_mult_csr_SIMD.S matr_mult_csr2.S 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf main