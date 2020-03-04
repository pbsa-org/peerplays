#pragma once

#include <graphene/peerplays_sidechain/sidechain_net_handler.hpp>

#include <string>

#include <fc/signals.hpp>

namespace graphene { namespace peerplays_sidechain {

class sidechain_net_handler_peerplays : public sidechain_net_handler {
public:
   sidechain_net_handler_peerplays(peerplays_sidechain_plugin &_plugin, const boost::program_options::variables_map &options);
   virtual ~sidechain_net_handler_peerplays();

   void recreate_primary_wallet() override;
   void process_deposit(const son_wallet_deposit_object &swdo) override;
   void process_withdrawal(const son_wallet_withdraw_object &swwo) override;
   void process_signing() override;
   void complete_signing() override;

private:
   std::string create_multisignature_wallet(const std::vector<std::string> public_keys) override;
   std::string transfer(const std::string &from, const std::string &to, const uint64_t amount) override;
   std::string sign_transaction(const std::string &transaction) override;
   std::string send_transaction(const std::string &transaction) override;

   void on_applied_block(const signed_block &b);
};

}} // namespace graphene::peerplays_sidechain
