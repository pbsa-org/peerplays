#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

   struct nft_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type owner;
      account_id_type approved;
      vector<account_id_type> approved_operators;
      std::string metadata;

      account_id_type fee_payer()const { return owner; }
   };

   struct nft_safe_transfer_from_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type operator_;

      account_id_type from;
      account_id_type to;
      nft_id_type token_id;
      string data;

      account_id_type fee_payer()const { return operator_; }
   };

   struct nft_approve_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type owner;

      account_id_type approved;
      nft_id_type token_id;

      account_id_type fee_payer()const { return owner; }
   };

   struct nft_set_approval_for_all_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      asset fee;

      account_id_type owner;

      account_id_type operator_;
      bool approved;

      account_id_type fee_payer()const { return owner; }
   };

} } // graphene::chain

FC_REFLECT( graphene::chain::nft_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_safe_transfer_from_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_approve_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::nft_set_approval_for_all_operation::fee_parameters_type, (fee) )

FC_REFLECT( graphene::chain::nft_create_operation, (fee) (owner) (approved) (approved_operators) (metadata) )
FC_REFLECT( graphene::chain::nft_safe_transfer_from_operation, (fee) (operator_) (from) (to) (token_id) (data) )
FC_REFLECT( graphene::chain::nft_approve_operation, (fee) (owner) (approved) (token_id) )
FC_REFLECT( graphene::chain::nft_set_approval_for_all_operation, (fee) (owner) (operator_) (approved) )

