#ifndef CONNECTION_H
#define CONNECTION_H

class Connection {
public:
	Connection();
	Connection(int sock);
	
	int getSocket() const;
	void destroy();
	
	bool operator==(int sock);
	bool operator==(const Connection &connection);
	bool operator!=(int sock);
	
private:
	int sock_;
};

#endif