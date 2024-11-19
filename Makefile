# SHELL = /bin/bash
CC = gcc
FLAGS = -std=c99 -O0 -Wall -Werror -g -pedantic

climate: climate.c
	$(CC) $(FLAGS) climate.c -o climate

clean:
	rm -f climate
	rm -rf *.dSYM

testOutput: climate
	./climate data_tn.tdv data_wa.tdv data_multi.tdv > output.txt

