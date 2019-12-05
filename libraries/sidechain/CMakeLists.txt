file( GLOB SOURCES "*.cpp" )
file( GLOB HEADERS "include/*.hpp" )

file( GLOB SOURCES_BITCOIN "bitcoin/*.cpp" )
file( GLOB HEADERS_BITCOIN "include/bitcoin/*.hpp" )

add_library( sidechain STATIC ${SOURCES} ${HEADERS} ${SOURCES_BITCOIN} ${HEADERS_BITCOIN} )

target_link_libraries( sidechain fc graphene_chain )
target_include_directories( sidechain PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
