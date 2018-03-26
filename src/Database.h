#ifndef DATABASE_H
#define DATABASE_H

#include <string>

class Database {
public:
	Database();
	
	bool checkLogin(const std::string& username, const std::string& password);
	
private:
};

#endif