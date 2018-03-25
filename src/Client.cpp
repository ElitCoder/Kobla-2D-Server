#include "Main.h"

using namespace std;

extern SOCKET server;

extern std::vector<Client*> ac;

extern bool debugPacket;
extern bool debugs;

void Client::AddPacket(Packet pak, long delay)
{
	try
	{
		if(!this->disconnect)
		{
			unsigned char *datasend = new unsigned char[pak.getPos() + 30];
			memset(datasend, 0, pak.getPos() + 30);

			string lengd = IntToString(pak.getPos());

			int setpos = 0;
			datasend[0] = lengd.length();
			setpos++;

			for(unsigned int i = 0; i < lengd.length(); i++)
			{
				datasend[(i + 1)] = lengd.c_str()[i];
				setpos++;
			}

			memcpy(datasend + setpos, pak.getPacket(), pak.getPos());

			int fulllen = (pak.getPos() + setpos);

			SendPacket sp = SendPacket();

			sp.bu = datasend;
			sp.len = fulllen;
			sp.cp = 0;

			this->sps.push_back(sp);

			if(debugPacket)
			{
				log(SEND, "[%d] ", fulllen);

				for(int i = 0; i < fulllen; i++)
					printf("%02X ", datasend[i]);

				printf("\n");
			}
		}
	}

	catch(std::exception e)
	{
		log(ERR, "[void Client::AddPacket(Packet,long)] [Exception caught] [%s]\n", e.what());

		this->disconnect = true;
	}
}

void Client::SetIP(char *_ip)
{
	strcpy_s(this->ip, _ip);
}

char *Client::GetIP()
{
	return this->ip;
}

void Client::SetSocket(SOCKET s)
{
	this->sock = s;
}

SOCKET Client::GetSocket()
{
	return this->sock;
}

void Player::setId(int id)
{
	this->id = id;
}

int Player::getId()
{
	return this->id;
}
