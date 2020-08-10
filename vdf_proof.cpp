#include "./deps/chiavdf/src/verifier.h"
#include "./deps/chiavdf/src/prover_slow.h"
#include <chrono>

int main(int argc, char ** argv) {
  int discriminant_size_bits = 1024;
  int T = 10000;
  static const size_t N_STRS = 1024;
  std::string challenge_hash_str ("abcdabcdabcdabcdabcdabcdabcdabcb");
  auto challenge_hash_bits = std::vector<uint8_t>(challenge_hash_str.begin(), challenge_hash_str.end());
  integer D = CreateDiscriminant(
      challenge_hash_bits,
      discriminant_size_bits,
      0
  );
  std::vector<uint8_t> challenge_hash_bytes(challenge_hash_str.begin(), challenge_hash_str.end());
  auto result = ProveSlow(challenge_hash_bytes, discriminant_size_bits, T);
}