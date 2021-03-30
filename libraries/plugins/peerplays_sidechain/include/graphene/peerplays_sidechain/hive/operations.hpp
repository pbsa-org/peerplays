#pragma once

#include <graphene/peerplays_sidechain/hive/hive_operations.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

typedef fc::static_variant<
      vote_operation,
      comment_operation,

      transfer_operation,
      transfer_to_vesting_operation,
      withdraw_vesting_operation,

      limit_order_create_operation,
      limit_order_cancel_operation,

      feed_publish_operation,
      convert_operation,

      account_create_operation,
      account_update_operation>
      hive_operation;

}}} // namespace graphene::peerplays_sidechain::hive

namespace fc {

void to_variant(const graphene::peerplays_sidechain::hive::hive_operation &var, fc::variant &vo, uint32_t max_depth = 5);
void from_variant(const fc::variant &var, graphene::peerplays_sidechain::hive::hive_operation &vo, uint32_t max_depth = 5);

} // namespace fc

FC_REFLECT_TYPENAME(graphene::peerplays_sidechain::hive::hive_operation)
