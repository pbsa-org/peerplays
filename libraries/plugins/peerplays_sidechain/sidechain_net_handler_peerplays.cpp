#include <graphene/peerplays_sidechain/sidechain_net_handler_peerplays.hpp>

#include <algorithm>
#include <thread>

#include <boost/algorithm/hex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <fc/crypto/base64.hpp>
#include <fc/log/logger.hpp>
#include <fc/network/ip.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/sidechain_address_object.hpp>
#include <graphene/chain/son_info.hpp>
#include <graphene/chain/son_wallet_object.hpp>
#include <graphene/chain/protocol/son_wallet.hpp>

namespace graphene { namespace peerplays_sidechain {

sidechain_net_handler_peerplays::sidechain_net_handler_peerplays(peerplays_sidechain_plugin& _plugin, const boost::program_options::variables_map& options) :
      sidechain_net_handler(_plugin, options) {
   sidechain = sidechain_type::peerplays;
   plugin.database().applied_block.connect( [&] (const signed_block& b) { on_block_applied(b); } );
   plugin.database().changed_objects.connect( [&] (const vector<object_id_type>& ids, const flat_set<account_id_type>& impacted_accounts) { on_changed_objects(ids); } );
}

sidechain_net_handler_peerplays::~sidechain_net_handler_peerplays() {
}

void sidechain_net_handler_peerplays::recreate_primary_wallet() {
}

std::string sidechain_net_handler_peerplays::create_multisignature_wallet( const std::vector<std::string> public_keys ) {
   return "";
}

std::string sidechain_net_handler_peerplays::transfer( const std::string& from, const std::string& to, const uint64_t amount ) {
   return "";
}

std::string sidechain_net_handler_peerplays::sign_transaction( const std::string& transaction ) {
   return "";
}

std::string sidechain_net_handler_peerplays::send_transaction( const std::string& transaction ) {
   return "";
}

void sidechain_net_handler_peerplays::handle_event( const std::string& event_data ) {
   ilog("peerplays sidechain plugin:  sidechain_net_handler_peerplays::handle_event");
   ilog("                             event_data: ${event_data}", ("event_data", event_data));
}

void sidechain_net_handler_peerplays::on_block_applied(const signed_block& b) {
    for (const auto& trx: b.transactions) {
        for (auto op: trx.operations) {
            ilog("sidechain_net_handler_peerplays:  operation detected ${op}", ("op", op));
        }
    }
}

void sidechain_net_handler_peerplays::on_changed_objects(const vector<object_id_type>& changed_object_ids) {
    for (auto object_id: changed_object_ids) {
        const object* obj = plugin.database().find_object(object_id);

        const chain::account_balance_object * abo = dynamic_cast<const chain::account_balance_object*>(obj);
        if (abo != nullptr) {
           ilog("sidechain_net_handler_peerplays:  account_balance_object changed, id ${id}", ("id", abo->id));
           continue;
        }
    }
}

} } // graphene::peerplays_sidechain

