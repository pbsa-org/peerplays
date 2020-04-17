#include <fc/crypto/base58.hpp>
#include <graphene/peerplays_sidechain/bitcoin/utils.hpp>

namespace graphene { namespace peerplays_sidechain { namespace bitcoin {

fc::ecc::public_key_data create_public_key_data(const std::vector<char> &public_key) {
   FC_ASSERT(public_key.size() == 33);
   fc::ecc::public_key_data key;
   for (size_t i = 0; i < 33; i++) {
      key.at(i) = public_key[i];
   }
   return key;
}

bytes get_privkey_bytes(const std::string &privkey_base58) {
   const auto privkey = fc::from_base58(privkey_base58);
   // Remove version and hash
   return bytes(privkey.cbegin() + 1, privkey.cbegin() + 1 + 32);
}

bytes parse_hex(const std::string &str) {
   bytes vec(str.size() / 2);
   fc::from_hex(str, vec.data(), vec.size());
   return vec;
}

std::vector<bytes> get_pubkey_from_redeemScript(bytes script) {
   FC_ASSERT(script.size() >= 37);

   script.erase(script.begin());
   script.erase(script.end() - 2, script.end());

   std::vector<bytes> result;
   uint64_t count = script.size() / 34;
   for (size_t i = 0; i < count; i++) {
      result.push_back(bytes(script.begin() + (34 * i) + 1, script.begin() + (34 * (i + 1))));
   }
   return result;
}

bytes public_key_data_to_bytes(const fc::ecc::public_key_data &key) {
   bytes result;
   result.resize(key.size());
   std::copy(key.begin(), key.end(), result.begin());
   return result;
}

}}} // namespace graphene::peerplays_sidechain::bitcoin
