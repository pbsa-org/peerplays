#include <graphene/chain/offer_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/offer_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <iostream>

namespace graphene { namespace chain {

void_result offer_evaluator::do_evaluate( const offer_operation& op )
{ try {
   database& d = db();

   FC_ASSERT( op.offer_expiration_date > d.head_block_time() );
   FC_ASSERT( op.item_id > 0 );
   FC_ASSERT( op.fee.amount >= 0 );
   FC_ASSERT( op.minimum_price.amount >= 0 && op.maximum_price.amount > 0);
   FC_ASSERT( op.minimum_price.asset_id == op.maximum_price.asset_id );
   FC_ASSERT( op.maximum_price >= op.minimum_price );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) )  }

object_id_type offer_evaluator::do_apply( const offer_operation& o )
{ try {
   database& d = db();

   if (o.buying_item)
   {
       d.adjust_balance( o.issuer, -o.maximum_price);
       d.adjust_balance( o.transfer_agent_id, o.maximum_price );
   }

   const auto& offer_obj = db().create<offer_object>( [&]( offer_object& obj ){
		obj.issuer = o.issuer;

		obj.transfer_agent_id = o.transfer_agent_id;
		obj.item_id = o.item_id;

		obj.minimum_price = o.minimum_price;
		obj.maximum_price = o.maximum_price;

		obj.buying_item = o.buying_item;
		obj.offer_expiration_date = o.offer_expiration_date;
		auto& idx = d.get_index_type<account_index>().indices().get<by_id>();
		auto acc = idx.find(o.issuer);
		FC_ASSERT(acc != idx.end());

   });

   return offer_obj.id;
} FC_CAPTURE_AND_RETHROW((o)) }

void_result bid_evaluator::do_evaluate( const bid_operation& op )
{ try {
   	database& d = db();
   	offer_object offer = op.offer_id(d);

	FC_ASSERT( op.bid_price.asset_id == offer.minimum_price.asset_id);
	FC_ASSERT( offer.transfer_agent_id(d).whitelisted_accounts.find(op.bidder) != offer.transfer_agent_id(d).whitelisted_accounts.end() );
	FC_ASSERT( offer.minimum_price.amount == 0 || op.bid_price >= offer.minimum_price);
	FC_ASSERT( offer.maximum_price.amount == 0 || op.bid_price <= offer.maximum_price);
	if (offer.bidder) {
		FC_ASSERT((offer.buying_item && op.bid_price < *offer.bid_price) || (!offer.buying_item && op.bid_price > *offer.bid_price));
	}
	return void_result();
}  FC_CAPTURE_AND_RETHROW( (op) ) }

void_result bid_evaluator::do_apply( const bid_operation& op )
{ try {
   database& d = db();

   offer_object offer = op.offer_id(d);

   if (!offer.buying_item)
   {
       if(offer.bidder)
       {
          d.adjust_balance( *offer.bidder, *offer.bid_price );
          d.adjust_balance( offer.transfer_agent_id, -(*offer.bid_price ));
       }
       d.adjust_balance( op.bidder, -op.bid_price);
       d.adjust_balance( offer.transfer_agent_id, op.bid_price );
   }
   d.modify( op.offer_id(d), [&]( offer_object& o )
   {
      if (op.bid_price == ( offer.buying_item ? offer.minimum_price : offer.maximum_price ) ) {
         o.offer_expiration_date = d.head_block_time();
      }
      o.bidder = op.bidder;
      o.bid_price = op.bid_price;
   });
	return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result finalize_offer_evaluator::do_evaluate(const finalize_offer_operation& op)
{ try {
    database& d = db();
    offer_object offer = op.offer_id(d);

    // FC_ASSERT(*offer.bid_price == (offer.buying_item ? offer.minimum_price : offer.maximum_price));

    if (op.result != result_type::ExpiredNoBid)
    {
        FC_ASSERT(offer.bidder);
        FC_ASSERT((*offer.bid_price).amount >= 0);
    } else
    {
        FC_ASSERT(!offer.bidder);
    }

    switch (op.result) {
        case result_type::Expired:
        case result_type::ExpiredNoBid:
            FC_ASSERT(offer.offer_expiration_date <= d.head_block_time());
            break;
        default:
            FC_THROW_EXCEPTION( fc::assert_exception, "finalize_offer_operation: unknown result type." );
            break;
    }

    return void_result();
}  FC_CAPTURE_AND_RETHROW( (op) ) }

void_result finalize_offer_evaluator::do_apply(const finalize_offer_operation& op) {
    database& d = db();

    offer_object offer = op.offer_id(d);

    if (op.result != result_type::ExpiredNoBid)
    {
        if (offer.buying_item)
        {
            d.adjust_balance(*offer.bidder, *offer.bid_price);
            d.adjust_balance(offer.transfer_agent_id, -offer.maximum_price);
            if (offer.bid_price < offer.maximum_price)
            {
                d.adjust_balance(offer.issuer, offer.maximum_price - *offer.bid_price);
            }
        } else
        {
            d.adjust_balance(offer.transfer_agent_id, -(*offer.bid_price));
            d.adjust_balance(offer.issuer, *offer.bid_price);
        }
    } else
    {
        if (offer.buying_item)
        {
            d.adjust_balance(offer.issuer, offer.maximum_price);
            d.adjust_balance(offer.transfer_agent_id, -offer.maximum_price);
        }
    }
    d.create<offer_history_object>( [&]( offer_history_object& obj ) {
         obj.issuer = offer.issuer;

         obj.transfer_agent_id = offer.transfer_agent_id;
         obj.item_id = offer.item_id;
         obj.bidder = offer.bidder;
         obj.bid_price = offer.bid_price;
         obj.minimum_price = offer.minimum_price;
         obj.maximum_price = offer.maximum_price;

         obj.buying_item = offer.buying_item;
         obj.offer_expiration_date = offer.offer_expiration_date;
         std::cout << "get id" << uint64_t(obj.get_id()) << std::endl;
    });
    d.remove(op.offer_id(d));
    return void_result();
}

}
}