#include "Connection.h"

#include <unistd.h>

Connection::Connection() {
	sock_ = -1;
}

Connection::Connection(int sock) {
	sock_ = sock;
}

int Connection::getSocket() {
	return sock_;
}

void Connection::destroy() {
	close(sock_);
}