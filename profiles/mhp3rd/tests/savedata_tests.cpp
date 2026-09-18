// Checks for the save-data code that need no game data: the AES and CMAC
// primitives against their published test vectors, PARAM.SFO round trips,
// and the encryption, hashing and folder layout round trips.
//
//   mhp3rd_savedata_tests
//   mhp3rd_savedata_tests --check <save folder> <data file name> <game key, 32 hex digits> [plaintext out]
//
// The second form checks a save made elsewhere, for example one copied from
// a PSP: the PARAM.SFO hashes, the data file's hash, and that the decrypted
// data encrypts back to a file with the same hash.
#include "save_data/aes128.hpp"
#include "save_data/param_sfo.hpp"
#include "save_data/savedata_crypto.hpp"
#include "save_data/savedata_store.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace mhp3rd::savedata;

namespace {

int failures = 0;

void check(bool condition, const char *what) {
    std::printf("%s %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition) ++failures;
}

std::vector<std::uint8_t> from_hex(const std::string &hex) {
    std::vector<std::uint8_t> out;
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) out.push_back(static_cast<std::uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return out;
}

Block block(const std::string &hex) {
    Block b{};
    const auto bytes = from_hex(hex);
    std::copy(bytes.begin(), bytes.end(), b.begin());
    return b;
}

void test_aes() {
    // FIPS-197 appendix C.1.
    const Aes128 aes(block("000102030405060708090a0b0c0d0e0f"));
    const Block plain = block("00112233445566778899aabbccddeeff");
    const Block cipher = block("69c4e0d86a7b0430d8cdb78070b4c55a");
    check(aes.encrypt(plain) == cipher, "AES-128 encrypts the FIPS-197 vector");
    check(aes.decrypt(cipher) == plain, "AES-128 decrypts the FIPS-197 vector");
}

void test_cmac() {
    // RFC 4493 section 4.
    const Aes128 aes(block("2b7e151628aed2a6abf7158809cf4f3c"));
    const auto message = from_hex("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51"
                                  "30c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710");
    check(cmac(aes, {}) == block("bb1d6929e95937287fa37d129b756746"), "CMAC of the empty message");
    check(cmac(aes, std::span(message).first(16)) == block("070a16b46b4d4144f79bdd9dd04a287c"), "CMAC of 16 bytes");
    check(cmac(aes, std::span(message).first(40)) == block("dfa66747de9ae63030ca32611497c827"), "CMAC of 40 bytes");
    check(cmac(aes, message) == block("51f0bebf7e3b9d92fc49741779363cfe"), "CMAC of 64 bytes");
}

void test_param_sfo() {
    ParamSfo sfo;
    sfo.set_string("TITLE", "Title", 128u);
    sfo.set_integer("PARENTAL_LEVEL", 7u);
    sfo.set_binary("SAVEDATA_PARAMS", std::vector<std::uint8_t>(128u, 0x5Au), 128u);
    const auto bytes = sfo.serialize();
    const auto parsed = ParamSfo::parse(bytes);
    check(parsed.has_value(), "PARAM.SFO parses what it serializes");
    check(parsed && parsed->string("TITLE") == "Title", "PARAM.SFO keeps strings");
    check(parsed && parsed->integer("PARENTAL_LEVEL") == 7u, "PARAM.SFO keeps integers");
    check(parsed && parsed->binary("SAVEDATA_PARAMS") && parsed->binary("SAVEDATA_PARAMS")->at(127) == 0x5Au,
          "PARAM.SFO keeps binary values");
    check(parsed && parsed->serialize() == bytes, "PARAM.SFO serializes identically after a round trip");
    const auto offset = sfo.data_offset("SAVEDATA_PARAMS");
    check(offset && bytes.at(*offset) == 0x5Au && bytes.at(*offset + 127u) == 0x5Au, "PARAM.SFO reports data offsets");
}

void test_encryption() {
    const Block key = block("00112233445566778899aabbccddeeff");
    std::vector<std::uint8_t> plain(5000u);
    for (std::size_t i = 0; i < plain.size(); ++i) plain[i] = static_cast<std::uint8_t>(i * 7u + 3u);
    const Block random = block("0f0e0d0c0b0a09080706050403020100");
    for (const CryptMode mode : {CryptMode::Mode1, CryptMode::Mode3, CryptMode::Mode5}) {
        const std::string name = "mode " + std::to_string(static_cast<int>(mode)) + ": ";
        const auto encrypted = encrypt_data(plain, mode, &key, random);
        check(encrypted.size() == kEncryptedHeaderSize + 5008u, (name + "encryption adds the header and pads to 16").c_str());
        check(!std::equal(plain.begin(), plain.end(), encrypted.begin() + kEncryptedHeaderSize),
              (name + "encrypted data differs from the plaintext").c_str());
        const auto decrypted = decrypt_data(encrypted, mode, &key);
        check(decrypted && std::equal(plain.begin(), plain.end(), decrypted->begin()) &&
                  std::all_of(decrypted->begin() + 5000, decrypted->end(), [](std::uint8_t b) { return b == 0u; }),
              (name + "decryption restores the plaintext").c_str());
        if (mode == CryptMode::Mode1) continue;
        Block other = key;
        other[0] ^= 1u;
        const auto wrong = decrypt_data(encrypted, mode, &other);
        check(wrong && !std::equal(plain.begin(), plain.end(), wrong->begin()),
              (name + "another game key does not decrypt it").c_str());
        check(data_file_hash(encrypted, mode, &key) != data_file_hash(encrypted, mode, &other),
              (name + "the file hash depends on the game key").c_str());
    }
}

void test_store() {
    const auto root = std::filesystem::temp_directory_path() / "mhp3rd_savedata_tests";
    std::filesystem::remove_all(root);
    SaveFiles files;
    files.game_name = "TEST00000";
    files.save_name = "SLOT";
    files.file_name = "DATA.BIN";
    files.key = block("0102030405060708090a0b0c0d0e0f10");
    SaveContents contents;
    contents.data = std::vector<std::uint8_t>(4096u, 0x42u);
    contents.title = "Title";
    contents.savedata_title = "Save";
    contents.detail = "Detail";
    contents.parental_level = 1u;
    contents.icon0 = {1, 2, 3};
    std::string error;
    check(write_save(root, files, contents, error), "a save is written");
    const auto folder = root / "PSP" / "SAVEDATA" / "TEST00000SLOT";
    check(std::filesystem::exists(folder / "PARAM.SFO") && std::filesystem::exists(folder / "DATA.BIN") &&
              std::filesystem::exists(folder / "ICON0.PNG"),
          "the save uses the PSP folder layout");
    std::ifstream in(folder / "PARAM.SFO", std::ios::binary);
    const std::vector<std::uint8_t> sfo_bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const auto sfo = ParamSfo::parse(sfo_bytes);
    const auto offset = sfo ? sfo->data_offset("SAVEDATA_PARAMS") : std::nullopt;
    check(offset && verify_param_sfo(sfo_bytes, *offset), "PARAM.SFO carries valid hashes");
    const auto loaded = load_save(root, files);
    check(loaded.status == LoadStatus::Ok && loaded.contents.data == contents.data, "the save loads back");
    check(loaded.contents.title == "Title" && loaded.contents.detail == "Detail", "the titles load back");

    // A changed data file must be reported as broken.
    {
        std::fstream data(folder / "DATA.BIN", std::ios::binary | std::ios::in | std::ios::out);
        data.seekp(100);
        data.put('\x7f');
    }
    check(load_save(root, files).status == LoadStatus::Broken, "a modified data file is rejected");
    SaveFiles missing = files;
    missing.save_name = "OTHER";
    check(load_save(root, missing).status == LoadStatus::NoData, "a missing save reports no data");
    check(delete_save(root, files) && !std::filesystem::exists(folder), "a save is deleted");
    std::filesystem::remove_all(root);
}

} // namespace

std::vector<std::uint8_t> read_all(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

int check_save(const std::filesystem::path &folder, const std::string &file_name, const std::string &key_hex,
               const char *plain_out) {
    const Block key = block(key_hex);
    const auto sfo_bytes = read_all(folder / "PARAM.SFO");
    const auto file = read_all(folder / file_name);
    const auto sfo = ParamSfo::parse(sfo_bytes);
    check(sfo.has_value(), "PARAM.SFO parses");
    if (!sfo || file.empty()) return EXIT_FAILURE;
    check(sfo->serialize() == sfo_bytes, "PARAM.SFO has the layout this code writes");
    const auto offset = sfo->data_offset("SAVEDATA_PARAMS");
    const auto *params = sfo->binary("SAVEDATA_PARAMS");
    const auto mode = params != nullptr ? mode_from_flags(params->at(0)) : std::nullopt;
    std::printf("SAVEDATA_PARAMS flags 0x%02x\n", params != nullptr ? params->at(0) : 0u);
    check(mode.has_value(), "the save is encrypted in a known mode");
    if (!mode) return EXIT_FAILURE;
    check(offset && verify_param_sfo(sfo_bytes, *offset), "PARAM.SFO hashes match");
    const auto *list = sfo->binary("SAVEDATA_FILE_LIST");
    bool listed = false;
    for (std::size_t o = 0; list != nullptr && o + 32u <= list->size(); o += 32u) {
        if (std::string(reinterpret_cast<const char *>(&(*list)[o])) != file_name) continue;
        Block stored{};
        std::copy_n(list->begin() + static_cast<std::ptrdiff_t>(o + 13u), 16, stored.begin());
        listed = stored == data_file_hash(file, *mode, &key);
    }
    check(listed, "the data file matches its hash in SAVEDATA_FILE_LIST");
    const auto plain = decrypt_data(file, *mode, &key);
    check(plain.has_value(), "the data file decrypts");
    if (!plain) return EXIT_FAILURE;
    const auto again = encrypt_data(*plain, *mode, &key, block("00000000000000000000000000000000"));
    check(decrypt_data(again, *mode, &key) == plain, "the plaintext encrypts and decrypts back");
    if (plain_out != nullptr) {
        std::ofstream out(plain_out, std::ios::binary);
        out.write(reinterpret_cast<const char *>(plain->data()), static_cast<std::streamsize>(plain->size()));
    }
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
    if (argc >= 5 && std::string(argv[1]) == "--check")
        return check_save(argv[2], argv[3], argv[4], argc >= 6 ? argv[5] : nullptr);
    test_aes();
    test_cmac();
    test_param_sfo();
    test_encryption();
    test_store();
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
