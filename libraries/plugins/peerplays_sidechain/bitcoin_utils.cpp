#include <graphene/peerplays_sidechain/bitcoin_utils.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/sha256.hpp>

namespace graphene { namespace peerplays_sidechain {

static const unsigned char OP_IF = 0x63;
static const unsigned char OP_ENDIF = 0x68;
static const unsigned char OP_SWAP = 0x7c;
static const unsigned char OP_ADD = 0x93;
static const unsigned char OP_GREATERTHAN = 0xa0;
static const unsigned char OP_CHECKSIG = 0xac;

class WriteBytesStream{
public:
   WriteBytesStream(bytes& buffer);

   bool write( const char* d, size_t s ) {
      storage_.insert(storage_.end(), d, d + s);
      return true;
   }

   bool put(char c) {
      storage_.push_back(c);
      return true;
   }
private:
   bytes& storage_;
};


void add_data_to_script(bytes& script, const bytes& data)
{
   WriteBytesStream str(script);
   fc::raw::pack(str, data, 2);
}

bytes generate_redeem_script(fc::flat_map<fc::ecc::public_key, int> key_data)
{
   int total_weight = 0;
   bytes result;
   add_data_to_script(result, {0});
   for(auto& p: key_data)
   {
      total_weight += p.second;
      result.push_back(OP_SWAP);
      auto raw_data = p.first.serialize();
      add_data_to_script(result, bytes(raw_data.begin(), raw_data.begin() + raw_data.size()));
      result.push_back(OP_CHECKSIG);
      result.push_back(OP_IF);
      add_data_to_script(result, {static_cast<unsigned char>(p.second)});
      result.push_back(OP_ADD);
      result.push_back(OP_ENDIF);
   }
   int threshold_weight = 2 * total_weight / 3;
   add_data_to_script(result, {static_cast<unsigned char>(threshold_weight)});
   result.push_back(OP_GREATERTHAN);
   return result;
}

std::string p2sh_address_from_redeem_script(const bytes& script, bitcoin_network network)
{
   bytes data;
   // add version byte
   switch (network) {
   case(mainnet):
      data.push_back(5);
      break;
   case(testnet):
      data.push_back(196);
      break;
   case(regtest):
      data.push_back(196);
      break;
   default:
      FC_THROW("Unknown bitcoin network type [${type}]", ("type", network));
   }
   // add redeem script hash
   fc::ripemd160 h = fc::ripemd160::hash(reinterpret_cast<const char*>(&script[0]), script.size());
   data.insert(data.end(), h.data(), h.data() + h.data_size());
   // calc control sum
   fc::sha256 cs = fc::sha256::hash(fc::sha256::hash(reinterpret_cast<const char*>(&data[0]), data.size()));
   // add first 4 bytes of control sum
   data.insert(data.end(), cs.data(), cs.data() + 4);
   // return base58 encoded data
   return fc::to_base58(reinterpret_cast<const char*>(&data[0]), data.size());
}

}}
