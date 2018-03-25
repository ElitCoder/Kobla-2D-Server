#ifndef CONNECTION_H
#define CONNECTION_H

class Connection {
public:
	Connection();
	Connection(int sock);
	
	int getSocket();
	void destroy();
	
private:
	int sock_;
};

#endif