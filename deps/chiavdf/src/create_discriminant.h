#ifndef CREATE_DISCRIMINANT_H
#define CREATE_DISCRIMINANT_H

#include "proof_common.h"

integer CreateDiscriminant(std::vector<uint8_t>& seed, int length = 1024, int iteration = 0) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    integer D = HashPrime(seed, length, {0, 1, 2, length - 1}, iteration) * integer(-1);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << std::endl;
    // std::cout << "CreateDiscriminant = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
    // std::cout << "D = " << D.to_string() << std::endl;
    return D;
}

#endif // CREATE_DISCRIMINANT_H
