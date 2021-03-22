#pragma once

#include <cstdint>
#include <vector>

#include <fc/optional.hpp>

#include <graphene/chain/protocol/types.hpp>
#include <graphene/peerplays_sidechain/hive/authority.hpp>
#include <graphene/peerplays_sidechain/hive/types.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

struct account_update_operation {
   std::string account;
   fc::optional<authority> owner;
   fc::optional<authority> active;
   fc::optional<authority> posting;
   hive::public_key_type memo_key;
   std::string json_metadata;
};

}}} // namespace graphene::peerplays_sidechain::hive

FC_REFLECT(graphene::peerplays_sidechain::hive::account_update_operation,
           (account)(owner)(active)(posting)(memo_key)(json_metadata))
