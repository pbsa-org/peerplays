/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <graphene/chain/database.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/custom_permission_object.hpp>
#include <graphene/chain/custom_account_authority_object.hpp>
#include <graphene/chain/offer_object.hpp>

#include <ctime>
#include <algorithm>

namespace graphene { namespace chain {

const asset_object& database::get_core_asset() const
{
   return *_p_core_asset_obj;
}

const asset_dynamic_data_object& database::get_core_dynamic_data() const
{
   return *_p_core_dynamic_data_obj;
}

const global_property_object& database::get_global_properties()const
{
   return *_p_global_prop_obj;
}

const chain_property_object& database::get_chain_properties()const
{
   return *_p_chain_property_obj;
}

const dynamic_global_property_object& database::get_dynamic_global_properties() const
{
   return *_p_dyn_global_prop_obj;
}

const fee_schedule&  database::current_fee_schedule()const
{
   return std::ref( *get_global_properties().parameters.current_fees );
}

time_point_sec database::head_block_time()const
{
   return get_dynamic_global_properties().time;
}

uint32_t database::head_block_num()const
{
   return get_dynamic_global_properties().head_block_number;
}

block_id_type database::head_block_id()const
{
   return get_dynamic_global_properties().head_block_id;
}

decltype( chain_parameters::block_interval ) database::block_interval( )const
{
   return get_global_properties().parameters.block_interval;
}

const chain_id_type& database::get_chain_id( )const
{
   return get_chain_properties().chain_id;
}

const node_property_object& database::get_node_properties()const
{
   return _node_property_object;
}

node_property_object& database::node_properties()
{
   return _node_property_object;
}

uint32_t database::last_non_undoable_block_num() const
{
   return head_block_num() - _undo_db.size();
}

std::vector<uint32_t> database::get_seeds(asset_id_type for_asset, uint8_t count_winners) const
{
   FC_ASSERT( count_winners <= 64 );
   std::string salted_string = std::string(_random_number_generator._seed) + std::to_string(for_asset.instance.value);
   auto seeds_hash = fc::sha256::hash(salted_string);
   uint32_t* seeds = (uint32_t*)(seeds_hash._hash);

   std::vector<uint32_t> result;
   result.reserve(64);

   for( int s = 0; s < 8; ++s ) {
      auto sub_seeds_hash = fc::sha256::hash(std::to_string(seeds[s]) + std::to_string(for_asset.instance.value));
      uint32_t* sub_seeds = (uint32_t*) sub_seeds_hash._hash;
      for( int ss = 0; ss < 8; ++ss ) {
         result.push_back(sub_seeds[ss]);
      }
   }
   return result;
}

const std::vector<uint32_t> database::get_winner_numbers( asset_id_type for_asset, uint32_t count_members, uint8_t count_winners ) const
{
   std::vector<uint32_t> result;
   if( count_members < count_winners ) count_winners = count_members;
   if( count_winners == 0 ) return result;
   result.reserve(count_winners);

   auto seeds = get_seeds(for_asset, count_winners);

   for (auto current_seed = seeds.begin(); current_seed != seeds.end(); ++current_seed) {
      uint8_t winner_num = *current_seed % count_members;
      while( std::find(result.begin(), result.end(), winner_num) != result.end() ) {
         *current_seed = (*current_seed * 1103515245 + 12345) / 65536; //using gcc's consts for pseudorandom
         winner_num = *current_seed % count_members;
      }
      result.push_back(winner_num);
      if (result.size() >= count_winners) break;
   }
   
   FC_ASSERT(result.size() == count_winners);
   return result;
}

const account_statistics_object& database::get_account_stats_by_owner( account_id_type owner )const
{
   auto& idx = get_index_type<account_stats_index>().indices().get<by_owner>();
   auto itr = idx.find( owner );
   FC_ASSERT( itr != idx.end(), "Can not find account statistics object for owner ${a}", ("a",owner) );
   return *itr;
}

const witness_schedule_object& database::get_witness_schedule_object()const
{
   return *_p_witness_schedule_obj;
}

vector<authority> database::get_account_custom_authorities(account_id_type account, const operation& op)const
{
   const auto& pindex = get_index_type<custom_permission_index>().indices().get<by_account_and_permission>();
   const auto& cindex = get_index_type<custom_account_authority_index>().indices().get<by_permission_and_op>();
   auto prange = pindex.equal_range(boost::make_tuple(account));
   time_point_sec now = head_block_time();
   vector<authority> custom_auths;
   for(const custom_permission_object& pobj : boost::make_iterator_range(prange.first, prange.second))
   {
      auto crange = cindex.equal_range(boost::make_tuple(pobj.id, op.which()));
      for(const custom_account_authority_object& cobj : boost::make_iterator_range(crange.first, crange.second))
      {
         if(now >= cobj.valid_from && now < cobj.valid_to)
         {
            custom_auths.push_back(pobj.auth);
         }
      }
   }
   return custom_auths;
}

bool database::item_locked(const nft_id_type &item) const
{
   const auto &offer_idx = get_index_type<offer_index>();
   const auto &oidx = dynamic_cast<const base_primary_index &>(offer_idx);
   const auto &market_items = oidx.get_secondary_index<graphene::chain::offer_item_index>();

   auto items_itr = market_items._locked_items.find(item);
   return (items_itr != market_items._locked_items.end());
}
} }
