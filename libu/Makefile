MAP?=thread
OBJ=reduce_linear.o bio.o map_$(MAP).o crt1.o __start_main.o
CFLAGS=-Wall -std=gnu99 -fPIC -I ~tmijieux/public/include -O3 -fopenmp -funroll-all-loops

LDFLAGS=-L ~tmijieux/public/lib/ -lgc

all: libu.a # libu.so

libu.a: $(OBJ)
	$(RM) $@
	ar cr $@ $^

__start_main.o:
	$(CC) $(CFLAGS) -Wall -S __start_main.c -o __start_main.s 
	sed -i 's/__start_main/__start\.main/g' __start_main.s
	$(CC) -c __start_main.s -o $@ $(LDFLAGS)

libu.so: $(OBJ)
	$(CC) -o $@ -fPIC -shared -pthread $^

clean:
	$(RM) *.o

fullclean:
	$(RM) libu.a libu.so
