#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

    struct son_create_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type owner_account;
        std::string     url;

        account_id_type fee_payer()const { return owner_account; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

    struct son_update_operation : public base_operation
    {
         struct fee_parameters_type { uint64_t fee = 0; };

         asset fee;
         son_id_type son_id;
         account_id_type owner_account;
         std::string new_url;

         account_id_type fee_payer()const { return owner_account; }
         share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

    struct son_delete_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type owner_account;

        account_id_type fee_payer()const { return owner_account; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::son_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::son_create_operation, (fee)(owner_account)(url) )

FC_REFLECT( graphene::chain::son_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::son_update_operation, (fee)(son_id)(owner_account)(new_url) )

FC_REFLECT( graphene::chain::son_delete_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::son_delete_operation, (fee)(owner_account) )
