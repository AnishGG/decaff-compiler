compiler: 	compiler.y compiler.l
			bison -d compiler.y
			flex compiler.l
			gcc -o $@ compiler.tab.c lex.yy.c -lfl
