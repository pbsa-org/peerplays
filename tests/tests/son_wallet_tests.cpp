#include <boost/test/unit_test.hpp>

#include "../common/database_fixture.hpp"

#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/son_wallet_object.hpp>
#include <graphene/chain/son_wallet_evaluator.hpp>

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

      op.payer = gpo.parameters.get_son_btc_account_id();

      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);
   }
   generate_block();

   BOOST_TEST_MESSAGE("Check son_wallet_create_operation results");

   const auto& idx = db.get_index_type<son_wallet_index>().indices().get<by_valid_from>();
   BOOST_REQUIRE( idx.size() == 1 );
}

BOOST_AUTO_TEST_SUITE_END()
