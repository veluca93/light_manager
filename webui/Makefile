.PHONY: all

all:
	cargo build -j $(shell grep -c '^processor' /proc/cpuinfo)

clean:
	cargo clean
