#ifndef THREAD_H
#define THREAD_H

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "encryption.h"

#define PORT 8081
#define THREAD_POOL_SIZE 10
#define BUFFER_SIZE 1024
#define USERFILE "user.txt"
#define TRANSFER_MESSAGE 1
#define TRANSFER_FILE 2
#define CHUNK_SIZE 2*1024*1024

#define SERVERDIR "serverdir/"
#define HOMEDIR "home/"
#define TMPDIR "serverdir/tmp/"
#define TMPFILE "tmpfile"

using namespace std;

struct ClientData {
    int clientSocket;
    SSL *clientssl;
    bool isLogin;
    string username;
    int receiveType;
    string messageSender;
    string message;
};

// Thread
void InitializeThreadPool();
void EnqueueClient(int ClientSocket, SSL *tmpssl);
void *HandleClient(void *arg);
void *WorkerThread(void *arg);


// HandleClient
void MessageHandling(string message, ClientData *clientdata);
void Registration(string message, ClientData *clientdata);
void Login(string message, ClientData *clientdata);
void Logout(ClientData *clientdata);

void MessageCommand2(vector<string> arguments, ClientData *clientdata);
void MessageCommand(vector<string> arguements, ClientData *clientdata);
pair<string, ClientData*> MessageCommandSender(string username, ClientData *clientdata);
void ReceiveHandler(ClientData *clientdata);

void ReceiveNotification(ClientData *clientdata);
void CommandComplete(ClientData *clientdata);

void TransferCommand(vector<string> arguments, ClientData *clientdata);

void AddUser(string message);
bool isUserExist(string message);

#endif // THREAD_H