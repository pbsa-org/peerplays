#include <graphene/chain/nft_evaluator.hpp>
#include <graphene/chain/nft_object.hpp>

namespace graphene { namespace chain {

void_result nft_metadata_create_evaluator::do_evaluate( const nft_metadata_create_operation& op )
{ try {
   const auto& idx_nft_md_by_name = db().get_index_type<nft_metadata_index>().indices().get<by_name>();
   FC_ASSERT( idx_nft_md_by_name.find(op.name) == idx_nft_md_by_name.end(), "NFT name already in use" );
   const auto& idx_nft_md_by_symbol = db().get_index_type<nft_metadata_index>().indices().get<by_symbol>();
   FC_ASSERT( idx_nft_md_by_symbol.find(op.symbol) == idx_nft_md_by_symbol.end(), "NFT symbol already in use" );
   FC_ASSERT( (op.revenue_partner && op.revenue_split) || (!op.revenue_partner && !op.revenue_split), "NFT revenue partner info invalid" );
   if(op.revenue_partner) {
      (*op.revenue_partner)(db());
      FC_ASSERT( *op.revenue_split >= 0.0 && *op.revenue_split <= 1.0, "Revenue split percent invalid" );
   }
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_metadata_create_evaluator::do_apply( const nft_metadata_create_operation& op )
{ try {
   const auto& new_nft_metadata_object = db().create<nft_metadata_object>( [&]( nft_metadata_object& obj ){
      obj.owner = op.owner;
      obj.name = op.name;
      obj.symbol = op.symbol;
      obj.base_uri = op.base_uri;
      obj.revenue_partner = op.revenue_partner;
      obj.revenue_split = op.revenue_split;
   });
   return new_nft_metadata_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_metadata_update_evaluator::do_evaluate( const nft_metadata_update_operation& op )
{ try {
   const auto& idx_nft_md = db().get_index_type<nft_metadata_index>().indices().get<by_id>();
   auto itr_nft_md = idx_nft_md.find(op.nft_metadata_id);
   FC_ASSERT( itr_nft_md != idx_nft_md.end(), "NFT metadata not found" );
   FC_ASSERT( itr_nft_md->owner == op.owner, "Only owner can modify NFT metadata" );
   FC_ASSERT( (op.revenue_partner && op.revenue_split) || (!op.revenue_partner && !op.revenue_split), "NFT revenue partner info invalid" );
   if(op.revenue_partner) {
      (*op.revenue_partner)(db());
      FC_ASSERT( *op.revenue_split >= 0.0 && *op.revenue_split <= 1.0, "Revenue split percent invalid" );
   }
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_metadata_update_evaluator::do_apply( const nft_metadata_update_operation& op )
{ try {
   db().modify(db().get(op.nft_metadata_id), [&] ( nft_metadata_object& obj ) {
      if( op.name.valid() )
         obj.name = *op.name;
      if( op.symbol.valid() )
         obj.symbol = *op.symbol;
      if( op.base_uri.valid() )
         obj.base_uri = *op.base_uri;
      if( op.revenue_partner.valid() )
         obj.revenue_partner = op.revenue_partner;
      if( op.revenue_split.valid() )
         obj.revenue_split = op.revenue_split;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_mint_evaluator::do_evaluate( const nft_mint_operation& op )
{ try {
   const auto& idx_nft_md = db().get_index_type<nft_metadata_index>().indices().get<by_id>();
   auto itr_nft_md = idx_nft_md.find(*op.nft_metadata_id);
   FC_ASSERT( itr_nft_md != idx_nft_md.end(), "NFT metadata not found" );
   FC_ASSERT( itr_nft_md->owner == op.payer, "Only metadata owner can mint NFT" );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_mint_evaluator::do_apply( const nft_mint_operation& op )
{ try {
   const auto& new_nft_object = db().create<nft_object>( [&]( nft_object& obj ){
      obj.nft_metadata_id = *op.nft_metadata_id;
      obj.owner = op.owner;
      obj.approved = op.approved;
      obj.approved_operators = op.approved_operators;
   });
   return new_nft_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_safe_transfer_from_evaluator::do_evaluate( const nft_safe_transfer_from_operation& op )
{ try {
   const auto& idx_nft = db().get_index_type<nft_index>().indices().get<by_id>();
   const auto& idx_acc = db().get_index_type<account_index>().indices().get<by_id>();

   auto itr_nft = idx_nft.find(op.token_id);
   FC_ASSERT( itr_nft != idx_nft.end(), "NFT does not exists" );

   FC_ASSERT(!db().item_locked(op.token_id), "Item(s) is already on sale on market, transfer is not allowed");

   auto itr_operator = idx_acc.find(op.operator_);
   FC_ASSERT( itr_operator != idx_acc.end(), "Operator account does not exists" );

   auto itr_owner = idx_acc.find(itr_nft->owner);
   FC_ASSERT( itr_owner != idx_acc.end(), "Owner account does not exists" );

   auto itr_from = idx_acc.find(op.from);
   FC_ASSERT( itr_from != idx_acc.end(), "Sender account does not exists" );
   FC_ASSERT( itr_from->id == itr_owner->id, "Sender account is not owner of this NFT" );

   auto itr_to = idx_acc.find(op.to);
   FC_ASSERT( itr_to != idx_acc.end(), "Receiver account does not exists" );

   auto itr_approved_op = std::find(itr_nft->approved_operators.begin(), itr_nft->approved_operators.end(), op.operator_);
   FC_ASSERT( (itr_nft->owner == op.operator_) || (itr_nft->approved == itr_operator->id) || (itr_approved_op != itr_nft->approved_operators.end()), "Operator is not NFT owner or approved operator" );

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
         obj.approved = {};
         obj.approved_operators.clear();
      });
   }
   return op.token_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_approve_evaluator::do_evaluate( const nft_approve_operation& op )
{ try {
   const auto& idx_nft = db().get_index_type<nft_index>().indices().get<by_id>();
   const auto& idx_acc = db().get_index_type<account_index>().indices().get<by_id>();

   auto itr_nft = idx_nft.find(op.token_id);
   FC_ASSERT( itr_nft != idx_nft.end(), "NFT does not exists" );

   auto itr_owner = idx_acc.find(op.operator_);
   FC_ASSERT( itr_owner != idx_acc.end(), "Owner account does not exists" );

   auto itr_approved = idx_acc.find(op.approved);
   FC_ASSERT( itr_approved != idx_acc.end(), "Approved account does not exists" );

   auto itr_approved_op = std::find(itr_nft->approved_operators.begin(), itr_nft->approved_operators.end(), op.operator_);
   FC_ASSERT( (itr_nft->owner == itr_owner->id) || (itr_approved_op != itr_nft->approved_operators.end()), "Sender is not NFT owner or approved operator" );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type nft_approve_evaluator::do_apply( const nft_approve_operation& op )
{ try {
   const auto& idx = db().get_index_type<nft_index>().indices().get<by_id>();
   auto itr = idx.find(op.token_id);
   if (itr != idx.end())
   {
      db().modify(*itr, [&op](nft_object &obj) {
         obj.approved = op.approved;
         //auto itr = std::find(obj.approved_operators.begin(), obj.approved_operators.end(), op.approved);
         //if (itr == obj.approved_operators.end()) {
         //   obj.approved_operators.push_back(op.approved);
         //}
      });
   }
   return op.token_id;
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_set_approval_for_all_evaluator::do_evaluate( const nft_set_approval_for_all_operation& op )
{ try {
   const auto& idx_acc = db().get_index_type<account_index>().indices().get<by_id>();

   auto itr_operator = idx_acc.find(op.operator_);
   FC_ASSERT( itr_operator != idx_acc.end(), "Operator account does not exists" );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_set_approval_for_all_evaluator::do_apply( const nft_set_approval_for_all_operation& op )
{ try {
   const auto &idx = db().get_index_type<nft_index>().indices().get<by_owner>();
   const auto &idx_range = idx.equal_range(op.owner);
   std::for_each(idx_range.first, idx_range.second, [&](const nft_object &obj) {
      db().modify(obj, [&op](nft_object &obj) {
         auto itr = std::find(obj.approved_operators.begin(), obj.approved_operators.end(), op.operator_);
         if ((op.approved) && (itr == obj.approved_operators.end())) {
            obj.approved_operators.push_back(op.operator_);
         }
         if ((!op.approved) && (itr != obj.approved_operators.end())) {
            obj.approved_operators.erase(itr);
         }
      });
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain

