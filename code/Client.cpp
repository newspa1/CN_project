#include "client.h"
#include "encryption.h"

SSL *ssl;
char chunkBuffer[CHUNK_SIZE];

int main() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);

    if(connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) >= 0) {
        isLogin = false;
        cout << "Connect to server\n";
    } else {
        cout << "Connection failed\n";
        exit(0);
    }
    
    SSL_CTX *ctx = CreateContext(false);
    ConfigureClientContext(ctx);
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, clientSocket);

    if(SSL_connect(ssl) != 1) {
        cout << "SSL connection to server failed\n\n";
    } else {
        cout << "SSL connection to server success\n\n";

        char response[BUFFER_SIZE];
        while(true) {
            memset(response, 0, sizeof(response));
            SSL_read(ssl, response, sizeof(response));
	        ResponseHandling(string(response));
        }
    }

    close(clientSocket);
}

void ResponseHandling(string response) {
    string component;
	stringstream ss(response);
	vector<string> arguments;

	while(getline(ss, component, ' ')) { // Handle Command line
		arguments.push_back(component);
	}

    string command = arguments[0];
    if(command == "Logout") {
        Logout();
    } else if(command == "Registration") {
        Registration();
    } else if(command == "Login") {
        Login();
    } else if(command == "Message") {
        MessageCommand();
    } else if(command == "List") {
        ListFilesInDirectory(arguments);
    } else if(command == "TransferFile") {
        string filename = arguments[1];
        TransferFileCommand(filename);
    } else if(command == "Message_Receive") {
        string sender = arguments[1];
        MessageReceive(sender); 
    } else if(command == "File_Receive") {
        string filename = arguments[1];
        string sender = arguments[2];
        FileReceive(filename, sender); 
    } else { // Nothing
        if(isLogin) {
            cout << Username << ":~$(registration/logout/message/ls/transfer) > ";
        } else {
            cout << "Please enter the command (registration/login/logout): ";
        }
        string message;
        getline(cin, message);
        SSL_write(ssl, message.c_str(), message.size());
    }
}

void ListFilesInDirectory(vector<string>& arguments) {
    for(int i = 1; i < arguments.size(); i++) {
        cout << "> " << arguments[i] << '\n';
    }
    NotifyServer();
}

void TransferFileCommand(string filename) {
    ifstream file;
    file.open(HOMEDIR + Username + '/' + filename, ios::binary);

    // Check file exists or not
    bool exist = true;
    if(!file.is_open()) {
        exist = false;
        SSL_write(ssl, &exist, sizeof(exist));
        cout << "Unknown file: " << filename << "\n\n";
        return;
    }
    SSL_write(ssl, &exist, sizeof(exist));

    // Read and send file to server
    while(file.read(chunkBuffer, sizeof(chunkBuffer)) || file.gcount() > 0) {
        int bytesSend = SSL_write(ssl, chunkBuffer, file.gcount());
        if(bytesSend <= 0)
            break;
    }
    
    // Send end of file signal
    const char *endSignal = "EOF";
    int bytesSend = SSL_write(ssl, endSignal, strlen(endSignal));

    file.close();
    cout << "Complete transfer\n";
}

void Registration() {
    string message;
    cout << "Please Enter Username: ";
    getline(cin, message);
    SSL_write(ssl, message.c_str(), message.size());

    bool status;
    SSL_read(ssl, &status, sizeof(status));

    if(status) { // Success
        cout << "Registration success.\n";
        NotifyServer();
    } else { // Failed
        cout << "Registration failed.\n";
        NotifyServer();
    }

}

void Login() {
    string username;
    cout << "Please Enter Username: ";
    getline(cin, username);
    SSL_write(ssl, username.c_str(), username.size());
    
    bool status;
    SSL_read(ssl, &status, sizeof(status));

    if(status) { // Success
        cout << "Login success.\n";
        isLogin = true;
        Username = username;
        NotifyServer();
    } else { // Failed
        cout << "Login failed.\n";
        NotifyServer();
    }
}

void Logout() {
	if(!isLogin) { // Exit from process
		exit(0);
	}

    string message;
    cout << "Logout.\n";
    isLogin = false;
    Username = "";
    NotifyServer();
}

void MessageCommand() {
    NotifyServer();

    bool status;
    SSL_read(ssl, &status, sizeof(status));

    if(!status) { // Failed
        cout << "Unknown User or User is offline.\n";
        NotifyServer();
        return;
    } 

    string message;
    cout << "Please Enter the Message you want to send: ";
    getline(cin, message);
    SSL_write(ssl, message.c_str(), message.size()); // Send Message
    SSL_read(ssl, &status, sizeof(status));

    cout << "Succefully send message\n";
    NotifyServer();
}

void MessageReceive(string sender) {
    NotifyServer();
    char message[BUFFER_SIZE] = {0};
    SSL_read(ssl, message, sizeof(message));

    cout << "\n====Message====\n";
    cout << "Sender: " << sender << "\n";
    cout << "=================\n";
    cout <<  message << "\n";
    cout << "=================\n\n";

    NotifyServer();
}

void FileReceive(string filename, string sender) {
    NotifyServer();

    // Save file in home/{username}/filename
    ofstream receivefile;
    receivefile.open(HOMEDIR + Username + "/" + filename, ios::binary);

    int bytesReceive;
	cout << "File transfer start\n";
	while(true) {
		bytesReceive = SSL_read(ssl, chunkBuffer, sizeof(chunkBuffer));
		
		// Receive end of file signal
		if(bytesReceive == 3 && strncmp(chunkBuffer, "EOF", 3) == 0)
			break;
		
		receivefile.write(chunkBuffer, bytesReceive);
	}
	receivefile.close();
	cout << "File transfer success\n";
}

void NotifyServer() {
	bool tmp = true;
	SSL_write(ssl, &tmp, sizeof(tmp));
}
