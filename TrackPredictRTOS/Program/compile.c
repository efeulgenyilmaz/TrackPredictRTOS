#include <stdlib.h>

int main() {
    system("g++ -std=c++11 dtst.c external.cpp main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system  `pkg-config --cflags --libs opencv4`");
    return 0;
}
