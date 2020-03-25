#include <boost/test/unit_test.hpp>
#include <graphene/peerplays_sidechain/bitcoin_utils.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/base58.hpp>
#include <secp256k1.h>

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

BOOST_AUTO_TEST_CASE(tx_data_serialization)
{
   bytes source_tx({0, 1, 2, 3, 4, 5});
   std::vector<uint64_t> source_ins({6, 7, 8, 9, 10});
   std::string buf = save_tx_data_to_string(source_tx, source_ins);
   bytes destination_tx;
   std::vector<uint64_t> destination_ins;
   read_tx_data_from_string(buf, destination_tx, destination_ins);
   BOOST_REQUIRE(source_tx == destination_tx);
   BOOST_REQUIRE(source_ins == destination_ins);
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
   // print old keys
   for(auto key: priv_old)
   {
      fc::sha256 secret = key.get_secret();
      bytes data({239});
      data.insert(data.end(), secret.data(), secret.data() + secret.data_size());
      fc::sha256 cs = fc::sha256::hash(fc::sha256::hash((char*)&data[0], data.size()));
      data.insert(data.end(), cs.data(), cs.data() + 4);
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], i + 1));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);

   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::testnet);
   // This address was filled with testnet transaction 508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766
   BOOST_REQUIRE(old_pw == "tb1qfhstznulf5cmjzahlkmnuuvs0tkjtwjlme3ugz8jzfjanf8h5rwsp45t7e");

   bytes scriptPubKey = lock_script_for_redeem_script(redeem_old);

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
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_new;
   for(unsigned i = 0; i < 15; ++i)
      weights_new.push_back(std::make_pair(pub_new[i], 16 - i));
   // redeem script for new PW
   bytes redeem_new =generate_redeem_script(weights_new);
   // New PW address
   std::string new_pw = p2wsh_address_from_redeem_script(redeem_new, bitcoin_network::testnet);
   BOOST_REQUIRE(new_pw == "tb1qzegrz8r8z8ddfkql8595d90czng6eyjmx4ur73ls4pq57jg99qhsh9fd2y");

   // try to move funds from old wallet to new one

   // get unspent outputs for old wallet with list_uspent (address should be
   // added to wallet with import_address before). It should return
   // 1 UTXO: [508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766:0]
   // with 20000 satoshis
   // So, we creating a raw transaction with 1 input and one output that gets
   // 20000 - fee satoshis with createrawtransaction call (bitcoin_rpc_client::prepare_tx)
   // Here we just serialize the transaction without scriptSig in inputs then sign it.
   btc_outpoint outpoint;
   outpoint.hash = fc::uint256("508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766");
   // reverse hash due to the different from_hex algo
   std::reverse(outpoint.hash.data(), outpoint.hash.data() + outpoint.hash.data_size());
   outpoint.n = 0;
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
   std::vector<uint64_t> in_amounts({20000});
   std::vector<fc::optional<fc::ecc::private_key>> keys_to_sign;
   for(auto key: priv_old)
      keys_to_sign.push_back(fc::optional<fc::ecc::private_key>(key));
   bytes signed_tx =sign_pw_transfer_transaction(unsigned_tx, in_amounts, redeem_old, keys_to_sign);
   // this is real testnet tx with id 1734a2f6192c3953c90f9fd7f69eba16eeb0922207f81f3af32d6534a6f8e850
   BOOST_CHECK(fc::to_hex((char*)&signed_tx[0], signed_tx.size()) == "020000000001016617ba8fec01d942ef23dfa26c99badceb682050c5e67ec5b76de65dd6368a500000000000ffffffff01384a0000000000002200201650311c6711dad4d81f3d0b4695f814d1ac925b35783f47f0a8414f4905282f10473044022028cf6df7ed5c2761d7aa2af20717c8b5ace168a7800d6a566f2c1ae28160cae502205e01a3d91f5b9870577e36fbc26ce0cecc3e628cc376c7016364ec3f370703140147304402205c9a88cbe41eb9c6a16ba1d747456222cbe951d04739d21309ef0c0cf00727f202202d06db830ee5823882c7b6f82b708111a8f37741878896cd3558fb91efe8076401473044022009c3184fc0385eb7ed8dc0374791cbdace0eff0dc27dd80ac68f8cb81110f700022042267e8a8788c314347234ea10db6c1ec21a2d423b784cbfbaadf3b2393c44630147304402202363ce306570dc0bbf6d18d41b67c6488a014a91d8e24c03670b4f65523aca12022029d04c114b8e93d982cadee89d80bb25c5c8bc437d6cd2bfce8e0d83a08d14410148304502210087b4742e5cf9c77ca9f99928e7c7087e7d786e09216485628509e4e0b2f29d7e02207daf2eaee9fe8bf117074be137b7ae4b8503a4f6d263424e8e6a16405d5b723c0147304402204f1c3ed8cf595bfaf79d90f4c55c04c17bb6d446e3b9beca7ee6ee7895c6b752022022ac032f219a81b2845d0a1abfb904e40036a3ad332e7dfada6fda21ef7080b501483045022100d020eca4ba1aa77de9caf98f3a29f74f55268276860b9fa35fa16cfc00219dd8022028237de6ad063116cf8182d2dd45a09cb90c2ec8104d793eb3635a1290027cd6014730440220322193b0feba7356651465b86463c7619cd3d96729df6242e9571c74ff1c3c2902206e1de8e77b71c7b6031a934b52321134b6a8d138e2124e90f6345decbd543efb01483045022100d70ade49b3f17812785a41711e107b27c3d4981f8e12253629c07ec46ee511af02203e1ea9059ed9165eeff827002c7399a30c478a9b6f2b958621bfbc6713ab4dd30147304402206f7f10d9993c7019360276bbe790ab587adadeab08088593a9a0c56524aca4df02207c147fe2e51484801a4e059e611e7514729d685a5df892dcf02ba59d455e678101483045022100d5071b8039364bfaa53ef5e22206f773539b082f28bd1fbaaea995fa28aae0f5022056edf7a7bdd8a9a54273a667be5bcd11191fc871798fb44f6e1e35c95d86a81201483045022100a39f8ffbcd9c3f0591fc731a9856c8e024041017cba20c9935f13e4abcf9e9dc0220786823b8cd55664ff9ad6277899aacfd56fa8e48c38881482418b7d50ca27211014730440220361d3b87fcc2b1c12a9e7c684c78192ccb7fe51b90c281b7058384b0b036927a0220434c9b403ee3802b4e5b53feb9bb37d2a9d8746c3688da993549dd9d9954c6800147304402206dc4c3a4407fe9cbffb724928aa0597148c14a20d0d7fbb36ad5d3e2a3abf85e022039ef7baebbf08494495a038b009c6d4ff4b91c38db840673b87f6c27c3b53e7e01483045022100cadac495ea78d0ce9678a4334b8c43f7fafeea5a59413cc2a0144addb63485f9022078ca133e020e3afd0e79936337afefc21d84d3839f5a225a0f3d3eebc15f959901fd5c02007c21030e88484f2bb5dcfc0b326e9eb565c27c8291efb064d060d226916857a2676e62ac635193687c2102151ad794a3aeb3cf9c190120da3d13d36cd8bdf21ca1ccb15debd61c601314b0ac635293687c2103b45a5955ea7847d121225c752edaeb4a5d731a056a951a876caaf6d1f69adb7dac635393687c2102def03a6ffade4ffb0017c8d93859a247badd60e2d76d00e2a3713f6621932ec1ac635493687c21035f17aa7d58b8c3ee0d87240fded52b27f3f12768a0a54ba2595e0a929dd87155ac635593687c2103c8582ac6b0bd20cc1b02c6a86bad2ea10cadb758fedd754ba0d97be85b63b5a7ac635693687c21028148a1f9669fc4471e76f7a371d7cc0563b26e0821d9633fd37649744ff54edaac635793687c2102f0313701b0035f0365a59ce1a3d7ae7045e1f2fb25c4656c08071e5baf51483dac635893687c21024c4c25d08173b3c4d4e1375f8107fd7040c2dc0691ae1bf6fe82b8c88a85185fac635993687c210360fe2daa8661a3d25d0df79875d70b1c3d443ade731caafda7488cb68b4071b0ac635a93687c210250e41a6a4abd7b0b3a49eaec24a6fafa99e5aa7b1e3a5aabe60664276df3d937ac635b93687c2103045a32125930ca103c7d7c79b6f379754796cd4ea7fb0059da926e415e3877d3ac635c93687c210344943249d7ca9b47316fef0c2a413dda3a75416a449a29f310ab7fc9d052ed70ac635d93687c2103c62967320b63df5136ff1ef4c7959ef5917ee5a44f75c83e870bc488143d4d69ac635e93687c21020429f776e15770e4dc52bd6f72e6ed6908d51de1c4a64878433c4e3860a48dc4ac635f93680150a000000000");
}

BOOST_AUTO_TEST_CASE(pw_separate_sign)
{
   // key set for the old Primary Wallet
   std::vector<fc::ecc::private_key> priv_old;
   for(unsigned i = 0; i < 15; ++i)
   {
      const char* seed = reinterpret_cast<const char*>(&i);
      fc::sha256 h = fc::sha256::hash(seed, sizeof(i));
      priv_old.push_back(fc::ecc::private_key::generate_from_seed(h));
   }
   // print old keys
   for(auto key: priv_old)
   {
      fc::sha256 secret = key.get_secret();
      bytes data({239});
      data.insert(data.end(), secret.data(), secret.data() + secret.data_size());
      fc::sha256 cs = fc::sha256::hash(fc::sha256::hash((char*)&data[0], data.size()));
      data.insert(data.end(), cs.data(), cs.data() + 4);
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], i + 1));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);

   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::testnet);
   // This address was filled with testnet transaction 508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766
   BOOST_REQUIRE(old_pw == "tb1qfhstznulf5cmjzahlkmnuuvs0tkjtwjlme3ugz8jzfjanf8h5rwsp45t7e");

   bytes scriptPubKey = lock_script_for_redeem_script(redeem_old);

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
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_new;
   for(unsigned i = 0; i < 15; ++i)
      weights_new.push_back(std::make_pair(pub_new[i], 16 - i));
   // redeem script for new PW
   bytes redeem_new =generate_redeem_script(weights_new);
   // New PW address
   std::string new_pw = p2wsh_address_from_redeem_script(redeem_new, bitcoin_network::testnet);
   BOOST_REQUIRE(new_pw == "tb1qzegrz8r8z8ddfkql8595d90czng6eyjmx4ur73ls4pq57jg99qhsh9fd2y");

   // try to move funds from old wallet to new one

   // get unspent outputs for old wallet with list_uspent (address should be
   // added to wallet with import_address before). It should return
   // 1 UTXO: [508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766:0]
   // with 20000 satoshis
   // So, we creating a raw transaction with 1 input and one output that gets
   // 20000 - fee satoshis with createrawtransaction call (bitcoin_rpc_client::prepare_tx)
   // Here we just serialize the transaction without scriptSig in inputs then sign it.
   btc_outpoint outpoint;
   outpoint.hash = fc::uint256("508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766");
   // reverse hash due to the different from_hex algo
   std::reverse(outpoint.hash.data(), outpoint.hash.data() + outpoint.hash.data_size());
   outpoint.n = 0;
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
   std::vector<uint64_t> in_amounts({20000});

   // prepare tx with dummy signs
   bytes partially_signed_tx = add_dummy_signatures_for_pw_transfer(unsigned_tx, redeem_old, 15);

   // sign with every old key one by one
   for(unsigned idx = 0; idx < 15; idx++)
      partially_signed_tx = partially_sign_pw_transfer_transaction(partially_signed_tx, in_amounts, priv_old[idx], idx);

   // now this is real testnet tx with id 1734a2f6192c3953c90f9fd7f69eba16eeb0922207f81f3af32d6534a6f8e850
   BOOST_CHECK(fc::to_hex((char*)&partially_signed_tx[0], partially_signed_tx.size()) == "020000000001016617ba8fec01d942ef23dfa26c99badceb682050c5e67ec5b76de65dd6368a500000000000ffffffff01384a0000000000002200201650311c6711dad4d81f3d0b4695f814d1ac925b35783f47f0a8414f4905282f10473044022028cf6df7ed5c2761d7aa2af20717c8b5ace168a7800d6a566f2c1ae28160cae502205e01a3d91f5b9870577e36fbc26ce0cecc3e628cc376c7016364ec3f370703140147304402205c9a88cbe41eb9c6a16ba1d747456222cbe951d04739d21309ef0c0cf00727f202202d06db830ee5823882c7b6f82b708111a8f37741878896cd3558fb91efe8076401473044022009c3184fc0385eb7ed8dc0374791cbdace0eff0dc27dd80ac68f8cb81110f700022042267e8a8788c314347234ea10db6c1ec21a2d423b784cbfbaadf3b2393c44630147304402202363ce306570dc0bbf6d18d41b67c6488a014a91d8e24c03670b4f65523aca12022029d04c114b8e93d982cadee89d80bb25c5c8bc437d6cd2bfce8e0d83a08d14410148304502210087b4742e5cf9c77ca9f99928e7c7087e7d786e09216485628509e4e0b2f29d7e02207daf2eaee9fe8bf117074be137b7ae4b8503a4f6d263424e8e6a16405d5b723c0147304402204f1c3ed8cf595bfaf79d90f4c55c04c17bb6d446e3b9beca7ee6ee7895c6b752022022ac032f219a81b2845d0a1abfb904e40036a3ad332e7dfada6fda21ef7080b501483045022100d020eca4ba1aa77de9caf98f3a29f74f55268276860b9fa35fa16cfc00219dd8022028237de6ad063116cf8182d2dd45a09cb90c2ec8104d793eb3635a1290027cd6014730440220322193b0feba7356651465b86463c7619cd3d96729df6242e9571c74ff1c3c2902206e1de8e77b71c7b6031a934b52321134b6a8d138e2124e90f6345decbd543efb01483045022100d70ade49b3f17812785a41711e107b27c3d4981f8e12253629c07ec46ee511af02203e1ea9059ed9165eeff827002c7399a30c478a9b6f2b958621bfbc6713ab4dd30147304402206f7f10d9993c7019360276bbe790ab587adadeab08088593a9a0c56524aca4df02207c147fe2e51484801a4e059e611e7514729d685a5df892dcf02ba59d455e678101483045022100d5071b8039364bfaa53ef5e22206f773539b082f28bd1fbaaea995fa28aae0f5022056edf7a7bdd8a9a54273a667be5bcd11191fc871798fb44f6e1e35c95d86a81201483045022100a39f8ffbcd9c3f0591fc731a9856c8e024041017cba20c9935f13e4abcf9e9dc0220786823b8cd55664ff9ad6277899aacfd56fa8e48c38881482418b7d50ca27211014730440220361d3b87fcc2b1c12a9e7c684c78192ccb7fe51b90c281b7058384b0b036927a0220434c9b403ee3802b4e5b53feb9bb37d2a9d8746c3688da993549dd9d9954c6800147304402206dc4c3a4407fe9cbffb724928aa0597148c14a20d0d7fbb36ad5d3e2a3abf85e022039ef7baebbf08494495a038b009c6d4ff4b91c38db840673b87f6c27c3b53e7e01483045022100cadac495ea78d0ce9678a4334b8c43f7fafeea5a59413cc2a0144addb63485f9022078ca133e020e3afd0e79936337afefc21d84d3839f5a225a0f3d3eebc15f959901fd5c02007c21030e88484f2bb5dcfc0b326e9eb565c27c8291efb064d060d226916857a2676e62ac635193687c2102151ad794a3aeb3cf9c190120da3d13d36cd8bdf21ca1ccb15debd61c601314b0ac635293687c2103b45a5955ea7847d121225c752edaeb4a5d731a056a951a876caaf6d1f69adb7dac635393687c2102def03a6ffade4ffb0017c8d93859a247badd60e2d76d00e2a3713f6621932ec1ac635493687c21035f17aa7d58b8c3ee0d87240fded52b27f3f12768a0a54ba2595e0a929dd87155ac635593687c2103c8582ac6b0bd20cc1b02c6a86bad2ea10cadb758fedd754ba0d97be85b63b5a7ac635693687c21028148a1f9669fc4471e76f7a371d7cc0563b26e0821d9633fd37649744ff54edaac635793687c2102f0313701b0035f0365a59ce1a3d7ae7045e1f2fb25c4656c08071e5baf51483dac635893687c21024c4c25d08173b3c4d4e1375f8107fd7040c2dc0691ae1bf6fe82b8c88a85185fac635993687c210360fe2daa8661a3d25d0df79875d70b1c3d443ade731caafda7488cb68b4071b0ac635a93687c210250e41a6a4abd7b0b3a49eaec24a6fafa99e5aa7b1e3a5aabe60664276df3d937ac635b93687c2103045a32125930ca103c7d7c79b6f379754796cd4ea7fb0059da926e415e3877d3ac635c93687c210344943249d7ca9b47316fef0c2a413dda3a75416a449a29f310ab7fc9d052ed70ac635d93687c2103c62967320b63df5136ff1ef4c7959ef5917ee5a44f75c83e870bc488143d4d69ac635e93687c21020429f776e15770e4dc52bd6f72e6ed6908d51de1c4a64878433c4e3860a48dc4ac635f93680150a000000000");
}

BOOST_AUTO_TEST_CASE(pw_separate_sign2)
{
   // key set for the old Primary Wallet
   std::vector<fc::ecc::private_key> priv_old;
   for(unsigned i = 0; i < 15; ++i)
   {
      const char* seed = reinterpret_cast<const char*>(&i);
      fc::sha256 h = fc::sha256::hash(seed, sizeof(i));
      priv_old.push_back(fc::ecc::private_key::generate_from_seed(h));
   }
   // print old keys
   for(auto key: priv_old)
   {
      fc::sha256 secret = key.get_secret();
      bytes data({239});
      data.insert(data.end(), secret.data(), secret.data() + secret.data_size());
      fc::sha256 cs = fc::sha256::hash(fc::sha256::hash((char*)&data[0], data.size()));
      data.insert(data.end(), cs.data(), cs.data() + 4);
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], i + 1));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);

   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::testnet);
   // This address was filled with testnet transaction 508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766
   BOOST_REQUIRE(old_pw == "tb1qfhstznulf5cmjzahlkmnuuvs0tkjtwjlme3ugz8jzfjanf8h5rwsp45t7e");

   bytes scriptPubKey = lock_script_for_redeem_script(redeem_old);

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
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_new;
   for(unsigned i = 0; i < 15; ++i)
      weights_new.push_back(std::make_pair(pub_new[i], 16 - i));
   // redeem script for new PW
   bytes redeem_new =generate_redeem_script(weights_new);
   // New PW address
   std::string new_pw = p2wsh_address_from_redeem_script(redeem_new, bitcoin_network::testnet);
   BOOST_REQUIRE(new_pw == "tb1qzegrz8r8z8ddfkql8595d90czng6eyjmx4ur73ls4pq57jg99qhsh9fd2y");

   // try to move funds from old wallet to new one

   // get unspent outputs for old wallet with list_uspent (address should be
   // added to wallet with import_address before). It should return
   // 1 UTXO: [508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766:0]
   // with 20000 satoshis
   // So, we creating a raw transaction with 1 input and one output that gets
   // 20000 - fee satoshis with createrawtransaction call (bitcoin_rpc_client::prepare_tx)
   // Here we just serialize the transaction without scriptSig in inputs then sign it.
   btc_outpoint outpoint;
   outpoint.hash = fc::uint256("508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766");
   // reverse hash due to the different from_hex algo
   std::reverse(outpoint.hash.data(), outpoint.hash.data() + outpoint.hash.data_size());
   outpoint.n = 0;
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
   std::vector<uint64_t> in_amounts({20000});

   // gather all signatures from all SONs separatelly
   std::vector<std::vector<bytes> > signature_set;
   for(auto key: priv_old)
   {
      std::vector<bytes> signatures = signatures_for_raw_transaction(unsigned_tx, in_amounts, redeem_old, key);
      signature_set.push_back(signatures);
   }

   // create signed tx with all signatures
   bytes signed_tx = add_signatures_to_unsigned_tx(unsigned_tx, signature_set, redeem_old);

   // now this is real testnet tx with id 1734a2f6192c3953c90f9fd7f69eba16eeb0922207f81f3af32d6534a6f8e850
   BOOST_CHECK(fc::to_hex((char*)&signed_tx[0], signed_tx.size()) == "020000000001016617ba8fec01d942ef23dfa26c99badceb682050c5e67ec5b76de65dd6368a500000000000ffffffff01384a0000000000002200201650311c6711dad4d81f3d0b4695f814d1ac925b35783f47f0a8414f4905282f10473044022028cf6df7ed5c2761d7aa2af20717c8b5ace168a7800d6a566f2c1ae28160cae502205e01a3d91f5b9870577e36fbc26ce0cecc3e628cc376c7016364ec3f370703140147304402205c9a88cbe41eb9c6a16ba1d747456222cbe951d04739d21309ef0c0cf00727f202202d06db830ee5823882c7b6f82b708111a8f37741878896cd3558fb91efe8076401473044022009c3184fc0385eb7ed8dc0374791cbdace0eff0dc27dd80ac68f8cb81110f700022042267e8a8788c314347234ea10db6c1ec21a2d423b784cbfbaadf3b2393c44630147304402202363ce306570dc0bbf6d18d41b67c6488a014a91d8e24c03670b4f65523aca12022029d04c114b8e93d982cadee89d80bb25c5c8bc437d6cd2bfce8e0d83a08d14410148304502210087b4742e5cf9c77ca9f99928e7c7087e7d786e09216485628509e4e0b2f29d7e02207daf2eaee9fe8bf117074be137b7ae4b8503a4f6d263424e8e6a16405d5b723c0147304402204f1c3ed8cf595bfaf79d90f4c55c04c17bb6d446e3b9beca7ee6ee7895c6b752022022ac032f219a81b2845d0a1abfb904e40036a3ad332e7dfada6fda21ef7080b501483045022100d020eca4ba1aa77de9caf98f3a29f74f55268276860b9fa35fa16cfc00219dd8022028237de6ad063116cf8182d2dd45a09cb90c2ec8104d793eb3635a1290027cd6014730440220322193b0feba7356651465b86463c7619cd3d96729df6242e9571c74ff1c3c2902206e1de8e77b71c7b6031a934b52321134b6a8d138e2124e90f6345decbd543efb01483045022100d70ade49b3f17812785a41711e107b27c3d4981f8e12253629c07ec46ee511af02203e1ea9059ed9165eeff827002c7399a30c478a9b6f2b958621bfbc6713ab4dd30147304402206f7f10d9993c7019360276bbe790ab587adadeab08088593a9a0c56524aca4df02207c147fe2e51484801a4e059e611e7514729d685a5df892dcf02ba59d455e678101483045022100d5071b8039364bfaa53ef5e22206f773539b082f28bd1fbaaea995fa28aae0f5022056edf7a7bdd8a9a54273a667be5bcd11191fc871798fb44f6e1e35c95d86a81201483045022100a39f8ffbcd9c3f0591fc731a9856c8e024041017cba20c9935f13e4abcf9e9dc0220786823b8cd55664ff9ad6277899aacfd56fa8e48c38881482418b7d50ca27211014730440220361d3b87fcc2b1c12a9e7c684c78192ccb7fe51b90c281b7058384b0b036927a0220434c9b403ee3802b4e5b53feb9bb37d2a9d8746c3688da993549dd9d9954c6800147304402206dc4c3a4407fe9cbffb724928aa0597148c14a20d0d7fbb36ad5d3e2a3abf85e022039ef7baebbf08494495a038b009c6d4ff4b91c38db840673b87f6c27c3b53e7e01483045022100cadac495ea78d0ce9678a4334b8c43f7fafeea5a59413cc2a0144addb63485f9022078ca133e020e3afd0e79936337afefc21d84d3839f5a225a0f3d3eebc15f959901fd5c02007c21030e88484f2bb5dcfc0b326e9eb565c27c8291efb064d060d226916857a2676e62ac635193687c2102151ad794a3aeb3cf9c190120da3d13d36cd8bdf21ca1ccb15debd61c601314b0ac635293687c2103b45a5955ea7847d121225c752edaeb4a5d731a056a951a876caaf6d1f69adb7dac635393687c2102def03a6ffade4ffb0017c8d93859a247badd60e2d76d00e2a3713f6621932ec1ac635493687c21035f17aa7d58b8c3ee0d87240fded52b27f3f12768a0a54ba2595e0a929dd87155ac635593687c2103c8582ac6b0bd20cc1b02c6a86bad2ea10cadb758fedd754ba0d97be85b63b5a7ac635693687c21028148a1f9669fc4471e76f7a371d7cc0563b26e0821d9633fd37649744ff54edaac635793687c2102f0313701b0035f0365a59ce1a3d7ae7045e1f2fb25c4656c08071e5baf51483dac635893687c21024c4c25d08173b3c4d4e1375f8107fd7040c2dc0691ae1bf6fe82b8c88a85185fac635993687c210360fe2daa8661a3d25d0df79875d70b1c3d443ade731caafda7488cb68b4071b0ac635a93687c210250e41a6a4abd7b0b3a49eaec24a6fafa99e5aa7b1e3a5aabe60664276df3d937ac635b93687c2103045a32125930ca103c7d7c79b6f379754796cd4ea7fb0059da926e415e3877d3ac635c93687c210344943249d7ca9b47316fef0c2a413dda3a75416a449a29f310ab7fc9d052ed70ac635d93687c2103c62967320b63df5136ff1ef4c7959ef5917ee5a44f75c83e870bc488143d4d69ac635e93687c21020429f776e15770e4dc52bd6f72e6ed6908d51de1c4a64878433c4e3860a48dc4ac635f93680150a000000000");
}

BOOST_AUTO_TEST_CASE(pw_partially_sign)
{
   // key set for the old Primary Wallet
   std::vector<fc::ecc::private_key> priv_old;
   for(unsigned i = 0; i < 15; ++i)
   {
      const char* seed = reinterpret_cast<const char*>(&i);
      fc::sha256 h = fc::sha256::hash(seed, sizeof(i));
      priv_old.push_back(fc::ecc::private_key::generate_from_seed(h));
   }
   // print old keys
   for(auto key: priv_old)
   {
      fc::sha256 secret = key.get_secret();
      bytes data({239});
      data.insert(data.end(), secret.data(), secret.data() + secret.data_size());
      fc::sha256 cs = fc::sha256::hash(fc::sha256::hash((char*)&data[0], data.size()));
      data.insert(data.end(), cs.data(), cs.data() + 4);
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], i + 1));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);

   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::testnet);
   // This address was filled with testnet transaction 508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766
   BOOST_REQUIRE(old_pw == "tb1qfhstznulf5cmjzahlkmnuuvs0tkjtwjlme3ugz8jzfjanf8h5rwsp45t7e");

   bytes scriptPubKey = lock_script_for_redeem_script(redeem_old);

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
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_new;
   for(unsigned i = 0; i < 15; ++i)
      weights_new.push_back(std::make_pair(pub_new[i], 16 - i));
   // redeem script for new PW
   bytes redeem_new =generate_redeem_script(weights_new);
   // New PW address
   std::string new_pw = p2wsh_address_from_redeem_script(redeem_new, bitcoin_network::testnet);
   BOOST_REQUIRE(new_pw == "tb1qzegrz8r8z8ddfkql8595d90czng6eyjmx4ur73ls4pq57jg99qhsh9fd2y");

   // try to move funds from old wallet to new one

   // Spent 1 UTXO: [7007b77fcd5fe097d02679252aa112900d08ab20c06052f4148265b21b1f9fbf:0]
   // with 29999 satoshis
   // So, we creating a raw transaction with 1 input and one output that gets
   // 29999 - fee satoshis with createrawtransaction call (bitcoin_rpc_client::prepare_tx)
   btc_outpoint outpoint;
   outpoint.hash = fc::uint256("7007b77fcd5fe097d02679252aa112900d08ab20c06052f4148265b21b1f9fbf");
   // reverse hash due to the different from_hex algo
   std::reverse(outpoint.hash.data(), outpoint.hash.data() + outpoint.hash.data_size());
   outpoint.n = 0;
   btc_in input;
   input.prevout = outpoint;
   input.nSequence = 0xffffffff;
   btc_out output;
   output.nValue = 29000;
   output.scriptPubKey = lock_script_for_redeem_script(redeem_new);
   btc_tx tx;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   tx.hasWitness = false;
   tx.vin.push_back(input);
   tx.vout.push_back(output);
   bytes unsigned_tx;
   tx.to_bytes(unsigned_tx);
   std::vector<uint64_t> in_amounts({29999});

   // prepare tx with dummy signs
   bytes partially_signed_tx = add_dummy_signatures_for_pw_transfer(unsigned_tx, redeem_old, 15);

   // sign with every old key one by one except the first one
   for(unsigned idx = 1; idx < 15; idx++)
      partially_signed_tx = partially_sign_pw_transfer_transaction(partially_signed_tx, in_amounts, priv_old[idx], idx);

   // now this is real testnet tx with id e86455c40da6993b6fed70daea2046287b206ab5c16e1ab58c4dfb4a7d6efb84
   BOOST_CHECK(fc::to_hex((char*)&partially_signed_tx[0], partially_signed_tx.size()) == "02000000000101bf9f1f1bb2658214f45260c020ab080d9012a12a257926d097e05fcd7fb707700000000000ffffffff0148710000000000002200201650311c6711dad4d81f3d0b4695f814d1ac925b35783f47f0a8414f4905282f10483045022100c4c567419754c5c1768e959a35633012e8d22ccc90d7cd1b88d6d430a513fbbd0220729c2a3520d0cae7dd6dcd928624ffa3e0b6ce0c4f5c340653a6c18549182588014830450221008c868ea2cdf5b23bdf9e6c7d7c283b8424aeb4aec43621424baef1ee77dd399a02205431f608006f0f0dcd392fab4f25328808b45d4a73852a197e947b289faefece01483045022100aecac85bbb81bc0a4e127c15090c5ab82a62b9e27a9a6eb8eddf8de294aa9d920220482f2ba8d7b62e9f3f7a68b0ef3236bc56e44481d3eb59f62d1daf4b191dc86001483045022100eb27943f8b511a36b1a843f9b3ddf6930aece5a3c0be697dbafc921924fc049c022065ba3e1e4ad57f56337143136c5d3ee3f56dd60f36e798f07b5646e29343d7320147304402206e24158484ebb2cd14b9c410ecd04841d806d8464ce9a827533484c8ad8d921b022021baec9cd0ad46e7b19c8de7df286093b835df5c6243e90b14f5748dc1b7c13901473044022067bfaf0e39d72e49a081d4e43828746ab7524c4764e445173dd96cc7e6187d46022063ef107375cc45d1c26b1e1c87b97694f71645187ad871db9c05b8e981a0da8601483045022100da0162de3e4a5268b616b9d01a1a4f64b0c371c6b44fb1f740a264455f2bc20d02203a0b45a98a341722ad65ae4ad68538d617b1cfbb229751f875615317eaf15dd4014830450221008220c4f97585e67966d4435ad8497eb89945f13dd8ff24048b830582349041a002204cb03f7271895637a31ce6479d15672c2d70528148e3cd6196e6f722117745c50147304402203e83ab4b15bb0680f82779335acf9a3ce45316150a4538d5e3d25cb863fcec5702204b3913874077ed2cae4e10f8786053b6f157973a54d156d5863f13accca595f50147304402201420d2a2830278ffff5842ecb7173a23642f179435443e780b3d1fe04be5c32e02203818202390e0e63b4309b89f9cce08c0f4dfa539c2ed59b05e24325671e2747c0147304402205624ca9d47ae04afd8fff705706d6853f8c679abb385f19e01c36f9380a0bad602203dc817fc55497e4c1759a3dbfff1662faca593a9f10d3a9b3e24d5ee3165d4400147304402203a959f9a34587c56b86826e6ab65644ab19cbd09ca078459eb59956b02bc753002206df5ded568d0e3e3645f8cb8ca02874dd1bfa82933eb5e01ff2e5a773633e51601483045022100a84ed5be60b9e095d40f3f6bd698425cb9c4d8f95e8b43ca6c5120a6c599e9eb022064c703952d18d753f9198d78188a26888e6b06c832d93f8075311d57a13240160147304402202e71d3af33a18397b90072098881fdbdb8d6e4ffa34d21141212dd815c97d00f02207195f1c06a8f44ca72af15fdaba89b07cf6daef9be981c432b9f5c10f1e374200100fd5c02007c21030e88484f2bb5dcfc0b326e9eb565c27c8291efb064d060d226916857a2676e62ac635193687c2102151ad794a3aeb3cf9c190120da3d13d36cd8bdf21ca1ccb15debd61c601314b0ac635293687c2103b45a5955ea7847d121225c752edaeb4a5d731a056a951a876caaf6d1f69adb7dac635393687c2102def03a6ffade4ffb0017c8d93859a247badd60e2d76d00e2a3713f6621932ec1ac635493687c21035f17aa7d58b8c3ee0d87240fded52b27f3f12768a0a54ba2595e0a929dd87155ac635593687c2103c8582ac6b0bd20cc1b02c6a86bad2ea10cadb758fedd754ba0d97be85b63b5a7ac635693687c21028148a1f9669fc4471e76f7a371d7cc0563b26e0821d9633fd37649744ff54edaac635793687c2102f0313701b0035f0365a59ce1a3d7ae7045e1f2fb25c4656c08071e5baf51483dac635893687c21024c4c25d08173b3c4d4e1375f8107fd7040c2dc0691ae1bf6fe82b8c88a85185fac635993687c210360fe2daa8661a3d25d0df79875d70b1c3d443ade731caafda7488cb68b4071b0ac635a93687c210250e41a6a4abd7b0b3a49eaec24a6fafa99e5aa7b1e3a5aabe60664276df3d937ac635b93687c2103045a32125930ca103c7d7c79b6f379754796cd4ea7fb0059da926e415e3877d3ac635c93687c210344943249d7ca9b47316fef0c2a413dda3a75416a449a29f310ab7fc9d052ed70ac635d93687c2103c62967320b63df5136ff1ef4c7959ef5917ee5a44f75c83e870bc488143d4d69ac635e93687c21020429f776e15770e4dc52bd6f72e6ed6908d51de1c4a64878433c4e3860a48dc4ac635f93680150a000000000");
}

BOOST_AUTO_TEST_CASE(pw_partially_sign_regtest)
{
   // key set for the old Primary Wallet
   std::vector<std::string> priv_old_str;
   priv_old_str.push_back("cSKyTeXidmj93dgbMFqgzD7yvxzA7QAYr5j9qDnY9seyhyv7gH2m");
   priv_old_str.push_back("cQBBNyEw6P3pgc2NjPpKR2YoCpio9s3qEMkFkY7v9hByLAxeLQ3s");
   priv_old_str.push_back("cQLKc4UgKyCdXY3PosndszEZTsB6mTrg4avZF6kDphrULKd2W6L4");
   priv_old_str.push_back("cN43k9sqQimgzChZm9Qz1V1bdkjVwB3mcSHsEuj6bfUa4SP2AsTk");
   priv_old_str.push_back("cNBmovk8MnGmQBs7scJ3p1fRftdV1EyXMuUsX5DfmWg7wtmUMFBe");
   priv_old_str.push_back("cPQNr8RFGHEZVv6GoTPSTca24vZXzvQSJTUzuwArLzxXdhh7TY3H");
   priv_old_str.push_back("cRNwxg1aozGvACsVWFE7xGtUBq7Fw4it2GQGBb2XLvaVidoCZim4");
   priv_old_str.push_back("cPmakGTVWfsXsT8gSPZp9bcMkUAjSQYgwUtHKqn7cvTjv9vZvBix");
   priv_old_str.push_back("cSafFKRU6YFxS56jcAfdrKoLYvfnwHEdvJ3LzvjJciMGz3dsktpV");
   priv_old_str.push_back("cSDxZhz4ceyBLcbQwEHWU7G4ER7zZ1YkvkWf28MgyEkLznmU4YGh");
   priv_old_str.push_back("cPPiiVPx1C2upFNUFBkJejqC23UdqFHQd997jJxPz5Y3Hsd22LWu");
   priv_old_str.push_back("cRLAbigw6N7TyooJn4j4egoKmdmrJYFEQtsWbYsWMCfZBn6AZLF2");
   priv_old_str.push_back("cVn2ARgPNeCpiNjAJK863DntnmftoAKjUF9Zqsnuuo3Ybv5Y5XrQ");
   priv_old_str.push_back("cNgGKKPAxDXD8SBronD1MSX8df17jTBvcSD1uugepFgstUbRvpy4");
   priv_old_str.push_back("cUjR52VietRnVCmY56TEVRgkkst7RzjZqDoJqkL3tNrs1NkrgjU6");
   priv_old_str.push_back("cVkEokoui82vfPtEWS8pKhLZiWHKbFvdXxsKDen32Uz71zpqLZfc");
   std::vector<fc::ecc::private_key> priv_old;
   for(std::string buf: priv_old_str)
   {
      ilog(buf);
      priv_old.push_back(*graphene::utilities::wif_to_key(buf));
   }
   std::vector<fc::ecc::public_key> pub_old;
   for(auto& key: priv_old)
      pub_old.push_back(key.get_public_key());
   std::vector<std::string> pub_old_str;
   for(auto& pkey:pub_old) {
      fc::ecc::public_key_data kd = pkey.serialize();
      pub_old_str.push_back(fc::to_hex(kd.data, kd.size()));
   }
   unsigned idx = 0;
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "03456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "02d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "0228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "02ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccf");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "0317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7f");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "0266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccf");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "0229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8da");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "03df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "02bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "0287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8");
   ilog("${i}: ${k}", ("i", idx)("k", pub_old_str[idx]));
   BOOST_CHECK(pub_old_str[idx++] == "02053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73");
   // old key weights
   std::vector<std::pair<fc::ecc::public_key, uint64_t> > weights_old;
   for(unsigned i = 0; i < 15; ++i)
      weights_old.push_back(std::make_pair(pub_old[i], 0));
   // redeem script for old PW
   bytes redeem_old =generate_redeem_script(weights_old);
   ilog("Old redeem script: ${rs}", ("rs", fc::to_hex((char*)&redeem_old[0], redeem_old.size())));

   // Old PW address
   std::string old_pw = p2wsh_address_from_redeem_script(redeem_old, bitcoin_network::regtest);
   ilog("Old address: ${a}", ("a", old_pw));
   // This address was filled with regnet transaction 508a36d65de66db7c57ee6c5502068ebdcba996ca2df23ef42d901ec8fba1766
   BOOST_REQUIRE(old_pw == "bcrt1q6a5l7jw8q9kmvcv3yjfyncp0fk69q0vcwslty5e800xd4ugm8s9qgv5n9d");

   bytes scriptPubKey = lock_script_for_redeem_script(redeem_old);

   // New PW address
   std::string new_pw = "bcrt1qndxlam8rcpydx2tzq25dcp0shdl46t3rku83sn4mauzaaj35wlkqnkaf05";

   // try to move funds from old wallet to new one

   btc_tx tx;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   tx.hasWitness = false;
   tx.vin.push_back(btc_in("6c74b80aa14963886e937dbb92d9c2a15bb2df72625a894404f01efc1d0fd211", 0));
   tx.vin.push_back(btc_in("b858735e559b9993102d8bffcd0ee0997bb875f5b96df212b208288bca05de44", 0));
   tx.vin.push_back(btc_in("4663dda957cd12e4fec90753cd0ff2e54b38bb2e0072b83ae2103a89013b0a4e", 0));
   tx.vin.push_back(btc_in("9483280d39d762110247ec36be92807133a886b7e3dda2618a5ccdbcc4f63b6a", 0));
   tx.vin.push_back(btc_in("85fc91688d85df15cec4ccea0f16b12f8aa7fff5be69fff1b5852a5844f9897e", 0));
   tx.vin.push_back(btc_in("8c1bd2917fb03ad1ca11d172a924d992d033cf2241a7527f52d486d96758f785", 1));
   tx.vin.push_back(btc_in("7e7155b1f30aa2578dd62c2b0bc0c640251f5ab8c885caa5793a51572c0718b4", 1));
   tx.vin.push_back(btc_in("471876ffab6df19d2afaabd461985d56ae98b5c4d00b286d493c584b963a7bd5", 0));
   tx.vin.push_back(btc_in("c89e84af06bee814f7ec443ab5e3c513db7e5c0febc66b7c402a64b53cdb7dd8", 1));
   tx.vin.push_back(btc_in("73789c490f7ecfbaa7b70f508658e12bdd29564ba567f5ca7ee639a2bf5434e2", 1));
   tx.vout.push_back(btc_out("bcrt1qndxlam8rcpydx2tzq25dcp0shdl46t3rku83sn4mauzaaj35wlkqnkaf05", 0.10720788 * 100000000.0));
   bytes unsigned_tx;
   tx.to_bytes(unsigned_tx);
   std::vector<uint64_t> in_amounts({uint64_t(0.00030000 * 100000000.0), uint64_t(0.00030000 * 100000000.0), uint64_t(0.00030000 * 100000000.0),
                                     uint64_t(0.10000000 * 100000000.0), uint64_t(0.00230000 * 100000000.0), uint64_t(0.00030000 * 100000000.0),
                                    uint64_t(0.00030000 * 100000000.0), uint64_t(0.00030000 * 100000000.0), uint64_t(0.00230000 * 100000000.0),
                                    uint64_t(0.00230000 * 100000000.0)});

   // prepare tx with dummy signs
   bytes partially_signed_tx = add_dummy_signatures_for_pw_transfer(unsigned_tx, redeem_old, 15);

   // sign with every old key
   for(unsigned idx = 0; idx < 15; idx++)
      partially_signed_tx = partially_sign_pw_transfer_transaction(partially_signed_tx, in_amounts, priv_old[idx], idx);

   // now this is real tx with id
   std::string signed_tx = fc::to_hex((char*)&partially_signed_tx[0], partially_signed_tx.size());
   ilog(signed_tx);
   BOOST_CHECK(signed_tx == "0200000000010a11d20f1dfc1ef00444895a6272dfb25ba1c2d992bb7d936e886349a10ab8746c00000000000000000044de05ca8b2808b212f26db9f575b87b99e00ecdff8b2d1093999b555e7358b80000000000000000004e0a3b01893a10e23ab872002ebb384be5f20fcd5307c9fee412cd57a9dd63460000000000000000006a3bf6c4bccd5c8a61a2dde3b786a833718092be36ec47021162d7390d2883940000000000000000007e89f944582a85b5f1ff69bef5ffa78a2fb1160feaccc4ce15df858d6891fc8500000000000000000085f75867d986d4527f52a74122cf33d092d924a972d111cad13ab07f91d21b8c010000000000000000b418072c57513a79a5ca85c8b85a1f2540c6c00b2b2cd68d57a20af3b155717e010000000000000000d57b3a964b583c496d280bd0c4b598ae565d9861d4abfa2a9df16dabff761847000000000000000000d87ddb3cb5642a407c6bc6eb0f5c7edb13c5e3b53a44ecf714e8be06af849ec8010000000000000000e23454bfa239e67ecaf567a54b5629dd2be15886500fb7a7bacf7e0f499c7873010000000000000000011496a300000000002200209b4dfeece3c048d3296202a8dc05f0bb7f5d2e23b70f184ebbef05deca3477ec1047304402207db155ee38f165763e26b395509860134d374d73eba3a5d31b926f2e09f5f20902203d74b5227ecb13cb2429c545b89e80ade80d2f94e7bd287f4bdedfba7401bd4a014830450221009cf60021ae2aa54ec74f2aec94a477030c02a47d322417761c1687425500705902204d6c79b9befab550945b79fa7b52c44fe09f145da1abfc3f50599e111a0499de0147304402207bc0c0afea50ee3a6820d70e9259e467c36293b9a17c155e2876f8b94106623802203a991532af42a6fca256ceafd305f7f840894335ed67cab3de0f7eefe6f07800014830450221009fcb486493628fbcb1721b7dbcfeacb294f5c37ba340b50f3fd2984f192c5c08022076539ee19184732b1d40b629e652b21e984f59b18051104e312a03e990038da501483045022100c314bb0e20356175be7ecd36cac2f419ca1c7717044d46df4ecbf83160626cd402201bed733766df117a01900f88f5bda4e23d673066fcce235ed9b104689b38a6d901483045022100973f888056e40b00d7078d1a8430cdc1cabdfd953e66f6e50c1dcc95e685ccd102207d0a03af781f3a5f6c206beacf0577ae659a1a29d88c98553d63217749b2cb410147304402206c01b207fb871575e84802563d1bde2be63cd3f8361db61657d73c85d33e2a050220220dd81d19707fa48bc5d617946dacb8d998efbc1f1eb30baaf87de405dcd25401483045022100e345b6cadb4c4acb290efd54221dd69b90558e714402018846e3fd1e40a1c2a902205581c05ad4a53bd4326b817b6b34afe7a1170e5072ff6210fdac7c6262711f3b01473044022018d271533719f51cffec8fdf16ebb3ebf23074c651b4016de5fd5cf86623313002205866ef7d2c0449d4d3fe2634e325437e9eb58df0feb37a06b2455a15b4a5417a01483045022100eb8d60ee246df99ebdab17a79730bffb913fe393e2f3952fcd15620cba9877da02206bd6c4ceed846192803dc92189c3c28bfb47b8ec42fdab79dad35e996990122601483045022100a143bce0b95c7ab2cd05484cf9902c9c975ec5592212d426768eceabcf88a37902206f71f6a87bc098512097d7cd17440c41452060b10e7775c0ab407d23b0288a1d01483045022100940952f39bcaef581949b223c35ccb5deb92c0e1e7e56f9a591d6f3a3931acbe02200e46d20f9f3a4e3d142b72ca13d6054268b5abc00aaece7f38feff9984067acc01483045022100e98833b64cae0f268edf426963186c86ff0f3a918da14a05ab3b31971e9ad8780220559d0f5d3efb0fc4853a5f139b445270c4db24bda41cb3642efbdbec15727f6301483045022100c7eb0b8e664860671da8af9de525d0ef09d6f9dc0dd39ce9c67355dda26ed38902200f6e9cc86d780645af6a0d3abe3dab73ef4345a7f23b6c03badda53a3bb47eb201483045022100a919596f79ac10364ede00b2218ec53502d910b401a142947076e93507bf637102202dc10960ed637447d64ab9b9229f6e32507c3628b094f7e83290dc9f15d2e32101fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a01047304402205b19fe5af18fbc26bf992aaca8766509bc3bb909f79ae34f064d0b021e32e54102206ca753fe5d58baa8c3b4295f38f6a1336b81c632a57da737ac309a2f21b03f9c014830450221009fa3c12e0522c40e8ba735b6c7a3b7b141e65b4996c64a0bf4c3a266cab2c88b02201f8f2ab5947511cbd883a9fdde6725c03ed18cde10f96435ef15d2048971a89201483045022100c737122bfadf4212a1977b17080e4773d221e9edd3465b4a0e47bde06869467802203ed47eaa93a93450f56c012cf5a7df8180243e134025828c91cba0568b9d4a220147304402204e8ff30ab4af218f1b93a92c60dc23d3790041842c97025705baa6c975adc4f30220439046dadfd44f12ce2322cb7da034e5185145b8e5ca65c0ae8a0216c3f0bf3b0147304402202a76f7554badb798a80cd11843fa69cac48f30567998ad3e900e6d8f1281143402201abe99104cc8d6b34cef297c516e29ee1c62b6043c25dda8c835ca910a2f8dc701483045022100ed68f56a119f6b44f03643ddf2fbb37a9dffa9a5e62945bed3a485bc11119ade022068c391fd6a4f6cf2626d831af0a4e9e6665afafbe364d34801c7a2859dbaca850147304402206e021d0bf2a3987109de286afd6e768e696949ecbf64275d9c016c004f32cb92022045ae7791bde9df3748d1b2f845d2717d0b2226d261c89995c6034732d079a2f40147304402202ae093bf8e60b8d913a1aa5ea73ecaa97a074691fadfbd177ac676603ac2dc9802204d02b4e244f5e212917b175465fc63be2003f37c4dc6a9274a02ddbe89a677f201483045022100bad98910fd4ea612ff3e8c1ef7bfbc4a038ccd1c31d1ee591cefbb5746384319022000e5a952635dd67b17ef276c6ea5c88b038c725e42540a27e6ea10f5364d1fb201483045022100dc36b0f3546ffd8d9fd2b1518b8bbdc6a4c3d78f42066701d621f529537d61700220038425f3622a35a4fe4c7ea03945ebad8d45cb8ed2e18e8377fe0f6f67c36d7501483045022100b6b0c75c3ace9a893d881c50bf613f54617792db1b27a929939a8a651b23af20022076348bb2a6ab1ce412d270c414b39496e5b91b0c33c9b916d12cdaa1d48bc97e01473044022015ecca7dfd4543e4a05aeeba9a7672cd56c0c6388e7a03dcfcbc83ba43aae39802200a4a3c87002d193308bef2e3460b67fbb3fdd2f7ec23ee4440ff5813f8b42d26014830450221009a7106c4ae1f3c915adceec089e711520cd81f3cda6267fd943d502398ddf8e60220411fa85bbadca9024b6abfc4aa2588487c5065ad74481a9971391e113176faf601483045022100d223e323928ae4e82ef40518eec46f25d20d37be7d9d6def6ac2e7523cd33a690220343a15b71b51b6026a205848f584a2211954c410d76fa179c78e6ef22a9c7c1e01483045022100fb55303481d50d2f4dc256a70ec68e8ffd165e3b540c9c0a2f82a310ed0435d902202c8fe3cdaac363aedcc632f6559bfc9f67d847f53818a6943fb63f36094cb4ca01fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a01047304402204d1245239904deb71512d1e50e869a38a292f1e760e11fe07c5891082e70302602204a2814908c51a5d87a7e9ad0d606d43268bf6db96f2ccefe9f3410ce3150984a01483045022100a836a7fb4a7859972b63fd145f84a895399df6769d991385a0fdc9d7d634cb6402206e5e14f541b65e7a7d66891c2eebf7f94623fffb525606b8aa6bdcdca6ab7bb101473044022062e964446b72d3acbaac0d69d0191ad0841f11d05fc428bbec986798848d7fd802207b20fc8cad251ef75a2ee2fed8d111924dfdf2235c752860878655d44572a9a90147304402206b8e941c120c565dbe9fceb47789cde04242560bd09eab97f9b92105ab05365202202b6a6388c97d6b9609c4f67dd533e2a4581819ec0937cede94701b0a14cdde2b01483045022100828a17b992e72549764e175ca812666c7eeabc2ef794c39dc58b7091cab9b56102202b90ed54ba4e7fd17c7d9eaf089e6f53e14b9cb311a35f17a5f42fd1f880ee890147304402200928dbe9495ded8fa3493288954a9ba6323abd927f56e4b6eec5f91a1e5eef5702207c00dcae802abf0ee1ba5f956937bbf18f8ae62b1e224e351be04d5081aa98300147304402207e339e3297015ac08cb66c3ecdd30a6174362ab020ddb9f4d61c5576e3101e940220025e3d91e945e9649088a3c87d52f1d89296ba166b7a3f00216e41897851982901473044022072206545652cc69418d0921fb888cfd2a1d329aead3dcb30ace686512078dbf402204f9765c043ffdd19c91c0cfd6ca2e887bee71999fab7d5adb190e876dd8c582c0147304402200fe5704d3b75290687286d4c1e364acae86635b0f66b00f508e522a81ef05e4d02204d4401bcc1149ae6940fd04e1324637bebe399055b9596449b0aac82ae422c6e01483045022100f6b6809fa27dc83c9ed4ad6d6185043e0a1e962a18f2d04d5340760b632b49b00220526978fb3ba2f140aaf1f3af76c0756d3f035808d9dd6204543196d24b337c36014730440220365555d8084d5d995fd3ba7fe5be41f1f0174a9926faddbe36584f220ce34b3d02200d44748f88f9b90060b4129291909e9bd2ddb968eb9515322350424f62478daf01473044022071e3f26b54e17024e8fcbe43d372150824f1809e1ce818ca8de3fcc94a2890ea022030f634275db5a601c096e9b490fa0246fab5e6cebebb155051f0acd8df33eeab01473044022059237c8203986ed34d1e04e77b41b05a77392d264c224fe3180f3b9f19e6e874022038a049528bf710448d26dc4bf32ea2a77094452a375a21e8bb8b32c6f58ade640147304402205e3afa617d32fa514841ec1ac109c89ace00a44fd3aab80e6a3a0a73098b415502201e745d8b6a067e9077dc40cc1eaf911727f2818bebacc4bc818372649edb83230147304402207cd2a9f95ce7cda0f9e9bc5d996ff29ae78ec959c7de49b3694240ad658f648d022024cfd33d817b7e9d00eee084fa471b439bacc0e03f0076d3146cc6f86bab180501fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a010483045022100c8940a65600e231360690ea359f90b3ffb32ae85d06b6063a977c281b203f6b902202a6b68c9a70eb583dfaa2f0395d270afa5a6f546db62bf82f00f537c6c838d0b01483045022100d4741b752c0c62b5bbbf116409cca92e3deb2728aad5751a02732c9d0285184a0220050193e931884523fec85e0cd950484b0a6f5fcc4a5cf4c00e938e149626bb6a01483045022100c062bf58b30a7ffc5140954324335e85384c6d3764abf3be14a208314cc443e702206ba8287c3e60f49e7ea13175dc54c9484c740b3ae4b8f9eb158790f3b6cc83270147304402202feb4825ca3c19974dd57219116f3e3be046c5731f38bbdcf55f651f8cd9de0202203e84f2f475c0ab39c3660ac2ad53523ab4154ab988272ece25ed3877f5026fdd0147304402206b5f3fb356294ffc0915906c71d312a11ba3e162bf14976819f9b93d980e5e5302204440a4413f1dc44725333047fe82fee284c88d2462912b957e86c2d370abb3ab014830450221009cdbfacd0e20dca1182aef1330be5174d837c5a19ac3f6342ecb6cb42f6f4dc5022001fd578d99539657ea2d21a8c97a735b1e2c7de7594a4608c34e4f85d75bb85d01483045022100bcdb59e620fcac087f3b8f8afdc94c6e0cb0d348cec30a349c0ff169b103e145022047b329cf66766f4f0e49086b9533a0a27f6101f4b828fa594af0ce26e586354701483045022100cbaf7578252e903e9fbd024b8dbc82db36aea056368bea73c40470e920ecf85402207966c73a0b43b4418521d7a1cba2d191b879772e6698f4ac9aaf2a7de70b1b5301483045022100fa10c4215b4f8a16dd6d8644a3693cd0506eeb35c1e9f8bc68602d041772878f022027338a06082265aeec2e21b8d35d5908e35f10269b5f7de76bcf10baa5975cec014830450221009052f45cb4abff3ead83eaee2570814c85208e8dba2c68a36ffc66dda152891802206fbcba5c9cbb1a1e1eed873d4a7bf1cdcb5a552355275c3b3bc77aa9e2c0666b01483045022100fb153a80140da5a1fae16029d30a4eafc56052c33116294163b96dcf952f24ff02201d6e16fe60a84ce46195b368cdcf11566f432ed93902f2f5f2b27c090d1f9c3a01483045022100fc895597fd2a78067ed33eadbabb0e4ae57e18bb17f9c0ee64b9a29d78ba243802201da821758f73b84d786130e0c9e5079f593e2094c44efa3f7bf1bc7bea4ae37f01483045022100c42a68ad62124ad2a09e3e05ec6f21189906219a27a9f04d7996d93f44a3ffa702200674f015ee0a5e0bf8a19025a6dd48ce888e17b9707aa03dc929f4f818fcd333014730440220381e4e919e8577adc34ee0ac01f497fdbe926ee12a29209270681a2a1ad6aa69022058300fdf0c7eaabc7455ae21833f6b87e5da87c532b648e9110087d365ac6fc6014830450221008ce8d2a68028998a300ba9ef16992e1da64d02ee477cc38f1078175f52e9a34e022021560a595e2fde0736310850d4e72786faa2c9826052b6da81630b133e6339f701fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a01048304502210083e07369c7ac7aecb162bc4ed74cde58bd59a9cbd60e63910fb7e81d09b51aa602203c440d3b3b428eebfef310fcdfa43c4b1aabb52245e3d81d71bfe2059a57934001483045022100a25206259163454f1151ca2944c2306d1b7820949154cb3520cd028901d8887e02201544c8fbda531ac20ef2a8bcea3d156464ae69f3fda543fec3499fc5b5b9413201483045022100beca384f3c82cfd7bf222bff8e07ba9cccc2dc14c8d3d5798dab000ddc942f2a02201e82958930e54ef828719d6bfa438ebcbc2637e21a52eb6cb7d72ea71395941501483045022100ce3d7423c804909aba9bc3ea53bbdb230c81ef43de2dac671f35068e5ccb10f20220164f00b18d3f7553dc2962938a52ef2e518d13a13c3190193d785b942354a9d70147304402200586c91eb7b2c7da54d10a0365ced19c00cfad2f5fcc12b162c3bea7a2792bc7022006207706d27b0fdfc645c108392fffac0a4ec5628eb00979f30bc0211cd3972c0148304502210097c34ea0665e0f0ef14cbd897256fe95f55a229b7dc11dbcf33e0c332eef53e302204e62f4222e5cb5150a7ad02b299e6d58da974b456fca684a0f87354147dd59a101483045022100d9b1fa91c75324a46d93999b713026bebc733181dcafe53acf944c9620acde6202207e305bcb36087df6d3b757d917f88ada96019bc03b214dab43234229427d7e4c0147304402202370cdad64396bc5d53c75e0c77c59c072049c456b069a3a9d31e8e535a7b64e0220567ec5dc8aaf9c5a1276a956acf3bf7d0799836659b114ca14353009dc0c936201483045022100a5226054e2c79e8a8f5cb176b9eaf51a222962022c0f610e1875a6249b76048902207ba6c355bca8b60e32a5b6a4f9e903c99fec0674a3588f4f202b5233584039ce01483045022100f7ea393da6c3d7b76d51aad3286cf40c92eb440ea7fed919afd87fb02aaaefe702201a73073bfd7ea069f728306dd658c0cfacd6cf382a94f5565d461d79b251706501483045022100a7bf92f24639fc1ec801a0b187dd533a0b2a6685fb2bb5c61d128cd89d4646c50220644cb80f9a94543cdcb5748e9102ac800da225cdedf960dfc67a95e84267d4ab01483045022100a4aad121403618e9e6423c9eddd7576ad92a2fa7e1a7ba6bba095d895aed5d670220031d387e8717f963904fb803c41e179337e530d27e70abee192fc8abb3df705001483045022100cfe493d8600fbd6e0b1069fde83b348f570a7444c69782c711fc4714abee2f2202206b2b723600c0c1301cd299009592888ba7a0a0ce328494a7c6f505e0d065d48f0147304402206590e35ba0a38526ce8b3a55bbf4920d82c5e2bb708bfe3664aa4992c1e50b1e02201fc5306d6e7c99f970d5a46f3729554681a175eb7b39e8f6dc2a04b5fcec1ca40147304402207c261c7f013c22ab365c171fb573614c0b4983f2696cdfd2f4a32780a1a1da6002205f4065812a9efdb353e39fe30e220f418e56a921ad3150de17fdea8593f42d4301fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a01047304402200e9317c6881c79d7b8821a00631b7b6da7975166eeaa8011926aa2e6edf1fd9c0220477f62029224bbd02b3c7e2f2b93a2957d9da77f745ce30b5e3f2f4c7cde021f0147304402203e58e99ba7a850fc7f14e8dd3a0f778df8ae123469ec34d1d6cae11fcd7458be022038d16bc90d8121c95dc4f096793ec41f4fc811b723d05bf999fd72d2f9ceff3401483045022100ad134c9299eadac89b84142810c7c4b2f40027dda0a1b292c883dc1a8d51b4bd02207fc5490a7c332f7114e5eea3442dbd070bcaad9cc4a914abf12167db4606de20014730440220710841aaedbc2b3ad80b87217a15bf0593796399a24df99084701cc52e87126f0220289f386c8a143f85bd0e966af5a46a77d1686c4b3fb64f356cb0ef43a8beba4c0148304502210080ba44e19a1a7c4d4e5332b6f4bb37d34de9216084084ef116f2bb0f1cfbe44702203d8b0fab68f173e446f6d2cf24d22ed9f4916eec5c11219b5f8c7c001391cf0501483045022100f16d380742c9c754d9c40b4fff0deda1f41dfacf5eda608ea9461452192e6729022006278a634f724f5e1d7d7dc89af2f2c1e7a501c0f4391a237294157d9072269f01483045022100f3d4264c8e6d7859149b88f16815f558073c634b13d8b247106b13d2164d455a02204246c997d488dfdcdef1b536ce025978d88ef8a87aaaa9d563e9b35c9f3eb568014730440220136ea1953441e877690ed5939e819a525f75987558689f35708f421a8cd8dd0b022027353a41bfd3af03a7319aee5dac73a9586ce9e807bf8908fd5f66576bd439f10147304402201e05d1bbde094efaca59cf4435f58059dbb42029d8a88964980bb50acbe48537022014f64fe5ef181847cb0d46698b7cf373ddb02be5d5b5b30082ca51c304f812e501483045022100f6d48e22b8923ef648ade10c5feaf24729737d338894010abd6672fe18aebcde0220036990c20772e1483a9ba057045699cb427b22d5d1432fb2930890eef92f6adf01483045022100f10732e04a13fa923daa2b61412f07894ffcfc49252d33e4afc07048ec41f54802200840e47fb6480a76c0490cf3d0993d5bed0c86fcd0bb6cbfb4669d9a648fc72a014730440220262cf09b07aa4bfcda9a70e3b1abb4f30a21d41880ab8f707271337daea3e4c002207f683b2db04b9eef1dea647bf1089979c7ed086df7acadb97859abcc70ab184801483045022100b42fe60eeecf200df1797408018e302d0a3b51700a3adb551c3418a0fada02290220429157135e49022d1ec4b6601cf331014c404352e4a051bfaff5068b235b9ea4014730440220018091c482bc1fbdbe7b928fb52610466ee39f0d5b3ebc165d49eec21e03184c022076a0950675cfb883a2302783d020ac33bc352fbc71a6251a9775955b634ae96001473044022033239984046e52a0e7a405d6acf36aa2d08e5de4b827daa8958b1820a0eb876e02200bf27b3509a32a13fc7386f312659e44b55f63846827c2d6e7fe5cc96e0599d101fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a010483045022100e545511bef9ae99df8bf2960fac7af2903c3485c7208327373813f94a1dcd0ee022025c13be243c33c9a7e346cc11126d613bec70e028e0f306c163d0957d255a572014730440220237ede20811535ee5319c2c2318dba1ba12cea4c17b4e221eb84ba79cc723f6202201c1f3cce5bb2bc2581362d7ef52d477bae3f1a7dbcb4fd6013fbb13ab722462201473044022036b111551c660275c4e129f1662eb457b3ebb7f6558e8f6a577f2f2ddb11921b02204273c8cd17fa83fa4c2eded3b58d3f16bf7d76bedf82f33bee787382d2b2be3d01473044022069c8d8bdda144669ee962aaedbb40308d9d9e899ec26663f24d97dce64493177022015351b208605e63711bfbff904383c9718247b63b3aebd98a3003d71442c003c01473044022003b71247d06d965f28b40558f9b663a9782669aa288d60458a5acf519e14a71402205c573dd4dcf8a8b0f0245cd02629fa007b301446fea9ea8f725c8c74d5c258d2014730440220613f6262c7d41d29c71a549dd5a11d93d4a73b85f73bc3f62c8cb227312bd0cd022015c50572d085812dad0b9a168ac61fd050f15cdc6d442e0d38c7956bc1266a360147304402201b66cae429299b09145b0c1682f2f869fad1ca66767d9252d2b960dfb09fa31302205bfd025c6754946a043647cc98bf7fb03f43911127cf9f8ea86672839661030b014730440220052a9ee03a5f053ff7cb853369baf38141f1dc3d8e92db82e8b5d0a3f1bfbb32022030387e7610a3aa9df72a667537f1017dfe133d83f18a36b8c0dbf2d63849acad01473044022077a83ad3c789d3bc6e3d66bb3737423529ffdffb5c8c1b1c4c52660e372aee0a02200e7936b138921c073125bfb1afb64998aa8f5bdeffbc2f1545bf1a660dcf8c230147304402204a09a44e7398622b85d87e424808f98804b361a8ec1ed3c84ec6092c6c08c85702204b0e5630be7d045c55e18bf9b51cf94820304a3b84f7e3c780901d58dd413ae801483045022100bf383f9b113bac3dcf4c1d067f4fc6735c1a4987518ab426c1ca58585efa0e4202200f3206dab7ab5a110c94cca0d441a2d13a9c4efb0259f5541155d42cf272c0c6014830450221009a8c8402a5f5601c2d4c696d8128bbe7770711e37b398687b47730b43aa9e3c102207b96dd8a37ed6602cab561894b690f32a567ae830d1ae45eeae2d93997444b8c01473044022004d82b15c885b1fa73bae77b05c78a53c798aaad9e10a292d6fdd66e30c4ae9f022017049a9936f7993e2c8b4abd73762453f2b66dd4c49935a21664ed103fb176ec01483045022100f156639cd5d427c6be0cc69d8714183126584840c1f00230cf197f751721ec7f0220367dc57d503651814e08639da039872d2c74be61f931e40758565a9cdeabd317014730440220406e0ef1501404d33af9bb00023c67f00306181f28c95b343c2ee8ee52343d9902203c3ea84c738ba9d1ecfcb6ea268d8242916454c640fa62556ce600da757ba65a01fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a01047304402205dfe539ba79e8ba09030571ed9f4e89f1fa2f4ba31b7076167fdd22051a475f60220668d433052d54b0e22ea8875fae036ce3f288e8dda2a485898e09e2e0df3714c01483045022100d77c1f2cb752e6e3756b41759be4903b6495b99f9687fb778b5108f933001fbe02205dc6a51ea122aae72914e56696bbd621739cde69c286c7b39ccb1174bdaea0a90147304402204854453691a74355075330091967009f130c8690f9498a3bfe1b8bd350623bbe022079d6314e1361c141265534870c0658a4e306f925d934c910d768f8241b36ea75014730440220689ff0886f46c5f360fcbf329fca64a885d96ece0a745f9fc1c420848b7e97ec0220279421c5231b6999cb3e6931b3b3d7b46b9df4945ece79e28d1da8f05ee253aa014830450221009ddb2f8625caa1e961332c6f72683ef29b14bb86db752e7be91afd3fed12cb5802203f18deba3758f661174826ea18ae23685dccdcee8477ff93d848f0c66c589c1e014830450221008f49ffd84f2fb3b7a8750deabaf927efedfb3c377789ecafa4df169e35bd114102203bc2282681a57b6005145ab47fdcca203f6167a137df4e3b7575d43bfa9dc20d01473044022022a28bdcaa547b4caae17262fa282bac7b7341a445a45a94824e7d9c6ec960cc0220275a1b69cfc82af256154535f2db35222fa6d68f3b1c5f863a0935e6c0ed52fb0147304402207bae974aa8e1c0e70f417150de2a19af30f1f94475e927e71eb002c34e5c19cd022066afff23259f026f2f39acf84827bd69b3730869ce221273f59a54b973990a4401483045022100cc680c54973e8ce7bf56a9f88405dbb2670088d04c471dff516ba9412a0a47c70220306b3a3f8c74986d6124b28710e23a079d466a18fa20492c0d75e7392f816d720148304502210089ea8c17d39298a418b35c6c10d08ead1e4a3fb10b6aa6ce2febecec09216aaf022022770fec2b11ab864bd48a8a396e13cb5c8477094acd330197bea39fe8a4325d01483045022100dc4157425967abedf596dba8ee386850607e3419f2fe2384209d041e6431a736022067dd1f780816802c0504df5bc1aa23f7726b1b494f9e235ab836fbaf8cdb7a4d0147304402201e557d52fdad69765d7d13a49c0e5775265937ca978fa0186114c13cff08ee9902203c0f2fb9fa5db44ca8e82b23f95e5de12b6348b2e127ee18ee3fffa9f752adbe01473044022010db4975ddc177da1f097cbad5b825e0f957941d9bd4420c83952bd8329c35bc0220679d3a0fca7347d7b66dd589ce5bc0388180ae668dccb487f8926795b24bddc3014830450221008a3c9008d5b79e517e3798f96e9cdd01580a5d7f8bf4a484ef435d199c923cb80220071f32c95955e70c3a3960d31bbba078b3d8faa8b5cef4065821b9abe0a717c40147304402207a80361acac1301b00bcd55d8d1dd7615a9b5857af055736a5b3020b9adf4b760220031ad2ba0d68f723a7d775eb4b616444bbe607828e5a35c0449d9266bade12b201fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba7142478a659f2d689bbc70ed37235255172a8ac630093687c210287bcbd4f5d357f89a86979b386402445d7e9a5dccfd16146d1d2ab0dc2c32ae8ac630093687c2102053859d76aa375d6f343a60e3678e906c008015e32fe4712b1fd2b26473bdd73ac6300936800a010473044022022bac48961d4d9b82602076613db0f0157edc54b935a3f2cfe5f8251ee39a0c5022014b7d16861267201b16c9eecea310a16765331f71ba438ec2cb89ce5c14d559d01473044022072b8c3e234755aaaf0e8dc38fbe09f9bd347eeec055774db8ea0bbba1b6a88d9022049804afd116feaf8501a7870f7674d8baa75ce7a5155865930f4990eaebfe0b501483045022100826db725fddc8f65d3f9829cbabff53aed34ec819bd6e886acc6c7d9ce0a92b102204bd4977285fc01cacbe97d048475c6bdbc1ac0c2ebc866ddf5dae65a59c4fb6d0147304402204c19ffa64565dedd5e0f210fa934337d3374c947d9a38af8300dcbe5475147db022035f5c3696f893142fe74a4ad7cb920fa740a0a65f49b55cac50d8652c50c693e01483045022100c40c221f592fcb99a236f1389841a513e6636081e2842a6b0e50248fc3237294022056237fc4b335f9bab4f7cd1e23adba2628275be1c2bbb54a258995e31ad978ae0147304402205dd297c3906df3bd738befe95067c3dcf9f041bedce9f9585f1eb09fde757df8022045ed30e2b882eb6219f8e032a0f79ed35a66e06128a8b8dcffd9b7eaa8e4d6b801483045022100bb97c2044c3fe3fe2187a642cc4be605082ee2c6a629b74fe7742012bf2f4da702203706a03c6bfe352fc490db1265b91b31912ce01d4d655c8f5b1cea0f7e30ae760147304402200699861aa64c277f75b8a142a4d465d3c3c181553e03403132fab88323a62cae022023fbc02071e1679c892315001da9dc449e99c5d410f19d8ac679081810f1a653014830450221008c99b59c7abe31fb1fdbd0148d13d37c5129d04c8e7f54e62060646d553aedfc02204c20013c9ec294928b1b53a37b6257f9d501b6b01ca2bd116bb33e0b5e8c892901473044022030645ba65b7e3e885e04691722fe02e3852d4f2d93929b6367a95874fae75020022068dfd2966113a1aee5982a100b3985b62d0b295d827a58bb2eb900fcd805d2c601473044022034326627b7bb36967d1e7997c66b37b7f94d27e89f51e1a4d35bbd90778fb27c022031fe8886796eb57679fa1ea8749c7ad5eea386d97a7f9dab362af7eb8e204565014730440220545755318229e9686366d4b6676e5dc713a220e3268c4bfbeaef153c2c1ef80102201b5055f264268a369095e1abdba04b705eb95bcd83b1aaabd3d1029b3353b6100147304402204119c579dbc9d33398e3a11e15a6af7ca641e2e66ed521744d3a5e91436dc2e6022041f59e9a0190b42959b84a10bceac0d1f97b18da1dac0ff674fe291ada33ef9a0147304402206b6b0a7a4191aa04604988424104ee1a990535879ca6b50d15fd1b44968476d502207252c45423b7211c697b0b64ce4ac58e36773baa594e08f619d62c3e63f4827001473044022040c2ea048000214918ade6d01b06ac191393ac6be117967e3a29dc00d372fc3302201e38d51fb420e849a532372e3e723511bc037762b875068a3c3c9e3c2b02a3a101fd5b02007c2103456772301e221026269d3095ab5cb623fc239835b583ae4632f99a15107ef275ac630093687c2102d67c26cf20153fe7625ca1454222d3b3aeb53b122d8a0f7d32a3dd4b2c2016f4ac630093687c21025f7cfda933516fd590c5a34ad4a68e3143b6f4155a64b3aab2c55fb851150f61ac630093687c210228155bb1ddcd11c7f14a2752565178023aa963f84ea6b6a052bddebad6fe9866ac630093687c21037500441cfb4484da377073459511823b344f1ef0d46bac1efd4c7c466746f666ac630093687c2102ef0d79bfdb99ab0be674b1d5d06c24debd74bffdc28d466633d6668cc281cccfac630093687c210317941e4219548682fb8d8e172f0a8ce4d83ce21272435c85d598558c8e060b7fac630093687c210266065b27f7e3d3ad45b471b1cd4e02de73fc4737dc2679915a45e293c5adcf84ac630093687c21023821cc3da7be9e8cdceb8f146e9ddd78a9519875ecc5b42fe645af690544bccfac630093687c210229ff2b2106b76c27c393e82d71c20eec32bcf1f0cf1a9aca8a237269a67ff3e5ac630093687c21024d113381cc09deb8a6da62e0470644d1a06de82be2725b5052668c8845a4a8daac630093687c2103df2462a5a2f681a3896f61964a65566ff77448be9a55a6da18506fd9c6c051c1ac630093687c2102bafba3096f546cc5831ce1e49ba71decoderawtransaction");
}
