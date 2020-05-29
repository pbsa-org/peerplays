#include <boost/test/unit_test.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/protocol.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/custom_permission_object.hpp>
#include <graphene/chain/custom_account_authority_object.hpp>

#include <graphene/db/simple_index.hpp>

#include <fc/crypto/digest.hpp>
#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE(custom_permission_tests, database_fixture)

BOOST_AUTO_TEST_CASE(permission_create_fail_test)
{
   try
   {
      generate_blocks(HARDFORK_RBAC_TIME);
      generate_block();
      set_expiration(db, trx);
      ACTORS((alice)(bob));
      upgrade_to_lifetime_member(alice);
      upgrade_to_lifetime_member(bob);
      transfer(committee_account, alice_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      transfer(committee_account, bob_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      // alice  fails to create custom permission
      {
         custom_permission_create_operation op;
         op.owner_account = alice_id;
         op.auth = authority(1, bob_id, 1);
         op.permission_name = "123";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "1ab";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = ".abc";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "abc.";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "ABC";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "active";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "owner";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "abcdefghijk";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "ab";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "***";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         op.permission_name = "a12";
         BOOST_CHECK_NO_THROW(op.validate());
         op.permission_name = "a1b";
         BOOST_CHECK_NO_THROW(op.validate());
         op.permission_name = "abc";
         BOOST_CHECK_NO_THROW(op.validate());
         op.permission_name = "abc123defg";
         BOOST_CHECK_NO_THROW(op.validate());
         BOOST_REQUIRE(pidx.size() == 0);
      }
      {
         custom_permission_create_operation op;
         op.permission_name = "abc";
         BOOST_CHECK_THROW(op.validate(), fc::exception);
         const fc::ecc::private_key tpvk = fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("test")));
         const public_key_type tpbk(tpvk.get_public_key());
         op.auth = authority(1, address(tpbk), 1);
         BOOST_CHECK_THROW(op.validate(), fc::exception);

         BOOST_REQUIRE(pidx.size() == 0);
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(permission_create_success_test)
{
   try
   {
      generate_blocks(HARDFORK_RBAC_TIME);
      generate_block();
      set_expiration(db, trx);
      ACTORS((alice)(bob)(charlie)(dave)(erin));
      upgrade_to_lifetime_member(alice);
      upgrade_to_lifetime_member(bob);
      upgrade_to_lifetime_member(charlie);
      upgrade_to_lifetime_member(dave);
      upgrade_to_lifetime_member(erin);
      transfer(committee_account, alice_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      transfer(committee_account, bob_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      transfer(committee_account, charlie_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      transfer(committee_account, dave_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      transfer(committee_account, erin_id, asset(1000 * GRAPHENE_BLOCKCHAIN_PRECISION));
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      // Alice creates a permission abc
      {
         custom_permission_create_operation op;
         op.permission_name = "abc";
         op.owner_account = alice_id;
         op.auth = authority(1, bob_id, 1);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
         BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
         BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
      }
      // Alice tries to create a permission with same name but fails
      {
         custom_permission_create_operation op;
         op.permission_name = "abc";
         op.owner_account = alice_id;
         op.auth = authority(1, bob_id, 1);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
         BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
         BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(permission_update_test)
{
   try
   {
      INVOKE(permission_create_success_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      GET_ACTOR(charlie);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
      BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
      // Alice tries to update permission with same auth but fails
      {
         custom_permission_update_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = alice_id;
         op.new_auth = authority(1, bob_id, 1);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice tries to update permission with no auth but fails
      {
         custom_permission_update_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice tries to update permission with charlie onwer_account but fails
      {
         custom_permission_update_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = charlie_id;
         op.new_auth = authority(1, charlie_id, 1);
         trx.operations.push_back(op);
         sign(trx, charlie_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice updates permission abc with wrong permission_id
      {
         custom_permission_update_operation op;
         op.permission_id = custom_permission_id_type(1);
         op.owner_account = alice_id;
         op.new_auth = authority(1, charlie_id, 1);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice updates permission abc with new auth
      {
         BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
         BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
         custom_permission_update_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = alice_id;
         op.new_auth = authority(1, charlie_id, 1);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
         BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
         BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, charlie_id, 1));
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(account_authority_create_test)
{
   try
   {
      INVOKE(permission_create_success_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      const auto &cidx = db.get_index_type<custom_account_authority_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
      BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
      generate_block();
      // Alice creates a new account auth linking with permission abc
      {
         custom_account_authority_create_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.valid_from = db.head_block_time();
         op.valid_to = db.head_block_time() + fc::seconds(10 * db.block_interval());
         op.operation_type = operation::tag<transfer_operation>::value;
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         BOOST_REQUIRE(cidx.size() == 1);
         generate_block();
      }
      // Alice creates the same account auth linking with permission abc
      {
         custom_account_authority_create_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.valid_from = db.head_block_time();
         op.valid_to = db.head_block_time() + fc::seconds(11 * db.block_interval());
         op.operation_type = operation::tag<transfer_operation>::value;
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         BOOST_REQUIRE(cidx.size() == 2);
         generate_block();
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(account_authority_update_test)
{
   try
   {
      INVOKE(account_authority_create_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      const auto &cidx = db.get_index_type<custom_account_authority_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(cidx.size() == 2);
      generate_block();
      // Alice update the account auth linking with permission abc
      {
         custom_account_authority_update_operation op;
         op.auth_id = custom_account_authority_id_type(0);
         fc::time_point_sec expiry = db.head_block_time() + fc::seconds(50 * db.block_interval());
         op.new_valid_to = expiry;
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
         BOOST_REQUIRE(cidx.size() == 2);
         BOOST_REQUIRE(custom_account_authority_id_type(0)(db).valid_to == expiry);
         BOOST_REQUIRE(custom_account_authority_id_type(1)(db).valid_to < expiry);
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(account_authority_delete_test)
{
   try
   {
      INVOKE(account_authority_create_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      const auto &cidx = db.get_index_type<custom_account_authority_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(cidx.size() == 2);
      generate_block();
      // Alice deletes account auth linking with permission abc
      {
         custom_account_authority_delete_operation op;
         op.auth_id = custom_account_authority_id_type(0);
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
         BOOST_REQUIRE(cidx.size() == 1);
      }
      // Alice deletes the account auth linking with permission abc
      {
         custom_account_authority_delete_operation op;
         op.auth_id = custom_account_authority_id_type(1);
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
         BOOST_REQUIRE(cidx.size() == 0);
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(permission_delete_test)
{
   try
   {
      INVOKE(account_authority_create_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      const auto &cidx = db.get_index_type<custom_account_authority_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(custom_permission_id_type(0)(db).permission_name == "abc");
      BOOST_REQUIRE(custom_permission_id_type(0)(db).auth == authority(1, bob_id, 1));
      BOOST_REQUIRE(cidx.size() == 2);
      // Alice tries to delete permission abc with wrong owner_account
      {
         custom_permission_delete_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = bob_id;
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice tries to delete permission abc with wrong permission_id
      {
         custom_permission_delete_operation op;
         op.permission_id = custom_permission_id_type(1);
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 1);
      }
      // Alice deletes permission abc
      {
         custom_permission_delete_operation op;
         op.permission_id = custom_permission_id_type(0);
         op.owner_account = alice_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         BOOST_REQUIRE(pidx.size() == 0);
         BOOST_REQUIRE(cidx.size() == 0);
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(transfer_op_custom_permission_test)
{
   try
   {
      INVOKE(account_authority_create_test);
      GET_ACTOR(alice);
      GET_ACTOR(bob);
      const auto &pidx = db.get_index_type<custom_permission_index>().indices().get<by_id>();
      const auto &cidx = db.get_index_type<custom_account_authority_index>().indices().get<by_id>();
      BOOST_REQUIRE(pidx.size() == 1);
      BOOST_REQUIRE(cidx.size() == 2);
      // alice->bob transfer_operation op with active auth, success
      generate_block();
      {
         transfer_operation op;
         op.amount.asset_id = asset_id_type(0);
         op.amount.amount = 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
         op.from = alice_id;
         op.to = bob_id;
         op.fee.asset_id = asset_id_type(0);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
      }
      // alice->bob transfer_operation op with the created custom account auth, success
      {
         transfer_operation op;
         op.amount.asset_id = asset_id_type(0);
         op.amount.amount = 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
         op.from = alice_id;
         op.to = bob_id;
         op.fee.asset_id = asset_id_type(0);
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
      }
      // alice->bob transfer_operation op with extra unnecessary sigs (both active and the custom auth), fails
      {
         transfer_operation op;
         op.amount.asset_id = asset_id_type(0);
         op.amount.amount = 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
         op.from = alice_id;
         op.to = bob_id;
         op.fee.asset_id = asset_id_type(0);
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         generate_block();
      }
      // bob->alice transfer_operation op with alice active auth sig, fails
      {
         transfer_operation op;
         op.amount.asset_id = asset_id_type(0);
         op.amount.amount = 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
         op.from = bob_id;
         op.to = alice_id;
         op.fee.asset_id = asset_id_type(0);
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         BOOST_CHECK_THROW(PUSH_TX(db, trx), fc::exception);
         trx.clear();
         generate_block();
      }
      // bob->alice transfer_operation op with bob active auth sig, success
      {
         transfer_operation op;
         op.amount.asset_id = asset_id_type(0);
         op.amount.amount = 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
         op.from = bob_id;
         op.to = alice_id;
         op.fee.asset_id = asset_id_type(0);
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         PUSH_TX(db, trx);
         trx.clear();
         generate_block();
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
