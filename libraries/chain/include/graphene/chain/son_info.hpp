#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/peerplays_sidechain/defs.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   /**
    * @class son_info
    * @brief tracks information about a SON info required to re/create primary wallet
    * @ingroup object
    */
   struct son_info {
      son_id_type son_id;
      uint64_t total_votes = 0;
      public_key_type signing_key;
      flat_map<peerplays_sidechain::sidechain_type, string> sidechain_public_keys;
   };

} }

FC_REFLECT( graphene::chain::son_info,
      (son_id)(total_votes)(signing_key)(sidechain_public_keys) )
