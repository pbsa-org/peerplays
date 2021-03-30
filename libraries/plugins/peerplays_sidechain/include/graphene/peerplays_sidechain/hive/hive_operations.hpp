#pragma once

#include <cstdint>
#include <vector>

#include <fc/optional.hpp>

#include <graphene/chain/protocol/types.hpp>
#include <graphene/peerplays_sidechain/hive/authority.hpp>
#include <graphene/peerplays_sidechain/hive/types.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

struct vote_operation {};

struct comment_operation {};

struct transfer_operation {};

struct transfer_to_vesting_operation {};

struct withdraw_vesting_operation {};

struct limit_order_create_operation {};

struct limit_order_cancel_operation {};

struct feed_publish_operation {};

struct convert_operation {};

struct account_create_operation {};

struct account_update_operation {
   hive::account_name_type account;
   fc::optional<authority> owner;
   fc::optional<authority> active;
   fc::optional<authority> posting;
   hive::public_key_type memo_key;
   std::string json_metadata;
};

}}} // namespace graphene::peerplays_sidechain::hive

FC_REFLECT(graphene::peerplays_sidechain::hive::vote_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::comment_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::transfer_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::transfer_to_vesting_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::withdraw_vesting_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::limit_order_create_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::limit_order_cancel_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::feed_publish_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::convert_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::account_create_operation, )
FC_REFLECT(graphene::peerplays_sidechain::hive::account_update_operation,
           (account)(owner)(active)(posting)(memo_key)(json_metadata))
