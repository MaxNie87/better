#include "webrtc/dtls_context.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "core/logger.h"

namespace csk {

DtlsContext &DtlsContext::instance() {
    static DtlsContext ctx;
    return ctx;
}

DtlsContext::DtlsContext() {
    generate_certificate();

    ctx_ = SSL_CTX_new(DTLS_server_method());
    if (!ctx_) throw std::runtime_error("Failed to create SSL_CTX");

    SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       [](int, X509_STORE_CTX *) { return 1; });

    if (SSL_CTX_use_certificate(ctx_, cert_) != 1)
        throw std::runtime_error("SSL_CTX_use_certificate failed");
    if (SSL_CTX_use_PrivateKey(ctx_, pkey_) != 1)
        throw std::runtime_error("SSL_CTX_use_PrivateKey failed");

    SSL_CTX_set_tlsext_use_srtp(ctx_, "SRTP_AES128_CM_SHA1_80");

    fingerprint_ = compute_fingerprint();
    CSK_LOG_I("DTLS fingerprint: {}", fingerprint_);
}

DtlsContext::~DtlsContext() {
    if (ctx_) SSL_CTX_free(ctx_);
    if (cert_) X509_free(cert_);
    if (pkey_) EVP_PKEY_free(pkey_);
}

void DtlsContext::generate_certificate() {
    pkey_ = EVP_PKEY_new();
    RSA *rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    EVP_PKEY_assign_RSA(pkey_, rsa);

    cert_ = X509_new();
    X509_set_version(cert_, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(cert_), 1);
    X509_gmtime_adj(X509_get_notBefore(cert_), 0);
    X509_gmtime_adj(X509_get_notAfter(cert_), 365 * 24 * 3600);
    X509_set_pubkey(cert_, pkey_);

    X509_NAME *name = X509_get_subject_name(cert_);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (unsigned char *)"CamStreamKit", -1, -1, 0);
    X509_set_issuer_name(cert_, name);
    X509_sign(cert_, pkey_, EVP_sha256());
}

std::string DtlsContext::compute_fingerprint() {
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int n = 0;
    X509_digest(cert_, EVP_sha256(), md, &n);

    std::ostringstream ss;
    for (unsigned int i = 0; i < n; i++) {
        if (i > 0) ss << ':';
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)md[i];
    }
    return ss.str();
}

}  // namespace csk
