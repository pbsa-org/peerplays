#include <boost/test/unit_test.hpp>

#include "../common/database_fixture.hpp"

#include <graphene/chain/son_object.hpp>
#include <graphene/chain/son_evaluator.hpp>

using namespace graphene::chain;
using namespace graphene::chain::test;

class test_create_son_member_evaluator: public create_son_evaluator {
public:
   test_create_son_member_evaluator( database& link_db ):
                                     ptr_trx_state( new transaction_evaluation_state( &link_db ) )
   {
       trx_state = ptr_trx_state.get();
   }
   std::unique_ptr<transaction_evaluation_state> ptr_trx_state;
};

class test_update_son_member_evaluator: public update_son_evaluator {
public:
   test_update_son_member_evaluator( database& link_db ):
                                     ptr_trx_state( new transaction_evaluation_state( &link_db ) )
   {
      trx_state = ptr_trx_state.get();
   }
   std::unique_ptr<transaction_evaluation_state> ptr_trx_state;
};

class test_delete_son_member_evaluator: public delete_son_evaluator {
public:
   test_delete_son_member_evaluator( database& link_db ):
                                     ptr_trx_state( new transaction_evaluation_state( &link_db ) )
   {
      trx_state = ptr_trx_state.get();
   }
   std::unique_ptr<transaction_evaluation_state> ptr_trx_state;
};

BOOST_FIXTURE_TEST_SUITE( son_operation_tests, database_fixture )

BOOST_AUTO_TEST_CASE( create_son_test ) {
   generate_blocks( HARDFORK_SON_TIME );
   while( db.head_block_time() <= HARDFORK_SON_TIME )
   {
       generate_block();
   }
   std::string test_url = "https://create_son_test";
   test_create_son_member_evaluator test_eval( db );
   son_create_operation op;
   op.owner_account = account_id_type(1);
   op.url = test_url;

   BOOST_CHECK_NO_THROW( test_eval.do_evaluate( op ) );
   auto id = test_eval.do_apply( op );
   const auto& idx = db.get_index_type<son_member_index>().indices().get<by_account>();

   BOOST_REQUIRE( idx.size() == 1 );

   auto obj = idx.find( op.owner_account );
   BOOST_REQUIRE( obj != idx.end() );
   BOOST_CHECK( obj->url == test_url );
   BOOST_CHECK( id == obj->id );
}

BOOST_AUTO_TEST_CASE( update_son_test ){
   INVOKE(create_son_test);
   std::string new_url = "https://anewurl.com";
   test_update_son_member_evaluator test_eval( db );
   son_update_operation op;
   op.owner_account = account_id_type(1);
   op.new_url = new_url;
   op.son_id = son_id_type(0);

   BOOST_CHECK_NO_THROW( test_eval.do_evaluate( op ) );
   auto id = test_eval.do_apply( op );
   const auto& idx = db.get_index_type<son_member_index>().indices().get<by_account>();

   BOOST_REQUIRE( idx.size() == 1 );

   auto obj = idx.find( op.owner_account );
   BOOST_REQUIRE( obj != idx.end() );
   BOOST_CHECK( obj->url == new_url );
   BOOST_CHECK( id == obj->id );
}

BOOST_AUTO_TEST_CASE( delete_son_test ) {
   INVOKE(create_son_test);
   test_delete_son_member_evaluator test_eval( db );

   son_delete_operation delete_op;
   delete_op.owner_account = account_id_type(1);
   delete_op.son_id = son_id_type(0);

   BOOST_CHECK_NO_THROW( test_eval.do_evaluate( delete_op ) );
   test_eval.do_apply( delete_op );

   const auto& idx = db.get_index_type<son_member_index>().indices().get<by_account>();
   BOOST_REQUIRE( idx.empty() );
}

BOOST_AUTO_TEST_CASE( update_delete_not_own ) { // fee payer needs to be the son object owner
try {
   generate_blocks(HARDFORK_SON_TIME);
   while (db.head_block_time() <= HARDFORK_SON_TIME) {
      generate_block();
   }
   generate_block();
   set_expiration(db, trx);

   ACTORS((alice)(bob));

   upgrade_to_lifetime_member(alice);
   upgrade_to_lifetime_member(bob);

   set_expiration(db, trx);
   std::string test_url = "https://create_son_test";

   // alice became son
   {
      son_create_operation op;
      op.owner_account = alice_id;
      op.url = test_url;
      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);
   }
   generate_block();

   set_expiration(db, trx);
   trx.clear();

   const auto& idx = db.get_index_type<son_member_index>().indices().get<by_account>();
   BOOST_REQUIRE( idx.size() == 1 );
   auto obj = idx.find( alice_id );
   BOOST_REQUIRE( obj != idx.end() );
   BOOST_CHECK( obj->url == test_url );

   // bob tries to update a son object he dont own
   {
      son_update_operation op;
      op.owner_account = bob_id;
      op.new_url = "whatever";
      op.son_id = son_id_type(0);

      trx.operations.push_back(op);
      sign(trx, bob_private_key);
      GRAPHENE_REQUIRE_THROW(PUSH_TX( db, trx ), fc::exception);
   }
   generate_block();

   set_expiration(db, trx);
   trx.clear();

   obj = idx.find( alice_id );
   BOOST_REQUIRE( obj != idx.end() );
   // not changing
   BOOST_CHECK( obj->url == "https://create_son_test" );

   // bob tries to delete a son object he dont own
   {
      son_delete_operation op;
      op.owner_account = bob_id;
      op.son_id = son_id_type(0);

      trx.operations.push_back(op);
      sign(trx, bob_private_key);
      GRAPHENE_REQUIRE_THROW(PUSH_TX( db, trx ), fc::exception);

   }
   generate_block();

   obj = idx.find( alice_id );
   // not deleting
   BOOST_REQUIRE( obj != idx.end() );
   BOOST_CHECK( obj->son_member_account.instance ==  alice_id.instance);

}
catch (fc::exception &e) {
   edump((e.to_detail_string()));
   throw;
}
}

BOOST_AUTO_TEST_SUITE_END()
