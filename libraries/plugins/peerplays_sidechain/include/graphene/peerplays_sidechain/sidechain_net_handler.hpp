#pragma once

#include <boost/program_options.hpp>

namespace graphene { namespace peerplays_sidechain {

class sidechain_net_handler {
public:
    sidechain_net_handler(const boost::program_options::variables_map& options);
    virtual ~sidechain_net_handler();

    virtual std::string create_multisignature_wallet( const std::vector<std::string> public_keys ) = 0;
    virtual std::string transfer( const std::string& from, const std::string& to, const uint64_t amount ) = 0;
    virtual std::string sign_transaction( const std::string& transaction ) = 0;
    virtual std::string send_transaction( const std::string& transaction ) = 0;

protected:
    virtual void handle_event( const std::string& event_data ) = 0;

private:

};

} } // graphene::peerplays_sidechain

