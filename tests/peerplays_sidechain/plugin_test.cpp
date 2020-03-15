#include <boost/test/unit_test.hpp>

#include "son_fixture.hpp"

#include <graphene/peerplays_sidechain/peerplays_sidechain_plugin.hpp>
#include <graphene/utilities/key_conversion.hpp>

using namespace graphene;
using namespace graphene::chain;
using namespace graphene::chain::test;
using namespace graphene::peerplays_sidechain;

BOOST_FIXTURE_TEST_SUITE( plugin, son_fixture )

BOOST_AUTO_TEST_CASE(init)
{
   generate_blocks(HARDFORK_SON_TIME + 1);
   set_expiration(db, trx);

   // create sons
   std::vector<fc::ecc::private_key> keys;
   std::vector<son_id_type> sons;
   std::string test_url = "https://son_test";
   for (size_t i = 0; i < 16; i++) {
      fc::ecc::private_key privkey = generate_private_key(fc::to_string(i));
      keys.push_back(privkey);
      fc::ecc::public_key pubkey = privkey.get_public_key();
      sons.push_back(create_son("son" + fc::to_string(i), test_url, privkey, pubkey));
      ilog("son created: ${i}", ("i", i));
   }

   BOOST_CHECK(sons.size() == 16);

   // start plugin
   peerplays_sidechain_plugin plugin;
   boost::program_options::variables_map cfg;
   {
      std::vector<std::string> data;
      for (const auto& son_id: sons)
         data.push_back("\"" + std::string(object_id_type(son_id)) + "\"");
      cfg.emplace("son-id", boost::program_options::variable_value(data, false));
   }
   {
      std::vector<std::string> data;
      for (const fc::ecc::private_key& key: keys) {
         chain::public_key_type pubkey = key.get_public_key();
         std::string value = "[\"" + std::string(pubkey) + "\", \"" + graphene::utilities::key_to_wif(key) + "\"]";
         data.push_back(value);
      }
      cfg.emplace("peerplays-private-key", boost::program_options::variable_value(data, false));
   }
   {
      cfg.emplace("bitcoin-node-ip", boost::program_options::variable_value(std::string("99.79.189.95"), false));
      cfg.emplace("bitcoin-node-zmq-port", boost::program_options::variable_value(uint32_t(11111), false));
      cfg.emplace("bitcoin-node-rpc-port", boost::program_options::variable_value(uint32_t(22222), false));
      cfg.emplace("bitcoin-node-rpc-user", boost::program_options::variable_value(std::string("1"), false));
      cfg.emplace("bitcoin-node-rpc-password", boost::program_options::variable_value(std::string("1"), false));
      std::vector<std::string> data;
      for (const fc::ecc::private_key& key: keys) {
         chain::public_key_type pubkey = key.get_public_key();
         std::string value = "[\"" + std::string(pubkey) + "\", \"" + graphene::utilities::key_to_wif(key) + "\"]";
         data.push_back(value);
      }
      cfg.emplace("bitcoin-private-key", boost::program_options::variable_value(data, false));
   }
   plugin.plugin_set_app(&app);
   BOOST_CHECK_NO_THROW(plugin.plugin_initialize(cfg));
   BOOST_CHECK_NO_THROW(plugin.plugin_startup());
   BOOST_CHECK_NO_THROW(plugin.plugin_shutdown());
}

BOOST_AUTO_TEST_SUITE_END()
