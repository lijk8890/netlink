#created by ljk<ljk@xdja.com>
ifndef CC
CC = gcc
endif
CFLAGS += -g -O0 -Wall -fPIC
CFLAGS += -D_DEBUG
CFLAGS += -I./
LDFLAGS += -L./
LIBS += -ldl

.PHONY : all clean

SRCS += test.c

OBJS = $(SRCS:.c=.o)

TARGET = test

all : ${TARGET}

${TARGET} : ${OBJS}
	${CC} -o $@ $^ ${LDFLAGS} ${LIBS}
	@echo "$@"

%.o : %.c %.h
	${CC} ${CFLAGS} -o $@ -c $<

clean :
	rm -rf ${OBJS} ${TARGET}
