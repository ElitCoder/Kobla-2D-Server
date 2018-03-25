#include "Connection.h"

#include <unistd.h>

Connection::Connection() {
	sock_ = -1;
}

Connection::Connection(int sock) {
	sock_ = sock;
}

int Connection::getSocket() const {
	return sock_;
}

void Connection::destroy() {
	close(sock_);
}

bool Connection::operator==(int sock) {
	return sock == sock_;
}

bool Connection::operator==(const Connection& connection) {
	return connection.getSocket() == getSocket();
}

bool Connection::operator!=(int sock) {
	return (*this) == sock;
}