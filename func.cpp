#include "func.h"

void sys_err(const char* str) {
    perror(str);
    exit(1);
}

void process(int a) {
    std::cout << "Processing task " <<a<< std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}
