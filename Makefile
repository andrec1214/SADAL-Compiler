CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

BINS = sadal-lex sadal-parse sadal

all: $(BINS)

sadal-lex: prog1.o lex.o
	$(CXX) $(CXXFLAGS) -o $@ $^

sadal-parse: prog2.o parser.o lex.o
	$(CXX) $(CXXFLAGS) -o $@ $^

sadal: prog3.o parserInterp.o val.o lex.o
	$(CXX) $(CXXFLAGS) -o $@ $^

lex.o: lex.cpp lex.h
parser.o prog2.o: parser.h lex.h
parserInterp.o prog3.o: parserInterp.h lex.h val.h
val.o: val.cpp val.h

test: all
	./tests/run.sh

clean:
	rm -f *.o $(BINS)

.PHONY: all test clean
