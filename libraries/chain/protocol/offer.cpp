#include <graphene/chain/protocol/offer.hpp>

namespace graphene
{
namespace chain
{
share_type offer_operation::calculate_fee(const fee_parameters_type &schedule) const
{
  share_type core_fee_required = schedule.fee;
  return core_fee_required;
}

void offer_operation::validate() const
{
  for (const auto &item_id : item_ids)
  {
    FC_ASSERT(item_id > 0);
  }
  FC_ASSERT(fee.amount >= 0);
  FC_ASSERT(minimum_price.asset_id == maximum_price.asset_id);
  FC_ASSERT(minimum_price.amount >= 0 && maximum_price.amount > 0);
  FC_ASSERT(maximum_price >= minimum_price);
}

share_type bid_operation::calculate_fee(const fee_parameters_type &schedule) const
{
  share_type core_fee_required = schedule.fee;
  return core_fee_required;
}

void bid_operation::validate() const
{
  FC_ASSERT(fee.amount.value >= 0);
  FC_ASSERT(bid_price.amount.value >= 0);
}

void finalize_offer_operation::validate() const
{
  FC_ASSERT(fee.amount.value >= 0);
}

share_type finalize_offer_operation::calculate_fee(const fee_parameters_type &k) const
{
  share_type core_fee_required = k.fee;
  return core_fee_required;
}

} // namespace chain
} // namespace graphene