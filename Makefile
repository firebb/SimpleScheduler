CC = g++ -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG) -lpthread
OBJECTS = scheduler.o

all: scheduler baseline

scheduler: $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $(OBJECTS)

baseline: baseline.cpp
	$(CC) -o $@ $<

scheduler.o: scheduler.cpp job.h execution_plan.h intermediate.h
	$(CC) $(CFLAGS) $<

format: *.cpp *.h
	clang-format -i -style=google *.cpp

clean:
	rm -f *.o scheduler baseline
