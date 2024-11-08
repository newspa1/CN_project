#include "thread.h"
#include "encryption.h"

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;

vector<pthread_t> ThreadPool;
queue<ClientData*> ClientQueue;

void *HandleClient(void *arg) {
    ClientData *clientdata = (ClientData*)arg;

    char message[BUFFER_SIZE];
	string type = "Nothing";
	SSL_write(clientdata->clientssl, type.c_str(), type.size());

	while(true) {
		memset(message, 0, sizeof(message));
		SSL_read(clientdata->clientssl, message, sizeof(message));
		cout << "Recieve message from client: " << message << ", fd = " << clientdata->clientSocket << "\n";
		MessageHandling(string(message), clientdata);
	}
}

void *WorkerThread(void *arg) {
    while(true) {
        pthread_mutex_lock(&queueMutex);

        while(ClientQueue.empty()) { // wait for new client connection
            pthread_cond_wait(&queueCond, &queueMutex);
        }

        ClientData *clientdata = ClientQueue.front();
        ClientQueue.pop();

        pthread_mutex_unlock(&queueMutex);

        HandleClient(clientdata);
        
        close(clientdata->clientSocket);
        delete clientdata;
    }
} 

void EnqueueClient(int ClientSocket, SSL *tmpssl) {
    ClientData *clientdata = new ClientData{ClientSocket, tmpssl, false, "", 0, "", ""};
    pthread_mutex_lock(&queueMutex);
    ClientQueue.push(clientdata);
    pthread_cond_signal(&queueCond); // Remind WorkerThread there is a new client connection 
    pthread_mutex_unlock(&queueMutex);
}

void InitializeThreadPool() {
	for(int i = 0; i < THREAD_POOL_SIZE; i++) {
		pthread_t thread;
		pthread_create(&thread, nullptr, WorkerThread, nullptr);
		ThreadPool.push_back(thread);
	}
}