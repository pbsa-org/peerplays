#include <graphene/peerplays_sidechain/peerplays_sidechain_plugin.hpp>

#include <fc/log/logger.hpp>
#include <graphene/peerplays_sidechain/sidechain_net_manager.hpp>
#include <graphene/utilities/key_conversion.hpp>

namespace bpo = boost::program_options;

namespace graphene { namespace peerplays_sidechain {

namespace detail
{

class peerplays_sidechain_plugin_impl
{
   public:
      peerplays_sidechain_plugin_impl(peerplays_sidechain_plugin& _plugin)
         : _self( _plugin )
      { }
      virtual ~peerplays_sidechain_plugin_impl();

      peerplays_sidechain_plugin& _self;

      peerplays_sidechain::sidechain_net_manager _net_manager;

};

peerplays_sidechain_plugin_impl::~peerplays_sidechain_plugin_impl()
{
   return;
}

} // end namespace detail

peerplays_sidechain_plugin::peerplays_sidechain_plugin() :
   my( new detail::peerplays_sidechain_plugin_impl(*this) )
{
}

peerplays_sidechain_plugin::~peerplays_sidechain_plugin()
{
   try {
      if( _heartbeat_task.valid() )
         _heartbeat_task.cancel_and_wait(__FUNCTION__);
   } catch(fc::canceled_exception&) {
      //Expected exception. Move along.
   } catch(fc::exception& e) {
      edump((e.to_detail_string()));
   }
   return;
}

std::string peerplays_sidechain_plugin::plugin_name()const
{
   return "peerplays_sidechain";
}

void peerplays_sidechain_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
   auto default_priv_key = fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("nathan")));
   string son_id_example = fc::json::to_string(chain::son_id_type(5));

   cli.add_options()
         ("son-id,w", bpo::value<vector<string>>(), ("ID of SON controlled by this node (e.g. " + son_id_example + ", quotes are required)").c_str())
         ("peerplays-private-key", bpo::value<vector<string>>()->composing()->multitoken()->
               DEFAULT_VALUE_VECTOR(std::make_pair(chain::public_key_type(default_priv_key.get_public_key()), graphene::utilities::key_to_wif(default_priv_key))),
               "Tuple of [PublicKey, WIF private key]")

         ("bitcoin-node-ip", bpo::value<string>()->default_value("99.79.189.95"), "IP address of Bitcoin node")
         ("bitcoin-node-zmq-port", bpo::value<uint32_t>()->default_value(11111), "ZMQ port of Bitcoin node")
         ("bitcoin-node-rpc-port", bpo::value<uint32_t>()->default_value(22222), "RPC port of Bitcoin node")
         ("bitcoin-node-rpc-user", bpo::value<string>()->default_value("1"), "Bitcoin RPC user")
         ("bitcoin-node-rpc-password", bpo::value<string>()->default_value("1"), "Bitcoin RPC password")
         ("bitcoin-address", bpo::value<string>()->default_value("2N911a7smwDzUGARg8s7Q1ViizFCw6gWcbR"), "Bitcoin address")
         ("bitcoin-public-key", bpo::value<string>()->default_value("02d0f137e717fb3aab7aff99904001d49a0a636c5e1342f8927a4ba2eaee8e9772"), "Bitcoin public key")
         ("bitcoin-private-key", bpo::value<string>()->default_value("cVN31uC9sTEr392DLVUEjrtMgLA8Yb3fpYmTRj7bomTm6nn2ANPr"), "Bitcoin private key")
         ;
   cfg.add(cli);
}

void peerplays_sidechain_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   ilog("peerplays sidechain plugin:  plugin_initialize()");

   if( options.count( "bitcoin-node-ip" ) && options.count( "bitcoin-node-zmq-port" ) && options.count( "bitcoin-node-rpc-port" )
      && options.count( "bitcoin-node-rpc-user" ) && options.count( "bitcoin-node-rpc-password" )
      && options.count( "bitcoin-address" ) && options.count( "bitcoin-public-key" ) && options.count( "bitcoin-private-key" ) )
   {
      my->_net_manager.create_handler(network::bitcoin, options);
   } else {
      wlog("Haven't set up bitcoin sidechain parameters");
   }

   LOAD_VALUE_SET(options, "son-id", _sons, chain::son_id_type)
   if( options.count("peerplays-private-key") )
   {
      const std::vector<std::string> key_id_to_wif_pair_strings = options["peerplays-private-key"].as<std::vector<std::string>>();
      for (const std::string& key_id_to_wif_pair_string : key_id_to_wif_pair_strings)
      {
         auto key_id_to_wif_pair = graphene::app::dejsonify<std::pair<chain::public_key_type, std::string> >(key_id_to_wif_pair_string, 5);
         ilog("Public Key: ${public}", ("public", key_id_to_wif_pair.first));
         fc::optional<fc::ecc::private_key> private_key = graphene::utilities::wif_to_key(key_id_to_wif_pair.second);
         if (!private_key)
         {
            // the key isn't in WIF format; see if they are still passing the old native private key format.  This is
            // just here to ease the transition, can be removed soon
            try
            {
               private_key = fc::variant(key_id_to_wif_pair.second, 2).as<fc::ecc::private_key>(1);
            }
            catch (const fc::exception&)
            {
               FC_THROW("Invalid WIF-format private key ${key_string}", ("key_string", key_id_to_wif_pair.second));
            }
         }
         _private_keys[key_id_to_wif_pair.first] = *private_key;
      }
   }
}

void peerplays_sidechain_plugin::plugin_startup()
{
   ilog("peerplays sidechain plugin:  plugin_startup()");

   if( !_sons.empty() && !_private_keys.empty() )
   {
      ilog("Starting heartbeats for ${n} sons.", ("n", _sons.size()));
      schedule_heartbeat_loop();
   } else
      elog("No sons configured! Please add SON IDs and private keys to configuration.");
   ilog("peerplays sidechain plugin:  plugin_startup() end");
}

void peerplays_sidechain_plugin::schedule_heartbeat_loop()
{
   fc::time_point now = fc::time_point::now();
   int64_t time_to_next_heartbeat = 180000000;

   fc::time_point next_wakeup( now + fc::microseconds( time_to_next_heartbeat ) );

   _heartbeat_task = fc::schedule([this]{heartbeat_loop();},
                                         next_wakeup, "SON Heartbeat Production");
}

void peerplays_sidechain_plugin::heartbeat_loop()
{
   chain::database& d = database();
   chain::son_id_type son_id = *(_sons.begin());
   const chain::global_property_object& gpo = d.get_global_properties();
   auto it = std::find(gpo.active_sons.begin(), gpo.active_sons.end(), son_id);
   if(it != gpo.active_sons.end()) {
      ilog("peerplays_sidechain_plugin:  sending heartbeat");
      chain::son_heartbeat_operation op;
      const auto& idx = d.get_index_type<chain::son_index>().indices().get<by_id>();
      auto son_obj = idx.find( son_id );
      op.owner_account = son_obj->son_account;
      op.son_id = son_id;
      op.ts = fc::time_point::now() + fc::seconds(0);
      chain::signed_transaction trx = d.create_signed_transaction(_private_keys.begin()->second, op);
      fc::future<bool> fut = fc::async( [&](){
         try {
            d.push_transaction(trx);
            p2p_node().broadcast(net::trx_message(trx));
            return true;
         } catch(fc::exception e){
            ilog("peerplays_sidechain_plugin:  sending heartbeat failed with exception ${e}",("e", e.what()));
            return false;
         }
      });
      fut.wait(fc::seconds(10));
   }
   schedule_heartbeat_loop();
}
} } // graphene::peerplays_sidechain

