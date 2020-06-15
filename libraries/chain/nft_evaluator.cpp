#include <graphene/chain/nft_evaluator.hpp>
#include <graphene/chain/nft_object.hpp>

namespace graphene { namespace chain {

void_result nft_create_evaluator::do_evaluate( const nft_create_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_create_evaluator::do_apply( const nft_create_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_safe_transfer_from_evaluator::do_evaluate( const nft_safe_transfer_from_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_safe_transfer_from_evaluator::do_apply( const nft_safe_transfer_from_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_approve_evaluator::do_evaluate( const nft_approve_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_approve_evaluator::do_apply( const nft_approve_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }


void_result nft_set_approval_for_all_evaluator::do_evaluate( const nft_set_approval_for_all_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result nft_set_approval_for_all_evaluator::do_apply( const nft_set_approval_for_all_operation& op )
{ try {

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // graphene::chain

