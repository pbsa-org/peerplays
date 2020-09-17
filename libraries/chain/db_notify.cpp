/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <fc/container/flat.hpp>

#include <graphene/protocol/authority.hpp>
#include <graphene/protocol/operations.hpp>
#include <graphene/protocol/transaction.hpp>

#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/chain/confidential_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/transaction_history_object.hpp>
#include <graphene/chain/impacted.hpp>
#include <graphene/chain/tournament_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/betting_market_object.hpp>
#include <graphene/chain/offer_object.hpp>
#include <graphene/chain/custom_permission_object.hpp>
#include <graphene/chain/nft_object.hpp>

using namespace fc;
using namespace graphene::chain;

// TODO:  Review all of these, especially no-ops
struct get_impacted_account_visitor
{
   flat_set<account_id_type>& _impacted;
   get_impacted_account_visitor( flat_set<account_id_type>& impact ):_impacted(impact) {}
   typedef void result_type;

   void operator()( const transfer_operation& op )
   {
      _impacted.insert( op.to );
   }

   void operator()( const asset_claim_fees_operation& op ){}
   void operator()( const limit_order_create_operation& op ) {}
   void operator()( const limit_order_cancel_operation& op )
   {
      _impacted.insert( op.fee_paying_account );
   }
   void operator()( const call_order_update_operation& op ) {}
   void operator()( const fill_order_operation& op )
   {
      _impacted.insert( op.account_id );
   }

   void operator()( const account_create_operation& op )
   {
      _impacted.insert( op.registrar );
      _impacted.insert( op.referrer );
      add_authority_accounts( _impacted, op.owner );
      add_authority_accounts( _impacted, op.active );
   }

   void operator()( const account_update_operation& op )
   {
      _impacted.insert( op.account );
      if( op.owner )
         add_authority_accounts( _impacted, *(op.owner) );
      if( op.active )
         add_authority_accounts( _impacted, *(op.active) );
   }

   void operator()( const account_whitelist_operation& op )
   {
      _impacted.insert( op.account_to_list );
   }

   void operator()( const account_upgrade_operation& op ) {}
   void operator()( const account_transfer_operation& op )
   {
      _impacted.insert( op.new_owner );
   }

   void operator()( const asset_create_operation& op ) {}
   void operator()( const asset_update_operation& op )
   {
      if( op.new_issuer )
         _impacted.insert( *(op.new_issuer) );
   }

   void operator()( const asset_update_bitasset_operation& op ) {}
   void operator()( const asset_update_dividend_operation& op ) {}
   void operator()( const asset_dividend_distribution_operation& op )
   {
      _impacted.insert( op.account_id );
   }

   void operator()( const asset_update_feed_producers_operation& op ) {}

   void operator()( const asset_issue_operation& op )
   {
      _impacted.insert( op.issue_to_account );
   }

   void operator()( const asset_reserve_operation& op ) {}
   void operator()( const asset_fund_fee_pool_operation& op ) {}
   void operator()( const asset_settle_operation& op ) {}
   void operator()( const asset_global_settle_operation& op ) {}
   void operator()( const asset_publish_feed_operation& op ) {}
   void operator()( const witness_create_operation& op )
   {
      _impacted.insert( op.witness_account );
   }
   void operator()( const witness_update_operation& op )
   {
      _impacted.insert( op.witness_account );
   }

   void operator()( const proposal_create_operation& op )
   {
      vector<authority> other;
      for( const auto& proposed_op : op.proposed_ops )
         operation_get_required_authorities( proposed_op.op, _impacted, _impacted, other );
      for( auto& o : other )
         add_authority_accounts( _impacted, o );
   }

   void operator()( const proposal_update_operation& op ) {}
   void operator()( const proposal_delete_operation& op ) {}

   void operator()( const withdraw_permission_create_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_update_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_claim_operation& op )
   {
      _impacted.insert( op.withdraw_from_account );
   }

   void operator()( const withdraw_permission_delete_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const committee_member_create_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_global_parameters_operation& op ) {}

   void operator()( const vesting_balance_create_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const vesting_balance_withdraw_operation& op ) {}
   void operator()( const worker_create_operation& op ) {}
   void operator()( const custom_operation& op ) {}
   void operator()( const assert_operation& op ) {}
   void operator()( const balance_claim_operation& op ) {}

   void operator()( const override_transfer_operation& op )
   {
      _impacted.insert( op.to );
      _impacted.insert( op.from );
      _impacted.insert( op.issuer );
   }

   void operator()( const transfer_to_blind_operation& op )
   {
      _impacted.insert( op.from );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const blind_transfer_operation& op )
   {
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const transfer_from_blind_operation& op )
   {
      _impacted.insert( op.to );
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
   }

   void operator()( const asset_settle_cancel_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const fba_distribute_operation& op )
   {
      _impacted.insert( op.account_id );
   }
   void operator()(const sport_create_operation&){}
   void operator()(const sport_update_operation&){}
   void operator()(const sport_delete_operation&){}
   void operator()(const event_group_create_operation&){}
   void operator()(const event_group_update_operation& op ) {}
   void operator()(const event_group_delete_operation& op ) {}
   void operator()(const event_create_operation&){}
   void operator()(const event_update_operation& op ) {}
   void operator()(const event_update_status_operation& op ) {}
   void operator()(const betting_market_rules_create_operation&){}
   void operator()(const betting_market_rules_update_operation& op ) {}
   void operator()(const betting_market_group_create_operation&){}
   void operator()(const betting_market_group_update_operation& op ) {}
   void operator()(const betting_market_create_operation&){}
   void operator()(const betting_market_update_operation&){}
   void operator()(const bet_place_operation&){}
   void operator()(const betting_market_group_resolve_operation&){}
   void operator()(const betting_market_group_resolved_operation &){}
   void operator()(const betting_market_group_cancel_unmatched_bets_operation&){}
   void operator()(const bet_matched_operation &){}
   void operator()(const bet_cancel_operation&){}
   void operator()(const bet_canceled_operation &){}
   void operator()(const bet_adjusted_operation &){}

   void operator()( const tournament_create_operation& op )
   {
      _impacted.insert( op.creator );
      _impacted.insert( op.options.whitelist.begin(), op.options.whitelist.end() );
   }
   void operator()( const tournament_join_operation& op )
   {
      _impacted.insert( op.payer_account_id );
      _impacted.insert( op.player_account_id );
   }
   void operator()( const tournament_leave_operation& op )
   {
      //if account canceling registration is not the player, it must be the payer
      if (op.canceling_account_id != op.player_account_id)
        _impacted.erase( op.canceling_account_id );
      _impacted.erase( op.player_account_id );
   }
   void operator()( const game_move_operation& op )
   {
      _impacted.insert( op.player_account_id );
   }
   void operator()( const tournament_payout_operation& op )
   {
      _impacted.insert( op.payout_account_id );
   }
   void operator()( const affiliate_payout_operation& op )
   {
      _impacted.insert( op.affiliate );
   }
   void operator()( const affiliate_referral_payout_operation& op ) { }
   void operator()( const lottery_asset_create_operation& op ) {}
   void operator()( const ticket_purchase_operation& op )
   {
      _impacted.insert( op.buyer );
   }
   void operator()( const lottery_reward_operation& op ) {
      _impacted.insert( op.winner );
   }
   void operator()( const lottery_end_operation& op ) {
      for( auto participant : op.participants ) {
         _impacted.insert(participant.first);
      }
   }
   void operator()( const sweeps_vesting_claim_operation& op ) {
      _impacted.insert( op.account );
   }
   void operator()( const custom_permission_create_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const custom_permission_update_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const custom_permission_delete_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const custom_account_authority_create_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const custom_account_authority_update_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const custom_account_authority_delete_operation& op ){
      _impacted.insert( op.owner_account );
   }
   void operator()( const nft_metadata_create_operation& op ) {
      _impacted.insert( op.owner );
   }
   void operator()( const nft_metadata_update_operation& op ) {
      _impacted.insert( op.owner );
   }
   void operator()( const nft_mint_operation& op ) {
      _impacted.insert( op.owner );
   }
   void operator()( const nft_safe_transfer_from_operation& op ) {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }
   void operator()( const nft_approve_operation& op ) {
      _impacted.insert( op.operator_ );
      _impacted.insert( op.approved );
   }
   void operator()( const nft_set_approval_for_all_operation& op ) {
      _impacted.insert( op.owner );
      _impacted.insert( op.operator_ );
   }
   void operator()( const offer_operation& op ) { 
      _impacted.insert( op.issuer );   
   }
   void operator()( const bid_operation& op ) {
	  _impacted.insert( op.bidder );
   }
   void operator()( const cancel_offer_operation& op ) {
	  _impacted.insert( op.issuer );
   }
   void operator()( const finalize_offer_operation& op ) {
       _impacted.insert( op.fee_paying_account );
   }
};

void graphene::chain::operation_get_impacted_accounts( const operation& op, flat_set<account_id_type>& result )
{
  get_impacted_account_visitor vtor = get_impacted_account_visitor( result );
  op.visit( vtor );
}

void graphene::chain::transaction_get_impacted_accounts( const transaction& tx, flat_set<account_id_type>& result )
{
  for( const auto& op : tx.operations )
    operation_get_impacted_accounts( op, result );
}

void get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )
{
   if( obj->id.space() == protocol_ids )
   {
      switch( (object_type)obj->id.type() )
      {
        case null_object_type:
        case base_object_type:
           return;
        case account_object_type:{
           accounts.insert( obj->id );
           break;
        } case asset_object_type:{
           const auto& aobj = dynamic_cast<const asset_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case force_settlement_object_type:{
           const auto& aobj = dynamic_cast<const force_settlement_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->owner );
           break;
        } case committee_member_object_type:{
           const auto& aobj = dynamic_cast<const committee_member_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->committee_member_account );
           break;
        } case witness_object_type:{
           const auto& aobj = dynamic_cast<const witness_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->witness_account );
           break;
        } case limit_order_object_type:{
           const auto& aobj = dynamic_cast<const limit_order_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->seller );
           break;
        } case call_order_object_type:{
           const auto& aobj = dynamic_cast<const call_order_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->borrower );
           break;
        } case custom_object_type:{
          break;
        } case proposal_object_type:{
           const auto& aobj = dynamic_cast<const proposal_object*>(obj);
           assert( aobj != nullptr );
           transaction_get_impacted_accounts( aobj->proposed_transaction, accounts );
           break;
        } case operation_history_object_type:{
           const auto& aobj = dynamic_cast<const operation_history_object*>(obj);
           assert( aobj != nullptr );
           operation_get_impacted_accounts( aobj->op, accounts );
           break;
        } case withdraw_permission_object_type:{
           const auto& aobj = dynamic_cast<const withdraw_permission_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->withdraw_from_account );
           accounts.insert( aobj->authorized_account );
           break;
        } case vesting_balance_object_type:{
           const auto& aobj = dynamic_cast<const vesting_balance_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->owner );
           break;
        } case worker_object_type:{
           const auto& aobj = dynamic_cast<const worker_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->worker_account );
           break;
        } case balance_object_type:{
           /** these are free from any accounts */
           break;
        } case tournament_object_type:{
           auto aobj = dynamic_cast<const tournament_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->creator);
           accounts.insert(aobj->options.whitelist.begin(), aobj->options.whitelist.end());
           break;
        } case tournament_details_object_type:{
           auto aobj = dynamic_cast<const tournament_details_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->registered_players.begin(), aobj->registered_players.end());
           std::transform(aobj->payers.begin(), aobj->payers.end(), std::inserter(accounts, accounts.end()),
                          [](const auto& pair) { return pair.first; });
           std::for_each(aobj->players_payers.begin(), aobj->players_payers.end(),
                         [&accounts](const auto& pair) {
               accounts.insert(pair.first);
               accounts.insert(pair.second);
           });
           break;
        } case match_object_type:{
           auto aobj = dynamic_cast<const match_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->players.begin(), aobj->players.end());
           std::for_each(aobj->game_winners.begin(), aobj->game_winners.end(),
                         [&accounts](const auto& set) { accounts.insert(set.begin(), set.end()); });
           accounts.insert(aobj->match_winners.begin(), aobj->match_winners.end());
           break;
        } case game_object_type:{
           auto aobj = dynamic_cast<const game_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->players.begin(), aobj->players.end());
           accounts.insert(aobj->winners.begin(), aobj->winners.end());
           break;
        } case sport_object_type:
           break;
          case event_group_object_type:
           break;
          case event_object_type:
           break;
          case betting_market_rules_object_type:
           break;
          case betting_market_group_object_type:
           break;
          case betting_market_object_type:
           break;
          case bet_object_type:{
           auto aobj = dynamic_cast<const bet_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->bettor_id);
           break;
        } case custom_permission_object_type:{
           auto aobj = dynamic_cast<const custom_permission_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->account);
           add_authority_accounts(accounts, aobj->auth);
           break;
        } case custom_account_authority_object_type:
           break;
          case offer_object_type:{
           auto aobj = dynamic_cast<const offer_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->issuer);
           if (aobj->bidder.valid())
               accounts.insert(*aobj->bidder);
           break;
        } case nft_metadata_object_type:{
           auto aobj = dynamic_cast<const nft_metadata_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->owner);
           if (aobj->revenue_partner.valid())
               accounts.insert(*aobj->revenue_partner);
           break;
        } case nft_object_type:{
           auto aobj = dynamic_cast<const nft_object*>(obj);
           assert(aobj != nullptr);
           accounts.insert(aobj->owner);
           accounts.insert(aobj->approved);
           accounts.insert(aobj->approved_operators.begin(), aobj->approved_operators.end());
           break;
        }
      }
   }
   else if( obj->id.space() == implementation_ids )
   {
      switch( (impl_object_type)obj->id.type() )
      {
             case impl_global_property_object_type:
              break;
             case impl_dynamic_global_property_object_type:
              break;
             case impl_reserved0_object_type:
              break;
             case impl_asset_dynamic_data_object_type:
              break;
             case impl_asset_bitasset_data_object_type:
              break;
             case impl_account_balance_object_type:{
              const auto& aobj = dynamic_cast<const account_balance_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert( aobj->owner );
              break;
           } case impl_account_statistics_object_type:{
              const auto& aobj = dynamic_cast<const account_statistics_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert( aobj->owner );
              break;
           } case impl_transaction_history_object_type:{
              const auto& aobj = dynamic_cast<const transaction_history_object*>(obj);
              FC_ASSERT( aobj != nullptr );
              transaction_get_impacted_accounts( aobj->trx, accounts );
              break;
           } case impl_blinded_balance_object_type:{
              const auto& aobj = dynamic_cast<const blinded_balance_object*>(obj);
              assert( aobj != nullptr );
              for( const auto& a : aobj->owner.account_auths )
                accounts.insert( a.first );
              break;
           } case impl_block_summary_object_type:
              break;
             case impl_account_transaction_history_object_type:
              break;
             case impl_chain_property_object_type:
              break;
             case impl_witness_schedule_object_type:
              break;
             case impl_budget_record_object_type:
              break;
             case impl_special_authority_object_type:
              break;
             case impl_buyback_object_type:
              break;
             case impl_fba_accumulator_object_type:
              break;
             case impl_asset_dividend_data_object_type:{
              const auto& aobj = dynamic_cast<const asset_dividend_data_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert(aobj->dividend_distribution_account);
              break;
           } case impl_pending_dividend_payout_balance_for_holder_object_type:{
              const auto& aobj = dynamic_cast<const pending_dividend_payout_balance_for_holder_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert(aobj->owner);
              break;
           } case impl_total_distributed_dividend_balance_object_type:
              break;
             case impl_betting_market_position_object_type:{
              const auto& aobj = dynamic_cast<const betting_market_position_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert(aobj->bettor_id);
              break;
           } case impl_global_betting_statistics_object_type:
              break;
             case impl_lottery_balance_object_type:
              break;
             case impl_sweeps_vesting_balance_object_type:{
              const auto& aobj = dynamic_cast<const sweeps_vesting_balance_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert(aobj->owner);
              break;
           } case impl_offer_history_object_type:{
              const auto& aobj = dynamic_cast<const offer_history_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert(aobj->issuer);
              if (aobj->bidder.valid())
                  accounts.insert(*aobj->bidder);
              break;
           }
      }
   }
} // end get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )

namespace graphene { namespace chain {

void database::notify_applied_block( const signed_block& block )
{
   GRAPHENE_TRY_NOTIFY( applied_block, block )
}

void database::notify_on_pending_transaction( const signed_transaction& tx )
{
   GRAPHENE_TRY_NOTIFY( on_pending_transaction, tx )
}

void database::notify_changed_objects()
{ try {
   if( _undo_db.enabled() ) 
   {
      const auto& head_undo = _undo_db.head();

      // New
      if( !new_objects.empty() )
      {
        vector<object_id_type> new_ids;  new_ids.reserve(head_undo.new_ids.size());
        flat_set<account_id_type> new_accounts_impacted;
        for( const auto& item : head_undo.new_ids )
        {
          new_ids.push_back(item);
          auto obj = find_object(item);
          if(obj != nullptr)
            get_relevant_accounts(obj, new_accounts_impacted);
        }

        GRAPHENE_TRY_NOTIFY( new_objects, new_ids, new_accounts_impacted)
      }

      // Changed
      if( !changed_objects.empty() )
      {
        vector<object_id_type> changed_ids;  changed_ids.reserve(head_undo.old_values.size());
        flat_set<account_id_type> changed_accounts_impacted;
        for( const auto& item : head_undo.old_values )
        {
          changed_ids.push_back(item.first);
          get_relevant_accounts(item.second.get(), changed_accounts_impacted);
        }

        GRAPHENE_TRY_NOTIFY( changed_objects, changed_ids, changed_accounts_impacted)
      }

      // Removed
      if( !removed_objects.empty() )
      {
        vector<object_id_type> removed_ids; removed_ids.reserve( head_undo.removed.size() );
        vector<const object*> removed; removed.reserve( head_undo.removed.size() );
        flat_set<account_id_type> removed_accounts_impacted;
        for( const auto& item : head_undo.removed )
        {
          removed_ids.emplace_back( item.first );
          auto obj = item.second.get();
          removed.emplace_back( obj );
          get_relevant_accounts(obj, removed_accounts_impacted);
        }

        GRAPHENE_TRY_NOTIFY( removed_objects, removed_ids, removed, removed_accounts_impacted)
      }
   }
} FC_CAPTURE_AND_LOG( (0) ) }

} }
