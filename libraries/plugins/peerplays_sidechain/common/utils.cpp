#include <graphene/peerplays_sidechain/common/utils.hpp>

std::string account_id_to_string(graphene::chain::account_id_type account_id) {
   std::string account_id_str = fc::to_string(account_id.space_id) + "." +
                                fc::to_string(account_id.type_id) + "." +
                                fc::to_string((uint64_t)account_id.instance);
   return account_id_str;
}

std::string asset_id_to_string(graphene::chain::asset_id_type asset_id) {
   std::string asset_id_str = fc::to_string(asset_id.space_id) + "." +
                              fc::to_string(asset_id.type_id) + "." +
                              fc::to_string((uint64_t)asset_id.instance);
   return asset_id_str;
}
