all: finalEx.cpp
	g++ finalEx.cpp -o finalEx
all-GDB: finalEx.cpp
	g++ -g finalEx.cpp -o finalEx