#include <iostream>
#include "function.h"

void print_num(int num) {
    std::cout << num << "\n";
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    function<void(int)> fun = print_num;
    fun(-9);
    function<void()> lambda = [](){
        std::cout << "lol, it works\n";
    };
    lambda();
    function<void(int)> f(print_num);
    f(10);
    function<void(void)> f1 = std::bind(print_num, 3);
    f1();
    function<void()> ff(nullptr);
    function<void()> fff(ff);
    //fff();
    return 0;
}