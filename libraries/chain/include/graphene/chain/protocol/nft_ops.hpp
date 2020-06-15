#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

   struct nft_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type payer;
      nft_id_type nft_id;

      account_id_type fee_payer()const { return payer; }
   };

   struct nft_safe_transfer_from_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type payer;
      nft_id_type nft_id;

      account_id_type fee_payer()const { return payer; }
   };

   struct nft_approve_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type payer;
      nft_id_type nft_id;

      account_id_type fee_payer()const { return payer; }
   };

   struct nft_set_approval_for_all_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type payer;
      nft_id_type nft_id;

      account_id_type fee_payer()const { return payer; }
   };

} } // graphene::chain

FC_REFLECT( graphene::chain::nft_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_safe_transfer_from_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_approve_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_set_approval_for_all_operation::fee_parameters_type, (fee) )

FC_REFLECT( graphene::chain::nft_create_operation, (fee) (payer) (nft_id) )
FC_REFLECT( graphene::chain::nft_safe_transfer_from_operation, (fee) (payer) (nft_id) )
FC_REFLECT( graphene::chain::nft_approve_operation, (fee) (payer) (nft_id) )
FC_REFLECT( graphene::chain::nft_set_approval_for_all_operation, (fee) (payer) (nft_id) )

