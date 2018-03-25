#include <string>
#include <sys/types.h>
#include <sys/socket.h>

#include "Main.h"
#include "BufferCheck.h"

using namespace std;

extern int version;

extern int forum;
extern char *temp;

extern bool debugs;

void Client::STARTversion()
{
	this->validateClient = true;

	Packet pak = Packet();

	pak.addHeader(0x1);
	pak.addInt(version);

	pak.ready();
	this->AddPacket(pak, 0);

	if(debugs)
		log(DEBUG, "::STARTversion() function sent.\n");
}

void Client::Login(char *b, int len)
{
	if(this->online)
		return;

	if(len >= 6)
	{
		if((unsigned)this->nextLogin <= getTimestamp())
		{
			if(this->loginTries == 5)
				this->loginTries = 0;

			int pos = 3;

			string acc = getString(b, (pos + 1), (int)b[pos]);;
			pos += ((int)b[pos] + 1);
			string pass = getString(b, (pos + 1), (int)b[pos]);

			string newpass = acc;
			newpass += pass;

			if(acc.length() > 0 && acc.length() < 20 && newpass.length() > 0 && newpass.length() < 40)
			{
				Packet f = Packet();

				f.changeHead(0x11);
				f.addHeader(0x22);
				f.addString((char*)acc.c_str());
				f.addString((char*)newpass.c_str());
				
				f.ready();

				send(forum, (const char*)f.getPacket(), f.getPos(), 0);
				recv(forum, temp, sizeof(temp), 0);

				Packet pak = Packet();

				pak.addHeader(0x2);
				pak.addInt(temp[3]);
				pak.addLongInt(0);

				pak.ready();
				this->AddPacket(pak, 0);

				if(temp[3] == 1)
				{
					this->online = true;
					this->username = acc;
					this->password = pass;

					PlayerLog("[Login] [%s] [%s] [Accepted]\n", this->username.c_str(), this->password.c_str());

					if(debugs)
						log(DEBUG, "Player logged in: %s.\n", this->username.c_str());

					LD(LOGIN, "Login: [%s] [%s].\n", this->username.c_str(), this->password.c_str());
				}

				else
				{
					this->loginTries++;

					if(this->loginTries >= 5)
						this->nextLogin = (getTimestamp() + 15000);

					switch(temp[3])
					{
						case 2: PlayerLog("[Login] [%s] [%s] [Wrong username]\n", acc.c_str(), pass.c_str());
							break;

						case 3: PlayerLog("[Login] [%s] [%s] [Wrong password]\n", acc.c_str(), pass.c_str());
							break;

						case 4: PlayerLog("[Login] [%s] [%s] [Already connected]\n", acc.c_str(), pass.c_str());
							break;

						case 6: PlayerLog("[Login] [%s] [%s] [Account is banned]\n", acc.c_str(), pass.c_str());
							break;
					}
				}
			}

			else
				log(ERR, "[void Client::Login(char*,int)] [Short data] [%s] [%s [%d] [%d]\n", acc.c_str(), newpass.c_str(), acc.length(), newpass.length());
		}

		else
		{
			Packet pak = Packet();

			pak.addHeader(0x2);
			pak.addInt(7);
			pak.addLongInt((this->nextLogin - getTimestamp()));

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}

	else
		log(ERR, "[void Client::Login(char*,int)] [Short packet] [%d]\n", len);
}

void Client::Register(char *b, int len)
{
	if(this->online)
		return;

	if(len >= 6)
	{
		if((unsigned)this->nextRegister <= getTimestamp())
		{
			if(this->registerTries == 5)
				this->registerTries = 0;

			int pos = 3;

			string acc = getString(b, (pos + 1), (int)b[pos]);
			pos += ((int)b[pos] + 1);
			string pass = getString(b, (pos + 1), (int)b[pos]);

			string newpass = acc;
			newpass += pass;

			if(acc.length() > 0 && acc.length() < 20 && newpass.length() > 0 && newpass.length() < 40)
			{
				Packet f = Packet();

				f.changeHead(0x11);
				f.addHeader(0x2);
				f.addString((char*)acc.c_str());
				f.addString((char*)newpass.c_str());

				f.ready();

				send(forum, (const char*)f.getPacket(), f.getPos(), 0);
				recv(forum, temp, sizeof(temp), 0);

				Packet pak = Packet();

				pak.addHeader(0x3);
				pak.addInt(temp[3]);

				pak.ready();
				this->AddPacket(pak, 0);

				if(temp[3] != 1)
				{
					this->registerTries++;

					if(this->registerTries >= 5)
						this->nextRegister = (getTimestamp() + 15000);
				}

				switch(temp[3])
				{
					case 1: PlayerLog("[Register] [%s] [%s] [Accepted]\n", acc.c_str(), pass.c_str());
						break;

					case 2: PlayerLog("[Register] [%s] [%s] [Username already taken]\n", acc.c_str(), pass.c_str());
						break;

					case 3: PlayerLog("[Register] [%s] [%s] [Not allowed letters in name]\n", acc.c_str(), pass.c_str());
						break;
				}
			}
			
			else
				log(ERR, "[void Client::Register(char*,int)] [Short data] [%s] [%s] [%d] [%d]\n", acc.c_str(), newpass.c_str(), acc.length(), newpass.length());

			if(debugs)
				log(DEBUG, "Register packet.\n");
		}

		else
		{
			Packet pak = Packet();

			pak.addHeader(0x3);
			pak.addInt(7);
			pak.addLongInt((this->nextRegister - getTimestamp()));

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}

	else
		log(ERR, "[void Client::Register(char*,int)] [Short packet] [%d]\n", len);
}

void Client::Logout()
{
	this->online = false;

	Packet f = Packet();

	f.changeHead(0x11);
	f.addHeader(0x24);
	f.addString((char*)this->username.c_str());

	f.ready();

	send(forum, (const char*)f.getPacket(), f.getPos(), 0);

	if(debugs)
		log(DEBUG, "Player logged out.\n");

	LD(LOGIN, "Logged out: [%s] [%s].\n", this->username.c_str(), this->password.c_str());
}