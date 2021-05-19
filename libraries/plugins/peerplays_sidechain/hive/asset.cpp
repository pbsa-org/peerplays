#include <graphene/peerplays_sidechain/hive/asset.hpp>

#include <fc/io/json.hpp>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/rational.hpp>

#define ASSET_AMOUNT_KEY "amount"
#define ASSET_PRECISION_KEY "precision"
#define ASSET_NAI_KEY "nai"

namespace fc {

void to_variant(const graphene::peerplays_sidechain::hive::asset &var, fc::variant &vo, uint32_t max_depth) {
   try {
      variant v = mutable_variant_object(ASSET_AMOUNT_KEY, boost::lexical_cast<std::string>(var.amount.value))(ASSET_PRECISION_KEY, uint64_t(TESTS_PRECISION))(ASSET_NAI_KEY, TESTS_NAI);
      vo = v;
   }
   FC_CAPTURE_AND_RETHROW()
}

void from_variant(const fc::variant &var, graphene::peerplays_sidechain::hive::asset &vo, uint32_t max_depth) {
   try {
      FC_ASSERT(var.is_object(), "Asset has to be treated as object.");

      const auto &v_object = var.get_object();

      FC_ASSERT(v_object.contains(ASSET_AMOUNT_KEY), "Amount field doesn't exist.");
      FC_ASSERT(v_object[ASSET_AMOUNT_KEY].is_string(), "Expected a string type for value '${key}'.", ("key", ASSET_AMOUNT_KEY));
      vo.amount = boost::lexical_cast<int64_t>(v_object[ASSET_AMOUNT_KEY].as<std::string>(max_depth));
      FC_ASSERT(vo.amount >= 0, "Asset amount cannot be negative");

      FC_ASSERT(v_object.contains(ASSET_PRECISION_KEY), "Precision field doesn't exist.");
      FC_ASSERT(v_object[ASSET_PRECISION_KEY].is_uint64(), "Expected an unsigned integer type for value '${key}'.", ("key", ASSET_PRECISION_KEY));

      FC_ASSERT(v_object.contains(ASSET_NAI_KEY), "NAI field doesn't exist.");
      FC_ASSERT(v_object[ASSET_NAI_KEY].is_string(), "Expected a string type for value '${key}'.", ("key", ASSET_NAI_KEY));

      vo.symbol = TESTS_SYMBOL_SER;
   }
   FC_CAPTURE_AND_RETHROW()
}
} // namespace fc
