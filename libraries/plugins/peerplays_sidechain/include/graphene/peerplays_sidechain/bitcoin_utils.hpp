#pragma once
#include <graphene/peerplays_sidechain/defs.hpp>

namespace graphene { namespace peerplays_sidechain {

enum bitcoin_network {
   mainnet,
   testnet,
   regtest
};

bytes generate_redeem_script(fc::flat_map<fc::ecc::public_key, int> key_data);
std::string p2sh_address_from_redeem_script(const bytes& script, bitcoin_network network = mainnet);
bytes sign_raw_transaction(const bytes& unsigned_tx, const fc::ecc::private_key& priv_key);

}}
