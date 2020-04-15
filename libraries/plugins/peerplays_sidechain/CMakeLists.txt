file(GLOB HEADERS "include/graphene/peerplays_sidechain/*.hpp")

add_library( peerplays_sidechain
             peerplays_sidechain_plugin.cpp
             sidechain_net_manager.cpp
             sidechain_net_handler.cpp
             sidechain_net_handler_bitcoin.cpp
             sidechain_net_handler_peerplays.cpp
             bitcoin_utils.cpp
           )

if (ENABLE_DEV_FEATURES)
   message ("Dev features enabled")
   target_compile_definitions(peerplays_sidechain PRIVATE ENABLE_DEV_FEATURES)
endif()
unset(ENABLE_DEV_FEATURES)
unset(ENABLE_DEV_FEATURES CACHE)

target_link_libraries( peerplays_sidechain graphene_chain graphene_app fc zmq )
target_include_directories( peerplays_sidechain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   peerplays_sidechain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/peerplays_sidechain" )

