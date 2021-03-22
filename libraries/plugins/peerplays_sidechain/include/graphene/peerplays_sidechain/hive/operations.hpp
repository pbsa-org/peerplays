#pragma once

#include <graphene/peerplays_sidechain/hive/hive_operations.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

typedef fc::static_variant<
      account_update_operation>
      hive_operation;

}}} // namespace graphene::peerplays_sidechain::hive

FC_REFLECT_TYPENAME(graphene::peerplays_sidechain::hive::hive_operation)
