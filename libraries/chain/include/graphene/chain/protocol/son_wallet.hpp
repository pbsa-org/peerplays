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

    struct son_wallet_update_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type payer;
        son_wallet_id_type son_wallet_id;
        graphene::peerplays_sidechain::sidechain_type sidechain;
        string address;

        account_id_type fee_payer()const { return payer; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

    struct son_wallet_close_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type payer;
        son_wallet_id_type son_wallet_id;

        account_id_type fee_payer()const { return payer; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

} } // namespace graphene::chain

FC_REFLECT(graphene::chain::son_wallet_create_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_create_operation, (fee)(payer) )
FC_REFLECT(graphene::chain::son_wallet_update_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_update_operation, (fee)(payer)(son_wallet_id)(sidechain)(address) )
FC_REFLECT(graphene::chain::son_wallet_close_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_close_operation, (fee)(payer)(son_wallet_id) )
