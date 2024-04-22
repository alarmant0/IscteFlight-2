# Makefile for eval targets

# Target must be select
.DEFAULT_GOAL := error

CC = gcc
SED = sed

ifndef SOURCE
    $(error SOURCE is not defined)
endif

CFLAGS = -g -Wall -D_EVAL=$(SOURCE) -Wno-format-extra-args -lm

ifdef DEBUG
    CFLAGS += -D_EVAL_DEBUG=1
endif

error :
	$(error No target was selected)

.PHONY: clean

clean :
	rm -f cliente-eval
	rm -f servidor-eval
	rm -f cliente.c servidor.c

cliente.c : cliente.sed $(SOURCE)/cliente.c
	$(SED) -f cliente.sed $(SOURCE)/cliente.c > cliente.c

servidor.c : servidor.sed $(SOURCE)/servidor.c
	$(SED) -f servidor.sed $(SOURCE)/servidor.c > servidor.c

cliente-eval : cliente-eval.c eval.c cliente.c common.h eval.h
	$(CC) -D_CLIENTE eval.c cliente.c cliente-eval.c -o cliente-eval $(CFLAGS)
	rm cliente.c

servidor-eval : servidor-eval.c eval.c servidor.c common.h eval.h
	$(CC) -D_SERVIDOR  eval.c servidor.c servidor-eval.c -o servidor-eval $(CFLAGS)
	rm servidor.c