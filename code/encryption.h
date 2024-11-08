#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <csignal>
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX *CreateContext(bool isServer);
void ConfigureServerContext(SSL_CTX *tmpctx);
void ConfigureClientContext(SSL_CTX *tmpctx);

#endif // ENCRYPTION_H