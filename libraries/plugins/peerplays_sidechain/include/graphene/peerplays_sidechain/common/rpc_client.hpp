#pragma once

#include <cstdint>
#include <string>

#include <fc/network/http/connection.hpp>

namespace graphene { namespace peerplays_sidechain {

class rpc_client {
public:
   rpc_client(std::string _ip, uint32_t _port, std::string _user, std::string _password);

protected:
   std::string send_post_request(std::string method, std::string params, bool show_log);
   fc::http::reply send_post_request(std::string body, bool show_log);

   std::string ip;
   uint32_t port;
   std::string user;
   std::string password;

   uint32_t request_id;

   fc::http::header authorization;
};

}} // namespace graphene::peerplays_sidechain
