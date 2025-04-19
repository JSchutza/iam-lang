# Makefile for IAM Language C++ version

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: iam_lang

iam_lang: iam_lang.cpp
	$(CXX) $(CXXFLAGS) -o iam_lang iam_lang.cpp

clean:
	rm -f iam_lang iam_lang.exe *.o

run: iam_lang
	./iam_lang

test: iam_lang
	./iam_lang examples.iam 