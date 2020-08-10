# include "./deps/chiavdf/src/verifier.h"
// #include "prover_slow.h"
#include <chrono>

int main(int argc, char ** argv) {
  int discriminant_size_bits = 1024;
  int T = 10000;

  static const size_t N_STRS = 1024;
  static const size_t S_LEN = 32;
  
    // -0xd2fc77b2e865b3b9feae79f1986a9f894a7731993fcbb8713d08be84b1410b76e256d93cab0cf33c522bfbfb8fab56ad0c6d2d37806f1b4e39596a3280491516cd962407854c00a8c51fa4df1790f19ee8f0047d629476126b4aba27b22b20deb009550475a4cd7e7d39c2c62241a9effaef94603fc199c224cc2573512f6818ffa3e9aa36094d892f2180cac70524c72c7262f088d863c6a58533c175b0b3b75fc7b75b92d2261bd25d58e9b99ceb4a824ab0dbb121ed0c5fa173d8524ad1286c903f70067037127f2ff9dc84cd93f2124bf7f4488b7b2c8985d70ef22fcc4e078595f97cef607d549799d0143b9537bf8843d98e1a222a3136322e2567159f
    // -0xf3d39d4c26adb179a6237259bb05b3a6ea5ecb3ca39fd9a14fc6e27b5efa4d18aa875406d95593972e21a2d56a55c301e9e15f64aa890adce268546d239a26f4aba8c4477d6ff470afc5d8f74cef4ca53309315a8dbb8cceccd30aab000aa805a165ae5b9715a631ce2c89773b06e9e0d7fe679f30502ef652b80a4cb3d26113aa6beec01ea89403d3897fc6a3629a578456fea7f8f2c64704d938ba935080fdfd81eab3e764cb8da2f4f878a41273dccb75a5be435b3bdb8a9ab60c7dd130f35c707ad92cd4e3d3d56d249b8758bcdf9acc0c6fc846d55eb128fa74c7bbf6f9df195f016cafc664e75bf5d02d00f488ec0a9bd95e96243419d090c776074d6f

  std::string challenge_hash_str ("abcdabcdabcdabcdabcdabcdabcdabcb");
  auto challenge_hash_bits = std::vector<uint8_t>(challenge_hash_str.begin(), challenge_hash_str.end());
  integer D = CreateDiscriminant(
      challenge_hash_bits,
      discriminant_size_bits,
      0
  );

//   integer D(
//           "-0xf3d39d4c26adb179a6237259bb05b3a6ea5ecb3ca39fd9a14fc6e27b5efa4d18aa875406d95593972e21a2d56a55c301e9e15f64aa890adce268546d239a26f4aba8c4477d6ff470afc5d8f74cef4ca53309315a8dbb8cceccd30aab000aa805a165ae5b9715a631ce2c89773b06e9e0d7fe679f30502ef652b80a4cb3d26113aa6beec01ea89403d3897fc6a3629a578456fea7f8f2c64704d938ba935080fdfd81eab3e764cb8da2f4f878a41273dccb75a5be435b3bdb8a9ab60c7dd130f35c707ad92cd4e3d3d56d249b8758bcdf9acc0c6fc846d55eb128fa74c7bbf6f9df195f016cafc664e75bf5d02d00f488ec0a9bd95e96243419d090c776074d6f"
//       );
    form x=form::from_abd(
        integer(
            "0x2"
        ),
        integer(
            "0x1"
        ),
        D
    );
    form y=form::from_abd(
        integer(
            "0x693a9717fda00d9a1809872aff094ee88c0c376add089c18b09ccc82f23cb55dda0504e30119592809c3ee0e1fcbf071b1f95a25083743d693004c839360609ee8d98d7567b8636c239bce5ffd3ec8a497716164a49f0363742659860cbf839013fc1fba6097fe7330efc02f07f870ca67d3ab31829b128b5b0bbf962eb020f4"
        ),
        integer(
            "-0x4c2b28395a246fca2bb9a48e47c7d2e4c70604738a923bfab80e91da91b02122013dd43549d7934dc04149cd139fcfd5526280055d59292fe549bac1dec779935a404152676343a91f0b159addc3c025f7b13b770fc636996b6ef85bf7f9a14a3253f137af25da3fad93f04d8d2a11d322ddc4f2999d35e25194cc5fd3cfa877"
        ),
        D
    );
    // form proof=form::from_abd(
    //     integer(
    //         "0xd1ecaa776883e2246cba653f548d6e8c11c9f325eb2868ad8170d0a01d3bf9d9ee315103a25ec816acac7fdb7d05d6cbe9398c36e2de1efa60eecb35e5ff61960bfe66175449eadd9d0c1d529d9876cded844003126426a7abe75bea8e08a332e61b82209142c121c74c4a0640ec869863faae17c3dd2e44334ea5c3afd7f53"
    //     ),
    //     integer(
    //         "0x16f6e03f0de22e9f9953122222b1e51e61ebeef902de71423c2c037164a5577dd47d3771249bc6ad85f39e7e7107c015f9d238ab7077ffb55a3bc4bae1db0775e6e322d007929dd0dd26e6332718c6a69f31b0beefd5f1ad1c270ae0a2220e58f144a014c2e6924c6851f97f94b921bae6b0fa48de3160e0c3dab01720c436d"
    //     ),
    //     D
    // );

    // bool is_valid = false;
    // VerifyWesolowskiProof(D, x, y, proof, T, is_valid);
    // cout << "Result: " << is_valid << "\n";
}

// std::vector<uint8_t> challenge_hash_bytes(challenge_hash_str.begin(), challenge_hash_str.end());
// auto result = ProveSlow(challenge_hash_bytes, discriminant_size_bits, T);
