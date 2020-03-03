#include <graphene/chain/sidechain_transaction_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/sidechain_transaction_object.hpp>
#include <graphene/chain/son_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/hardfork.hpp>

namespace graphene
{
namespace chain
{

void_result bitcoin_transaction_send_evaluator::do_evaluate(const bitcoin_transaction_send_operation &op)
{
   try
   {
      FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
      FC_ASSERT( op.payer == GRAPHENE_SON_ACCOUNT, "SON paying account must be set as payer." );
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

object_id_type bitcoin_transaction_send_evaluator::do_apply(const bitcoin_transaction_send_operation &op)
{
   try
   {
      const auto &new_bitcoin_transaction_object = db().create<bitcoin_transaction_object>([&](bitcoin_transaction_object &obj) {
         obj.processed = false;
         obj.unsigned_tx = op.unsigned_tx;
         obj.redeem_script = op.redeem_script;
         obj.in_amounts = op.in_amounts;
         obj.signatures = op.signatures;
      });
      return new_bitcoin_transaction_object.id;
   }
   FC_CAPTURE_AND_RETHROW((op))
}

void_result bitcoin_transaction_sign_evaluator::do_evaluate(const bitcoin_transaction_sign_operation &op)
{
   try
   {
      FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK"); // can be removed after HF date pass
      const auto &tx_idx = db().get_index_type<bitcoin_transaction_index>().indices().get<by_id>();
      const auto &tx_itr = tx_idx.find(op.tx_id);
      FC_ASSERT(tx_idx.end() != tx_itr, "bitcoin transaction not found");

      // Checks can this SON sign this tx
      auto can_this_son_sign_this_tx = [&]() {
         const auto &sidx = db().get_index_type<son_index>().indices().get<graphene::chain::by_account>();
         auto son_obj = sidx.find(op.payer);
         if (son_obj == sidx.end())
         {
            return false;
         }
         auto it = tx_itr->signatures.find(son_obj->id);
         if (it == tx_itr->signatures.end())
            return false;
         // tx is not signed with this son already
         return it->second.empty();
      };

      FC_ASSERT(can_this_son_sign_this_tx(), "Invalid approval received");
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

object_id_type bitcoin_transaction_sign_evaluator::do_apply(const bitcoin_transaction_sign_operation &op)
{
   try
   {
      const auto &bitcoin_tx = op.tx_id(db());
      const auto &sidx = db().get_index_type<son_index>().indices().get<graphene::chain::by_account>();
      auto son_obj = sidx.find(op.payer);

      db().modify(bitcoin_tx, [&](bitcoin_transaction_object &btx) {
         btx.signatures[son_obj->id] = op.signatures;
      });

      db().modify( son_obj->statistics( db() ), [&]( son_statistics_object& sso ) {
         sso.txs_signed += 1;
      } );

      return bitcoin_tx.id;
   }
   FC_CAPTURE_AND_RETHROW((op))
}


void_result bitcoin_send_transaction_process_evaluator::do_evaluate(const bitcoin_send_transaction_process_operation &op)
{
   try
   {
      FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
      FC_ASSERT( op.payer == GRAPHENE_SON_ACCOUNT, "SON paying account must be set as payer." );
      const auto& btidx = db().get_index_type<bitcoin_transaction_index>().indices().get<by_id>();
      const auto btobj = btidx.find(op.bitcoin_transaction_id);
      FC_ASSERT(btobj != btidx.end(), "Bitcoin Transaction Object not found");
      FC_ASSERT(btobj->processed == false, "Bitcoin Transaction already processed");
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

object_id_type bitcoin_send_transaction_process_evaluator::do_apply(const bitcoin_send_transaction_process_operation &op)
{
   try
   {
      const auto &btidx = db().get_index_type<bitcoin_transaction_index>().indices().get<by_id>();
      auto btobj = btidx.find(op.bitcoin_transaction_id);
      if (btobj != btidx.end())
      {
         db().modify(*btobj, [&op](bitcoin_transaction_object &bto) {
            bto.processed = true;
         });
      }
      return op.bitcoin_transaction_id;
   }
   FC_CAPTURE_AND_RETHROW((op))
}

} // namespace chain
} // namespace graphene
