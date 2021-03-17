#include <graphene/peerplays_sidechain/sidechain_net_handler_hive.hpp>

#include <algorithm>
#include <iomanip>
#include <thread>

#include <boost/algorithm/hex.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fc/crypto/base64.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/log/logger.hpp>
#include <fc/network/ip.hpp>
#include <fc/smart_ref_impl.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/son_wallet.hpp>
#include <graphene/chain/son_info.hpp>
#include <graphene/chain/son_wallet_object.hpp>
#include <graphene/utilities/key_conversion.hpp>

namespace graphene { namespace peerplays_sidechain {

hive_node_rpc_client::hive_node_rpc_client(std::string _ip, uint32_t _port, std::string _user, std::string _password) :
      rpc_client(_ip, _port, _user, _password) {
}

std::string hive_node_rpc_client::block_api_get_block(uint32_t block_number) {
   std::string params = "{ \"block_num\": " + std::to_string(block_number) + " }";
   return send_post_request("block_api.get_block", params, false);
}

std::string hive_node_rpc_client::database_api_get_dynamic_global_properties() {
   return send_post_request("database_api.get_dynamic_global_properties", "", false);
}

hive_wallet_rpc_client::hive_wallet_rpc_client(std::string _ip, uint32_t _port, std::string _user, std::string _password) :
      rpc_client(_ip, _port, _user, _password) {
}

std::string hive_wallet_rpc_client::get_account(std::string account) {
   std::string params = "[\"" + account + "\"]";
   return send_post_request("get_account", params, true);
}

std::string hive_wallet_rpc_client::lock() {
   return send_post_request("lock", "", true);
}

std::string hive_wallet_rpc_client::unlock(std::string password) {
   std::string params = "[\"" + password + "\"]";
   return send_post_request("unlock", params, true);
}

std::string hive_wallet_rpc_client::update_account_auth_key(std::string account_name, std::string type, std::string public_key, std::string weight) {
   std::string params = "[\"" + account_name + "\", \"" + type + "\", \"" + public_key + "\", " + weight + "]";
   return send_post_request("update_account_auth_key", params, true);
}

std::string hive_wallet_rpc_client::update_account_auth_account(std::string account_name, std::string type, std::string auth_account, std::string weight) {
   std::string params = "[\"" + account_name + "\", \"" + type + "\", \"" + auth_account + "\", " + weight + "]";
   return send_post_request("update_account_auth_account", params, true);
}

std::string hive_wallet_rpc_client::update_account_auth_threshold(std::string account_name, std::string type, std::string threshold) {
   std::string params = "[\"" + account_name + "\", \"" + type + "\", " + threshold + "]";
   return send_post_request("update_account_auth_account", params, true);
}

sidechain_net_handler_hive::sidechain_net_handler_hive(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map &options) :
      sidechain_net_handler(_plugin, options) {
   sidechain = sidechain_type::hive;

   node_ip = options.at("hive-node-ip").as<std::string>();
   node_rpc_port = options.at("hive-node-rpc-port").as<uint32_t>();
   if (options.count("hive-node-rpc-user")) {
      node_rpc_user = options.at("hive-node-rpc-user").as<std::string>();
   } else {
      node_rpc_user = "";
   }
   if (options.count("hive-node-rpc-password")) {
      node_rpc_password = options.at("hive-node-rpc-password").as<std::string>();
   } else {
      node_rpc_password = "";
   }

   wallet_ip = options.at("hive-wallet-ip").as<std::string>();
   wallet_rpc_port = options.at("hive-wallet-rpc-port").as<uint32_t>();
   if (options.count("hive-wallet-rpc-user")) {
      wallet_rpc_user = options.at("hive-wallet-rpc-user").as<std::string>();
   } else {
      wallet_rpc_user = "";
   }
   if (options.count("hive-wallet-rpc-password")) {
      wallet_rpc_password = options.at("hive-wallet-rpc-password").as<std::string>();
   } else {
      wallet_rpc_password = "";
   }

   if (options.count("hive-private-key")) {
      const std::vector<std::string> pub_priv_keys = options["hive-private-key"].as<std::vector<std::string>>();
      for (const std::string &itr_key_pair : pub_priv_keys) {
         auto key_pair = graphene::app::dejsonify<std::pair<std::string, std::string>>(itr_key_pair, 5);
         ilog("Hive Public Key: ${public}", ("public", key_pair.first));
         if (!key_pair.first.length() || !key_pair.second.length()) {
            FC_THROW("Invalid public private key pair.");
         }
         private_keys[key_pair.first] = key_pair.second;
      }
   }

   fc::http::connection conn;
   try {
      conn.connect_to(fc::ip::endpoint(fc::ip::address(node_ip), node_rpc_port));
   } catch (fc::exception &e) {
      elog("No Hive node running at ${ip} or wrong rpc port: ${port}", ("ip", node_ip)("port", node_rpc_port));
      FC_ASSERT(false);
   }
   try {
      conn.connect_to(fc::ip::endpoint(fc::ip::address(wallet_ip), wallet_rpc_port));
   } catch (fc::exception &e) {
      elog("No Hive wallet running at ${ip} or wrong rpc port: ${port}", ("ip", wallet_ip)("port", wallet_rpc_port));
      FC_ASSERT(false);
   }

   node_rpc_client = new hive_node_rpc_client(node_ip, node_rpc_port, node_rpc_user, node_rpc_password);

   wallet_rpc_client = new hive_wallet_rpc_client(wallet_ip, wallet_rpc_port, wallet_rpc_user, wallet_rpc_password);

   last_block_received = 0;
   schedule_hive_listener();
   event_received.connect([this](const std::string &event_data) {
      std::thread(&sidechain_net_handler_hive::handle_event, this, event_data).detach();
   });
}

sidechain_net_handler_hive::~sidechain_net_handler_hive() {
}

bool sidechain_net_handler_hive::process_proposal(const proposal_object &po) {

   ilog("Proposal to process: ${po}, SON id ${son_id}", ("po", po.id)("son_id", plugin.get_current_son_id()));

   bool should_approve = false;

   //   const chain::global_property_object &gpo = database.get_global_properties();
   //
   //   int32_t op_idx_0 = -1;
   //   chain::operation op_obj_idx_0;
   //
   //   if (po.proposed_transaction.operations.size() >= 1) {
   //      op_idx_0 = po.proposed_transaction.operations[0].which();
   //      op_obj_idx_0 = po.proposed_transaction.operations[0];
   //   }
   //
   //   switch (op_idx_0) {
   //
   //   case chain::operation::tag<chain::son_wallet_update_operation>::value: {
   //      should_approve = false;
   //      break;
   //   }
   //
   //   case chain::operation::tag<chain::son_wallet_deposit_process_operation>::value: {
   //      son_wallet_deposit_id_type swdo_id = op_obj_idx_0.get<son_wallet_deposit_process_operation>().son_wallet_deposit_id;
   //      const auto &idx = database.get_index_type<son_wallet_deposit_index>().indices().get<by_id>();
   //      const auto swdo = idx.find(swdo_id);
   //      if (swdo != idx.end()) {
   //
   //         uint32_t swdo_block_num = swdo->block_num;
   //         std::string swdo_sidechain_transaction_id = swdo->sidechain_transaction_id;
   //         uint32_t swdo_op_idx = std::stoll(swdo->sidechain_uid.substr(swdo->sidechain_uid.find_last_of("-") + 1));
   //
   //         const auto &block = database.fetch_block_by_number(swdo_block_num);
   //
   //         for (const auto &tx : block->transactions) {
   //            if (tx.id().str() == swdo_sidechain_transaction_id) {
   //               operation op = tx.operations[swdo_op_idx];
   //               transfer_operation t_op = op.get<transfer_operation>();
   //
   //               asset sidechain_asset = asset(swdo->sidechain_amount, fc::variant(swdo->sidechain_currency, 1).as<asset_id_type>(1));
   //               price sidechain_asset_price = database.get<asset_object>(sidechain_asset.asset_id).options.core_exchange_rate;
   //               asset peerplays_asset = asset(sidechain_asset.amount * sidechain_asset_price.base.amount / sidechain_asset_price.quote.amount);
   //
   //               should_approve = (gpo.parameters.son_account() == t_op.to) &&
   //                                (swdo->peerplays_from == t_op.from) &&
   //                                (sidechain_asset == t_op.amount) &&
   //                                (swdo->peerplays_asset == peerplays_asset);
   //               break;
   //            }
   //         }
   //      }
   //      break;
   //   }
   //
   //   case chain::operation::tag<chain::son_wallet_withdraw_process_operation>::value: {
   //      should_approve = false;
   //      break;
   //   }
   //
   //   case chain::operation::tag<chain::sidechain_transaction_settle_operation>::value: {
   //      should_approve = true;
   //      break;
   //   }
   //
   //   default:
   //      should_approve = false;
   //      elog("==================================================");
   //      elog("Proposal not considered for approval ${po}", ("po", po));
   //      elog("==================================================");
   //   }

   should_approve = true;

   return should_approve;
}

void sidechain_net_handler_hive::process_primary_wallet() {
   const auto &swi = database.get_index_type<son_wallet_index>().indices().get<by_id>();
   const auto &active_sw = swi.rbegin();
   if (active_sw != swi.rend()) {

      if ((active_sw->addresses.find(sidechain) == active_sw->addresses.end()) ||
          (active_sw->addresses.at(sidechain).empty())) {

         if (proposal_exists(chain::operation::tag<chain::son_wallet_update_operation>::value, active_sw->id)) {
            return;
         }

         const chain::global_property_object &gpo = database.get_global_properties();

         //auto active_sons = gpo.active_sons;
         //string reply_str = create_primary_wallet_address(active_sons);

         //std::stringstream active_pw_ss(reply_str);
         //boost::property_tree::ptree active_pw_pt;
         //boost::property_tree::read_json(active_pw_ss, active_pw_pt);
         //if (active_pw_pt.count("error") && active_pw_pt.get_child("error").empty()) {

         proposal_create_operation proposal_op;
         proposal_op.fee_paying_account = plugin.get_current_son_object().son_account;
         uint32_t lifetime = (gpo.parameters.block_interval * gpo.active_witnesses.size()) * 3;
         proposal_op.expiration_time = time_point_sec(database.head_block_time().sec_since_epoch() + lifetime);

         //std::stringstream res;
         //boost::property_tree::json_parser::write_json(res, active_pw_pt.get_child("result"));

         son_wallet_update_operation swu_op;
         swu_op.payer = gpo.parameters.son_account();
         swu_op.son_wallet_id = active_sw->id;
         swu_op.sidechain = sidechain;
         swu_op.address = "son-account";

         proposal_op.proposed_ops.emplace_back(swu_op);

         //const auto &prev_sw = std::next(active_sw);
         //if (prev_sw != swi.rend()) {
         //   std::string new_pw_address = active_pw_pt.get<std::string>("result.address");
         //   std::string tx_str = create_primary_wallet_transaction(*prev_sw, new_pw_address);
         //   if (!tx_str.empty()) {
         //      sidechain_transaction_create_operation stc_op;
         //      stc_op.payer = gpo.parameters.son_account();
         //      stc_op.object_id = prev_sw->id;
         //      stc_op.sidechain = sidechain;
         //      stc_op.transaction = tx_str;
         //      stc_op.signers = prev_sw->sons;
         //      proposal_op.proposed_ops.emplace_back(stc_op);
         //   }
         //}

         signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
         try {
            trx.validate();
            database.push_transaction(trx, database::validation_steps::skip_block_size_check);
            if (plugin.app().p2p_node())
               plugin.app().p2p_node()->broadcast(net::trx_message(trx));
         } catch (fc::exception &e) {
            elog("Sending proposal for son wallet update operation failed with exception ${e}", ("e", e.what()));
            return;
         }
         //}
      }
   }
}

void sidechain_net_handler_hive::process_sidechain_addresses() {
   //   const auto &sidechain_addresses_idx = database.get_index_type<sidechain_address_index>();
   //   const auto &sidechain_addresses_by_sidechain_idx = sidechain_addresses_idx.indices().get<by_sidechain>();
   //   const auto &sidechain_addresses_by_sidechain_range = sidechain_addresses_by_sidechain_idx.equal_range(sidechain);
   //   std::for_each(sidechain_addresses_by_sidechain_range.first, sidechain_addresses_by_sidechain_range.second,
   //                 [&](const sidechain_address_object &sao) {
   //                     bool retval = true;
   //                    if (sao.expires == time_point_sec::maximum()) {
   //                       if (sao.deposit_address == "") {
   //                          sidechain_address_update_operation op;
   //                           op.payer = plugin.get_current_son_object().son_account;
   //                           op.sidechain_address_id = sao.id;
   //                           op.sidechain_address_account = sao.sidechain_address_account;
   //                           op.sidechain = sao.sidechain;
   //                           op.deposit_public_key = sao.deposit_public_key;
   //                           op.deposit_address = sao.withdraw_address;
   //                           op.deposit_address_data = sao.withdraw_address;
   //                           op.withdraw_public_key = sao.withdraw_public_key;
   //                           op.withdraw_address = sao.withdraw_address;
   //
   //                           signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), op);
   //                           try {
   //                              trx.validate();
   //                              database.push_transaction(trx, database::validation_steps::skip_block_size_check);
   //                              if (plugin.app().p2p_node())
   //                                 plugin.app().p2p_node()->broadcast(net::trx_message(trx));
   //                              retval = true;
   //                           } catch (fc::exception &e) {
   //                              elog("Sending transaction for update deposit address operation failed with exception ${e}", ("e", e.what()));
   //                              retval = false;
   //                           }
   //                       }
   //                    }
   //                     return retval;
   //                 });
   return;
}

bool sidechain_net_handler_hive::process_deposit(const son_wallet_deposit_object &swdo) {

   //   const chain::global_property_object &gpo = database.get_global_properties();
   //
   //   asset_issue_operation ai_op;
   //   ai_op.issuer = gpo.parameters.son_account();
   //   price btc_price = database.get<asset_object>(database.get_global_properties().parameters.btc_asset()).options.core_exchange_rate;
   //   ai_op.asset_to_issue = asset(swdo.peerplays_asset.amount * btc_price.quote.amount / btc_price.base.amount, database.get_global_properties().parameters.btc_asset());
   //   ai_op.issue_to_account = swdo.peerplays_from;
   //
   //   signed_transaction tx;
   //   auto dyn_props = database.get_dynamic_global_properties();
   //   tx.set_reference_block(dyn_props.head_block_id);
   //   tx.set_expiration(database.head_block_time() + gpo.parameters.maximum_time_until_expiration);
   //   tx.operations.push_back(ai_op);
   //   database.current_fee_schedule().set_fee(tx.operations.back());
   //
   //   std::stringstream ss;
   //   fc::raw::pack(ss, tx, 1000);
   //   std::string tx_str = boost::algorithm::hex(ss.str());
   //
   //   if (!tx_str.empty()) {
   //      const chain::global_property_object &gpo = database.get_global_properties();
   //
   //      sidechain_transaction_create_operation stc_op;
   //      stc_op.payer = gpo.parameters.son_account();
   //      stc_op.object_id = swdo.id;
   //      stc_op.sidechain = sidechain;
   //      stc_op.transaction = tx_str;
   //      stc_op.signers = gpo.active_sons;
   //
   //      proposal_create_operation proposal_op;
   //      proposal_op.fee_paying_account = plugin.get_current_son_object().son_account;
   //      proposal_op.proposed_ops.emplace_back(stc_op);
   //      uint32_t lifetime = (gpo.parameters.block_interval * gpo.active_witnesses.size()) * 3;
   //      proposal_op.expiration_time = time_point_sec(database.head_block_time().sec_since_epoch() + lifetime);
   //
   //      signed_transaction trx = database.create_signed_transaction(plugin.get_private_key(plugin.get_current_son_id()), proposal_op);
   //      try {
   //         trx.validate();
   //         database.push_transaction(trx, database::validation_steps::skip_block_size_check);
   //         if (plugin.app().p2p_node())
   //            plugin.app().p2p_node()->broadcast(net::trx_message(trx));
   //         return true;
   //      } catch (fc::exception &e) {
   //         elog("Sending proposal for deposit sidechain transaction create operation failed with exception ${e}", ("e", e.what()));
   //         return false;
   //      }
   //   }
   return false;
}

bool sidechain_net_handler_hive::process_withdrawal(const son_wallet_withdraw_object &swwo) {
   return false;
}

std::string sidechain_net_handler_hive::process_sidechain_transaction(const sidechain_transaction_object &sto) {

   //   std::stringstream ss_trx(boost::algorithm::unhex(sto.transaction));
   //   signed_transaction trx;
   //   fc::raw::unpack(ss_trx, trx, 1000);
   //
   //   fc::optional<fc::ecc::private_key> privkey = graphene::utilities::wif_to_key(get_private_key(plugin.get_current_son_object().sidechain_public_keys.at(sidechain)));
   //   signature_type st = trx.sign(*privkey, database.get_chain_id());
   //
   std::stringstream ss_st;
   //   fc::raw::pack(ss_st, st, 1000);
   std::string st_str = boost::algorithm::hex(ss_st.str());

   return st_str;
}

std::string sidechain_net_handler_hive::send_sidechain_transaction(const sidechain_transaction_object &sto) {

   //   std::stringstream ss_trx(boost::algorithm::unhex(sto.transaction));
   //   signed_transaction trx;
   //   fc::raw::unpack(ss_trx, trx, 1000);
   //
   //   for (auto signature : sto.signatures) {
   //      if (!signature.second.empty()) {
   //         std::stringstream ss_st(boost::algorithm::unhex(signature.second));
   //         signature_type st;
   //         fc::raw::unpack(ss_st, st, 1000);
   //
   //         trx.signatures.push_back(st);
   //         trx.signees.clear();
   //      }
   //   }
   //
   //   try {
   //      trx.validate();
   //      database.push_transaction(trx, database::validation_steps::skip_block_size_check);
   //      if (plugin.app().p2p_node())
   //         plugin.app().p2p_node()->broadcast(net::trx_message(trx));
   //      return trx.id().str();
   //   } catch (fc::exception &e) {
   //      elog("Sidechain transaction failed with exception ${e}", ("e", e.what()));
   //      return "";
   //   }

   return "";
}

int64_t sidechain_net_handler_hive::settle_sidechain_transaction(const sidechain_transaction_object &sto) {
   int64_t settle_amount = 0;
   return settle_amount;
}

void sidechain_net_handler_hive::schedule_hive_listener() {
   fc::time_point now = fc::time_point::now();
   int64_t time_to_next = 1000;

   fc::time_point next_wakeup(now + fc::milliseconds(time_to_next));

   _listener_task = fc::schedule([this] {
      hive_listener_loop();
   },
                                 next_wakeup, "SON Hive listener task");
}

void sidechain_net_handler_hive::hive_listener_loop() {
   schedule_hive_listener();

   std::string reply = node_rpc_client->database_api_get_dynamic_global_properties();

   if (!reply.empty()) {
      std::stringstream ss(reply);
      boost::property_tree::ptree json;
      boost::property_tree::read_json(ss, json);
      if (json.count("result")) {
         uint64_t head_block_number = json.get<uint64_t>("result.head_block_number");
         if (head_block_number != last_block_received) {
            std::string event_data = std::to_string(head_block_number);
            handle_event(event_data);
            last_block_received = head_block_number;
         }
      }
   }
}

void sidechain_net_handler_hive::handle_event(const std::string &event_data) {
   std::string block = node_rpc_client->block_api_get_block(std::atoll(event_data.c_str()));
   if (block != "") {
      std::stringstream ss(block);
      boost::property_tree::ptree block_json;
      boost::property_tree::read_json(ss, block_json);

      for (const auto &tx_child : block_json.get_child("result.block.transactions")) {
         const auto &tx = tx_child.second;

         for (const auto &ops : tx.get_child("operations")) {
            const auto &op = ops.second;

            std::string operation_type = op.get<std::string>("type");
            ilog("Transactions: ${operation_type}", ("operation_type", operation_type));

            if (operation_type == "transfer_operation") {
               const auto &op_value = op.get_child("value");

               std::string from = op_value.get<std::string>("from");
               std::string to = op_value.get<std::string>("to");

               const auto &amount_child = op_value.get_child("amount");

               uint64_t amount = amount_child.get<uint64_t>("amount");
               uint64_t precision = amount_child.get<uint64_t>("precision");
               std::string nai = amount_child.get<std::string>("nai");

               ilog("Transfer from ${from} to ${to}, amount ${amount}, precision ${precision}, nai ${nai}",
                    ("from", from)("to", to)("amount", amount)("precision", precision)("nai", nai));
            }
         }
      }

      //boost::property_tree::ptree transactions_json = block_json.get_child("result.block.transactions");
      //ss.str("");
      //boost::property_tree::write_json(ss, transactions_json);
      //elog("Transactions: ${ss}", ("ss", ss.str()));

      //{"jsonrpc":"2.0","result":{"block":{"previous":"00006dd6c2b98bc7d1214f61f1c797bb22edb4cd","timestamp":"2021-03-10T12:18:21","witness":"initminer","transaction_merkle_root":"55d89a7b615a4d25f32d9160ce6714a5de5f2b05","extensions":[],"witness_signature":"1f0e2115cb18d862a279de93f3d4d82df4210984e26231db206de8b37e26b2aa8048f21fc7447f842047fea7ffa2a481eede07d379bf9577ab09b5395434508d86","transactions":[{"ref_block_num":28118,"ref_block_prefix":3347823042,"expiration":"2021-03-10T12:18:48","operations":[{"type":"transfer_operation","value":{"from":"initminer","to":"deepcrypto8","amount":{"amount":"100000000","precision":3,"nai":"@@000000021"},"memo":""}}],"extensions":[],"signatures":["1f55c34b9fab328de76d7c4afd30ca1b64742f46d2aee759b66fc9b0e9d90653a44dbad1ef583c9578666abc23db0ca540f32746d7ac4ff7a6394d28a2c9ef29f3"]}],"block_id":"00006dd7a264f6a8d833ad88a7eeb3abdd483af3","signing_key":"TST6LLegbAgLAy28EHrffBVuANFWcFgmqRMW13wBmTExqFE9SCkg4","transaction_ids":["73eb9d7a19b9bcb941500f4a9924c13fe3b94c4a"]}},"id":"block_api.get_block"}
      //{"jsonrpc":"2.0","result":{"block":{"previous":"00006de5714685397129f52693504ed3abde8e44","timestamp":"2021-03-10T12:19:06","witness":"initminer","transaction_merkle_root":"8b9bcb4b0aed33624a68abdf2860e76136ae9313","extensions":[],"witness_signature":"20f0743c1c3f63230f8af615e14ca0c5143ddfde0c5cee83c24486276223ceb21e7950f1b503750aad73f979bbdf6a298c9e22a079cc1397ed9d4a6eb8aeccea79","transactions":[{"ref_block_num":28133,"ref_block_prefix":965035633,"expiration":"2021-03-10T12:19:33","operations":[{"type":"transfer_operation","value":{"from":"initminer","to":"deepcrypto8","amount":{"amount":"100000000","precision":3,"nai":"@@000000021"},"memo":""}}],"extensions":[],"signatures":["1f519b0e13ee672108670540f846ad7cef676c94e3169e2d4c3ff12b5dad6dc412154cb45677b2caa0f839b5e2826ae96d6bbf36987ab40a928c3e0081e10a082e"]}],"block_id":"00006de655bac50cb40e05fc02eaef112ccae454","signing_key":"TST6LLegbAgLAy28EHrffBVuANFWcFgmqRMW13wBmTExqFE9SCkg4","transaction_ids":["28c09bb777827fbf41023c50600aef65e0c83a8b"]}},"id":"block_api.get_block"}
      //{"jsonrpc":"2.0","result":{"block":{"previous":"00006dfe471f2224d4b6c3189c7a2a5ed261c277","timestamp":"2021-03-10T12:20:21","witness":"initminer","transaction_merkle_root":"e407272ab2e383e5fef487651d88e0c0c3617e29","extensions":[],"witness_signature":"1f48152244ad2036e3761248a93a9c38fb8c7ee8a0721f9f47ae267b63808e56c223ab15641ec2baad6be4a54db5df04ead9b7f1107dba267a89c02c49d8115fbf","transactions":[{"ref_block_num":28158,"ref_block_prefix":606216007,"expiration":"2021-03-10T12:20:48","operations":[{"type":"transfer_operation","value":{"from":"initminer","to":"deepcrypto8","amount":{"amount":"100000000","precision":3,"nai":"@@000000021"},"memo":""}}],"extensions":[],"signatures":["1f5b4c7a8695b6c68b6ee96000367ffa96c23d9f4ed8ca36f639b38351b8198d18626f3b8277ca5c92bd537c68db5f9730f3f9df59a8ce631e9dcf7ce53032796b"]}],"block_id":"00006dff4bcd9a790193924fe44d0bdc3fde1c83","signing_key":"TST6LLegbAgLAy28EHrffBVuANFWcFgmqRMW13wBmTExqFE9SCkg4","transaction_ids":["50b8f4b268a0fa3a34698dd6f8152117343e6c06"]}},"id":"block_api.get_block"}

      //const auto &vins = extract_info_from_block(block);

      //const auto &sidechain_addresses_idx = database.get_index_type<sidechain_address_index>().indices().get<by_sidechain_and_deposit_address_and_expires>();

      /*for (const auto &v : vins) {
         // !!! EXTRACT DEPOSIT ADDRESS FROM SIDECHAIN ADDRESS OBJECT
         const auto &addr_itr = sidechain_addresses_idx.find(std::make_tuple(sidechain, v.address, time_point_sec::maximum()));
         if (addr_itr == sidechain_addresses_idx.end())
            continue;

         std::stringstream ss;
         ss << "bitcoin"
            << "-" << v.out.hash_tx << "-" << v.out.n_vout;
         std::string sidechain_uid = ss.str();

         sidechain_event_data sed;
         sed.timestamp = database.head_block_time();
         sed.block_num = database.head_block_num();
         sed.sidechain = addr_itr->sidechain;
         sed.sidechain_uid = sidechain_uid;
         sed.sidechain_transaction_id = v.out.hash_tx;
         sed.sidechain_from = "";
         sed.sidechain_to = v.address;
         sed.sidechain_currency = "BTC";
         sed.sidechain_amount = v.out.amount;
         sed.peerplays_from = addr_itr->sidechain_address_account;
         sed.peerplays_to = database.get_global_properties().parameters.son_account();
         price btc_price = database.get<asset_object>(database.get_global_properties().parameters.btc_asset()).options.core_exchange_rate;
         sed.peerplays_asset = asset(sed.sidechain_amount * btc_price.base.amount / btc_price.quote.amount);
         sidechain_event_data_received(sed);
      }*/
   }
}

void sidechain_net_handler_hive::on_applied_block(const signed_block &b) {
   //   for (const auto &trx : b.transactions) {
   //      size_t operation_index = -1;
   //      for (auto op : trx.operations) {
   //         operation_index = operation_index + 1;
   //         if (op.which() == operation::tag<transfer_operation>::value) {
   //            transfer_operation transfer_op = op.get<transfer_operation>();
   //            if (transfer_op.to != plugin.database().get_global_properties().parameters.son_account()) {
   //               continue;
   //            }
   //
   //            std::stringstream ss;
   //            ss << "peerplays"
   //               << "-" << trx.id().str() << "-" << operation_index;
   //            std::string sidechain_uid = ss.str();
   //
   //            sidechain_event_data sed;
   //            sed.timestamp = database.head_block_time();
   //            sed.block_num = database.head_block_num();
   //            sed.sidechain = sidechain_type::peerplays;
   //            sed.sidechain_uid = sidechain_uid;
   //            sed.sidechain_transaction_id = trx.id().str();
   //            sed.sidechain_from = fc::to_string(transfer_op.from.space_id) + "." + fc::to_string(transfer_op.from.type_id) + "." + fc::to_string((uint64_t)transfer_op.from.instance);
   //            sed.sidechain_to = fc::to_string(transfer_op.to.space_id) + "." + fc::to_string(transfer_op.to.type_id) + "." + fc::to_string((uint64_t)transfer_op.to.instance);
   //            sed.sidechain_currency = fc::to_string(transfer_op.amount.asset_id.space_id) + "." + fc::to_string(transfer_op.amount.asset_id.type_id) + "." + fc::to_string((uint64_t)transfer_op.amount.asset_id.instance);
   //            sed.sidechain_amount = transfer_op.amount.amount;
   //            sed.peerplays_from = transfer_op.from;
   //            sed.peerplays_to = transfer_op.to;
   //            price asset_price = database.get<asset_object>(transfer_op.amount.asset_id).options.core_exchange_rate;
   //            sed.peerplays_asset = asset(transfer_op.amount.amount * asset_price.base.amount / asset_price.quote.amount);
   //            sidechain_event_data_received(sed);
   //         }
   //      }
   //   }
}

}} // namespace graphene::peerplays_sidechain
