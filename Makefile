test:
	cd tests/build && make && ./erc-test

build:
	cd build && make && ./erc

.PHONY: test build
