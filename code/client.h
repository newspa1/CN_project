#ifndef CLIENT_H
#define CLIETN_H

#include <bits/stdc++.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8081
#define THREAD_POOL_SIZE 10
#define BUFFER_SIZE 1024
#define CHUNK_SIZE 2*1024*1024
#define HOMEDIR "home/"

using namespace std;
namespace fs = std::filesystem;

int clientSocket;
bool isLogin;
string Username;

void ResponseHandling(string response);
void Registration();
void Login();
void Logout();
void ListFilesInDirectory(vector<string>& arguments);
void MessageCommand();
void TransferFileCommand(string filename);

void MessageReceive(string sender);
void FileReceive(string filename, string sender);

void NotifyServer();

#endif // CLIENT_H