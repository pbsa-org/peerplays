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

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/vesting_balance_evaluator.hpp>
#include <graphene/chain/vesting_balance_object.hpp>

namespace graphene { namespace chain {

void_result vesting_balance_create_evaluator::do_evaluate( const vesting_balance_create_operation& op )
{ try {
   const database& d = db();

   const account_object& creator_account = op.creator( d );
   /* const account_object& owner_account = */ op.owner( d );

   // TODO: Check asset authorizations and withdrawals

   FC_ASSERT( op.amount.amount > 0 );
   FC_ASSERT( d.get_balance( creator_account.id, op.amount.asset_id ) >= op.amount );
   FC_ASSERT( !op.amount.asset_id(d).is_transfer_restricted() );

   if(d.head_block_time() >= HARDFORK_SON_TIME && op.balance_type == vesting_balance_type::son) // Todo: hf check can be removed after pass
      FC_ASSERT( op.amount.amount >= d.get_global_properties().parameters.son_vesting_amount() );

   if(d.head_block_time() < HARDFORK_GPOS_TIME) // Todo: can be removed after gpos hf time pass
      FC_ASSERT( op.balance_type == vesting_balance_type::normal);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

struct init_policy_visitor
{
   typedef void result_type;

   init_policy_visitor( vesting_policy& po,
                        const share_type& begin_balance,
                        const fc::time_point_sec& n ):p(po),init_balance(begin_balance),now(n){}

   vesting_policy&    p;
   share_type         init_balance;
   fc::time_point_sec now;

   void operator()( const linear_vesting_policy_initializer& i )const
   {
      linear_vesting_policy policy;
      policy.begin_timestamp = i.begin_timestamp;
      policy.vesting_cliff_seconds = i.vesting_cliff_seconds;
      policy.vesting_duration_seconds = i.vesting_duration_seconds;
      policy.begin_balance = init_balance;
      p = policy;
   }

   void operator()( const cdd_vesting_policy_initializer& i )const
   {
      cdd_vesting_policy policy;
      policy.vesting_seconds = i.vesting_seconds;
      policy.start_claim = i.start_claim;
      policy.coin_seconds_earned = 0;
      policy.coin_seconds_earned_last_update = now;
      p = policy;
   }
   void operator()( const dormant_vesting_policy_initializer& i )const
   {
      dormant_vesting_policy policy;
      p = policy;
   }
};

object_id_type vesting_balance_create_evaluator::do_apply( const vesting_balance_create_operation& op )
{ try {
   database& d = db();
   const time_point_sec now = d.head_block_time();

   FC_ASSERT( d.get_balance( op.creator, op.amount.asset_id ) >= op.amount );
   d.adjust_balance( op.creator, -op.amount );

   const vesting_balance_object& vbo = d.create< vesting_balance_object >( [&]( vesting_balance_object& obj )
   {
      //WARNING: The logic to create a vesting balance object is replicated in vesting_balance_worker_type::initializer::init.
      // If making changes to this logic, check if those changes should also be made there as well.
      obj.owner = op.owner;
      obj.balance = op.amount;
      if(op.balance_type == vesting_balance_type::gpos)
      {
         const auto &gpo = d.get_global_properties();
         // forcing gpos policy
         linear_vesting_policy p;
         p.begin_timestamp = now;
         p.vesting_cliff_seconds = gpo.parameters.gpos_vesting_lockin_period();
         p.vesting_duration_seconds = gpo.parameters.gpos_subperiod();
         obj.policy = p;
      }
      else {
         op.policy.visit(init_policy_visitor(obj.policy, op.amount.amount, now));
      }
      obj.balance_type = op.balance_type;
   } );
   return vbo.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

operation_result vesting_balance_withdraw_evaluator::start_evaluate( transaction_evaluation_state& eval_state, const operation& op, bool apply )
{ try {
   trx_state   = &eval_state;
   const auto& oper = op.get<vesting_balance_withdraw_operation>();

   //check_required_authorities(op);
   auto result = evaluate( oper );

   if( apply ) result = this->apply( oper );
   return result;
} FC_CAPTURE_AND_RETHROW() }

void_result vesting_balance_withdraw_evaluator::do_evaluate( const vesting_balance_withdraw_operation& op )
{ try {
   const database& d = db();
   const time_point_sec now = d.head_block_time();

   const vesting_balance_object& vbo = op.vesting_balance( d );
   if(vbo.balance_type == vesting_balance_type::normal)
   {
      FC_ASSERT( op.owner == vbo.owner, "", ("op.owner", op.owner)("vbo.owner", vbo.owner) );
      FC_ASSERT( vbo.is_withdraw_allowed( now, op.amount ), "Account has insufficient ${balance_type} Vested Balance to withdraw",
            ("balance_type", get_vesting_balance_type(vbo.balance_type))("now", now)("op", op)("vbo", vbo) );
      assert( op.amount <= vbo.balance );      // is_withdraw_allowed should fail before this check is reached
   }
   else if(now > HARDFORK_GPOS_TIME && vbo.balance_type == vesting_balance_type::gpos)
   {
      const account_id_type account_id = op.owner;
      vector<vesting_balance_object> vbos;
      auto vesting_range = d.get_index_type<vesting_balance_index>().indices().get<by_account>().equal_range(account_id);
      std::for_each(vesting_range.first, vesting_range.second,
                    [&vbos, now](const vesting_balance_object& balance) {
                        if(balance.balance.amount > 0 && balance.balance_type == vesting_balance_type::gpos
                         && balance.is_withdraw_allowed(now, balance.balance.amount) && balance.balance.asset_id == asset_id_type())
                           vbos.emplace_back(balance);
                    });

      asset total_amount;
      for (const vesting_balance_object& vesting_balance_obj : vbos)
      {
         total_amount += vesting_balance_obj.balance.amount;
      }
      FC_ASSERT( op.amount <= total_amount, "Account has either insufficient GPOS Vested Balance or lock-in period is not matured");
   }

   /* const account_object& owner_account =  op.owner( d ); */
   // TODO: Check asset authorizations and withdrawals
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result vesting_balance_withdraw_evaluator::do_apply( const vesting_balance_withdraw_operation& op )
{ try {
   database& d = db();

   const time_point_sec now = d.head_block_time();
   //Handling all GPOS withdrawls separately from normal and SONs(future extension).
   // One request/transaction would be sufficient to withdraw from multiple vesting balance ids
   const vesting_balance_object& vbo = op.vesting_balance( d );
   if(vbo.balance_type == vesting_balance_type::normal)
   {
      // Allow zero balance objects to stick around, (1) to comply
      // with the chain's "objects live forever" design principle, (2)
      // if it's cashback or worker, it'll be filled up again.

      d.modify( vbo, [&]( vesting_balance_object& vbo )
      {
         vbo.withdraw( now, op.amount );
      } );

      d.adjust_balance( op.owner, op.amount );
   }
   else if(now > HARDFORK_GPOS_TIME && vbo.balance_type == vesting_balance_type::gpos)
   {
      const account_id_type account_id = op.owner;
      vector<vesting_balance_id_type> ids;
      auto vesting_range = d.get_index_type<vesting_balance_index>().indices().get<by_account>().equal_range(account_id);
      std::for_each(vesting_range.first, vesting_range.second,
                    [&ids, now](const vesting_balance_object& balance) {
                        if(balance.balance.amount > 0 && balance.balance_type == vesting_balance_type::gpos
                         && balance.is_withdraw_allowed(now, balance.balance.amount) && balance.balance.asset_id == asset_id_type())
                           ids.emplace_back(balance.id);
                    });

      asset total_withdraw_amount = op.amount;
      for (const vesting_balance_id_type& id : ids)
      {
         const vesting_balance_object& vbo = id( d );
         if(total_withdraw_amount.amount > vbo.balance.amount)
         {
            total_withdraw_amount.amount -= vbo.balance.amount;
            d.adjust_balance( op.owner, vbo.balance );            
            d.modify( vbo, [&]( vesting_balance_object& vbo ) {vbo.withdraw( now, vbo.balance );} );
         }
         else
         {
            d.modify( vbo, [&]( vesting_balance_object& vbo ) {vbo.withdraw( now, total_withdraw_amount );} );
            d.adjust_balance( op.owner, total_withdraw_amount);
            break;
         }
      }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain
