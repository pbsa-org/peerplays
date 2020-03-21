#pragma once
#include <fc/optional.hpp>
#include <graphene/peerplays_sidechain/defs.hpp>

namespace graphene { namespace peerplays_sidechain {

enum bitcoin_network {
   mainnet,
   testnet,
   regtest
};

bytes generate_redeem_script(std::vector<std::pair<fc::ecc::public_key, uint64_t>> key_data);
std::string p2wsh_address_from_redeem_script(const bytes &script, bitcoin_network network = regtest);
bytes lock_script_for_redeem_script(const bytes &script);
bytes lock_script_from_pw_address(const std::string &address);

std::string get_weighted_multisig_address(const std::vector<std::pair<std::string, uint64_t>> &public_keys, bitcoin_network network = regtest);
bytes get_weighted_multisig_redeem_script(std::vector<std::pair<std::string, uint64_t>> public_keys);

std::vector<bytes> signatures_for_raw_transaction(const bytes &unsigned_tx,
                                                  std::vector<uint64_t> in_amounts,
                                                  const bytes &redeem_script,
                                                  const fc::ecc::private_key &priv_key);

/*
 * unsigned_tx - tx,  all inputs of which are spends of the PW P2SH address
 * returns signed transaction
 */
bytes sign_pw_transfer_transaction(const bytes &unsigned_tx,
                                   std::vector<uint64_t> in_amounts,
                                   const bytes &redeem_script,
                                   const std::vector<fc::optional<fc::ecc::private_key>> &priv_keys);

///
////// \brief Adds dummy signatures instead of real signatures
////// \param unsigned_tx
////// \param redeem_script
////// \param key_count
////// \return can be used as partially signed tx
bytes add_dummy_signatures_for_pw_transfer(const bytes &unsigned_tx,
                                           const bytes &redeem_script,
                                           unsigned int key_count);

///
/// \brief replaces dummy sgnatures in partially signed tx with real tx
/// \param partially_signed_tx
/// \param in_amounts
/// \param priv_key
/// \param key_idx
/// \return
///
bytes partially_sign_pw_transfer_transaction(const bytes &partially_signed_tx,
                                             std::vector<uint64_t> in_amounts,
                                             const fc::ecc::private_key &priv_key,
                                             unsigned int key_idx);

///
/// \brief Creates ready to publish bitcoin transaction from unsigned tx and
///        full set of the signatures. This is alternative way to create tx
///        with partially_sign_pw_transfer_transaction
/// \param unsigned_tx
/// \param signatures
/// \param redeem_script
/// \return
///
bytes add_signatures_to_unsigned_tx(const bytes &unsigned_tx,
                                    const std::vector<std::vector<bytes>> &signatures,
                                    const bytes &redeem_script);

void read_tx_data_from_string(const std::string &string_buf, bytes& tx, std::vector<uint64_t>& in_amounts);
std::string save_tx_data_to_string(const bytes& tx, const std::vector<uint64_t>& in_amounts);


struct btc_outpoint {
   fc::uint256 hash;
   uint32_t n;

   void to_bytes(bytes &stream) const;
   size_t fill_from_bytes(const bytes &data, size_t pos = 0);
};

struct btc_in {
   btc_in() = default;
   btc_in(const btc_in &) = default;
   btc_in(btc_in &&) = default;
   btc_in &operator=(const btc_in &) = default;

   btc_in(const std::string &txid, uint32_t out, uint32_t sequence = 0xffffffff) {
      prevout.n = out;
      prevout.hash = fc::uint256(txid);
      // reverse hash due to the different from_hex algo in bitcoin
      std::reverse(prevout.hash.data(), prevout.hash.data() + prevout.hash.data_size());
      nSequence = sequence;
   }

   btc_outpoint prevout;
   bytes scriptSig;
   uint32_t nSequence;
   std::vector<bytes> scriptWitness;

   void to_bytes(bytes &stream) const;
   size_t fill_from_bytes(const bytes &data, size_t pos = 0);
};

struct btc_out {
   btc_out() = default;
   btc_out(const btc_out &) = default;
   btc_out(btc_out &&) = default;
   btc_out &operator=(const btc_out &) = default;

   btc_out(const std::string &address, uint64_t amount) :
         nValue(amount),
         scriptPubKey(lock_script_from_pw_address(address)) {
   }

   int64_t nValue;
   bytes scriptPubKey;

   void to_bytes(bytes &stream) const;
   size_t fill_from_bytes(const bytes &data, size_t pos = 0);
};

struct btc_tx {
   std::vector<btc_in> vin;
   std::vector<btc_out> vout;
   int32_t nVersion;
   uint32_t nLockTime;
   bool hasWitness;

   void to_bytes(bytes &stream) const;
   size_t fill_from_bytes(const bytes &data, size_t pos = 0);
};

}} // namespace graphene::peerplays_sidechain
