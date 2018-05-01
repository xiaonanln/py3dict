
.PHONY: all build test clean
all: clean build test bench

build:
	python setup.py build
	cp build/lib.linux-x86_64-2.7/*.so ./

test: clean build
	python test_basic.py

clean:
	python setup.py clean
	-rm -rf *.so
	-rm -rf build
	

bench: clean build
	python bench.py

