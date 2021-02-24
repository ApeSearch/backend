CC=g++ -g3 -std=c++17 -Wall -pedantic -Wconversion -Wextra -Wreorder -fno-builtin

SOURCES=$(wildcard *.cpp)
OBJS=${SOURCES:.cpp=.o}

all: server test

server: ${OBJS}
	ld -r -o $@ ${OBJS} 

# HtmlParser: ${OBJS}
# 	ld -r -o $@ ${OBJS}

TESTDIR=tests
EXECDIR=tests/bin

TEST_SRC:=$(basename $(wildcard ${TESTDIR}/*.cpp))
$(TEST_SRC): %: %.cpp server
	@mkdir -p ${EXECDIR}
	${CC} -o ${EXECDIR}/$(notdir $@) $^ -pthread

test: ${TEST_SRC}

run_server: all
	@${EXECDIR}/run_server 6969

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${OBJS} ${EXECDIR}/* server
