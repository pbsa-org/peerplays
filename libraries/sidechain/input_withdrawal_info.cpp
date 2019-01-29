#include <sidechain/input_withdrawal_info.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/primary_wallet_vout_object.hpp>
#include <graphene/chain/bitcoin_address_object.hpp>

namespace sidechain {

uint64_t info_for_vin::count_id_info_for_vin = 0;

bool info_for_vin::comparer::operator() ( const info_for_vin& lhs, const info_for_vin& rhs ) const
{
   if( lhs.created != rhs.created ) {
      return lhs.created < rhs.created;
   }
   return lhs.id < rhs.id;
}

fc::optional<info_for_vin> input_withdrawal_info::get_info_for_pw_vin()
{
   fc::optional< primary_wallet_vout_object > vout = db.pw_vout_manager.get_latest_unused_vout();
   if( !vout.valid() ) {
      return fc::optional<info_for_vin>();
   }

   const auto& pw_address = db.get_latest_PW().address;

   info_for_vin vin;
   vin.identifier = vout->hash_id;
   vin.out = vout->vout;
   vin.address = pw_address.get_address();
   vin.script = pw_address.get_witness_script();

   return vin;
}

void input_withdrawal_info::insert_info_for_vin( const prev_out& out, const std::string& address, bytes script )
{
   info_for_vins.insert( info_for_vin( out, address, script ) );
}

void input_withdrawal_info::modify_info_for_vin( const info_for_vin& obj, const std::function<void( info_for_vin& e )>& func )
{
   info_for_vins.modify<by_identifier>( obj.identifier, func );
}

void input_withdrawal_info::mark_as_used_vin( const info_for_vin& obj )
{
   info_for_vins.modify<by_identifier>( obj.identifier, [&]( info_for_vin& o ) {
      o.created = true;
   });
}

void input_withdrawal_info::remove_info_for_vin( const info_for_vin& obj )
{
   info_for_vins.remove<by_identifier>( obj.identifier );
}

fc::optional<info_for_vin> input_withdrawal_info::find_info_for_vin( fc::sha256 identifier )
{ 
   return info_for_vins.find<by_identifier>( identifier );
}

std::vector<info_for_vin> input_withdrawal_info::get_info_for_vins()
{
   std::vector<info_for_vin> result;

   const auto& addr_idx = db.get_index_type<bitcoin_address_index>().indices().get<by_address>();

   info_for_vins.safe_for<by_id_and_not_created>( [&]( info_for_vin_index::index<by_id_and_not_created>::type::iterator itr_b,
                                                       info_for_vin_index::index<by_id_and_not_created>::type::iterator itr_e )
   {
      for( size_t i = 0; itr_b != itr_e && i < 5 && !itr_b->created; i++ ) {  // 5 amount vins to bitcoin transaction
         info_for_vin vin;
         vin.id = itr_b->id;
         vin.identifier = itr_b->identifier;
         vin.out.hash_tx = itr_b->out.hash_tx;
         vin.out.n_vout = itr_b->out.n_vout;
         vin.out.amount = itr_b->out.amount;
         vin.address = itr_b->address;
         const auto& address_itr = addr_idx.find( itr_b->address );
         FC_ASSERT( address_itr != addr_idx.end() );
         vin.script = address_itr->address.get_witness_script();
         
         result.push_back( vin );
         ++itr_b;
      }
   } );

   return result;
}

void input_withdrawal_info::insert_info_for_vout( const graphene::chain::account_id_type& payer, const std::string& data, const uint64_t& amount )
{
   db.create<graphene::chain::info_for_vout_object>([&](graphene::chain::info_for_vout_object& obj) {
      obj.payer = payer;
      obj.address = bitcoin_address( data );
      obj.amount = amount;
   });
}

void input_withdrawal_info::mark_as_used_vout( const graphene::chain::info_for_vout_object& obj )
{
   const auto& info_for_vout_idx = db.get_index_type<graphene::chain::info_for_vout_index>().indices().get< graphene::chain::by_id >();
   auto itr = info_for_vout_idx.find( obj.id );

   db.modify<graphene::chain::info_for_vout_object>( *itr, [&]( graphene::chain::info_for_vout_object& o ) {
      o.created = true;
   });
}

void input_withdrawal_info::remove_info_for_vout( const info_for_vout& obj )
{
   const auto& info_for_vout_idx = db.get_index_type<graphene::chain::info_for_vout_index>().indices().get< graphene::chain::by_id >();
   auto itr = info_for_vout_idx.find( obj.id );
   db.remove( *itr );
}

fc::optional<graphene::chain::info_for_vout_object> input_withdrawal_info::find_info_for_vout( graphene::chain::info_for_vout_id_type id )
{
   const auto& info_for_vout_idx = db.get_index_type<graphene::chain::info_for_vout_index>().indices().get< graphene::chain::by_id >();
   auto itr = info_for_vout_idx.find( id );
   if( itr != info_for_vout_idx.end() )
      return fc::optional<graphene::chain::info_for_vout_object>( *itr );
   return fc::optional<graphene::chain::info_for_vout_object>();
}

size_t input_withdrawal_info::size_info_for_vouts()
{
   const auto& info_for_vout_idx = db.get_index_type<graphene::chain::info_for_vout_index>().indices().get< graphene::chain::by_id >();
   return info_for_vout_idx.size();
}

std::vector<info_for_vout> input_withdrawal_info::get_info_for_vouts()
{
   std::vector<info_for_vout> result;

   const auto& info_for_vout_idx = db.get_index_type<graphene::chain::info_for_vout_index>().indices().get< graphene::chain::by_id_and_not_created >();
   auto itr = info_for_vout_idx.begin();
   for(size_t i = 0; i < 5 && itr != info_for_vout_idx.end() && !itr->created; i++) {
      info_for_vout vout;
      vout.payer = itr->payer;
      vout.address = itr->address;
      vout.amount = itr->amount;

      result.push_back( vout );
      ++itr;
   }

   return result;
}

}
