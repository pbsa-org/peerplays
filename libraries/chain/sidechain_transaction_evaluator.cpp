#include <graphene/chain/sidechain_transaction_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/sidechain_transaction_object.hpp>
#include <graphene/chain/son_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/hardfork.hpp>

namespace graphene { namespace chain {

void_result sidechain_transaction_create_evaluator::do_evaluate(const sidechain_transaction_create_operation &op)
{ try {
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   FC_ASSERT( op.payer == GRAPHENE_SON_ACCOUNT, "SON paying account must be set as payer." );

   //FC_ASSERT( (!op.son_wallet_id && !op.son_wallet_deposit_id && !op.son_wallet_withdraw_id), "Sidechain transaction origin not set." );
   //FC_ASSERT( op.son_wallet_id && op.son_wallet_deposit_id, "Sidechain transaction origin ambiguous. Single origin required." );
   //FC_ASSERT( op.son_wallet_id && op.son_wallet_withdraw_id, "Sidechain transaction origin ambiguous. Single origin required." );
   //FC_ASSERT( op.son_wallet_deposit_id && op.son_wallet_withdraw_id, "Sidechain transaction origin ambiguous. Single origin required." );
   FC_ASSERT( !op.transaction.empty(), "Sidechain transaction data not set." );

   return void_result();
} FC_CAPTURE_AND_RETHROW( ( op ) ) }

object_id_type sidechain_transaction_create_evaluator::do_apply(const sidechain_transaction_create_operation &op)
{ try {
   const auto &new_sidechain_transaction_object = db().create<sidechain_transaction_object>([&](sidechain_transaction_object &sto) {
      sto.sidechain = op.sidechain;
      sto.son_wallet_id = op.son_wallet_id;
      sto.son_wallet_deposit_id = op.son_wallet_deposit_id;
      sto.son_wallet_withdraw_id = op.son_wallet_withdraw_id;
      sto.transaction = op.transaction;
      sto.signatures = op.signatures;
      sto.valid = true;
      sto.completed = false;
      sto.sent = false;
   });
   return new_sidechain_transaction_object.id;
} FC_CAPTURE_AND_RETHROW( ( op ) ) }

//void_result sidechain_transaction_sign_evaluator::do_evaluate(const sidechain_transaction_sign_operation &op)
//{
//   try
//   {
//      FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK"); // can be removed after HF date pass
//      const auto &proposal_idx = db().get_index_type<proposal_index>().indices().get<by_id>();
//      const auto &proposal_itr = proposal_idx.find(op.proposal_id);
//      FC_ASSERT(proposal_idx.end() != proposal_itr, "proposal not found");
//      // Checks can this SON approve this proposal
//      auto can_this_son_approve_this_proposal = [&]() {
//         const auto &sidx = db().get_index_type<son_index>().indices().get<graphene::chain::by_account>();
//         auto son_obj = sidx.find(op.payer);
//         if (son_obj == sidx.end())
//         {
//            return false;
//         }
//         // TODO: Check if the SON is included in the PW script.
//         return true;
//      };
//
//      FC_ASSERT(can_this_son_approve_this_proposal(), "Invalid approval received");
//      return void_result();
//   }
//   FC_CAPTURE_AND_RETHROW((op))
//}
//
//object_id_type sidechain_transaction_sign_evaluator::do_apply(const sidechain_transaction_sign_operation &op)
//{
//   try
//   {
//      const auto &proposal = op.proposal_id(db());
//      const auto &sidx = db().get_index_type<son_index>().indices().get<graphene::chain::by_account>();
//      auto son_obj = sidx.find(op.payer);
//
//      db().modify(proposal, [&](proposal_object &po) {
//         auto sidechain_transaction_send_op = po.proposed_transaction.operations[0].get<sidechain_transaction_create_operation>();
//         sidechain_transaction_send_op.signatures[son_obj->id] = op.signatures;
//         po.proposed_transaction.operations[0] = sidechain_transaction_send_op;
//      });
//
//      db().modify( son_obj->statistics( db() ), [&]( son_statistics_object& sso ) {
//         sso.txs_signed += 1;
//      } );
//
//      update_proposal(op);
//   }
//   FC_CAPTURE_AND_RETHROW((op))
//}
//
//void sidechain_transaction_sign_evaluator::update_proposal(const sidechain_transaction_sign_operation &op)
//{
//   database &d = db();
//   proposal_update_operation update_op;
//
//   update_op.fee_paying_account = op.payer;
//   update_op.proposal = op.proposal_id;
//   update_op.active_approvals_to_add = {op.payer};
//
//   bool skip_fee_old = trx_state->skip_fee;
//   bool skip_fee_schedule_check_old = trx_state->skip_fee_schedule_check;
//   trx_state->skip_fee = true;
//   trx_state->skip_fee_schedule_check = true;
//
//   d.apply_operation(*trx_state, update_op);
//
//   trx_state->skip_fee = skip_fee_old;
//   trx_state->skip_fee_schedule_check = skip_fee_schedule_check_old;
//}
//
//void_result sidechain_transaction_send_evaluator::do_evaluate(const sidechain_transaction_send_operation &op)
//{
//   try
//   {
//      FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
//      FC_ASSERT( op.payer == GRAPHENE_SON_ACCOUNT, "SON paying account must be set as payer." );
//      const auto& btidx = db().get_index_type<sidechain_transaction_index>().indices().get<by_id>();
//      const auto btobj = btidx.find(op.sidechain_transaction_id);
//      FC_ASSERT(btobj != btidx.end(), "Bitcoin Transaction Object not found");
//      FC_ASSERT(btobj->processed == false, "Bitcoin Transaction already processed");
//      return void_result();
//   }
//   FC_CAPTURE_AND_RETHROW((op))
//}
//
//object_id_type sidechain_transaction_send_evaluator::do_apply(const sidechain_transaction_send_operation &op)
//{
//   try
//   {
//      const auto &btidx = db().get_index_type<sidechain_transaction_index>().indices().get<by_id>();
//      auto btobj = btidx.find(op.sidechain_transaction_id);
//      if (btobj != btidx.end())
//      {
//         db().modify(*btobj, [&op](sidechain_transaction_object &bto) {
//            bto.processed = true;
//         });
//      }
//      return op.sidechain_transaction_id;
//   }
//   FC_CAPTURE_AND_RETHROW((op))
//}

} // namespace chain
} // namespace graphene
