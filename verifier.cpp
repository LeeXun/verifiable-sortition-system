#include <emscripten/bind.h>
#include "./deps/chiavdf/src/verifier.h"

using namespace emscripten;

// challenge_hash_str is the merkle tree root (32B)
// num_iterations is T
bool VerifyVDF(const string challenge_hash_str,
              // const string& x_a, const string& x_b,
              const string y_a, const string y_b,
              const string proof_a, const string proof_b){
            //   uint64_t num_iterations = 10000) {
    int discriminant_size_bits = 1024;
    uint64_t num_iterations = 10000;
    // std::string challenge_hash_str(challenge_hash);
    auto challenge_hash_bits = std::vector<uint8_t>(challenge_hash_str.begin(), challenge_hash_str.end());
    integer D = CreateDiscriminant(
        challenge_hash_bits,
        discriminant_size_bits
    );
    // integer D(discriminant);
    form x = form::from_abd(
        // integer(x_a),
        // integer(x_b),
        integer(
            "0x2"
        ),
        integer(
            "0x1"
        ),
        D
    );
    form y = form::from_abd(
        integer(y_a),
        integer(y_b),
        D
    );
    form proof = form::from_abd(
        integer(proof_a),
        integer(proof_b),
        D
    );
    bool is_valid = false;
    VerifyWesolowskiProof(D, x, y, proof, num_iterations, is_valid);
    return is_valid;
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("VerifyVDF", &VerifyVDF);
}

// #include <emscripten/bind.h>

// using namespace emscripten;

// float lerp(float a, float b, float t) {
//     return (1 - t) * a + t * b;
// }

// EMSCRIPTEN_BINDINGS(my_module) {
//     function("lerp", &lerp);
// }