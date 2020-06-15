#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   class nft_object : public abstract_object<nft_object>
   {
      public:
         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = nft_object_type;

         account_id_type         owner;
         vector<account_id_type> approved_operators;
         std::string                  metadata;
   };

   struct by_owner;
   struct by_owner_and_id;
   using nft_multi_index_type = multi_index_container<
      nft_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_non_unique< tag<by_owner>,
            member<nft_object, account_id_type, &nft_object::owner>
         >,
         ordered_unique< tag<by_owner_and_id>,
            composite_key<nft_object,
               member<nft_object, account_id_type, &nft_object::owner>,
               member<object, object_id_type, &object::id>
            >
         >
      >
   >;
   using nft_index = generic_index<nft_object, nft_multi_index_type>;

} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::nft_object, (graphene::db::object),
                    (owner)
                    (approved_operators)
                    (metadata) )

