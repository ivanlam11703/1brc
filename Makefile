all:1brc

1brc: 1brc.cpp
	g++ 1brc.cpp -o 1brc -ggdb -Wall -Wextra -pedantic -O3

clean:
	rm 1brc
