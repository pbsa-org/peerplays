#include <graphene/peerplays_sidechain/hive/transaction.hpp>

#include <boost/algorithm/hex.hpp>

#include <fc/bitutil.hpp>
#include <fc/io/raw.hpp>

namespace graphene { namespace peerplays_sidechain { namespace hive {

digest_type transaction::digest() const {
   digest_type::encoder enc;
   fc::raw::pack(enc, *this);
   return enc.result();
}

transaction_id_type transaction::id() const {
   auto h = digest();
   transaction_id_type result;
   memcpy(result._hash, h._hash, std::min(sizeof(result), sizeof(h)));
   return result;
}

digest_type transaction::sig_digest(const chain_id_type &chain_id) const {
   digest_type::encoder enc;
   fc::raw::pack(enc, chain_id);
   fc::raw::pack(enc, *this);
   return enc.result();
}

void transaction::set_expiration(fc::time_point_sec expiration_time) {
   expiration = expiration_time;
}

void transaction::set_reference_block(const block_id_type &reference_block) {
   ref_block_num = fc::endian_reverse_u32(reference_block._hash[0]);
   ref_block_prefix = reference_block._hash[1];
}

void signed_transaction::clear() {
   operations.clear();
   signatures.clear();
}

const signature_type &signed_transaction::sign(const hive::private_key_type &key, const hive::chain_id_type &chain_id) {
   digest_type h = sig_digest(chain_id);
   auto sig = key.sign_compact(h, true);

   ilog("Signing1: chain_id  = ${chain_id}", ("chain_id", chain_id));
   ilog("Signing1: key       = ${key}", ("key", key));
   ilog("Signing1: this      = ${this}", ("this", *this));
   ilog("Signing1: h         = ${h}", ("h", h));
   ilog("Signing1: signature = ${sig}", ("sig", sig));

   std::stringstream ss_st;
   fc::raw::pack(ss_st, chain_id);
   ilog("Signing: ${ss_st}", ("ss_st", boost::algorithm::hex(ss_st.str())));

   ss_st.str("");
   fc::raw::pack(ss_st, *this);
   ilog("Signing: ${ss_st}", ("ss_st", boost::algorithm::hex(ss_st.str())));

   signatures.push_back(sig);
   return signatures.back();
}

signature_type signed_transaction::sign(const hive::private_key_type &key, const hive::chain_id_type &chain_id) const {
   digest_type::encoder enc;
   fc::raw::pack(enc, chain_id);
   fc::raw::pack(enc, *this);
   auto sig = key.sign_compact(enc.result(), true);

   ilog("Signing2: chain_id  = ${chain_id}", ("chain_id", chain_id));
   ilog("Signing2: key       = ${key}", ("key", key));
   ilog("Signing2: this      = ${this}", ("this", *this));
   ilog("Signing2: h         = ${h}", ("h", enc.result()));
   ilog("Signing2: signature = ${sig}", ("sig", sig));

   return sig;
}

}}} // namespace graphene::peerplays_sidechain::hive
