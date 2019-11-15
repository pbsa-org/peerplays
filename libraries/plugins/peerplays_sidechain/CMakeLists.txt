file(GLOB_RECURSE HEADERS "include/graphene/peerplays_sidechain/*.hpp")

add_library( peerplays_sidechain
             peerplays_sidechain_plugin.cpp
             bitcoin/bech32.cpp
             bitcoin/bitcoin_address.cpp
             bitcoin/bitcoin_script.cpp
             bitcoin/bitcoin_transaction.cpp
             bitcoin/segwit_addr.cpp
             bitcoin/utils.cpp
           )

target_link_libraries( peerplays_sidechain graphene_chain graphene_app )
target_include_directories( peerplays_sidechain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
install( TARGETS
   peerplays_sidechain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/peerplays_sidechain" )

