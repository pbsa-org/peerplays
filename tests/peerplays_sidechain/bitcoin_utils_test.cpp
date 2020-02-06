#include <boost/test/unit_test.hpp>
#include <graphene/peerplays_sidechain/bitcoin_utils.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>

using namespace graphene::peerplays_sidechain;

BOOST_AUTO_TEST_CASE(tx_serialization)
{
   // use real mainnet transaction
   // txid: 6189e3febb5a21cee8b725aa1ef04ffce7e609448446d3a8d6f483c634ef5315
   // json: {"txid":"6189e3febb5a21cee8b725aa1ef04ffce7e609448446d3a8d6f483c634ef5315","hash":"6189e3febb5a21cee8b725aa1ef04ffce7e609448446d3a8d6f483c634ef5315","version":1,"size":224,"vsize":224,"weight":896,"locktime":0,"vin":[{"txid":"55d079ca797fee81416b71b373abedd8722e33c9f73177be0166b5d5fdac478b","vout":0,"scriptSig":{"asm":"3045022100d82e57d4d11d3b811d07f2fa4ded2fb8a3b7bb1d3e9f293433de5c0d1093c3bd02206704ccd2ff437e2f7716b5e9f2502a9cbb41f1245a18b2b10296980f1ae38253[ALL] 02be9919a5ba373b1af58ad757db19e7c836116bb8138e0c6d99599e4db96568f4","hex":"483045022100d82e57d4d11d3b811d07f2fa4ded2fb8a3b7bb1d3e9f293433de5c0d1093c3bd02206704ccd2ff437e2f7716b5e9f2502a9cbb41f1245a18b2b10296980f1ae38253012102be9919a5ba373b1af58ad757db19e7c836116bb8138e0c6d99599e4db96568f4"},"sequence":4294967295}],"vout":[{"value":1.26491535,"n":0,"scriptPubKey":{"asm":"OP_DUP OP_HASH160 95783804d28e528fbc4b48c7700471e6845804eb OP_EQUALVERIFY OP_CHECKSIG","hex":"76a91495783804d28e528fbc4b48c7700471e6845804eb88ac","reqSigs":1,"type":"pubkeyhash","addresses":["1EdKhXv7zjGowPzgDQ4z1wa2ukVrXRXXkP"]}},{"value":0.0002,"n":1,"scriptPubKey":{"asm":"OP_HASH160 fb0670971091da8248b5c900c6515727a20e8662 OP_EQUAL","hex":"a914fb0670971091da8248b5c900c6515727a20e866287","reqSigs":1,"type":"scripthash","addresses":["3QaKF8zobqcqY8aS6nxCD5ZYdiRfL3RCmU"]}}]}
   // hex: "01000000018b47acfdd5b56601be7731f7c9332e72d8edab73b3716b4181ee7f79ca79d055000000006b483045022100d82e57d4d11d3b811d07f2fa4ded2fb8a3b7bb1d3e9f293433de5c0d1093c3bd02206704ccd2ff437e2f7716b5e9f2502a9cbb41f1245a18b2b10296980f1ae38253012102be9919a5ba373b1af58ad757db19e7c836116bb8138e0c6d99599e4db96568f4ffffffff028f1b8a07000000001976a91495783804d28e528fbc4b48c7700471e6845804eb88ac204e00000000000017a914fb0670971091da8248b5c900c6515727a20e86628700000000"
   fc::string strtx("01000000018b47acfdd5b56601be7731f7c9332e72d8edab73b3716b4181ee7f79ca79d055000000006b483045022100d82e57d4d11d3b811d07f2fa4ded2fb8a3b7bb1d3e9f293433de5c0d1093c3bd02206704ccd2ff437e2f7716b5e9f2502a9cbb41f1245a18b2b10296980f1ae38253012102be9919a5ba373b1af58ad757db19e7c836116bb8138e0c6d99599e4db96568f4ffffffff028f1b8a07000000001976a91495783804d28e528fbc4b48c7700471e6845804eb88ac204e00000000000017a914fb0670971091da8248b5c900c6515727a20e86628700000000");
   bytes bintx;
   bintx.resize(strtx.length() / 2);
   fc::from_hex(strtx, reinterpret_cast<char*>(&bintx[0]), bintx.size());
   btc_tx tx;
   BOOST_CHECK_NO_THROW(tx.fill_from_bytes(bintx));
   BOOST_CHECK(tx.nVersion == 1);
   BOOST_CHECK(tx.nLockTime == 0);
   BOOST_CHECK(tx.vin.size() == 1);
   BOOST_CHECK(tx.vout.size() == 2);
   bytes buff;
   tx.to_bytes(buff);
   BOOST_CHECK(bintx == buff);
}

BOOST_AUTO_TEST_CASE(pw_transfer)
{
   // key set for the old Primary Wallet
   std::vector<fc::ecc::private_key> priv_old;
   for(unsigned i = 0; i < 15; ++i)
   {
      const char* seed = reinterpret_cast<const char*>(&i);
      fc::sha256 h = fc::sha256::hash(seed, sizeof(i));
      priv_old.push_back(fc::ecc::private_key::generate_from_seed(h));
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, int> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], i + 1));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);
   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::testnet);
   // This address was filled with testnet transaction  8d8a466f6c829175a8bb747860828b59e7774be0bbf79ffdc70d5e75348180ca
   BOOST_REQUIRE(old_pw == "2NGLS3x8Vk3vN18672YmSnpASm7FxYcoWu6");

   // key set for the new Primary Wallet
   std::vector<fc::ecc::private_key> priv_new;
   for(unsigned i = 16; i < 31; ++i)
   {
      const char* seed = reinterpret_cast<const char*>(&i);
      fc::sha256 h = fc::sha256::hash(seed, sizeof(i));
      priv_new.push_back(fc::ecc::private_key::generate_from_seed(h));
   }
   std::vector<fc::ecc::public_key> pub_new;
   for(auto& key: priv_new)
      pub_new.push_back(key.get_public_key());
   // new key weights
   std::vector<std::pair<fc::ecc::public_key, int> > weights_new;
   for(unsigned i = 0; i < 15; ++i)
      weights_new.push_back(std::make_pair(pub_new[i], 16 - i));
   // redeem script for new PW
   bytes redeem_new =generate_redeem_script(weights_new);
   // New PW address
   std::string new_pw = p2wsh_address_from_redeem_script(redeem_new, bitcoin_network::testnet);

   BOOST_REQUIRE(new_pw == "2MyzbFRwNqj1Y4Q4oWELhDwz5DCHkTndE1S");

   // try to move funds from old wallet to new one

   // get unspent outputs for old wallet with list_uspent (address should be
   // added to wallet with import_address before). It should return
   // 1 UTXO: [8d8a466f6c829175a8bb747860828b59e7774be0bbf79ffdc70d5e75348180ca:1]
   // with 20000 satoshis
   // So, we creating a raw transaction with 1 input and one output that gets
   // 20000 - fee satoshis with createrawtransaction call (bitcoin_rpc_client::prepare_tx)
   // Here we just serialize the transaction without scriptSig in inputs then sign it.
   btc_outpoint outpoint;
   outpoint.hash = fc::uint256("8d8a466f6c829175a8bb747860828b59e7774be0bbf79ffdc70d5e75348180ca");
   // reverse hash due to the different from_hex algo
   std::reverse(outpoint.hash.data(), outpoint.hash.data() + outpoint.hash.data_size());
   outpoint.n = 1;
   btc_in input;
   input.prevout = outpoint;
   input.nSequence = 0xffffffff;
   btc_out output;
   output.nValue = 19000;
   output.scriptPubKey = lock_script_for_redeem_script(redeem_new);
   btc_tx tx;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   tx.hasWitness = false;
   tx.vin.push_back(input);
   tx.vout.push_back(output);
   bytes unsigned_tx;
   tx.to_bytes(unsigned_tx);
   ilog(fc::to_hex(reinterpret_cast<char*>(&unsigned_tx[0]), unsigned_tx.size()));
   std::vector<uint64_t> in_amounts({20000});
   std::vector<fc::optional<fc::ecc::private_key>> keys_to_sign;
   for(auto key: priv_old)
      keys_to_sign.push_back(fc::optional<fc::ecc::private_key>(key));
   bytes signed_tx =sign_pw_transfer_transaction(unsigned_tx, in_amounts, redeem_old, keys_to_sign);
   ilog(fc::to_hex(reinterpret_cast<char*>(&signed_tx[0]), signed_tx.size()));
}
