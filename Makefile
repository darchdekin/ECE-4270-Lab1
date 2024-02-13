mu-riscv: mu-riscv.c
	gcc -Wall -Wno-unused-result -g -O2 $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o *~ mu-riscv
