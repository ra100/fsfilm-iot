.PHONY: all build upload uploadfs upload-all test clean

all: build

build:
	pio run -e d1

upload:
	pio run --target upload -e d1

uploadfs:
	pio run --target uploadfs -e d1

upload-all: upload uploadfs

test:
	./run_tests.sh
	@echo "Running JavaScript tests..."
	node test/*.test.js

clean:
	pio run --target clean -e d1