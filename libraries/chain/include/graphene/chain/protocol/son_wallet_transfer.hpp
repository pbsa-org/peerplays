#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

    struct son_wallet_transfer_create_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type payer;

        std::string uid;
        fc::time_point_sec timestamp;
        peerplays_sidechain::sidechain_type sidechain;
        std::string transaction_id;
        std::string from;
        std::string to;
        int64_t amount;

        account_id_type fee_payer()const { return payer; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

    struct son_wallet_transfer_process_operation : public base_operation
    {
        struct fee_parameters_type { uint64_t fee = 0; };

        asset fee;
        account_id_type payer;

        son_wallet_transfer_id_type son_wallet_transfer_id;

        account_id_type fee_payer()const { return payer; }
        share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
    };

} } // namespace graphene::chain

FC_REFLECT(graphene::chain::son_wallet_transfer_create_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_transfer_create_operation, (fee)(payer) 
      (uid) (timestamp) (sidechain) (transaction_id) (from) (to) (amount))
FC_REFLECT(graphene::chain::son_wallet_transfer_process_operation::fee_parameters_type, (fee) )
FC_REFLECT(graphene::chain::son_wallet_transfer_process_operation, (fee)(payer) 
      (son_wallet_transfer_id))
