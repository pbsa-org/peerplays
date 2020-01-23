#pragma once
#include <graphene/peerplays_sidechain/defs.hpp>
#include <fc/optional.hpp>

namespace graphene { namespace peerplays_sidechain {

enum bitcoin_network {
   mainnet,
   testnet,
   regtest
};

bytes generate_redeem_script(fc::flat_map<fc::ecc::public_key, int> key_data);
std::string p2sh_address_from_redeem_script(const bytes& script, bitcoin_network network = mainnet);
/*
 * unsigned_tx - tx,  all inputs of which are spends of the PW P2SH address
 * returns signed transaction
 */
bytes sign_pw_transfer_transaction(const bytes& unsigned_tx, const bytes& redeem_script, const std::vector<fc::optional<fc::ecc::private_key>>& priv_keys);

}}
