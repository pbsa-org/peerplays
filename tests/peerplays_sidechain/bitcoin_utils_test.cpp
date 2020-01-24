#include <boost/test/unit_test.hpp>
#include <graphene/peerplays_sidechain/bitcoin_utils.hpp>
#include <fc/crypto/hex.hpp>

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
   bytes buff = tx.to_bytes();
   BOOST_CHECK(bintx == buff);
}
