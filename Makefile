OBJ = compiler
CFLAGS += -O3 -flto -Wall -DYYERROR_VERBOSE -std=c++11 `llvm-config --cppflags --libs all --ldflags --system-libs`
CXX = g++
LEX = flex 
YACC = bison -d --report=all --warnings=all
TYPE = main_codegen_visitor.o

$(OBJ) : $(OBJ).tab.o lex.yy.o $(TYPE)
	$(CXX) -o $(OBJ) $(OBJ).tab.o lex.yy.o $(TYPE) $(CFLAGS) 

%.o : %.c 
	$(CXX) $^ -c $(CFLAGS)

%.o : %.cpp
	$(CXX) $^ -c $(CFLAGS)

${OBJ}.tab.c : $(OBJ).y
	$(YACC) $(OBJ).y

lex.yy.c : $(OBJ).l $(OBJ).tab.c
	$(LEX) $(OBJ).l

clean :
	rm -f $(OBJ) lex.yy.c ${OBJ}.tab.c ${OBJ}.tab.h ${OBJ}.tab.o lex.yy.o main_codegen_visitor.o compiler.output

all : ${OBJ}
