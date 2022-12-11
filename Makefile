# 此部分对 VSCode 用户友好，并提供一些可扩展的操作
# 1. make 时将创建 build 目录并使用 qmake 构建(核心数为机器最大核心数)
# 2. make run 时将运行此程序(程序构建位置将在: build/src/spark-store)
# 3. make package 用来提供产出包功能?

CPUS=$(shell nproc)
PROGRAM=build/src/spark-store

all:
	mkdir -p build
	cd build && qmake ..
	cd build && make -j$(CPUS)

run: all
	@# exec $(shell find build/ -maxdepth 1 -type f -executable)
	exec $(PROGRAM)
