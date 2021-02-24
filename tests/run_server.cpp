#include "../server.h"

int main(int argc, char *argv[]) {
    (argc == 2) ? Server(atoi(argv[1])) : Server(0);
    return 0;
}