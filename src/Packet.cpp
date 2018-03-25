#include <iostream>
#include <string>

#include "Packet.h"
#include "Handle.h"

extern std::vector<Client*> ac;

void Packet::addHeader(int header)
{
	if(this->packetlen <= this->pos)
		this->newBuffer(3);

	this->packet[2] = (unsigned char)header;
}

void Packet::newBuffer(int musthave)
{
	unsigned char *newb = new unsigned char[(this->pos + 15001 + musthave)];
	memset(newb, 0, this->pos + 15001 + musthave);
	memcpy(newb, this->packet, this->pos);
	delete[] this->packet;
	this->packet = newb;
	this->packetlen = (this->pos + 15000 + musthave);
}

void Packet::addInt(int what)
{
	if(this->packetlen <= this->pos)
		this->newBuffer(1);

	this->packet[this->pos] = (unsigned char)what;
	this->pos++;
}

void Packet::addString(char add[])
{
	int length = strlen(add);

	if((unsigned)this->packetlen <= ((this->pos + strlen(add) + 1)))
		this->newBuffer((strlen(add) + 1));

	this->packet[this->pos] = (unsigned char)length;
	this->pos++;

	for(unsigned int i = 0; i < strlen(add); i++)
	{
		this->packet[this->pos] = add[i];
		this->pos++;
	}
}

void Packet::addLongString(char add[])
{
	int strleng = strlen(add);
	char buf[20] = "";

	_itoa_s(strleng, buf, 10);
	this->addString(buf);

	if((unsigned)this->packetlen <= ((this->pos + strlen(add))))
		this->newBuffer(strlen(add));

	for(int i = 0; i < strleng; i++)
	{
		this->packet[this->pos] = add[i];
		this->pos++;
	}
}

void Packet::addLongInt(int add)
{
	char buf[20] = "";

	_itoa_s(add, buf, 10);
	this->addString(buf);
}

void Packet::addFloat(float add)
{
	char buf[20] = "";

	sprintf_s(buf, 20, "%2.2f", add);
	this->addString(buf);
}

void Packet::addBool(bool a)
{
	if(a)
		this->addInt(1);
	else
		this->addInt(0);
}

std::string getString(char *packet, int pos, int strleng)
{
	if(strleng > 0 && strleng < 61440)
	{
		char *temp = new char[(strleng + 1)];
		std::string out = "";

		memset(temp, 0, (strleng + 1));

		for(int i = 0; i < strleng; i++)
			temp[i] = packet[(pos + i)];

		out = temp;

		delete[] temp;

		return out;
	}

	return "0";
}

void Packet::ready()
{
	if(mainhead == 0x3 || mainhead == 0x4)
	{
	}

	else
	{
		unsigned char *temp2 = new unsigned char[(this->pos + 3)];

		temp2[0] = (unsigned char)this->mainhead;
		temp2[1] = (this->pos + 1);

		memcpy(temp2 + 2, this->packet + 2, (this->pos - 2));

		delete[] this->packet;
		this->packet = temp2;
		this->pos += 1;
	}
}

void Packet::changeHead(int newhead)
{
	this->mainhead = newhead;
}

void SendAllOnline(Packet pak, long delay)
{
	try
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			if(ac.at(i)->ingame)
				ac.at(i)->AddPacket(pak, delay);
		}
	}

	catch(...)
	{
		log(ERR, "[void SendAllOnline(Packet,long)] [Exception caught]\n");
	}
}

void SendAllInParty(Packet pak, Party *pt, Map *map, SOCKET sock)
{
	bool doTo = true;
	bool doMap = true;

	if(sock == 0)
		doTo = false;

	if(map == NULL)
		doMap = false;

	if(doMap)
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			Client *c = ac.at(i);

			if(c->ingame)
			{
				if(c->p->pt != NULL)
				{
					if(c->p->pt == pt)
					{
						if(c->p->mapId != NULL)
						{
							if(map->id == c->p->mapId->id)
							{
								if(doTo)
								{
									if(sock != c->GetSocket())
										c->AddPacket(pak, 0);
								}

								else
									c->AddPacket(pak, 0);
							}
						}
					}
				}
			}
		}
	}

	else
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			Client *c = ac.at(i);

			if(c->ingame)
			{
				if(c->p->pt != NULL)
				{
					if(c->p->pt == pt)
					{
						if(doTo)
						{
							if(sock != c->GetSocket())
								c->AddPacket(pak, 0);
						}

						else
							c->AddPacket(pak, 0);
					}
				}
			}
		}
	}
}

void SendAllOnMap(Packet pak, long delay, int mId, SOCKET sock)
{
	bool doTo = true;

	if(sock == 0)
		doTo = false;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		Client *c = ac.at(i);

		if(c->ingame)
		{
			if(c->p != NULL)
			{
				if(c->p->mapId != NULL)
				{
					if(mId == c->p->mapId->id)
					{
						if(doTo)
						{
							if(sock != c->GetSocket())
								c->AddPacket(pak, delay);
						}

						else
							c->AddPacket(pak, delay);
					}
				}
			}
		}
	}
}

int Packet::getPos()
{
	return this->pos;
}

unsigned char *Packet::getPacket()
{
	return this->packet;
}

std::string IntToString(int intt)
{
	char itoado[20] = "";
	_itoa_s(intt, itoado, 20, 10);

	std::string retVal = itoado;
	return retVal;
}

std::string FloatToString(float floatt)
{
	char buf[20] = "";
	sprintf_s(buf, 20, "%2.2f", floatt);

	std::string retVal = buf;
	return retVal;
}