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
static const unsigned char OP_EQUAL = 0x87;
static const unsigned char OP_ADD = 0x93;
static const unsigned char OP_GREATERTHAN = 0xa0;
static const unsigned char OP_HASH160 = 0xa9;
static const unsigned char OP_CHECKSIG = 0xac;

class WriteBytesStream{
public:
   WriteBytesStream(bytes& buffer) : storage_(buffer) {}

   void write(const unsigned char* d, size_t s) {
      storage_.insert(storage_.end(), d, d + s);
   }

   bool put(unsigned char c) {
      storage_.push_back(c);
      return true;
   }

   void writedata8(uint8_t obj)
   {
       write((unsigned char*)&obj, 1);
   }

   void writedata16(uint16_t obj)
   {
       obj = htole16(obj);
       write((unsigned char*)&obj, 2);
   }

   void writedata32(uint32_t obj)
   {
       obj = htole32(obj);
       write((unsigned char*)&obj, 4);
   }

   void writedata64(uint64_t obj)
   {
       obj = htole64(obj);
       write((unsigned char*)&obj, 8);
   }

   void write_compact_int(uint64_t val)
   {
      if (val < 253)
      {
          writedata8(val);
      }
      else if (val <= std::numeric_limits<unsigned short>::max())
      {
          writedata8(253);
          writedata16(val);
      }
      else if (val <= std::numeric_limits<unsigned int>::max())
      {
          writedata8(254);
          writedata32(val);
      }
      else
      {
          writedata8(255);
          writedata64(val);
      }
   }

   void writedata(const bytes& data)
   {
      write_compact_int(data.size());
      write(&data[0], data.size());
   }
private:
   bytes& storage_;
};

class ReadBytesStream{
public:
   ReadBytesStream(const bytes& buffer, size_t pos = 0) : storage_(buffer), pos_(pos), end_(buffer.size()) {}

   size_t current_pos() const { return pos_; }
   void set_pos(size_t pos)
   {
      if(pos > end_)
         FC_THROW("Invalid position in BTC tx buffer");
      pos_ = pos;
   }

   inline bool read( unsigned char* d, size_t s ) {
     if( end_ - pos_ >= s ) {
       memcpy( d, &storage_[pos_], s );
       pos_ += s;
       return true;
     }
     FC_THROW( "invalid bitcoin tx buffer" );
   }

   inline bool get( unsigned char& c )
   {
     if( pos_ < end_ ) {
       c = storage_[pos_++];
       return true;
     }
     FC_THROW( "invalid bitcoin tx buffer" );
   }

   uint8_t readdata8()
   {
       uint8_t obj;
       read((unsigned char*)&obj, 1);
       return obj;
   }
   uint16_t readdata16()
   {
       uint16_t obj;
       read((unsigned char*)&obj, 2);
       return le16toh(obj);
   }
   uint32_t readdata32()
   {
       uint32_t obj;
       read((unsigned char*)&obj, 4);
       return le32toh(obj);
   }
   uint64_t readdata64()
   {
       uint64_t obj;
       read((unsigned char*)&obj, 8);
       return le64toh(obj);
   }

   uint64_t read_compact_int()
   {
       uint8_t size = readdata8();
       uint64_t ret = 0;
       if (size < 253)
       {
           ret = size;
       }
       else if (size == 253)
       {
           ret = readdata16();
           if (ret < 253)
               FC_THROW("non-canonical ReadCompactSize()");
       }
       else if (size == 254)
       {
           ret = readdata32();
           if (ret < 0x10000u)
               FC_THROW("non-canonical ReadCompactSize()");
       }
       else
       {
           ret = readdata64();
           if (ret < 0x100000000ULL)
               FC_THROW("non-canonical ReadCompactSize()");
       }
       if (ret > (uint64_t)0x02000000)
           FC_THROW("ReadCompactSize(): size too large");
       return ret;
   }

   void readdata(bytes& data)
   {
      size_t s = read_compact_int();
      data.clear();
      data.resize(s);
      read(&data[0], s);
   }

private:
   const bytes& storage_;
   size_t pos_;
   size_t end_;
};

void btc_outpoint::to_bytes(bytes& stream) const
{
   WriteBytesStream str(stream);
   // TODO: write size?
   str.write((unsigned char*)hash.data(), hash.data_size());
   str.writedata32(n);
}

size_t btc_outpoint::fill_from_bytes(const bytes& data, size_t pos)
{
   ReadBytesStream str(data, pos);
   // TODO: read size?
   str.read((unsigned char*)hash.data(), hash.data_size());
   n = str.readdata32();
   return str.current_pos();
}

void btc_in::to_bytes(bytes& stream) const
{
   prevout.to_bytes(stream);
   WriteBytesStream str(stream);
   str.writedata(scriptSig);
   str.writedata32(nSequence);
}

size_t btc_in::fill_from_bytes(const bytes& data, size_t pos)
{
   pos = prevout.fill_from_bytes(data, pos);
   ReadBytesStream str(data, pos);
   str.readdata(scriptSig);
   nSequence = str.readdata32();
   return str.current_pos();
}

void btc_out::to_bytes(bytes& stream) const
{
   WriteBytesStream str(stream);
   str.writedata64(nValue);
   str.writedata(scriptPubKey);
}

size_t btc_out::fill_from_bytes(const bytes& data, size_t pos)
{
   ReadBytesStream str(data, pos);
   nValue = str.readdata64();
   str.readdata(scriptPubKey);
   return str.current_pos();
}

void btc_tx::to_bytes(bytes& stream) const
{
   WriteBytesStream str(stream);
   str.writedata32(nVersion);
   if(hasWitness)
   {
      // write dummy inputs and flag
      str.write_compact_int(0);
      unsigned char flags = 1;
      str.put(flags);
   }
   str.write_compact_int(vin.size());
   for(const auto& in: vin)
      in.to_bytes(stream);
   str.write_compact_int(vout.size());
   for(const auto& out: vout)
      out.to_bytes(stream);
   if(hasWitness)
   {
      for(const auto& in: vin)
         str.writedata(in.scriptWitness);
   }
   str.writedata32(nLockTime);
}

size_t btc_tx::fill_from_bytes(const bytes& data, size_t pos)
{
   ReadBytesStream ds( data, pos );
   nVersion = ds.readdata32();
   unsigned char flags = 0;
   vin.clear();
   vout.clear();
   hasWitness = false;
   /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
   size_t vin_size = ds.read_compact_int();
   vin.resize(vin_size);
   pos = ds.current_pos();
   for(auto& in: vin)
      pos = in.fill_from_bytes(data, pos);
   ds.set_pos(pos);
   if (vin_size == 0) {
       /* We read a dummy or an empty vin. */
       ds.get(flags);
       if (flags != 0) {
          size_t vin_size = ds.read_compact_int();
          vin.resize(vin_size);
          pos = ds.current_pos();
          for(auto& in: vin)
             pos = in.fill_from_bytes(data, pos);
          ds.set_pos(pos);
          size_t vout_size = ds.read_compact_int();
          vout.resize(vout_size);
          pos = ds.current_pos();
          for(auto& out: vout)
             pos = out.fill_from_bytes(data, pos);
          ds.set_pos(pos);
          hasWitness = true;
       }
   } else {
       /* We read a non-empty vin. Assume a normal vout follows. */
      size_t vout_size = ds.read_compact_int();
      vout.resize(vout_size);
      pos = ds.current_pos();
      for(auto& out: vout)
         pos = out.fill_from_bytes(data, pos);
      ds.set_pos(pos);
   }
   if (hasWitness) {
       /* The witness flag is present, and we support witnesses. */
       for (auto& in: vin)
          ds.readdata(in.scriptWitness);

   }
   nLockTime = ds.readdata32();
   return ds.current_pos();
}


void add_data_to_script(bytes& script, const bytes& data)
{
   WriteBytesStream str(script);
   str.writedata(data);
}

bytes generate_redeem_script(std::vector<std::pair<fc::ecc::public_key, int> > key_data)
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

bytes lock_script_for_redeem_script(const bytes &script)
{
   bytes result;
   result.push_back(OP_HASH160);
   fc::ripemd160 h = fc::ripemd160::hash(reinterpret_cast<const char*>(&script[0]), script.size());
   bytes shash(h.data(), h.data() + h.data_size());
   add_data_to_script(result, shash);
   result.push_back(OP_EQUAL);
   return result;
}

bytes signature_for_raw_transaction(const bytes& unsigned_tx, const fc::ecc::private_key& priv_key)
{
   fc::sha256 digest = fc::sha256::hash(fc::sha256::hash(reinterpret_cast<const char*>(&unsigned_tx[0]), unsigned_tx.size()));
   fc::ecc::compact_signature res = priv_key.sign_compact(digest);
   return bytes(res.begin(), res.begin() + res.size());
}


bytes sign_pw_transfer_transaction(const bytes &unsigned_tx, const bytes& redeem_script, const std::vector<fc::optional<fc::ecc::private_key> > &priv_keys)
{
   btc_tx tx;
   tx.fill_from_bytes(unsigned_tx);
   fc::ecc::compact_signature dummy_sig;
   bytes dummy_data(dummy_sig.begin(), dummy_sig.begin() + dummy_sig.size());
   for(auto& in: tx.vin)
   {
      WriteBytesStream script(in.scriptSig);
      for(auto key: priv_keys)
      {
         if(key)
         {
            bytes signature = signature_for_raw_transaction(unsigned_tx, *key);
            script.writedata(signature);
         }
         else
         {
            script.writedata(dummy_data);
         }
      }
      script.writedata(redeem_script);
   }
   bytes ret;
   tx.to_bytes(ret);
   return ret;
}

}}
