CC	= gcc
CFLAGS	= -O0 -Wall
ALLPROGRAMS	= dining_philosophers

all: $(ALLPROGRAMS)

$(ALLPROGRAMS):	din_ph.c
	$(CC) $(CFLAGS) $^ -o $(ALLPROGRAMS)

.PHONY: clean

clean:
	@rm -rf $(ALLPROGRAMS)
