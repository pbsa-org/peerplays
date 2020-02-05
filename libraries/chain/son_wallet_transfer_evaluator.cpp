#include <graphene/chain/son_wallet_transfer_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/son_wallet_transfer_object.hpp>

namespace graphene { namespace chain {

void_result create_son_wallet_transfer_evaluator::do_evaluate(const son_wallet_transfer_create_operation& op)
{ try{
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   //FC_ASSERT(db().get_global_properties().parameters.get_son_btc_account_id() != GRAPHENE_NULL_ACCOUNT, "SON paying account not set.");
   FC_ASSERT( op.payer == db().get_global_properties().parameters.get_son_btc_account_id() );

   const auto& idx = db().get_index_type<son_wallet_transfer_index>().indices().get<by_uid>();
   FC_ASSERT(idx.find(op.uid) == idx.end(), "Already registered " + op.uid);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type create_son_wallet_transfer_evaluator::do_apply(const son_wallet_transfer_create_operation& op)
{ try {
    const auto& new_son_wallet_transfer_object = db().create<son_wallet_transfer_object>( [&]( son_wallet_transfer_object& obj ){
        obj.uid = op.uid;
        obj.timestamp = op.timestamp;
        obj.sidechain = op.sidechain;
        obj.transaction_id = op.transaction_id;
        obj.from = op.from;
        obj.to = op.to;
        obj.amount = op.amount;
        obj.processed = false;
    });
    return new_son_wallet_transfer_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result process_son_wallet_transfer_evaluator::do_evaluate(const son_wallet_transfer_process_operation& op)
{ try{
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   //FC_ASSERT(db().get_global_properties().parameters.get_son_btc_account_id() != GRAPHENE_NULL_ACCOUNT, "SON paying account not set.");
   FC_ASSERT( op.payer == db().get_global_properties().parameters.get_son_btc_account_id() );

   const auto& idx = db().get_index_type<son_wallet_transfer_index>().indices().get<by_id>();
   FC_ASSERT(idx.find(op.son_wallet_transfer_id) != idx.end(), "Son wallet transfer not found");
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type process_son_wallet_transfer_evaluator::do_apply(const son_wallet_transfer_process_operation& op)
{ try {
   const auto& idx = db().get_index_type<son_wallet_transfer_index>().indices().get<by_id>();
   auto itr = idx.find(op.son_wallet_transfer_id);
   if(itr != idx.end())
   {
       db().modify(*itr, [&op](son_wallet_transfer_object &swto) {
           swto.processed = true;
       });
   }
   return op.son_wallet_transfer_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
