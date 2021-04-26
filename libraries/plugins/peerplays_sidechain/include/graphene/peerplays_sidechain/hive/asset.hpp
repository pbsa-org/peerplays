#pragma once

#include <graphene/peerplays_sidechain/hive/asset_symbol.hpp>
#include <graphene/peerplays_sidechain/hive/types.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

struct asset {
   share_type amount;
   asset_symbol_type symbol;
};

}}} // namespace graphene::peerplays_sidechain::hive

namespace fc {
void to_variant(const graphene::peerplays_sidechain::hive::asset &var, fc::variant &vo, uint32_t max_depth);
void from_variant(const fc::variant &var, graphene::peerplays_sidechain::hive::asset &vo, uint32_t max_depth);
} // namespace fc

FC_REFLECT(graphene::peerplays_sidechain::hive::asset, (amount)(symbol))
