#ifndef PACKET_H
#define PACKET_H

#include "Connection.h"

#include <iostream>
//#include <winsock2.h>
//#include <windows.h>
#include <vector>
#include <cstring>

class Client;
class Party;
class Map;

std::string getString(char *packet, int pos, int strleng);

std::string IntToString(int intt);
std::string FloatToString(float floatt);

class Fragment
{
public:
	Fragment()
	{
		this->buf = NULL;
		this->len = 0;
	}

	unsigned char *buf;
	int len;
};

class CurrentPacket
{
public:
	CurrentPacket()
	{
		this->totallen = 0;
		this->fullbuf = NULL;
		this->waiting = false;
		this->iwant = 0;
	}

	void eraseme()
	{
		if(this->fullbuf != NULL)
			delete[] this->fullbuf;

		this->fullbuf = NULL;

		this->totallen = 0;

		for(unsigned int i = 0; i < this->frag.size(); i++)
		{
			if(this->frag.at(i).buf != NULL)
				delete[] this->frag.at(i).buf;
		}

		this->frag.clear();
		this->waiting = false;
		this->iwant = 0;
	}

	void compilefrag()
	{
		if(this->fullbuf != NULL)
			delete[] this->fullbuf;

		this->fullbuf = NULL;
		this->totallen = 0;

		if(this->frag.size() > 0)
		{
			for(unsigned int i = 0; i < this->frag.size(); i++)
				this->totallen += this->frag.at(i).len;

			this->fullbuf = new unsigned char[this->totallen];

			int cplace = 0;

			for(unsigned int i = 0; i < this->frag.size(); i++)
			{
				memcpy(this->fullbuf + cplace, this->frag.at(i).buf, this->frag.at(i).len);
				cplace += this->frag.at(i).len;
			}
		}
	}

	int getsize()
	{
		return this->frag.size();
	}

	int totallen;
	unsigned char *fullbuf;
	int iwant;

	std::vector<Fragment> frag;

	bool waiting;
};

class SendPacket
{
public:
	SendPacket()
	{
		this->bu = NULL;
		this->len = 0;
		this->cp = 0;
		this->forward = false;
	}

	unsigned char *bu;
	int len;
	int cp;
	bool forward;
};

class Packet
{
public:
	Packet()
	{
		this->pos = 3;
		this->mainhead = 0x3;
		this->packetlen = 1500;

		this->packet = new unsigned char[(this->packetlen + 1)];

		memset(this->packet, 0, (packetlen + 1));
	}

	Packet(const Packet &c)
	{
		this->packet = new unsigned char[c.pos];

		this->pos = c.pos;
		this->mainhead = c.mainhead;
		this->packetlen = c.packetlen;

		memset(this->packet, 0, c.pos);
		memcpy(this->packet, c.packet, c.pos);
	}

	~Packet()
	{
		delete[] this->packet;
	}

	void changeHead(int newhead);
	void newBuffer(int musthave);

	void addHeader(int header);
	void addInt(int what);
	void addLongInt(int add);
	void addString(const std::string& message);
	void addLongString(const char* add);
	void addBool(bool a);
	void addFloat(float add);

	void ready();

	int getPos();
	unsigned char *getPacket();

private:
	int pos;
	int mainhead;
	unsigned char *packet;
	int packetlen;
};

void SendAllOnline(Packet pak, long delay);
void SendAllOnMap(Packet pak, long delay, int mId, const Connection& connection);
void SendAllInParty(Packet pak, Party *pt, Map *map, const Connection& connection);

#endif