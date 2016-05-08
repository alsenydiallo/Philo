FLAGS = -std=c99 -Wall -pedantic
COMP = gcc

philo: philo.c random.c random.h
	$(COMP) $(FLAGS) philo.c random.c -lm -o philo

clean:
	rm philo

