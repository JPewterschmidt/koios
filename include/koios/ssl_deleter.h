#ifndef KOIOS_SSL_DELETER_H
#define KOIOS_SSL_DELETER_H

#include "koios/macros.h"

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>

namespace koios::ssl_detials
{
    class openssl_deleter
    {
    public:   
        void operator()(BIO* p)                 const noexcept { ::BIO_free(p);                 }
        void operator()(X509* p)                const noexcept { ::X509_free(p);                }
        void operator()(X509_STORE* p)          const noexcept { ::X509_STORE_free(p);          }
        void operator()(X509_STORE_CTX* p)      const noexcept { ::X509_STORE_CTX_free(p);      }
        void operator()(X509_VERIFY_PARAM* p)   const noexcept { ::X509_VERIFY_PARAM_free(p);   }
        void operator()(EVP_PKEY* p)            const noexcept { ::EVP_PKEY_free(p);            }
        void operator()(EVP_MD_CTX* p)          const noexcept { ::EVP_MD_CTX_free(p);          }
        void operator()(GENERAL_NAME* p)        const noexcept { ::GENERAL_NAME_free(p);        }

        void operator()(STACK_OF(X509)* st) const noexcept 
        {
            for (int i{}; i < sk_X509_num(st); ++i)
            {
                ::X509_free(sk_X509_value(st, i));
            }
            sk_X509_free(st);
        }

        void operator()(STACK_OF(GENERAL_NAME)* names) const noexcept
        {
            for (int i{}; i < sk_GENERAL_NAME_num(names); ++i)
            {
                ::GENERAL_NAME_free(sk_GENERAL_NAME_value(names, i));
            }
            sk_GENERAL_NAME_free(names);
        }
    };
}

#endif
