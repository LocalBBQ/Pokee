#include "save_data/aes128.hpp"

namespace mhfu::savedata {

Aes128::Aes128(const Block &key) {
    AES_init_ctx(&context_, key.data());
}

Block Aes128::encrypt(const Block &input) const {
    Block block = input;
    AES_ECB_encrypt(&context_, block.data());
    return block;
}

Block Aes128::decrypt(const Block &input) const {
    Block block = input;
    AES_ECB_decrypt(&context_, block.data());
    return block;
}

} // namespace mhfu::savedata
