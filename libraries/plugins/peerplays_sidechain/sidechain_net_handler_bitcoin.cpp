#include <graphene/peerplays_sidechain/sidechain_net_handler_bitcoin.hpp>
#include <graphene/peerplays_sidechain/bitcoin_utils.hpp>

#include <algorithm>
#include <thread>

#include <boost/algorithm/hex.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fc/crypto/base64.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/log/logger.hpp>
#include <fc/network/ip.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/son_wallet.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/sidechain_address_object.hpp>
#include <graphene/chain/sidechain_transaction_object.hpp>
#include <graphene/chain/son_info.hpp>
#include <graphene/chain/son_wallet_object.hpp>

namespace graphene { namespace peerplays_sidechain {

// =============================================================================

bitcoin_rpc_client::bitcoin_rpc_client(std::string _ip, uint32_t _rpc, std::string _user, std::string _password, std::string _wallet, std::string _wallet_password) :
      ip(_ip),
      rpc_port(_rpc),
      user(_user),
      password(_password),
      wallet(_wallet),
      wallet_password(_wallet_password) {
   authorization.key = "Authorization";
   authorization.val = "Basic " + fc::base64_encode(user + ":" + password);
}

bool bitcoin_rpc_client::connection_is_not_defined() const {
   return ip.empty() || rpc_port == 0 || user.empty() || password.empty();
}

std::string bitcoin_rpc_client::addmultisigaddress(const std::vector<std::string> public_keys) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"addmultisigaddress\", "
                                  "\"method\": \"addmultisigaddress\", \"params\": [");
   std::string params = "2, [";
   std::string pubkeys = "";
   for (std::string pubkey : public_keys) {
      if (!pubkeys.empty()) {
         pubkeys = pubkeys + ",";
      }
      pubkeys = pubkeys + std::string("\"") + pubkey + std::string("\"");
   }
   params = params + pubkeys + std::string("]");
   body = body + params + std::string("] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

std::string bitcoin_rpc_client::createrawtransaction(const std::vector<btc_txout> &ins, const fc::flat_map<std::string, double> outs) {
   std::string body("{\"jsonrpc\": \"1.0\", \"id\":\"createrawtransaction\", "
                    "\"method\": \"createrawtransaction\", \"params\": [");
   body += "[";
   bool first = true;
   for (const auto &entry : ins) {
      if (!first)
         body += ",";
      body += "{\"txid\":\"" + entry.txid_ + "\",\"vout\":" + std::to_string(entry.out_num_) + "}";
      first = false;
   }
   body += "],[";
   first = true;
   for (const auto &entry : outs) {
      if (!first)
         body += ",";
      body += "{\"" + entry.first + "\":" + std::to_string(entry.second) + "}";
      first = false;
   }
   body += std::string("]] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      if (json.count("result"))
         return ss.str();
   } else if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return std::string();
}

std::string bitcoin_rpc_client::createwallet(const std::string &wallet_name) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"createwallet\", \"method\": "
                                  "\"createwallet\", \"params\": [\"" +
                                  wallet_name + "\"] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

std::string bitcoin_rpc_client::encryptwallet(const std::string &passphrase) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"encryptwallet\", \"method\": "
                                  "\"encryptwallet\", \"params\": [\"" +
                                  passphrase + "\"] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

uint64_t bitcoin_rpc_client::estimatesmartfee() {
   static const auto confirmation_target_blocks = 6;

   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"estimatesmartfee\", "
                                 "\"method\": \"estimatesmartfee\", \"params\": [") +
                     std::to_string(confirmation_target_blocks) + std::string("] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return 0;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (json.count("result"))
      if (json.get_child("result").count("feerate")) {
         auto feerate_str = json.get_child("result").get_child("feerate").get_value<std::string>();
         feerate_str.erase(std::remove(feerate_str.begin(), feerate_str.end(), '.'), feerate_str.end());
         return std::stoll(feerate_str);
      }
   return 0;
}

std::string bitcoin_rpc_client::getblock(const std::string &block_hash, int32_t verbosity) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"getblock\", \"method\": "
                                  "\"getblock\", \"params\": [\"" +
                                  block_hash + "\", " + std::to_string(verbosity) + "] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

void bitcoin_rpc_client::importaddress(const std::string &address_or_script) {
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"importaddress\", "
                                 "\"method\": \"importaddress\", \"params\": [") +
                     std::string("\"") + address_or_script + std::string("\"") + std::string("] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      idump((address_or_script)(ss.str()));
      return;
   } else if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
}

std::vector<btc_txout> bitcoin_rpc_client::listunspent() {
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"pp_plugin\", \"method\": "
                                 "\"listunspent\", \"params\": [] }");

   const auto reply = send_post_request(body);

   std::vector<btc_txout> result;

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return result;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      if (json.count("result")) {
         for (auto &entry : json.get_child("result")) {
            btc_txout txo;
            txo.txid_ = entry.second.get_child("txid").get_value<std::string>();
            txo.out_num_ = entry.second.get_child("vout").get_value<unsigned int>();
            txo.amount_ = entry.second.get_child("amount").get_value<double>();
            result.push_back(txo);
         }
      }
   } else if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return result;
}

std::vector<btc_txout> bitcoin_rpc_client::listunspent_by_address_and_amount(const std::string &address, double minimum_amount) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"pp_plugin\", \"method\": "
                                  "\"listunspent\", \"params\": [");
   body += std::string("1,999999,[\"");
   body += address;
   body += std::string("\"],true,{\"minimumAmount\":");
   body += std::to_string(minimum_amount);
   body += std::string("}] }");

   const auto reply = send_post_request(body);

   std::vector<btc_txout> result;
   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return result;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      if (json.count("result")) {
         for (auto &entry : json.get_child("result")) {
            btc_txout txo;
            txo.txid_ = entry.second.get_child("txid").get_value<std::string>();
            txo.out_num_ = entry.second.get_child("vout").get_value<unsigned int>();
            txo.amount_ = entry.second.get_child("amount").get_value<double>();
            result.push_back(txo);
         }
      }
   } else if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return result;
}

std::string bitcoin_rpc_client::loadwallet(const std::string &filename) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"loadwallet\", \"method\": "
                                  "\"loadwallet\", \"params\": [\"" +
                                  filename + "\"] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

void bitcoin_rpc_client::sendrawtransaction(const std::string &tx_hex) {
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"sendrawtransaction\", "
                                 "\"method\": \"sendrawtransaction\", \"params\": [") +
                     std::string("\"") + tx_hex + std::string("\"") + std::string("] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      return;
   } else if (json.count("error") && !json.get_child("error").empty()) {
      const auto error_code = json.get_child("error").get_child("code").get_value<int>();
      if (error_code == -27) // transaction already in block chain
         return;

      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
}

std::string bitcoin_rpc_client::signrawtransactionwithkey(const std::string &tx_hash, const std::string &private_key) {
   return "";
}

std::string bitcoin_rpc_client::signrawtransactionwithwallet(const std::string &tx_hash) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"signrawtransactionwithwallet\", "
                                  "\"method\": \"signrawtransactionwithwallet\", \"params\": [");
   std::string params = "\"" + tx_hash + "\"";
   body = body + params + std::string("]}");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

std::string bitcoin_rpc_client::unloadwallet(const std::string &filename) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"unloadwallet\", \"method\": "
                                  "\"unloadwallet\", \"params\": [\"" +
                                  filename + "\"] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

std::string bitcoin_rpc_client::walletlock() {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"walletlock\", \"method\": "
                                  "\"walletlock\", \"params\": [] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return std::string();
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, json.get_child("result"));
      return ss.str();
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return "";
}

bool bitcoin_rpc_client::walletpassphrase(const std::string &passphrase, uint32_t timeout) {
   std::string body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"walletpassphrase\", \"method\": "
                                  "\"walletpassphrase\", \"params\": [\"" +
                                  passphrase + "\", " + std::to_string(timeout) + "] }");

   const auto reply = send_post_request(body);

   if (reply.body.empty()) {
      wlog("Bitcoin RPC call ${function} failed", ("function", __FUNCTION__));
      return false;
   }

   std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
   boost::property_tree::ptree json;
   boost::property_tree::read_json(ss, json);

   if (reply.status == 200) {
      return true;
   }

   if (json.count("error") && !json.get_child("error").empty()) {
      wlog("Bitcoin RPC call ${function} with body ${body} failed with reply '${msg}'", ("function", __FUNCTION__)("body", body)("msg", ss.str()));
   }
   return false;
}

fc::http::reply bitcoin_rpc_client::send_post_request(std::string body, bool show_log) {
   fc::http::connection conn;
   conn.connect_to(fc::ip::endpoint(fc::ip::address(ip), rpc_port));

   std::string url = "http://" + ip + ":" + std::to_string(rpc_port);

   if (wallet.length() > 0) {
      url = url + "/wallet/" + wallet;
   }

   fc::http::reply reply = conn.request("POST", url, body, fc::http::headers{authorization});

   if (show_log) {
      ilog("Request URL: ${url}", ("url", url));
      ilog("Request:     ${body}", ("body", body));
      std::stringstream ss(std::string(reply.body.begin(), reply.body.end()));
      ilog("Response:    ${ss}", ("ss", ss.str()));
   }

   return reply;
}

// =============================================================================

zmq_listener::zmq_listener(std::string _ip, uint32_t _zmq) :
      ip(_ip),
      zmq_port(_zmq),
      ctx(1),
      socket(ctx, ZMQ_SUB) {
   std::thread(&zmq_listener::handle_zmq, this).detach();
}

std::vector<zmq::message_t> zmq_listener::receive_multipart() {
   std::vector<zmq::message_t> msgs;

   int32_t more;
   size_t more_size = sizeof(more);
   while (true) {
      zmq::message_t msg;
      socket.recv(&msg, 0);
      socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);

      if (!more)
         break;
      msgs.push_back(std::move(msg));
   }

   return msgs;
}

void zmq_listener::handle_zmq() {
   socket.setsockopt(ZMQ_SUBSCRIBE, "hashblock", 9);
   //socket.setsockopt( ZMQ_SUBSCRIBE, "hashtx", 6 );
   //socket.setsockopt( ZMQ_SUBSCRIBE, "rawblock", 8 );
   //socket.setsockopt( ZMQ_SUBSCRIBE, "rawtx", 5 );
   socket.connect("tcp://" + ip + ":" + std::to_string(zmq_port));

   while (true) {
      auto msg = receive_multipart();
      const auto header = std::string(static_cast<char *>(msg[0].data()), msg[0].size());
      const auto block_hash = boost::algorithm::hex(std::string(static_cast<char *>(msg[1].data()), msg[1].size()));

      event_received(block_hash);
   }
}

// =============================================================================

sidechain_net_handler_bitcoin::sidechain_net_handler_bitcoin(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map &options) :
      sidechain_net_handler(_plugin, options) {
   sidechain = sidechain_type::bitcoin;

   ip = options.at("bitcoin-node-ip").as<std::string>();
   zmq_port = options.at("bitcoin-node-zmq-port").as<uint32_t>();
   rpc_port = options.at("bitcoin-node-rpc-port").as<uint32_t>();
   rpc_user = options.at("bitcoin-node-rpc-user").as<std::string>();
   rpc_password = options.at("bitcoin-node-rpc-password").as<std::string>();
   wallet = "";
   if (options.count("bitcoin-wallet")) {
      wallet = options.at("bitcoin-wallet").as<std::string>();
   }
   wallet_password = "";
   if (options.count("bitcoin-wallet-password")) {
      wallet_password = options.at("bitcoin-wallet-password").as<std::string>();
   }

   if (options.count("bitcoin-private-key")) {
      const std::vector<std::string> pub_priv_keys = options["bitcoin-private-key"].as<std::vector<std::string>>();
      for (const std::string &itr_key_pair : pub_priv_keys) {
         auto key_pair = graphene::app::dejsonify<std::pair<std::string, std::string>>(itr_key_pair, 5);
         ilog("Bitcoin Public Key: ${public}", ("public", key_pair.first));
         if (!key_pair.first.length() || !key_pair.second.length()) {
            FC_THROW("Invalid public private key pair.");
         }
         private_keys[key_pair.first] = key_pair.second;
      }
   }

   fc::http::connection conn;
   try {
      conn.connect_to(fc::ip::endpoint(fc::ip::address(ip), rpc_port));
   } catch (fc::exception e) {
      elog("No BTC node running at ${ip} or wrong rpc port: ${port}", ("ip", ip)("port", rpc_port));
      FC_ASSERT(false);
   }

   bitcoin_client = std::unique_ptr<bitcoin_rpc_client>(new bitcoin_rpc_client(ip, rpc_port, rpc_user, rpc_password, wallet, wallet_password));
   if (!wallet.empty()) {
      bitcoin_client->loadwallet(wallet);
   }

   listener = std::unique_ptr<zmq_listener>(new zmq_listener(ip, zmq_port));
   listener->event_received.connect([this](const std::string &event_data) {
      std::thread(&sidechain_net_handler_bitcoin::handle_event, this, event_data).detach();
   });
}

sidechain_net_handler_bitcoin::~sidechain_net_handler_bitcoin() {
}

void sidechain_net_handler_bitcoin::recreate_primary_wallet() {
   const auto &swi = database.get_index_type<son_wallet_index>().indices().get<by_id>();
   const auto &active_sw = swi.rbegin();
   if (active_sw != swi.rend()) {

      if ((active_sw->addresses.find(sidechain_type::bitcoin) == active_sw->addresses.end()) ||
          (active_sw->addresses.at(sidechain_type::bitcoin).empty())) {

         const chain::global_property_object &gpo = database.get_global_properties();

         auto active_sons = gpo.active_sons;
         vector<pair<string, uint64_t>> son_pubkeys_bitcoin;
         for ( const son_info& si : active_sons ) {
            son_pubkeys_bitcoin.push_back(
                     make_pair(
                        si.sidechain_public_keys.at(sidechain_type::bitcoin),
                        si.total_votes
                        )
                     );
         }

         string address = create_weighted_multisignature_wallet(son_pubkeys_bitcoin);

         ilog(address);

         son_wallet_update_operation op;
         op.payer = GRAPHENE_SON_ACCOUNT;
         op.son_wallet_id = (*active_sw).id;
         op.sidechain = sidechain_type::bitcoin;
         op.address = address;

         proposal_create_operation proposal_op;
         proposal_op.fee_paying_account = plugin.get_son_object(plugin.get_current_son_id()).son_account;
         proposal_op.proposed_ops.emplace_back( op_wrapper( op ) );
         uint32_t lifetime = ( gpo.parameters.block_interval * gpo.active_witnesses.size() ) * 3;
         proposal_op.expiration_time = time_point_sec( database.head_block_time().sec_since_epoch() + lifetime );

         signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
         try {
            database.push_transaction(trx, database::validation_steps::skip_block_size_check);
            if(plugin.app().p2p_node())
               plugin.app().p2p_node()->broadcast(net::trx_message(trx));
         } catch(fc::exception e){
            ilog("sidechain_net_handler:  sending proposal for son wallet update operation failed with exception ${e}",("e", e.what()));
            return;
         }

         const auto &prev_sw = std::next(active_sw);
         if (prev_sw != swi.rend()) {
            std::string prev_pw_address = prev_sw->addresses.at(sidechain_type::bitcoin);
            std::string active_pw_address = address;
            vector<son_info> sons = prev_sw->sons;
            transfer_all_btc(prev_pw_address, sons, active_pw_address);
         }
      }
   }
}

void sidechain_net_handler_bitcoin::process_deposit(const son_wallet_deposit_object &swdo) {
   transfer_deposit_to_primary_wallet(swdo);
}

void sidechain_net_handler_bitcoin::process_withdrawal(const son_wallet_withdraw_object &swwo) {
   transfer_withdrawal_from_primary_wallet(swwo);
}

static bool has_enough_signatures(const bitcoin_transaction_object &tx_object) {
   // TODO: Verify with weights calculation
   bool has_empty = false;
   for(auto s: tx_object.signatures)
      has_empty |= s.second.empty();
   return !has_empty;
}

void sidechain_net_handler_bitcoin::process_signing()
{
   const auto &idx = plugin.database().get_index_type<proposal_index>().indices().get<by_id>();
   vector<proposal_id_type> proposals;
   for (const auto &proposal : idx) {
      if (proposal.proposed_transaction.operations.size() != 1)
         continue;
      if (proposal.proposed_transaction.operations[0].which() != chain::operation::tag<chain::bitcoin_transaction_send_operation>::value) {
         continue;
      }
      bitcoin_transaction_send_operation tx_object = proposal.proposed_transaction.operations[0].get<bitcoin_transaction_send_operation>();
      // collect signatures
      auto sons = plugin.get_sons();
      for (son_id_type son_id : sons) {
         auto it = tx_object.signatures.find(son_id);
         if (it == tx_object.signatures.end())
            continue;
         if (it->second.empty())
         {
            bitcoin_transaction_sign_operation op;
            son_object s_obj= plugin.get_son_object(son_id);
            op.payer = s_obj.son_account;
            op.proposal_id = proposal.id;
            fc::ecc::private_key k = plugin.get_private_key(son_id);
            op.signatures = signatures_for_raw_transaction(tx_object.unsigned_tx, tx_object.in_amounts, tx_object.redeem_script, k);

            signed_transaction trx = plugin.database().create_signed_transaction(k, op);
            try {
               plugin.database().push_transaction(trx, database::validation_steps::skip_block_size_check);
               if(plugin.app().p2p_node())
                  plugin.app().p2p_node()->broadcast(net::trx_message(trx));
            } catch(fc::exception e){
               ilog("sidechain_net_handler: bitcoin transaction signing failed with exception ${e}",("e", e.what()));
               return;
            }
         }
      }
   }
}

void sidechain_net_handler_bitcoin::complete_signing()
{
   const auto &idx = plugin.database().get_index_type<bitcoin_transaction_index>().indices().get<by_processed>();
   const auto &idx_range = idx.equal_range(false);
   std::for_each(idx_range.first, idx_range.second,
                 [&](const bitcoin_transaction_object &tx_object) {
                     // check if all signatures collected
                     if (has_enough_signatures(tx_object)) {
                        publish_btc_tx(tx_object);
                        bitcoin_send_transaction_process_operation op;
                        op.payer = GRAPHENE_SON_ACCOUNT;
                        op.bitcoin_transaction_id = tx_object.id;

                        const chain::global_property_object& gpo = database.get_global_properties();
                        proposal_create_operation proposal_op;
                        proposal_op.fee_paying_account = plugin.get_son_object(plugin.get_current_son_id()).son_account;
                        proposal_op.proposed_ops.emplace_back( op_wrapper( op ) );
                        uint32_t lifetime = ( gpo.parameters.block_interval * gpo.active_witnesses.size() ) * 3;
                        proposal_op.expiration_time = time_point_sec( database.head_block_time().sec_since_epoch() + lifetime );

                        signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
                        try {
                           database.push_transaction(trx, database::validation_steps::skip_block_size_check);
                           if(plugin.app().p2p_node())
                              plugin.app().p2p_node()->broadcast(net::trx_message(trx));
                        } catch(fc::exception e){
                           ilog("sidechain_net_handler_bitcoin:  sending proposal for bitcoin send operation failed with exception ${e}",("e", e.what()));
                           return;
                        }
                     }
   });
}

std::string sidechain_net_handler_bitcoin::create_multisignature_wallet(const std::vector<std::string> public_keys) {
   return bitcoin_client->addmultisigaddress(public_keys);
}

std::string sidechain_net_handler_bitcoin::create_weighted_multisignature_wallet(const std::vector<std::pair<std::string, uint64_t>> &public_keys) {
   string address = get_weighted_multisig_address(public_keys);
   bitcoin_client->importaddress(address);
   return address;
}

std::string sidechain_net_handler_bitcoin::transfer(const std::string &from, const std::string &to, const uint64_t amount) {
   return "";
}

std::string sidechain_net_handler_bitcoin::sign_transaction(const std::string &transaction) {
   return "";
}

std::string sidechain_net_handler_bitcoin::send_transaction(const std::string &transaction) {
   return "";
}

std::string sidechain_net_handler_bitcoin::sign_and_send_transaction_with_wallet ( const std::string& tx_json )
{
   if (!wallet_password.empty()) {
      bitcoin_client->walletpassphrase(wallet_password, 60);
   }

   std::string unsigned_tx_hex = tx_json;

   std::string reply_str = bitcoin_client->signrawtransactionwithwallet(unsigned_tx_hex);
   ilog(reply_str);
   std::stringstream ss_stx(reply_str);
   boost::property_tree::ptree stx_json;
   boost::property_tree::read_json(ss_stx, stx_json);

   //if (!wallet_password.empty()) {
   //   bitcoin_client->walletlock();
   //}

   if (!(stx_json.count("error") && stx_json.get_child("error").empty()) || !stx_json.count("result") || !stx_json.get_child("result").count("hex")) {
      return "";
   }

   std::string signed_tx_hex = stx_json.get<std::string>("result.hex");

   bitcoin_client->sendrawtransaction(signed_tx_hex);

   return reply_str;
}

void sidechain_net_handler_bitcoin::transfer_all_btc(const std::string& from_address, const vector<son_info>& from_sons, const std::string& to_address)
{
   uint64_t fee_rate = bitcoin_client->estimatesmartfee();
   uint64_t min_fee_rate = 1000;
   fee_rate = std::max(fee_rate, min_fee_rate);

   double min_amount = ((double)fee_rate / 100000000.0); // Account only for relay fee for now
   double total_amount = 0.0;
   std::vector<btc_txout> unspent_utxo = bitcoin_client->listunspent_by_address_and_amount(from_address, 0);

   if(unspent_utxo.size() == 0) {
      wlog("Failed to find UTXOs to spend for ${pw}",("pw", from_address));
      return;
   } else {
      for(const auto& utx: unspent_utxo) {
         total_amount += utx.amount_;
      }

      if(min_amount >= total_amount) {
         wlog("Failed not enough BTC to transfer from ${fa}",("fa", from_address));
         return;
      }
   }

   btc_tx tx;
   tx.hasWitness = true;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   std::vector<uint64_t> amounts;
   for(const auto& utx: unspent_utxo)
   {
      tx.vin.push_back(btc_in(utx.txid_, utx.out_num_));
      amounts.push_back(uint64_t(utx.amount_ * 100000000.0));
   }
   tx.vout.push_back(btc_out(to_address, uint64_t((total_amount - min_amount) * 100000000.0)));

   std::vector<std::pair<fc::ecc::public_key, uint64_t> > key_data;
   for(auto si: from_sons)
   {
      fc::ecc::public_key pk = si.signing_key;
      key_data.push_back(std::make_pair(pk, si.total_votes));
   }
   std::sort(key_data.begin(), key_data.end(),
             [](std::pair<fc::ecc::public_key, uint64_t> p1, std::pair<fc::ecc::public_key, uint64_t> p2){
               return (p1.second > p2.second);
             }
   );
   bytes from_redeem_script = generate_redeem_script(key_data);

   bitcoin_transaction_send_operation op;
   op.payer = GRAPHENE_SON_ACCOUNT;
   op.redeem_script = from_redeem_script;
   op.in_amounts = amounts;
   tx.to_bytes(op.unsigned_tx);
   // add signatures
   std::set<son_id_type> plugin_sons = plugin.get_sons();
   for(auto si: from_sons)
   {
      if (plugin_sons.find(si.son_id) != plugin_sons.end())
      {
         fc::ecc::private_key k = plugin.get_private_key(si.son_id);
         std::vector<bytes> signatures = signatures_for_raw_transaction(op.unsigned_tx, amounts, from_redeem_script, k);
         op.signatures[si.son_id] = signatures;
      }
      else
      {
         op.signatures[si.son_id];
      }
   }

   const chain::global_property_object& gpo = database.get_global_properties();
   proposal_create_operation proposal_op;
   proposal_op.fee_paying_account = plugin.get_son_object(plugin.get_current_son_id()).son_account;
   proposal_op.proposed_ops.emplace_back( op_wrapper( op ) );
   uint32_t lifetime = ( gpo.parameters.block_interval * gpo.active_witnesses.size() ) * 3;
   proposal_op.expiration_time = time_point_sec( database.head_block_time().sec_since_epoch() + lifetime );

   signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
   try {
      database.push_transaction(trx, database::validation_steps::skip_block_size_check);
      if(plugin.app().p2p_node())
         plugin.app().p2p_node()->broadcast(net::trx_message(trx));
   } catch(fc::exception e){
      ilog("sidechain_net_handler:  sending proposal for son wallet update operation failed with exception ${e}",("e", e.what()));
      return;
   }
}

std::string sidechain_net_handler_bitcoin::transfer_deposit_to_primary_wallet(const son_wallet_deposit_object &swdo) {
   const auto &idx = database.get_index_type<son_wallet_index>().indices().get<by_id>();
   auto obj = idx.rbegin();
   if (obj == idx.rend() || obj->addresses.find(sidechain_type::bitcoin) == obj->addresses.end()) {
      return "";
   }

   std::string pw_address = obj->addresses.find(sidechain_type::bitcoin)->second;

   std::string txid = swdo.sidechain_transaction_id;
   std::string suid = swdo.sidechain_uid;
   std::string nvout = suid.substr(suid.find_last_of("-") + 1);
   uint64_t deposit_amount = swdo.sidechain_amount.value;
   uint64_t fee_rate = bitcoin_client->estimatesmartfee();
   uint64_t min_fee_rate = 1000;
   fee_rate = std::max(fee_rate, min_fee_rate);
   deposit_amount -= fee_rate; // Deduct minimum relay fee

   btc_tx tx;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   tx.hasWitness = true;
   tx.vin.push_back(btc_in(txid, std::stoul(nvout)));
   tx.vout.push_back(btc_out(pw_address, deposit_amount));

   bytes unsigned_tx;
   tx.to_bytes(unsigned_tx);

   std::string reply_str = fc::to_hex((char*)&unsigned_tx[0], unsigned_tx.size());
   return sign_and_send_transaction_with_wallet(reply_str);
}

std::string sidechain_net_handler_bitcoin::transfer_withdrawal_from_primary_wallet(const son_wallet_withdraw_object &swwo) {
   const auto &idx = database.get_index_type<son_wallet_index>().indices().get<by_id>();
   auto obj = idx.rbegin();
   if (obj == idx.rend() || obj->addresses.find(sidechain_type::bitcoin) == obj->addresses.end()) {
      return "";
   }

   std::string pw_address = obj->addresses.find(sidechain_type::bitcoin)->second;

   uint64_t fee_rate = bitcoin_client->estimatesmartfee();
   uint64_t min_fee_rate = 1000;
   fee_rate = std::max(fee_rate, min_fee_rate);

   double min_amount = ((double)(swwo.withdraw_amount.value + fee_rate) / 100000000.0); // Account only for relay fee for now
   double total_amount = 0.0;
   std::vector<btc_txout> unspent_utxo = bitcoin_client->listunspent_by_address_and_amount(pw_address, 0);

   if (unspent_utxo.size() == 0) {
      wlog("Failed to find UTXOs to spend for ${pw}", ("pw", pw_address));
      return "";
   } else {
      for (const auto &utx : unspent_utxo) {
         total_amount += utx.amount_;
      }

      if (min_amount > total_amount) {
         wlog("Failed not enough BTC to spend for ${pw}", ("pw", pw_address));
         return "";
      }
   }

   btc_tx tx;
   tx.nVersion = 2;
   tx.nLockTime = 0;
   tx.hasWitness = true;
   std::vector<uint64_t> amounts;
   for(const auto& utxo: unspent_utxo)
   {
      tx.vin.push_back(btc_in(utxo.txid_, utxo.amount_));
      amounts.push_back(uint64_t(utxo.amount_ * 100000000.0));
   }
   tx.vout.push_back(btc_out(swwo.withdraw_address, swwo.withdraw_amount.value));
   if((total_amount - min_amount) > 0.0)
   {
      tx.vout.push_back(btc_out(pw_address, (total_amount - min_amount) * 100000000.0));
   }

   auto from_sons = obj->sons;

   std::vector<std::pair<fc::ecc::public_key, uint64_t> > key_data;
   for(auto si: from_sons)
   {
      fc::ecc::public_key pk = si.signing_key;
      key_data.push_back(std::make_pair(pk, si.total_votes));
   }
   std::sort(key_data.begin(), key_data.end(),
             [](std::pair<fc::ecc::public_key, uint64_t> p1, std::pair<fc::ecc::public_key, uint64_t> p2){
               return (p1.second > p2.second);
             }
   );
   bytes from_redeem_script = generate_redeem_script(key_data);

   bitcoin_transaction_send_operation op;
   op.payer = GRAPHENE_SON_ACCOUNT;
   op.redeem_script = from_redeem_script;
   op.in_amounts = amounts;
   tx.to_bytes(op.unsigned_tx);
   // add signatures
   std::set<son_id_type> plugin_sons = plugin.get_sons();
   for(auto si: from_sons)
   {
      if (plugin_sons.find(si.son_id) != plugin_sons.end())
      {
         fc::ecc::private_key k = plugin.get_private_key(si.son_id);
         std::vector<bytes> signatures = signatures_for_raw_transaction(op.unsigned_tx, amounts, from_redeem_script, k);
         op.signatures[si.son_id] = signatures;
      }
      else
      {
         op.signatures[si.son_id];
      }
   }

   const chain::global_property_object& gpo = database.get_global_properties();
   proposal_create_operation proposal_op;
   proposal_op.fee_paying_account = plugin.get_son_object(plugin.get_current_son_id()).son_account;
   proposal_op.proposed_ops.emplace_back( op_wrapper( op ) );
   uint32_t lifetime = ( gpo.parameters.block_interval * gpo.active_witnesses.size() ) * 3;
   proposal_op.expiration_time = time_point_sec( database.head_block_time().sec_since_epoch() + lifetime );

   signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
   try {
      database.push_transaction(trx, database::validation_steps::skip_block_size_check);
      if(plugin.app().p2p_node())
         plugin.app().p2p_node()->broadcast(net::trx_message(trx));
   } catch(fc::exception e){
      ilog("sidechain_net_handler:  sending proposal for son wallet update operation failed with exception ${e}",("e", e.what()));
      return "";
   }
   return "";
}

void sidechain_net_handler_bitcoin::publish_btc_tx(const bitcoin_transaction_object &tx_object)
{
   std::vector<std::vector<bytes>> signatures;
   signatures.resize(tx_object.signatures.size());
   std::transform(tx_object.signatures.begin(), tx_object.signatures.end(),
                  signatures.begin(), [](const std::pair<son_id_type, std::vector<bytes>>& p) { return p.second; }
   );
   bytes signed_tx = add_signatures_to_unsigned_tx(tx_object.unsigned_tx, signatures, tx_object.redeem_script);
   bitcoin_client->sendrawtransaction(fc::to_hex((const char*)&signed_tx[0], signed_tx.size()));
}

void sidechain_net_handler_bitcoin::handle_event(const std::string &event_data) {
   std::string block = bitcoin_client->getblock(event_data);
   if (block != "") {
      const auto &vins = extract_info_from_block(block);

      const auto &sidechain_addresses_idx = database.get_index_type<sidechain_address_index>().indices().get<by_sidechain_and_deposit_address>();

      for (const auto &v : vins) {
         const auto &addr_itr = sidechain_addresses_idx.find(std::make_tuple(sidechain, v.address));
         if (addr_itr == sidechain_addresses_idx.end())
            continue;

         std::stringstream ss;
         ss << "bitcoin"
            << "-" << v.out.hash_tx << "-" << v.out.n_vout;
         std::string sidechain_uid = ss.str();

         sidechain_event_data sed;
         sed.timestamp = plugin.database().head_block_time();
         sed.sidechain = addr_itr->sidechain;
         sed.sidechain_uid = sidechain_uid;
         sed.sidechain_transaction_id = v.out.hash_tx;
         sed.sidechain_from = "";
         sed.sidechain_to = v.address;
         sed.sidechain_currency = "BTC";
         sed.sidechain_amount = v.out.amount;
         sed.peerplays_from = addr_itr->sidechain_address_account;
         sed.peerplays_to = GRAPHENE_SON_ACCOUNT;
         sed.peerplays_asset = asset(sed.sidechain_amount / 1000); // For Bitcoin, the exchange rate is 1:1, for others, get the exchange rate from market
         sidechain_event_data_received(sed);
      }
   }
}

std::vector<info_for_vin> sidechain_net_handler_bitcoin::extract_info_from_block(const std::string &_block) {
   std::stringstream ss(_block);
   boost::property_tree::ptree block;
   boost::property_tree::read_json(ss, block);

   std::vector<info_for_vin> result;

   for (const auto &tx_child : block.get_child("tx")) {
      const auto &tx = tx_child.second;

      for (const auto &o : tx.get_child("vout")) {
         const auto script = o.second.get_child("scriptPubKey");

         if (!script.count("addresses"))
            continue;

         for (const auto &addr : script.get_child("addresses")) { // in which cases there can be more addresses?
            const auto address_base58 = addr.second.get_value<std::string>();
            info_for_vin vin;
            vin.out.hash_tx = tx.get_child("txid").get_value<std::string>();
            string amount = o.second.get_child("value").get_value<std::string>();
            amount.erase(std::remove(amount.begin(), amount.end(), '.'), amount.end());
            vin.out.amount = std::stoll(amount);
            vin.out.n_vout = o.second.get_child("n").get_value<uint32_t>();
            vin.address = address_base58;
            result.push_back(vin);
         }
      }
   }

   return result;
}

// =============================================================================

}} // namespace graphene::peerplays_sidechain
