#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/sidechain_defs.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   /**
    * @class sidechain_transaction_object
    * @brief tracks state of sidechain transaction during signing process.
    * @ingroup object
    */
   class sidechain_transaction_object : public abstract_object<sidechain_transaction_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = sidechain_transaction_object_type;

         sidechain_type sidechain;
         optional<son_wallet_id_type> son_wallet_id;
         optional<son_wallet_deposit_id_type> son_wallet_deposit_id;
         optional<son_wallet_withdraw_id_type> son_wallet_withdraw_id;
         std::string transaction;
         std::vector<std::pair<son_id_type, bool>> signers;

         bool valid = false;
         bool complete = false;
         bool sent = false;
   };

   struct by_sidechain_and_complete;
   struct by_sidechain_and_complete_and_sent;
   using sidechain_transaction_multi_index_type = multi_index_container<
      sidechain_transaction_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_non_unique< tag<by_sidechain_and_complete>,
            composite_key<sidechain_transaction_object,
               member<sidechain_transaction_object, sidechain_type, &sidechain_transaction_object::sidechain>,
               member<sidechain_transaction_object, bool, &sidechain_transaction_object::complete>
            >
         >,
         ordered_non_unique< tag<by_sidechain_and_complete_and_sent>,
            composite_key<sidechain_transaction_object,
               member<sidechain_transaction_object, sidechain_type, &sidechain_transaction_object::sidechain>,
               member<sidechain_transaction_object, bool, &sidechain_transaction_object::complete>,
               member<sidechain_transaction_object, bool, &sidechain_transaction_object::sent>
            >
         >
      >
   >;
   using sidechain_transaction_index = generic_index<sidechain_transaction_object, sidechain_transaction_multi_index_type>;
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::sidechain_transaction_object, (graphene::db::object ),
                    (sidechain)
                    (son_wallet_id)
                    (son_wallet_deposit_id)
                    (son_wallet_withdraw_id)
                    (transaction)
                    (signers)
                    (valid)
                    (complete)
                    (sent) )
