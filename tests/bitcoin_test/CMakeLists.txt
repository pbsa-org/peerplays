add_executable( bitcoin_test main.cpp )
target_link_libraries( bitcoin_test
                       PRIVATE libbitcoin ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
