# INFORMATION ON SAT SOLVER #
IPASIRSOLVER ?= cadical
IPASIRLIBDIR = lib/cadical-2.1.0/build/
# ========================= #
# INFORMATION ON c++ compiler #
CC	=	clang++
# =========================== #

TARGET=taas-fudge
CFLAGS	?=	-Wall -DNDEBUG -O3 -std=c++0x
LINK	=	$(CC) $(CFLAGS)

all: $(TARGET)
clean:
	rm -f $(TARGET) *.o

taas-fudge: taas-fudge.o
	${LINK} -o $@ taas-fudge.o `pkg-config --cflags --libs glib-2.0` -lm -L$(IPASIRLIBDIR) -l$(IPASIRSOLVER)

taas-fudge.o: taas-fudge.cpp ipasir.h
	$(CC) $(CFLAGS) -c taas-fudge.cpp `pkg-config --cflags --libs glib-2.0` -lm
