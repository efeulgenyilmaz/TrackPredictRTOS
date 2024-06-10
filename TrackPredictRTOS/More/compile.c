#include <stdlib.h>

int main() {
    system("g++ neighbor.cpp -o neighbor  `pkg-config --cflags --libs opencv4`");
    system("g++ redfilter.cpp -o redfilter  `pkg-config --cflags --libs opencv4`");
    return 0;
}
