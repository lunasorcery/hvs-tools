CXX ?= clang

all: build/bindump build/unjam

build/bindump: bindump/main.cpp
	mkdir -p build/
	$(CXX) bindump/main.cpp -o build/bindump -std=c++11 -lstdc++ -Wall -Wextra -Wimplicit-fallthrough

build/unjam: unjam/main.cpp
	mkdir -p build/
	$(CXX) unjam/main.cpp -o build/unjam -std=c++11 -lstdc++ -Wall -Wextra

install: build/bindump build/unjam
	cp build/bindump /usr/local/bin/hvsbindump
	cp build/unjam /usr/local/bin/hvsunjam

clean:
	rm -rf build/
