#include <iostream>
#include "../cpp_fixed/fixed.h"

int main() {
    Fixed<7> f1("1");
    Fixed<7> f2(".001");
    Fixed<7> result = f1 + f2;
    
    std::cout << "f1: " << std::string(f1) << std::endl;
    std::cout << "f2: " << std::string(f2) << std::endl;
    std::cout << "result: " << std::string(result) << std::endl;
    std::cout << "expected: 1.001" << std::endl;
    
    std::cout << "f1 raw: " << f1.raw() << std::endl;
    std::cout << "f2 raw: " << f2.raw() << std::endl;
    std::cout << "result raw: " << result.raw() << std::endl;
    
    return 0;
}
