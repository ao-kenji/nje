#
# Makefile for nje
#
#	$Id: Makefile,v 1.1.1.1 2007/05/01 21:08:33 aoyama Exp $
#

PREFIX ?= /usr/local

PROG	= nje
SRCS	= nje.c

CFLAGS += -Wall

MAN	= nje.1

BINDIR ?= ${PREFIX}/bin
MANDIR ?= ${PREFIX}/man

.include <bsd.prog.mk>
