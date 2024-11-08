#include "thread.h"
#include "encryption.h"

SSL_CTX *CreateContext(bool isServer) {
    const SSL_METHOD *method;

    if(isServer)
        method = TLS_server_method();
    else
        method = TLS_client_method();
    
    SSL_CTX *tmpctx = SSL_CTX_new(method);
    if(!tmpctx) {
        perror("Unable to create SSL context.\n");
        exit(EXIT_FAILURE);
    }

    return tmpctx;
}

void ConfigureServerContext(SSL_CTX *tmpctx) {
    // Set the key and cert
    if (SSL_CTX_use_certificate_file(tmpctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) 
        exit(EXIT_FAILURE);
    if (SSL_CTX_use_PrivateKey_file(tmpctx, "key.pem", SSL_FILETYPE_PEM) <= 0 )
        exit(EXIT_FAILURE);
}

void ConfigureClientContext(SSL_CTX *tmpctx) {
    SSL_CTX_set_verify(tmpctx, SSL_VERIFY_PEER, NULL);

    if (!SSL_CTX_load_verify_locations(tmpctx, "cert.pem", NULL))
        exit(EXIT_FAILURE);

}