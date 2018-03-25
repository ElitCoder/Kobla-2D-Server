#include <iostream>
#include <vector>

#include "Packet.h"
#include "Main.h"

extern std::vector<Client*> ac;
extern std::vector<Player*> pvector;

extern std::string PasswordISC;

extern SOCKET forum;
extern char *temp;

extern bool debugs;

void Client::CheckISC(char *b)
{
	std::string pass = getString(b, 4, (int)b[3]);

	if(debugs)
		log(DEBUG, "ISC validation, password: %s %d %s %d %d.\n", PasswordISC.c_str(), PasswordISC.length(), pass.c_str(), pass.length(), (int)b[3]);

	if(strcmp(PasswordISC.c_str(), pass.c_str()) == 0)
	{
		this->isc = true;

		if(debugs)
		{
			if(this->isc)
				log(DEBUG, "ISC validation completed.\n");

			else
				log(DEBUG, "ISC validation failed.\n");
		}
	}
}
void Client::GetInfo()
{
	if(this->isc)
	{
		if(debugs)
			log(DEBUG, "Returns info to website.\n");

		int online = 0;

		for(unsigned int i = 0; i < ac.size(); i++)
		{
			if(ac.at(i)->ingame)
				online++;
		}

		Packet pak = Packet();

		pak.changeHead(0x4);
		pak.addHeader(0x2);
		pak.addLongInt(online);

		pak.ready();
		this->AddPacket(pak, 0);

		if(debugs)
			log(DEBUG, "Players online: %d.\n", online);
	}
}

void Client::GetCharactersInfo()
{
	if(this->isc)
	{
		Packet pak = Packet();

		pak.changeHead(0x4);
		pak.addHeader(0x5);
		pak.addLongInt(pvector.size());

		pak.ready();
		this->AddPacket(pak, 0);
	}
}

void Client::GetAccountsInfo()
{
	if(this->isc)
	{
		Packet fpak = Packet();

		fpak.changeHead(0x11);
		fpak.addHeader(0x31);

		fpak.ready();
		send(forum, (const char*)fpak.getPacket(), fpak.getPos(), 0);
		recv(forum, temp, 2048, 0);

		int accounts = atoi(getString(temp, 4, (int)temp[3]).c_str());

		Packet pak = Packet();

		pak.changeHead(0x4);
		pak.addHeader(0x6);
		pak.addLongInt(accounts);

		pak.ready();
		this->AddPacket(pak, 0);
	}
}

void Client::GetTopTen()
{
	if(this->isc)
	{
		std::vector<Player*> fp;

		for(unsigned int i = 0; i < pvector.size(); i++)
		{
			if(pvector.at(i)->access <= 50)
			{
				if(fp.size() > 0)
				{
					bool added = false;

					for(unsigned int y = 0; y < fp.size(); y++)
					{
						if(pvector.at(i)->level > fp.at(y)->level)
						{
							fp.insert((fp.begin() + y), pvector.at(i));

							added = true;

							break;
						}
					}

					if(!added)
						fp.push_back(pvector.at(i));
				}

				else
					fp.push_back(pvector.at(i));
			}
		}

		Packet pak = Packet();

		pak.changeHead(0x4);
		pak.addHeader(0x3);

		if(fp.size() < 10)
		{
			pak.addInt(fp.size());

			for(unsigned int i = 0; i < fp.size(); i++)
			{
				pak.addString(fp.at(i)->name);
				pak.addLongInt(fp.at(i)->level);
			}
		}

		else
		{
			pak.addInt(10);

			for(int i = 0; i < 10; i++)
			{
				pak.addString(fp.at(i)->name);
				pak.addLongInt(fp.at(i)->level);
			}
		}

		pak.ready();
		this->AddPacket(pak, 0);

		fp.clear();
	}
}

void Client::GetTopTenPvP()
{
	if(this->isc)
	{
		std::vector<Player*> fp;

		for(unsigned int i = 0; i < pvector.size(); i++)
		{
			if(pvector.at(i)->access <= 50)
			{
				if(fp.size() > 0)
				{
					bool added = false;

					for(unsigned int y = 0; y < fp.size(); y++)
					{
						if(pvector.at(i)->pkpoints > fp.at(y)->pkpoints)
						{
							fp.insert((fp.begin() + y), pvector.at(i));

							added = true;

							break;
						}
					}

					if(!added)
						fp.push_back(pvector.at(i));
				}

				else
					fp.push_back(pvector.at(i));
			}
		}

		Packet pak = Packet();

		pak.changeHead(0x4);
		pak.addHeader(0x3);

		if(fp.size() < 10)
		{
			pak.addInt(fp.size());

			for(unsigned int i = 0; i < fp.size(); i++)
			{
				pak.addString(fp.at(i)->name);
				pak.addLongInt(fp.at(i)->pkpoints);
			}
		}

		else
		{
			pak.addInt(10);

			for(int i = 0; i < 10; i++)
			{
				pak.addString(fp.at(i)->name);
				pak.addLongInt(fp.at(i)->pkpoints);
			}
		}

		pak.ready();
		this->AddPacket(pak, 0);

		fp.clear();
	}
}