add_subdirectory( sidechain )

file(GLOB HEADERS "include/graphene/peerplays_sidechain_history/*.hpp")

add_library( peerplays_sidechain
             peerplays_sidechain_plugin.cpp
           )

target_link_libraries( peerplays_sidechain sidechain graphene_chain graphene_app )
target_include_directories( peerplays_sidechain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sidechain/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sidechain/network/include" )

install( TARGETS
   peerplays_sidechain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/peerplays_sidechain" )
