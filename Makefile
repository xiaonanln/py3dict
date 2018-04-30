
.PHONY: all build test clean
all: build test bench

build:
	python setup.py build
	cp build/lib.linux-x86_64-2.7/py3dict.so ./

test: build
	python test_basic.py

clean:
	python setup.py clean

bench: build
	python bench.py

