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
   WriteBytesStream(bytes& buffer) : storage_(buffer) {}

   bool write( const char* d, size_t s ) {
      storage_.insert(storage_.end(), d, d + s);
      return true;
   }

   void add( const unsigned char* d, size_t s ) {
      storage_.insert(storage_.end(), d, d + s);
   }

   void add( const bytes& data) {
      storage_.insert(storage_.end(), data.begin(), data.end());
   }

   void add(int32_t val)
   {
      const char* data = reinterpret_cast<const char*>(&val);
      write(data, sizeof(val));
   }

   void add(uint32_t val)
   {
      const char* data = reinterpret_cast<const char*>(&val);
      write(data, sizeof(val));
   }

   bool put(char c) {
      storage_.push_back(c);
      return true;
   }
private:
   bytes& storage_;
};

class ReadBytesStream{
public:
   ReadBytesStream(const bytes& buffer, size_t pos = 0) : storage_(buffer), pos_(pos), end_(buffer.size()) {}

   inline bool read( char* d, size_t s ) {
     if( end_ - pos_ >= s ) {
       memcpy( d, &storage_[pos_], s );
       pos_ += s;
       return true;
     }
     FC_THROW( "invalid bitcoin tx buffer" );
   }

   void direct_read( unsigned char* d, size_t s ) {
      if( end_ - pos_ >= s ) {
        memcpy( d, &storage_[pos_], s );
        pos_ += s;
      }
      FC_THROW( "invalid bitcoin tx buffer" );
   }

   void direct_read( bytes& data, size_t s) {
      data.clear();
      data.resize(s);
      direct_read(&data[0], s);
   }

   void direct_read(int32_t& val)
   {
      char* data = reinterpret_cast<char*>(&val);
      read(data, sizeof(val));
   }

   void direct_read(uint32_t& val)
   {
      char* data = reinterpret_cast<char*>(&val);
      read(data, sizeof(val));
   }

   inline bool   get( unsigned char& c ) { return get( *(char*)&c ); }
   inline bool   get( char& c )
   {
     if( pos_ < end_ ) {
       c = storage_[pos_++];
       return true;
     }
     FC_THROW( "invalid bitcoin tx buffer" );
   }
private:
   const bytes& storage_;
   size_t pos_;
   size_t end_;
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

bytes signature_for_raw_transaction(const bytes& unsigned_tx, const fc::ecc::private_key& priv_key)
{
   fc::sha256 digest = fc::sha256::hash(fc::sha256::hash(reinterpret_cast<const char*>(&unsigned_tx[0]), unsigned_tx.size()));
   fc::ecc::compact_signature res = priv_key.sign_compact(digest);
   return bytes(res.begin(), res.begin() + res.size());
}

struct btc_outpoint
{
   fc::uint256 hash;
   uint32_t n;
};

struct btc_in
{
   btc_outpoint prevout;
   bytes scriptSig;
   uint32_t nSequence;
   bytes scriptWitness;
};

struct btc_out
{
   int64_t nValue;
   bytes scriptPubKey;
};

struct btc_tx
{
   std::vector<btc_in> vin;
   std::vector<btc_out> vout;
   int32_t nVersion;
   uint32_t nLockTime;
   bool hasWitness;

   bytes to_bytes() const
   {
      bytes res;
      WriteBytesStream str(res);
      str.add(nVersion);
      if(hasWitness)
      {
         std::vector<btc_in> dummy;
         fc::raw::pack(str, dummy);
         unsigned char flags = 1;
         str.put(flags);
      }
      fc::raw::pack(str, vin);
      fc::raw::pack(str, vout);
      if(hasWitness)
      {
         for(const auto& in: vin)
            fc::raw::pack(str, in.scriptWitness);
      }
      str.add(nLockTime);
      return res;
   }

   void fill_from_bytes(const bytes& data)
   {
      ReadBytesStream ds( data );
      ds.direct_read(nVersion);
      unsigned char flags = 0;
      vin.clear();
      vout.clear();
      hasWitness = false;
      /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
      fc::raw::unpack(ds, vin);
      if (vin.size() == 0) {
          /* We read a dummy or an empty vin. */
          ds.get(flags);
          if (flags != 0) {
             fc::raw::unpack(ds, vin);
             fc::raw::unpack(ds, vout);
             hasWitness = true;
          }
      } else {
          /* We read a non-empty vin. Assume a normal vout follows. */
         fc::raw::unpack(ds, vout);
      }
      if (hasWitness) {
          /* The witness flag is present, and we support witnesses. */
          for (size_t i = 0; i < vin.size(); i++) {
              fc::raw::unpack(ds, vin[i].scriptWitness);
          }
      }
      ds.direct_read(nLockTime);
   }
};

bytes sign_pw_transfer_transaction(const bytes &unsigned_tx, const bytes& redeem_script, const std::vector<fc::optional<fc::ecc::private_key> > &priv_keys)
{
   btc_tx tx;
   tx.fill_from_bytes(unsigned_tx);
   fc::ecc::compact_signature dummy;
   for(auto& in: tx.vin)
   {
      WriteBytesStream script(in.scriptSig);
      for(auto key: priv_keys)
      {
         if(key)
         {
            bytes signature = signature_for_raw_transaction(unsigned_tx, *key);
            script.add(signature);
         }
         else
         {
            script.add(dummy.begin(), dummy.size());
         }
      }
      script.add(redeem_script);
   }
   return tx.to_bytes();
}

}}

FC_REFLECT(graphene::peerplays_sidechain::btc_outpoint, (hash)(n))
FC_REFLECT(graphene::peerplays_sidechain::btc_in, (prevout)(scriptSig)(nSequence))
FC_REFLECT(graphene::peerplays_sidechain::btc_out, (nValue)(scriptPubKey))
