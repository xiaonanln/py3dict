
.PHONY: all build test clean
all: build test

build:
	python setup.py build

test:
	python test.py

clean:
	python setup.py clean