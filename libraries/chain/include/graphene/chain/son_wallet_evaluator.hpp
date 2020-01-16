#pragma once
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/son_wallet.hpp>

namespace graphene { namespace chain {

class create_son_wallet_evaluator : public evaluator<create_son_wallet_evaluator>
{
public:
    typedef son_wallet_create_operation operation_type;

    void_result do_evaluate(const son_wallet_create_operation& o);
    object_id_type do_apply(const son_wallet_create_operation& o);
};

} } // namespace graphene::chain
