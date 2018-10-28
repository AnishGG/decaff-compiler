override CFLAGS += -O3 -flto -Wall -DYYERROR_VERBOSE -std=c++11
FLEX=flex 
GRAMMAR=bison -d
PREFIX ?= /usr/local
OBJ=compiler
BINDIR=$(PREFIX)/bin
CXX ?= g++

$(OBJ) : $(OBJ).tab.o lex.yy.o PrintVisitor.o main.o 
	$(CXX) $^ -o $@ $(CFLAGS) 

%.o : %.c 
	$(CXX) $^ -c $(CFLAGS)

%.o : %.cpp
	$(CXX) $^ -c $(CFLAGS)

${OBJ}.tab.c : $(OBJ).y
	$(GRAMMAR) $(OBJ).y

lex.yy.c : $(OBJ).l $(OBJ).tab.c
	$(FLEX) $(OBJ).l

clean :
	rm -f $(OBJ) lex.yy.c ${OBJ}.tab.c ${OBJ}.tab.h ${OBJ}.tab.o lex.yy.o main.o compiler.output PrintVisitor.o

all : ${OBJ}
