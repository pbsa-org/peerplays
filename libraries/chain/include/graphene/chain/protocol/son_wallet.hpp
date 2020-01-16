#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

    struct son_wallet_create_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type payer;

        account_id_type fee_payer()const { return payer; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

} } // namespace graphene::chain

FC_REFLECT(graphene::chain::son_wallet_create_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_create_operation, (fee)(payer) )
