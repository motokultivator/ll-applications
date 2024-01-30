hello.bin: hello.elf esputil
	esputil/esputil mkbin hello.elf $@

init.o: init.S
	riscv64-linux-gnu-as -march=rv32imc -mabi=ilp32 init.S -o init.o

hello.elf: main.c esp32c3.ld esp32c3.h init.o
	riscv64-linux-gnu-gcc -W -Wall -Wextra -Wno-error -Wundef -Wshadow -pedantic \
	-Wdouble-promotion -fno-common -Wconversion -fno-stack-protector -fno-mudflap \
	-march=rv32imc -mabi=ilp32 -Os -ffunction-sections -fdata-sections \
	-I. $(CFLAGS) -Tesp32c3.ld -nostdlib -nostartfiles -static -Wl,--gc-sections $(LDFLAGS) \
	-D_FORTIFY_SOURCE=0 \
	 init.o main.c -o $@ 

flash: esputil hello.bin
	PORT=/dev/ttyACM0 esputil/esputil flash 0 hello.bin

esputil:
	cd esputil && $(MAKE)

clean:
	rm -rf *.o *.elf *.bin
