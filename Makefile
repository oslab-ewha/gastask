all: gastask

CFLAGS = -g -Wall -DDEBUG

gastask: gastask.o GA.o task.o cpu.o mem.o conf.o report.o
	gcc -o gastask $^ -lm

gastask.o: gastask.h
GA.o: gastask.h
task.o: gastask.h
cpu.o: gastask.h
mem.o: gastask.h
conf.o: gastask.h
report.o: gastask.h
