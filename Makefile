TEST_DIR 			= ./test
SRC_DIR 			= src
BUILD_DIR 			= build
INCLUDE_DIR 		= include
AUTO_INCLUDE_DIR 	= build/include

LLVM_BIN_DIR    = $(shell brew --prefix llvm)/bin
COMPILER		= clang++
BISON_FLAGS		= -Wconflicts-sr -Wconflicts-rr -Wcounterexamples
LLVM_FLAGS		= $(shell ${LLVM_BIN_DIR}/llvm-config --cxxflags --ldflags --system-libs --libs core)
OTHER_FLAGS 	= -w -Wc++11-extensions -fcolor-diagnostics -fansi-escape-codes -Wdelete-incomplete 

FLEX_CPP		= build/src/lex.yy.cpp
BISON_HPP		= build/include/parser.tab.hpp build/include/location.hpp
BISON_CPP 		= build/src/parser.tab.cpp
BASE_CPP 		= $(wildcard src/*.cpp)
BASE_HPP		= $(wildcard include/*.hpp) $(wildcard include/**/*.hpp)

CPP 			= ${BASE_CPP} ${FLEX_CPP} ${BISON_CPP}
HPP				= ${BASE_HPP} ${BASE_HPP}

build/prog: ${CPP} ${HPP}
	${COMPILER} -g ${LLVM_FLAGS} ${OTHER_FLAGS} -I${INCLUDE_DIR} -I${AUTO_INCLUDE_DIR} ${CPP} -o build/prog

${FLEX_CPP}: src/lexer.l
	flex -o build/src/lex.yy.cpp $<

${BISON_CPP} ${BISON_HPP}: src/parser.y
	bison ${BISON_FLAGS} -Hbuild/include/parser.tab.hpp -o build/src/parser.tab.cpp $<; \
	mv build/src/location.hpp build/include

run: build/prog
	./build/prog ${I}

cfg_graph:
	dot -Tjpg -Gdpi=300 -o out/imgs/cfg.jpg out/logs/cfg;

dtree_graph:
	dot -Tjpg -Gdpi=300 -o out/imgs/dtree.jpg out/logs/dtree;

out: build/prog
	@./build/prog input > output ${ARGS}
# @ в начале говорит не выводить сами команды

pout: out cfg_graph dtree_graph

print:
	echo ${BASE_HPP}

# make run I=test/1

.PHONY: mkdir clean run out cfg_graph dtree_graph pout

mkdir:
	mkdir -p build/src; \
	mkdir -p build/include
	
clean:
	rm -r build
