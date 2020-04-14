#pragma once
#include <graphene/peerplays_sidechain/bitcoin/types.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>

namespace graphene { namespace peerplays_sidechain { namespace bitcoin {

fc::ecc::public_key_data create_public_key_data( const std::vector<char>& public_key );

bytes get_privkey_bytes( const std::string& privkey_base58 );

bytes parse_hex( const std::string& str );

std::vector<bytes> get_pubkey_from_redeemScript( bytes script );

bytes public_key_data_to_bytes( const fc::ecc::public_key_data& key );

} } }