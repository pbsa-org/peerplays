#pragma once

#include <graphene/peerplays_sidechain/sidechain_net_handler.hpp>

#include <string>

#include <fc/network/http/connection.hpp>
#include <fc/signals.hpp>

namespace graphene { namespace peerplays_sidechain {

class hive_rpc_client {
public:
   hive_rpc_client(std::string _ip, uint32_t _rpc_port, std::string _user, std::string _password);

   std::string block_api_get_block(uint32_t block_number);
   std::string database_api_get_dynamic_global_properties();

private:
   std::string send_post_request(std::string method, std::string params, bool show_log);
   fc::http::reply send_post_request(std::string body, bool show_log);

   std::string ip;
   uint32_t rpc_port;
   std::string user;
   std::string password;

   fc::http::header authorization;
};

class sidechain_net_handler_hive : public sidechain_net_handler {
public:
   sidechain_net_handler_hive(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map &options);
   virtual ~sidechain_net_handler_hive();

   bool process_proposal(const proposal_object &po);
   void process_primary_wallet();
   void process_sidechain_addresses();
   bool process_deposit(const son_wallet_deposit_object &swdo);
   bool process_withdrawal(const son_wallet_withdraw_object &swwo);
   std::string process_sidechain_transaction(const sidechain_transaction_object &sto);
   std::string send_sidechain_transaction(const sidechain_transaction_object &sto);
   int64_t settle_sidechain_transaction(const sidechain_transaction_object &sto);

private:
   std::string ip;
   uint32_t rpc_port;
   std::string rpc_user;
   std::string rpc_password;

   hive_rpc_client *rpc_client;

   uint64_t last_block_received;
   fc::future<void> _listener_task;
   fc::signal<void(const std::string &)> event_received;
   void schedule_hive_listener();
   void hive_listener_loop();
   void handle_event(const std::string &event_data);

   void on_applied_block(const signed_block &b);
};

}} // namespace graphene::peerplays_sidechain
