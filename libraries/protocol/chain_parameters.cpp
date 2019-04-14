#include <graphene/protocol/chain_parameters.hpp>
#include <graphene/protocol/fee_schedule.hpp>

namespace graphene { namespace protocol {
   chain_parameters::chain_parameters() {
       current_fees = std::make_shared<fee_schedule>();
   }
}}