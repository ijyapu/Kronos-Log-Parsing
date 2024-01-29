CC = g++
FLAGS = -Wall -Werror -pedantic --std=c++17
LIB = -lboost_date_time
PROGRAM = ps7

.PHONY: all clean lint

all: $(PROGRAM)

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) -c $<

ps7: $(OBJECTS) main.o
	$(CC) $(FLAGS) -o $@ $^ $(LIB)

test: $(OBJECTS) test.o
	$(CC) $(FLAGS) -o $@ $^ $(LIB)

clean:
	rm *.o $(PROGRAM)

lint:
	cpplint *.cpp *.hpp
