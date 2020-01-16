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
    });
    return new_son_wallet_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
