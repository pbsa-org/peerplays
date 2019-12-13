#include <graphene/peerplays_sidechain/sidechain_net_handler.hpp>

#include <graphene/chain/sidechain_address_object.hpp>

#include <fc/log/logger.hpp>

namespace graphene { namespace peerplays_sidechain {

sidechain_net_handler::sidechain_net_handler(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map& options) :
   plugin( _plugin ) {
}

sidechain_net_handler::~sidechain_net_handler() {
}

std::vector<std::string> sidechain_net_handler::get_sidechain_addresses() {
   std::vector<std::string> result;

   switch (sidechain) {
      case sidechain_type::bitcoin:
      {
         const auto& sidechain_addresses_range = plugin.database().get_index_type<sidechain_address_index>().indices().get<by_sidechain>().equal_range(sidechain);
         std::for_each(sidechain_addresses_range.first, sidechain_addresses_range.second,
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

