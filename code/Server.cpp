#include "thread.h"
#include "encryption.h"

unordered_map<string, ClientData*> ClientOnlineMap; // Map to hold online clients  
char chunkBuffer[CHUNK_SIZE];

int main() {
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	
	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	listen(serverSocket, 10);
	cout << "Server is listening on port: " << PORT << "\n";

	InitializeThreadPool();

	SSL_CTX *ctx = CreateContext(true);
	ConfigureServerContext(ctx);

	while(true) {
		int clientSocket = accept(serverSocket, nullptr, nullptr);
		if(clientSocket >= 0) {
			SSL *ssl = SSL_new(ctx);
			SSL_set_fd(ssl, clientSocket);

			if(SSL_accept(ssl) <= 0) {
				exit(0);
			}

			cout << "Client connected, fd = " << clientSocket << "\n";
			EnqueueClient(clientSocket, ssl);
		}
	} 

	close(serverSocket);
	SSL_CTX_free(ctx);
	return 0;
}


void MessageHandling(string message, ClientData *clientdata) {
	string component;
	stringstream ss(message);
	vector<string> arguments;
	while(getline(ss, component, ' ')) { // Handle Command line
		arguments.push_back(component);
	}

	if(arguments.size() == 0) {
		string type = "Nothing";
		SSL_write(clientdata->clientssl, type.c_str(), type.size());
	} else {
		string command = arguments[0];
		cout << "command = " << command << "====\n";
		if(command == "registration") {
			Registration(message, clientdata);
		} else if(command == "login") {
			Login(message, clientdata);
		} else if(command == "logout") {
			Logout(clientdata);
		} else if(command == "message") {
			MessageCommand(arguments, clientdata);
		} else if(command == "transfer") {
			TransferCommand(arguments, clientdata);
			string type = "Nothing";
			SSL_write(clientdata->clientssl, type.c_str(), type.size());
		} else {
			ReceiveHandler(clientdata);
			string type = "Nothing";
			SSL_write(clientdata->clientssl, type.c_str(), type.size());
		}
	}
}

void TransferCommand(vector<string> arguments, ClientData *clientdata) {
	string type;
	if(!clientdata->isLogin) {
		type = "Nothing";
		SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}

	// Handle arguments
	if(arguments.size() < 3) {
		type = "Too few arguments";
		SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}

	// Transfer file
	if(arguments[2] == "--file") {
		string username = arguments[1];
		string filename = arguments[3];
		type = "TransferFile " + filename;
		SSL_write(clientdata->clientssl, type.c_str(), type.size());

		bool exist;
		SSL_read(clientdata->clientssl, &exist, sizeof(exist));
		// Check file exists or not, recipient is legal or not
		if(!exist && !(username == "--server") && !(username == clientdata->username 
				|| ClientOnlineMap.find(username) == ClientOnlineMap.end()))
			return;

		// Read file from sender and save in "tmp/" or "serverdir/"
		ofstream tmpfile;
		if(username == "--server")
			tmpfile.open(SERVERDIR + filename, ios::binary);
		else
			tmpfile.open(TMPDIR + filename, ios::binary);

		int bytesReceive;
		cout << "File transfer start\n";
		while(true) {
			bytesReceive = SSL_read(clientdata->clientssl, chunkBuffer, sizeof(chunkBuffer));
			
			// Receive end of file signal
			if(bytesReceive == 3 && strncmp(chunkBuffer, "EOF", 3) == 0)
				break;
			
			tmpfile.write(chunkBuffer, bytesReceive);
		}

		tmpfile.close();
		cout << "File transfer success\n";

		if(username == "--server")
			return;
		
		// Set receiver status
		ClientData *recipient = ClientOnlineMap[username];
		recipient->receiveType = TRANSFER_FILE;
		recipient->messageSender = clientdata->username;
		recipient->message = filename;
	}

}

void Registration(string message, ClientData *clientdata) {
	if(clientdata->isLogin) {  
		string type = "Nothing";
    	SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}

	string type = "Registration"; // Recieve Registration (message = Registration)
    SSL_write(clientdata->clientssl, type.c_str(), type.size());
	
	char username[BUFFER_SIZE] = {0};
	SSL_read(clientdata->clientssl, username, sizeof(username)); // Receive client username (message = username)
	
	if(!isUserExist(string(username))) {
		AddUser(string(username));
		cout << "Registreation success. Username: " << username << "\n";

		bool status = true;
		SSL_write(clientdata->clientssl, &status, sizeof(status));

		CommandComplete(clientdata);
	} else {
		bool status = false;
    	SSL_write(clientdata->clientssl, &status, sizeof(status));

		CommandComplete(clientdata);
	}
}

void Login(string message, ClientData *clientdata) {
	if(clientdata->isLogin) {
		string type = "Nothing";
    	SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}

	string type = "Login"; // Receive Login command
    SSL_write(clientdata->clientssl, type.c_str(), type.size()); 

	char username[BUFFER_SIZE] = {0};
	SSL_read(clientdata->clientssl, username, sizeof(username)); // Receive client username (message = username)

	if(isUserExist(string(username))) {
		cout << "Client Login success. Username: " << username << "\n";
		clientdata->isLogin = true;
		clientdata->username = string(username);
		ClientOnlineMap[clientdata->username] = clientdata; // Add clients to Online status

		bool status = true;
		SSL_write(clientdata->clientssl, &status, sizeof(status));

		CommandComplete(clientdata);
	} else {
		bool status = false;
		SSL_write(clientdata->clientssl, &status, sizeof(status));

		CommandComplete(clientdata);
	}
}

void Logout(ClientData *clientdata) {
	if(!clientdata->isLogin) {
		string type = "Nothing";
    	SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}

	ClientOnlineMap.erase(clientdata->username); // Remove clients' online status
	clientdata->username = "";
	clientdata->isLogin = false;

	string type = "Logout";
    SSL_write(clientdata->clientssl, type.c_str(), type.size());
}

void MessageCommand(vector<string> arguments, ClientData *clientdata) {
	if(!clientdata->isLogin) {
		string type = "Nothing";
		SSL_write(clientdata->clientssl, type.c_str(), type.size());
		return;
	}
	
	string message;
	ClientData *recipient;
	tie(message, recipient) = MessageCommandSender(arguments[1], clientdata);

	if(recipient != nullptr) {
		recipient->receiveType = 1;
		recipient->messageSender = clientdata->username;
		recipient->message = message;
	}
	
	CommandComplete(clientdata);
}

pair<string, ClientData*> MessageCommandSender(string username, ClientData *clientdata) {
	string type = "Message";
	SSL_write(clientdata->clientssl, type.c_str(), type.size());

	ReceiveNotification(clientdata);
	ClientData *recipient;
	bool status = true;
	if(username == clientdata->username || ClientOnlineMap.find(username) == ClientOnlineMap.end()) { // Message failed
		cout << "Message failed\n";
		status = false;
		SSL_write(clientdata->clientssl, &status, sizeof(status)); // Message Failed
		return make_pair("", nullptr);
	}
	recipient = ClientOnlineMap[username];
	SSL_write(clientdata->clientssl, &status, sizeof(status)); // Message success

	char message[BUFFER_SIZE] = {0};
	SSL_read(clientdata->clientssl, message, sizeof(message)); // Receive Message

	SSL_write(clientdata->clientssl, &status, sizeof(status));
	return make_pair(string(message), recipient);
}

void ReceiveHandler(ClientData *clientdata) {
	int receivetype = clientdata->receiveType;
	if(receivetype == TRANSFER_MESSAGE) { // Receive message
		string type = "Message_Receive " + clientdata->username;
		SSL_write(clientdata->clientssl, type.c_str(), type.size());

		// Send message
		ReceiveNotification(clientdata);
		SSL_write(clientdata->clientssl, clientdata->message.c_str(), clientdata->message.size());

		clientdata->receiveType = 0;
		clientdata->messageSender = "";
		clientdata->message = "";
		ReceiveNotification(clientdata);
	} else if(receivetype == TRANSFER_FILE) { // Receive file
		string type = "File_Receive " + clientdata->message + " " + clientdata->messageSender;
		SSL_write(clientdata->clientssl, type.c_str(), type.size());

		// Transfer file from "serverdir/tmp/""
    	ifstream file;
    	file.open(TMPDIR + clientdata->message, ios::binary);
		while(file.read(chunkBuffer, sizeof(chunkBuffer)) || file.gcount() > 0) {
			int bytesSend = SSL_write(clientdata->clientssl, chunkBuffer, file.gcount());
			if(bytesSend <= 0)
				break;
		}

		// Send end of file signal
		const char *endSignal = "EOF";
		int bytesSend = SSL_write(clientdata->clientssl, endSignal, strlen(endSignal));
		file.close();
		cout << "Complete transfer\n";
		// Remove tmpfile
		remove((TMPDIR + clientdata->message).c_str());

		clientdata->receiveType = 0;
		clientdata->messageSender = "";
		clientdata->message = "";
		ReceiveNotification(clientdata);
	}
}

void ReceiveNotification(ClientData *clientdata) {
	bool tmp;
	SSL_read(clientdata->clientssl, &tmp, sizeof(tmp));
}



void AddUser(string message) {
	ofstream userfile;
	userfile.open(USERFILE, ios::app);
	userfile << message + '\n';
	userfile.close();

	// Create homedir
	mkdir((HOMEDIR + message).c_str(), 0777);
}

void CommandComplete(ClientData *clientdata) {
	ReceiveNotification(clientdata);

	ReceiveHandler(clientdata);
	string type = "Nothing";
	SSL_write(clientdata->clientssl, type.c_str(), type.size());
}

bool isUserExist(string message) {
	ifstream userfile(USERFILE);
	string line;
	while(getline(userfile, line)) {
		istringstream iss(line);
		string username;
		iss >> username;

		if(username == message) { // User exists
			userfile.close();
			return true;
		}
	}
	userfile.close();
	return false;
}