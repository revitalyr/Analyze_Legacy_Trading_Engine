#include <iostream>
#include "../cpp_fixed/fixed.h"

int main() {
    Fixed<7> f1("1");
    Fixed<7> f2(".001");
    Fixed<7> result = f1 + f2;
    Fixed<7> expected("1.001");
    
    std::cout << "result: " << std::string(result) << std::endl;
    std::cout << "expected: " << std::string(expected) << std::endl;
    std::cout << "result == expected: " << (result == expected) << std::endl;
    std::cout << "result raw: " << result.raw() << std::endl;
    std::cout << "expected raw: " << expected.raw() << std::endl;
    
    return 0;
}
