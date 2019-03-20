#pragma once

#include <vm_interface.hpp>
#include <pp_state.hpp>
#include <graphene/chain/protocol/contract.hpp>

#include <aleth/libethereum/LastBlockHashesFace.h>

namespace graphene { namespace chain { struct database_fixture; } }

namespace vms { namespace evm {

using namespace vms::base;

class evm : public vm_interface
{

public:

   evm( const std::string& path, adapters adapter );

   std::pair<uint64_t, bytes> exec( const bytes& data, const bool commit ) override;

   std::vector< uint64_t > get_attracted_contracts( ) const override { return attracted_contracts; };

   void roll_back_db( const uint32_t& block_number ) override;

   std::vector<bytes> get_contracts() const override;

   bytes get_state_root() const;

   void set_state_root( const std::string& hash );

   friend struct graphene::chain::database_fixture;

private:

   evm_adapter& get_adapter() { return adapter.get<vms::evm::evm_adapter>(); }

   Transaction create_eth_transaction(const eth_op& eth) const;

   EnvInfo create_environment();

   void transfer_suicide_balances(const std::vector< std::pair< Address, Address > >& suicide_transfer);

   void delete_balances( const std::unordered_map< Address, Account >& accounts );

   std::vector< uint64_t > select_attracted_contracts( const std::unordered_map< Address, Account >& accounts );

   void clear_temporary_variables();

   std::vector< uint64_t > attracted_contracts;

   std::unique_ptr< SealEngineFace > se;

   pp_state state;

};

class last_block_hashes: public dev::eth::LastBlockHashesFace {

public:

   explicit last_block_hashes(){ m_lastHashes.clear(); }

   h256s precedingHashes(h256 const& _mostRecentHash) const override
   {
      if (m_lastHashes.empty() || m_lastHashes.front() != _mostRecentHash)
         m_lastHashes.resize(256);
      return m_lastHashes;
   }

   void clear() override { m_lastHashes.clear(); }

private:

   mutable h256s m_lastHashes;

};

} }

FC_REFLECT_ENUM( dev::eth::TransactionException, 
                 (None)
                 (Unknown)
                 (BadRLP)
                 (InvalidFormat)
                 (OutOfGasIntrinsic)
                 (InvalidSignature)
                 (InvalidNonce)
                 (NotEnoughCash)
                 (OutOfGasBase)
                 (BlockGasLimitReached)
                 (BadInstruction)
                 (BadJumpDestination)
                 (OutOfGas)
                 (OutOfStack)
                 (StackUnderflow)
                 (RevertInstruction)
                 (InvalidZeroSignatureFormat)
                 (AddressAlreadyUsed) )

FC_REFLECT_ENUM( dev::eth::CodeDeposit,
                 (None)
                 (Failed)
                 (Success) )

FC_REFLECT( dev::Address, (m_data) )
FC_REFLECT( dev::h2048, (m_data) )
FC_REFLECT( dev::h256, (m_data) )

FC_REFLECT( dev::eth::LogEntry,
            (address)
            (topics)
            (data) )

FC_REFLECT( dev::eth::TransactionReceipt,
            (m_statusCodeOrStateRoot)
            (m_gasUsed)
            (m_bloom)
            (m_log) )

FC_REFLECT( dev::eth::ExecutionResult, 
            (gasUsed)
            (excepted)
            (newAddress)
            (output)
            (codeDeposit)
            (gasRefunded)
            (depositSize)
            (gasForDeposit) )

