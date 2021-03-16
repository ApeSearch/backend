CC=g++ -g3 -std=c++17 -Wall -pedantic -Wconversion -Wextra -Wreorder -fno-builtin

ASSRC=libraries/AS/src
ASSOURCES=$(wildcard ${ASSRC}/*.cpp)
ASOBJS=${ASSOURCES:.cpp=.o}

SOURCES=$(wildcard *.cpp)
OBJS=${SOURCES:.cpp=.o}

all: server test

server: ${OBJS} ${ASOBJS}
	ld -r -o $@ ${OBJS}

# HtmlParser: ${OBJS}
# 	ld -r -o $@ ${OBJS}

TESTDIR=tests
EXECDIR=tests/bin

TEST_SRC:=$(basename $(wildcard ${TESTDIR}/*.cpp))
$(TEST_SRC): %: %.cpp server ${ASOBJS}
	@mkdir -p ${EXECDIR} 
	${CC} -Dtesting -o ${EXECDIR}/$(notdir $@) $^ -pthread

test: ${TEST_SRC}

run_server: all
	@${EXECDIR}/run_server 42069

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -Dtesting -c $< -o $@
%.o: %.cc
	${CC} -Dtesting -c $< -o $@

clean:
	rm -rf ${OBJS} ${ASOBJS} ${EXECDIR}/* server 
