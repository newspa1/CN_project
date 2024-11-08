run:
	g++ Server.cpp thread.cpp encryption.cpp -o server -lssl -lcrypto
	g++ Client.cpp encryption.cpp -o client -lssl -lcrypto


clean:
	rm server
	rm client

