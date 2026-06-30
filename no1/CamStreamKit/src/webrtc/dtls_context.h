#pragma once

#include <openssl/ssl.h>

#include <memory>
#include <string>

namespace csk {

class DtlsContext {
public:
    static DtlsContext &instance();

    SSL_CTX *ssl_ctx() const { return ctx_; }
    const std::string &fingerprint() const { return fingerprint_; }

private:
    DtlsContext();
    ~DtlsContext();

    void generate_certificate();
    std::string compute_fingerprint();

    SSL_CTX *ctx_ = nullptr;
    EVP_PKEY *pkey_ = nullptr;
    X509 *cert_ = nullptr;
    std::string fingerprint_;
};

}  // namespace csk
