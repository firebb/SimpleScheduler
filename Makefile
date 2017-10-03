CC = g++ -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG) -lpthread
OBJECTS = scheduler.o

all: scheduler

scheduler: $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $(OBJECTS)

scheduler.o: scheduler.cpp task.h job.h execution_plan.h intermediate.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o scheduler
