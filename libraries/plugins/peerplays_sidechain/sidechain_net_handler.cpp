#include <graphene/peerplays_sidechain/sidechain_net_handler.hpp>

#include <graphene/chain/sidechain_address_object.hpp>

#include <fc/log/logger.hpp>

namespace graphene { namespace peerplays_sidechain {

sidechain_net_handler::sidechain_net_handler(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map& options) :
   plugin( _plugin )
{
}

sidechain_net_handler::~sidechain_net_handler() {
}

graphene::chain::database& sidechain_net_handler::get_database()
{
   return plugin.database();
}

std::vector<std::string> sidechain_net_handler::get_sidechain_addresses() {
   std::vector<std::string> result;

   switch (sidechain) {
      case sidechain_type::bitcoin:
      {
         const auto& sidechain_addresses_idx = get_database().get_index_type<sidechain_address_index>();
         const auto& sidechain_addresses_by_sidechain_idx = sidechain_addresses_idx.indices().get<by_sidechain>();
         const auto& sidechain_addresses_by_sidechain_range = sidechain_addresses_by_sidechain_idx.equal_range(sidechain);
         std::for_each(sidechain_addresses_by_sidechain_range.first, sidechain_addresses_by_sidechain_range.second,
               [&result] (const sidechain_address_object& sao) {
            result.push_back(sao.address);
         });
         break;
      }
      default:
         assert(false);
   }

   return result;
}

} } // graphene::peerplays_sidechain

