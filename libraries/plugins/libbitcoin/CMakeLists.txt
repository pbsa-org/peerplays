include(ExternalProject)

set(LIBBITCOIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libbitcoin-system)
set(LIBBITCOIN_BIN ${CMAKE_CURRENT_BINARY_DIR}/libbitcoin)
set(LIBBITCOIN_STATIC_LIB ${LIBBITCOIN_BIN}/lib/libbitcoin-system.a)
set(LIBBITCOIN_INCLUDES ${LIBBITCOIN_DIR}/include)

file(MAKE_DIRECTORY ${LIBBITCOIN_INCLUDES})

ExternalProject_Add(
    bitcoinsystem
    PREFIX ${LIBBITCOIN_BIN}
    SOURCE_DIR ${LIBBITCOIN_DIR}
    DOWNLOAD_COMMAND  cd ${LIBBITCOIN_DIR} && ${LIBBITCOIN_DIR}/autogen.sh
    CONFIGURE_COMMAND ${LIBBITCOIN_DIR}/configure --srcdir=${LIBBITCOIN_DIR} --prefix=${LIBBITCOIN_BIN} --enable-static=yes --disable-shared --with-examples=no
    BUILD_COMMAND make -j4
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS ${LIBBITCOIN_STATIC_LIB}
)

add_library(libbitcoin STATIC IMPORTED GLOBAL)

set_target_properties(bitcoinsystem PROPERTIES IMPORTED_LOCATION ${LIBBITCOIN_STATIC_LIB})
set_target_properties(bitcoinsystem PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBBITCOIN_INCLUDES})

add_dependencies(libbitcoin bitcoinsystem)

set_target_properties(libbitcoin PROPERTIES IMPORTED_LOCATION ${LIBBITCOIN_STATIC_LIB})
set_target_properties(libbitcoin PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBBITCOIN_INCLUDES})
