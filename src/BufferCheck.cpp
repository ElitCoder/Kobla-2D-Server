#include <iostream>
#include <string>
#include <fstream>
#include <time.h>

#include "Main.h"
#include "BufferCheck.h"

extern Client *currentClient;

bool PacketBufferCheck(char *b, int len)
{
	if(len >= 3)
	{
		if(b[0] == 0x3)
		{
			if(b[2] == 0x2 || b[2] == 0x3)
			{
				if(len <= 6)
					return false;

				if(b[3] >= 20 || b[3] <= 0)
					return false;

				std::string getAccount = getString(b, 4, (int)b[3]);

				if(getAccount.length() >= 20)
					return false;

				int pos = ((int)b[3] + 4);

				if(len <= (pos + 1))
					return false;

				if(b[pos] >= 20 || b[pos] <= 0)
					return false;

				std::string getPassword = getString(b, (pos + 1), (int)b[pos]);

				if(getPassword.length() >= 20)
					return false;
			}

			else if(b[2] == 0x59)
			{
				if(len <= 4)
					return false;

				if(b[3] > 9 || b[3] <= 0)
					return false;
			}
		}

		else if(b[0] == 0x4)
		{
		}

		else
			return false;
	}

	else
		return false;

	return true;
}

void ErrorLog(char *message, ...)
{ 
	va_list args;
	char buf[1024];

	va_start(args, message);
	vsnprintf_s(buf, 1024, 1024, message, args);
	va_end(args);

	std::string ip = "127.0.0.1";
	std::string username = "NULL";
	std::string playername = "NULL";

	bool doPlayerName = false;
	bool doUserName = false;

	if(currentClient != NULL)
	{
		ip = currentClient->GetIP();

		if(currentClient->online)
		{
			doUserName = true;
			username = currentClient->username;
			
			if(currentClient->ingame)
			{
				doPlayerName = true;
				playername = currentClient->p->name;
			}
		}
	}

	time_t t = time(0);
	tm *lt = new tm();
	int res = localtime_s(lt, &t);

	char dates[4096];

	if(doUserName)
	{
		if(doPlayerName)
			sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] [U: %s] [P: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str(), username.c_str(), playername.c_str());

		else
			sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] [U: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str(), username.c_str());	
	}

	else
		sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str());	

	std::ofstream openMe;

	openMe.open("Error.txt", std::fstream::app);
	if(!openMe)
		log(DEBUG, "Could not open error log file.\n");

	else
	{
		openMe << dates;
		openMe << buf;
	}

	delete lt;

	openMe.close();
}

void PlayerLog(char *message, ...)
{ 
	va_list args;
	char buf[1024];

	va_start(args, message);
	vsnprintf_s(buf, 1024, 1024, message, args);
	va_end(args);

	std::string ip = "127.0.0.1";
	std::string username = "NULL";
	std::string playername = "NULL";

	bool doPlayerName = false;
	bool doUserName = false;

	if(currentClient != NULL)
	{
		ip = currentClient->GetIP();

		if(currentClient->online)
		{
			doUserName = true;
			username = currentClient->username;
			
			if(currentClient->ingame)
			{
				doPlayerName = true;
				playername = currentClient->p->name;
			}
		}
	}

	time_t t = time(0);
	tm *lt = new tm();
	int res = localtime_s(lt, &t);

	char dates[4096];

	if(doUserName)
	{
		if(doPlayerName)
			sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] [U: %s] [P: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str(), username.c_str(), playername.c_str());

		else
			sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] [U: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str(), username.c_str());	
	}

	else
		sprintf_s(dates, "[%02d/%02d/%04d - %02d:%02d:%02d] [IP: %s] ", lt->tm_mday, (lt->tm_mon + 1), (lt->tm_year + 1900), lt->tm_hour, lt->tm_min, lt->tm_sec, ip.c_str());	

	std::ofstream openMe;

	openMe.open("Log.txt", std::fstream::app);
	if(!openMe)
		log(DEBUG, "Could not open error log file.\n");

	else
	{
		openMe << dates;
		openMe << buf;
	}

	delete lt;

	openMe.close();
}