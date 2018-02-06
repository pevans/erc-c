STATIC_ANALYSIS=

test:
	cd tests/build && make && ./erc-test

build:
	cd build && make && ./erc

cmake:
	cd build && cmake ..
	cd tests/build && cmake ..

static:
	cd build && STATIC_ANALYSIS=1 cmake ..
	cd tests/build && STATIC_ANALYSIS=1 cmake ..

.PHONY: test build cmake static
