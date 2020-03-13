#include <boost/test/unit_test.hpp>

#include "son_fixture.hpp"

#include <graphene/peerplays_sidechain/peerplays_sidechain_plugin.hpp>

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
   for(size_t i = 0; i < 16; i++)
   {
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
   plugin.plugin_initialize(cfg);
}

BOOST_AUTO_TEST_SUITE_END()
