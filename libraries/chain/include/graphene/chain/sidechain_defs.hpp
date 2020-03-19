#pragma once

#include <fc/reflect/reflect.hpp>

namespace graphene { namespace chain {

enum class sidechain_type {
   bitcoin,
   ethereum,
   eos,
   peerplays
};

} }

FC_REFLECT_ENUM(graphene::chain::sidechain_type,
        (bitcoin)
        (ethereum)
        (eos)
        (peerplays) )
