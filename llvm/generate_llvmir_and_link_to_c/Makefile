a.out: main.cpp noise.c
	clang++ -g main.cpp -o codegen `llvm-config --cxxflags --ldflags --libs core` -lpthread -lcurses -ldl
	./codegen 2> gen.ll
	clang -c -S -emit-llvm noise.c
	llvm-link noise.ll gen.ll -o run.bc
	clang -o run run.bc
