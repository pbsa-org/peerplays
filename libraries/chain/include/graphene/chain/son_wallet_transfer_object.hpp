#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/peerplays_sidechain/defs.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   /**
    * @class son_wallet_transfer_object
    * @brief tracks information about a SON wallet transfer.
    * @ingroup object
    */
   class son_wallet_transfer_object : public abstract_object<son_wallet_transfer_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = son_wallet_transfer_object_type;

         std::string uid;
         time_point_sec timestamp;
         peerplays_sidechain::sidechain_type sidechain;
         std::string transaction_id;
         std::string from;
         std::string to;
         int64_t amount;

         bool processed;
   };

   struct by_uid;
   struct by_sidechain;
   struct by_processed;
   struct by_sidechain_and_processed;
   using son_wallet_transfer_multi_index_type = multi_index_container<
      son_wallet_transfer_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_uid>,
            member<son_wallet_transfer_object, std::string, &son_wallet_transfer_object::uid>
         >,
         ordered_non_unique< tag<by_sidechain>,
            member<son_wallet_transfer_object, peerplays_sidechain::sidechain_type, &son_wallet_transfer_object::sidechain>
         >,
         ordered_non_unique< tag<by_processed>,
            member<son_wallet_transfer_object, bool, &son_wallet_transfer_object::processed>
         >,
         ordered_non_unique< tag<by_sidechain_and_processed>,
            composite_key<son_wallet_transfer_object,
               member<son_wallet_transfer_object, peerplays_sidechain::sidechain_type, &son_wallet_transfer_object::sidechain>,
               member<son_wallet_transfer_object, bool, &son_wallet_transfer_object::processed>
            >
         >
      >
   >;
   using son_wallet_transfer_index = generic_index<son_wallet_transfer_object, son_wallet_transfer_multi_index_type>;
} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::son_wallet_transfer_object, (graphene::db::object),
                    (uid) (timestamp) (sidechain) (transaction_id) (from) (to) (amount)
                    (processed) )
