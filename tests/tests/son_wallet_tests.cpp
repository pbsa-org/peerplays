#include <boost/test/unit_test.hpp>

#include "../common/database_fixture.hpp"

#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/son_wallet_object.hpp>
#include <graphene/peerplays_sidechain/defs.hpp>

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( son_wallet_tests, database_fixture )

BOOST_AUTO_TEST_CASE( son_wallet_create_test ) {

   BOOST_TEST_MESSAGE("son_wallet_create_test");

   generate_block();
   set_expiration(db, trx);

   ACTORS((alice));

   generate_block();
   set_expiration(db, trx);

   {
      BOOST_TEST_MESSAGE("Send son_wallet_create_operation");

      son_wallet_create_operation op;

      op.payer = db.get_global_properties().parameters.get_son_btc_account_id();

      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);
   }
   generate_block();

   BOOST_TEST_MESSAGE("Check son_wallet_create_operation results");

   const auto& idx = db.get_index_type<son_wallet_index>().indices().get<by_id>();
   BOOST_REQUIRE( idx.size() == 1 );
   auto obj = idx.find(son_wallet_id_type(0));
   BOOST_REQUIRE( obj->addresses.at(graphene::peerplays_sidechain::sidechain_type::bitcoin) == "" );
   BOOST_REQUIRE( obj->expires == time_point_sec::maximum() );
}

BOOST_AUTO_TEST_CASE( son_wallet_update_test ) {

   BOOST_TEST_MESSAGE("son_wallet_update_test");

   INVOKE(son_wallet_create_test);
   GET_ACTOR(alice);

   {
      BOOST_TEST_MESSAGE("Send son_wallet_update_operation");

      son_wallet_update_operation op;

      op.payer = db.get_global_properties().parameters.get_son_btc_account_id();
      op.son_wallet_id = son_wallet_id_type(0);
      op.sidechain = graphene::peerplays_sidechain::sidechain_type::bitcoin;
      op.address = "bitcoin address";

      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);
   }
   generate_block();

   {
      BOOST_TEST_MESSAGE("Check son_wallet_update_operation results");

      const auto& idx = db.get_index_type<son_wallet_index>().indices().get<by_id>();
      BOOST_REQUIRE( idx.size() == 1 );
      auto obj = idx.find(son_wallet_id_type(0));
      BOOST_REQUIRE( obj->addresses.at(graphene::peerplays_sidechain::sidechain_type::bitcoin) == "bitcoin address" );
   }

}

BOOST_AUTO_TEST_CASE( son_wallet_close_test ) {

   BOOST_TEST_MESSAGE("son_wallet_close_test");

   INVOKE(son_wallet_create_test);
   GET_ACTOR(alice);

   {
      BOOST_TEST_MESSAGE("Send son_wallet_close_operation");

      son_wallet_close_operation op;

      op.payer = db.get_global_properties().parameters.get_son_btc_account_id();
      op.son_wallet_id = son_wallet_id_type(0);

      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);
   }
   generate_block();

   {
      BOOST_TEST_MESSAGE("Check son_wallet_close_operation results");

      const auto& idx = db.get_index_type<son_wallet_index>().indices().get<by_id>();
      BOOST_REQUIRE( idx.size() == 1 );
      auto obj = idx.find(son_wallet_id_type(0));
      BOOST_REQUIRE( obj->expires != time_point_sec::maximum() );
   }

}

BOOST_AUTO_TEST_SUITE_END()
