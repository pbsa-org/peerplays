#include <graphene/chain/son_wallet_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/son_wallet_object.hpp>

namespace graphene { namespace chain {

void_result create_son_wallet_evaluator::do_evaluate(const son_wallet_create_operation& op)
{ try{
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   FC_ASSERT(db().get_global_properties().parameters.get_son_btc_account_id() != GRAPHENE_NULL_ACCOUNT, "SON paying account not set.");
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type create_son_wallet_evaluator::do_apply(const son_wallet_create_operation& op)
{ try {
    const auto& new_son_wallet_object = db().create<son_wallet_object>( [&]( son_wallet_object& obj ){
        obj.valid_from = db().head_block_time();
        obj.expires = time_point_sec::maximum();
        obj.sons = db().get_global_properties().active_sons;
    });
    return new_son_wallet_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result update_son_wallet_evaluator::do_evaluate(const son_wallet_update_operation& op)
{ try{
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   const auto& idx = db().get_index_type<son_wallet_index>().indices().get<by_id>();
   FC_ASSERT( idx.find(op.son_wallet_id) != idx.end() );
   auto itr = idx.find(op.son_wallet_id);
   //FC_ASSERT( itr.addresses[op.sidechain] == "", "Sidechain wallet address already set");
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type update_son_wallet_evaluator::do_apply(const son_wallet_update_operation& op)
{ try {
   const auto& idx = db().get_index_type<son_wallet_index>().indices().get<by_id>();
   auto itr = idx.find(op.son_wallet_id);
   if(itr != idx.end())
   {
       db().modify(*itr, [&op](son_wallet_object &swo) {
           swo.addresses[op.sidechain] = op.address;
       });
   }
   return op.son_wallet_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result close_son_wallet_evaluator::do_evaluate(const son_wallet_close_operation& op)
{ try{
   FC_ASSERT(db().head_block_time() >= HARDFORK_SON_TIME, "Not allowed until SON HARDFORK");
   const auto& idx = db().get_index_type<son_wallet_index>().indices().get<by_id>();
   FC_ASSERT( idx.find(op.son_wallet_id) != idx.end() );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type close_son_wallet_evaluator::do_apply(const son_wallet_close_operation& op)
{ try {
   const auto& idx = db().get_index_type<son_wallet_index>().indices().get<by_id>();
   auto itr = idx.find(op.son_wallet_id);
   if(itr != idx.end())
   {
       db().modify(*itr, [&, &op](son_wallet_object &swo) {
           swo.expires = db().head_block_time();
       });
   }
   return op.son_wallet_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
