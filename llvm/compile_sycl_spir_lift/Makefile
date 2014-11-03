test: kernel.cpp dummy.cpp run.cpp
	/home/rnburn/proj/llvm/build/bin/clang++ -c -S -emit-llvm -std=sycl kernel.cpp
	llvm-as kernel.ll -o kernel.bc
	opt -f -load /home/rnburn/proj/llvm/build/lib/LLVMSplitHybrid.so -split_hybrid  kernel.bc > host.bc
	llvm-dis host.bc -o host.ll
	clang++ -c -S -emit-llvm pull_kernel.cpp
	clang++ -c -S -emit-llvm dummy.cpp
	llvm-link host.ll pull_kernel.ll dummy.ll -o run.bc
	clang++ run.bc

clean:
	rm *.bc
	rm *.ll
	rm a.out
