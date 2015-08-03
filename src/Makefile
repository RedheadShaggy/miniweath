CFLAGS += -Os -pedantic-errors

all: weather

%.o: %.c jsmn.h
	$(CC) -Wall -c $(CFLAGS) $< -o $@

weather: weather.o
	$(CC) $(LDFLAGS) -L. $< -o $@ -ljsmn

weather.o: weather.c libjsmn.a

clean:
	rm -f *.o weather
