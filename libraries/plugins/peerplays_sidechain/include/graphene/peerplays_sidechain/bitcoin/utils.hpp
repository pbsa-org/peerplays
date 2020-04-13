#pragma once
#include <graphene/peerplays_sidechain/bitcoin/types.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>

namespace graphene { namespace peerplays_sidechain { namespace bitcoin {

bytes parse_hex( const std::string& str );

std::vector<bytes> get_pubkey_from_redeemScript( bytes script );

bytes public_key_data_to_bytes( const fc::ecc::public_key_data& key );

} } }