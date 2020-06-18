#include <graphene/chain/nft_evaluator.hpp>
#include <graphene/chain/nft_object.hpp>

namespace graphene { namespace chain {

void_result nft_create_evaluator::do_evaluate( const nft_create_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_create_evaluator::do_apply( const nft_create_operation& op )
{ try {
   const auto& new_nft_object = db().create<nft_object>( [&]( nft_object& obj ){
      obj.owner = op.owner;
      obj.approved_operators = op.approved_operators;
      obj.metadata = op.metadata;
   });
   return new_nft_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_safe_transfer_from_evaluator::do_evaluate( const nft_safe_transfer_from_operation& op )
{ try {

    return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_safe_transfer_from_evaluator::do_apply( const nft_safe_transfer_from_operation& op )
{ try {
   const auto& idx = db().get_index_type<nft_index>().indices().get<by_id>();
   auto itr = idx.find(op.token_id);
   if (itr != idx.end())
   {
      db().modify(*itr, [&op](nft_object &obj) {
         obj.owner = op.to;
         obj.approved_operators.clear();
      });
   }
   return op.token_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_approve_evaluator::do_evaluate( const nft_approve_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_approve_evaluator::do_apply( const nft_approve_operation& op )
{ try {
   const auto& idx = db().get_index_type<nft_index>().indices().get<by_id>();
   auto itr = idx.find(op.token_id);
   if (itr != idx.end())
   {
      db().modify(*itr, [&op](nft_object &obj) {
         obj.approved_operators.push_back(op.approved);
      });
   }
   return op.token_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_set_approval_for_all_evaluator::do_evaluate( const nft_set_approval_for_all_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_set_approval_for_all_evaluator::do_apply( const nft_set_approval_for_all_operation& op )
{ try {
   const auto &idx = db().get_index_type<nft_index>().indices().get<by_owner>();
   const auto &idx_range = idx.equal_range(op.owner);
   std::for_each(idx_range.first, idx_range.second, [&](const nft_object &obj) {
      db().modify(obj, [&op](nft_object &obj) {
         if (op.approved) {
            obj.approved_operators.push_back(op.operator_);
         } else {
            //
         }
      });
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain

