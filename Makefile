CC		= gcc
CFLAGS		= -Wall
RM		= rm -f
BINDIR		= ./bin
SRCS		= $(wildcard *.c)
PROGS		= $(patsubst %.c, %, $(SRCS))

all: $(PROGS)

%: %.c
	mkdir -p ${BINDIR}
	${CC} ${CFLAGS} -o ${BINDIR}/$@ $<

clean:
	${RM} ${BINDIR}/*
