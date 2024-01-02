#ifndef KOIOS_SSL_PTR_H
#define KOIOS_SSL_PTR_H

#include "koios/macros.h"
#include "koios/ssl_deleter.h"

#include <memory>

KOIOS_NAMESPACE_BEG

using BIO_uptr                      = ::std::unique_ptr< BIO,                    ssl_detials::openssl_deleter>;
using X509_uptr                     = ::std::unique_ptr< X509,                   ssl_detials::openssl_deleter>;
using X509_STORE_uptr               = ::std::unique_ptr< X509_STORE,             ssl_detials::openssl_deleter>;
using X509_STORE_CTX_uptr           = ::std::unique_ptr< X509_STORE_CTX,         ssl_detials::openssl_deleter>;
using X509_VERIFY_PARAM_uptr        = ::std::unique_ptr< X509_VERIFY_PARAM,      ssl_detials::openssl_deleter>;
using GENERAL_NAME_uptr             = ::std::unique_ptr< GENERAL_NAME,           ssl_detials::openssl_deleter>;
using EVP_PKEY_uptr                 = ::std::unique_ptr< EVP_PKEY,               ssl_detials::openssl_deleter>;
using EVP_MD_CTX_uptr               = ::std::unique_ptr< EVP_MD_CTX,             ssl_detials::openssl_deleter>;
using STACK_OF_X509_uptr            = ::std::unique_ptr< STACK_OF(X509),         ssl_detials::openssl_deleter>;
using STACK_OF_GENERAL_NAME_uptr    = ::std::unique_ptr< STACK_OF(GENERAL_NAME), ssl_detials::openssl_deleter>;

KOIOS_NAMESPACE_END

#endif
