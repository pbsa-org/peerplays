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

class update_son_wallet_evaluator : public evaluator<update_son_wallet_evaluator>
{
public:
    typedef son_wallet_update_operation operation_type;

    void_result do_evaluate(const son_wallet_update_operation& o);
    object_id_type do_apply(const son_wallet_update_operation& o);
};

class close_son_wallet_evaluator : public evaluator<close_son_wallet_evaluator>
{
public:
    typedef son_wallet_close_operation operation_type;

    void_result do_evaluate(const son_wallet_close_operation& o);
    object_id_type do_apply(const son_wallet_close_operation& o);
};

} } // namespace graphene::chain
