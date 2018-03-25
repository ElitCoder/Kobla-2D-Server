#include <fstream>
#include <iostream>
#include <time.h>
#include <string>

#include "Main.h"
#include "Console.h"
#include "World.h"
#include "Load.h"
#include "Handle.h"
#include "Command.h"
#include "Item.h"
#include "BufferCheck.h"
#include "RenderWorld.h"

namespace col = JadedHoboConsole;

long starttime = 0;

fd_set f_readset, f_writeset, f_exset;
SOCKET server;

SOCKET forum;
char *temp;

char ISCIP[16] = "127.0.0.1";
int ISCPort = 5400;
std::string PasswordISC = "htatestkolka94";

std::vector<Player*> pvector;

bool enableAttack = true;
int maxMaps = 4;

std::vector<Client*> ac;

std::vector<Monster*> mobs;
std::vector<Party*> parties;
std::vector<Drop*> drops;

extern std::vector<NPC*> npcs;

extern bool debugs;
extern bool debugPacket;
extern bool enableClientText;

long nextStatusCheck = 0;
extern long statusDelay;

extern int mPlayerStartMap;
extern int mPlayerStartX;
extern int mPlayerStartY;

long refreshDelay = 2500;
long saveDelay = 300000;
long moveDelay = 5000;
long reviveDelay = 10000;
long monsterRespawnDelay = 30000;
long npcChatDelay = 20000;

extern long monsterAttackDelay;
extern long onlineCheckDelay;

extern WorldCommand *wc;

extern bool enableStatusCheck;

Client *currentClient = NULL;

bool enableLD = true;

int serverError = 0;
int version = 41;

void log(int logmsg, char *message, ...)
{ 
	va_list args;
	char buf[1024];

	va_start(args, message);
	vsnprintf_s(buf, 1024, 1024, message, args);
	va_end(args);

	if(logmsg == 4)
	{
		ErrorLog(buf);

		serverError++;
	}

	switch(logmsg)
	{
		case 0: std::cout << buf;
			break;
		case 1: std::cout << col::fg_green << "[INFO] " << col::fg_white << buf;
			break;
		case 3:	std::cout << col::fg_cyan << "[LOADING] " << col::fg_white << buf;
			break;
		case 5:	std::cout << col::fg_magenta << "[DEBUG] " << col::fg_white << buf;
			break;
		case 7: std::cout << col::fg_yellow << "[NETWORK] " << col::fg_white << buf;
			break;
		case 8: std::cout << col::fg_cyan << "[SEND] " << col::fg_white << buf;
			break;
		case 9: std::cout << col::fg_gray << "[RECV] " << col::fg_white << buf;
			break;
	}
}

void LD(int logmsg, char *message, ...)
{ 
	if(enableLD)
	{
		va_list args;
		char buf[1024];

		va_start(args, message);
		vsnprintf_s(buf, 1024, 1024, message, args);
		va_end(args);

		switch(logmsg)
		{
			case 0: std::cout << buf;
				break;
			case 1: std::cout << col::fg_green << "[INFO] " << col::fg_white << buf;
				break;
			case 3:	std::cout << col::fg_cyan << "[LOADING] " << col::fg_white << buf;
				break;
			case 5:	std::cout << col::fg_magenta << "[DEBUG] " << col::fg_white << buf;
				break;
			case 7: std::cout << col::fg_yellow << "[NETWORK] " << col::fg_white << buf;
				break;
			case 8: std::cout << col::fg_cyan << "[SEND] " << col::fg_white << buf;
				break;
			case 9: std::cout << col::fg_gray << "[RECV] " << col::fg_white << buf;
				break;
			case LOGIN: std::cout << col::fg_cyan << "[LOGIN] " << col::fg_white << buf;
				break;
			case GAME: std::cout << col::fg_cyan << "[GAME] " << col::fg_white << buf;
				break;
		}
	}
}

int ConnectForum()
{
	sockaddr_in server;
	sockaddr addr;

	server.sin_family = AF_INET;
	server.sin_port = htons(ISCPort);
	server.sin_addr.s_addr = inet_addr(ISCIP);

	memcpy(&addr, &server, sizeof(SOCKADDR_IN));

	forum = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(forum == INVALID_SOCKET)
		return 1;

	if(connect(forum, &addr, sizeof(addr)) != 0)
		return 2;

	int tcp = 1;
	setsockopt(forum, IPPROTO_TCP, TCP_NODELAY, (char*)&tcp, sizeof(int));

	Packet f = Packet();

	f.changeHead(0x11);
	f.addHeader(0x25);
	f.addString((char*)PasswordISC.c_str());

	f.ready();

	send(forum, (const char*)f.getPacket(), f.getPos(), 0);
	recv(forum, temp, sizeof(temp), 0);

	if(temp[3] != 1)
		return 3;

	return 0;
}

void main()
{
	WSADATA wsadata;

	struct sockaddr_in addr;

	int result;
	int portn = 2400;

	SetConsoleTitleA("Kobla - 0");

	temp = new char[2048];

	log(INFO, "Starting Kobla Server version 5.1\n");

	log(LOAD, "Socket system.. ");

	Config(portn);

	result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if(result != 0)
		log(ERR, "[void main()] [Unable to start Winsock]\n");

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server == INVALID_SOCKET)
		log(ERR, "[void main()] [Could not create master socket]\n");

	ZeroMemory((char*)&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(portn);

	if(bind(server, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		log(ERR, "[void main()] [Binding failed] [%d]\n", portn);

	if(listen(server, SOMAXCONN) == SOCKET_ERROR)
		log(ERR, "[void main()] [Listening failed]\n");

	log(0, "[done]\n");

	log(LOAD, "Map data.. ");
	log(0, "[done] [%d loaded]\n", LoadMaps());

	log(LOAD, "Item data.. ");
	log(0, "[done] [%d loaded]\n", LoadItems());

	log(LOAD, "Skill data.. ");
	log(0, "[done] [%d loaded]\n", LoadSkills());

	log(LOAD, "Buff data.. ");
	log(0, "[done] [%d loaded]\n", LoadBuffs());

	log(LOAD, "Class data.. ");
	log(0, "[done] [%d] loaded]\n", LoadClasses());

	log(LOAD, "Character data.. ");
	log(0, "[done] [%d loaded]\n", LoadChars());

	log(LOAD, "Monster data.. ");
	log(0, "[done] [%d loaded]\n", LoadMobs());

	log(LOAD, "NPC data.. ");
	log(0, "[done] [%d loaded]\n", LoadNPC());

	log(LOAD, "Extra-spawn data.. ");
	log(0, "[done] [%d loaded]\n", LoadExtraSpawns());

	FixFriends();

	starttime = GetTickCount();
	nextStatusCheck = (starttime + statusDelay);

	log(LOAD, "Connecting to ISC.. ");

	int res = ConnectForum();
	if(res == 0)
		log(0, "[connected]\n");
	else if(res == 1)
		log(0, "[socket error]\n");
	else if(res == 2)
		log(0, "[not connected]\n");
	else if(res == 3)
		log(0, "[wrong isc password]\n");

	srand((unsigned)time(NULL));

	log(INFO, "Kobla Server started.\n");
	log(0, "=======================================================\n");

	atexit(QuitKobla);

	currentClient = NULL;

	packetmain();

	closesocket(server);
	WSACleanup();
}

int HandlePackets(unsigned char *buf, int by, Client *u, int &worldPackets)
{
	unsigned char *bufr;
	int len2 = by;
	bool created = false;

	if(u->cp.getsize() > 0)
	{
		Fragment fr = Fragment();
		fr.buf = new unsigned char[by];
		memcpy(fr.buf, buf, by);
		fr.len = by;

		u->cp.frag.push_back(fr);

		u->cp.compilefrag();

		bufr = new unsigned char[u->cp.totallen];
		memcpy(bufr, u->cp.fullbuf, u->cp.totallen);
		len2 = u->cp.totallen;

		created = true;
	}

	else
	{
		bufr = buf;

		Fragment fr = Fragment();
		fr.buf = new unsigned char[by];
		memcpy(fr.buf, buf, by);
		fr.len = by;

		u->cp.frag.push_back(fr);
	}

	if(len2 < 3)
	{
		u->cp.waiting = true;
		u->cp.iwant = 13;

		if(created)
			delete[] bufr;

		return 1;
	}

	else
	{
		int lenofthis = bufr[0];
		
		if(1 + lenofthis > len2)
		{
			u->cp.waiting = true;
			u->cp.iwant = 13;

			if(created)
			delete[] bufr;

			return 1;
		}

		else
		{
			int pack = atoi(getString((char*)bufr, 1, lenofthis).c_str());
			int fullpacklen = (pack + lenofthis + 1);

			if(len2 < fullpacklen)
			{
				u->cp.waiting = true;
				u->cp.iwant = fullpacklen - len2;

				if(created)
					delete[] bufr;

				return 1;
			}

			if(len2 > fullpacklen)
			{
				int currPos = 0;

				while(len2 > currPos)
				{
					int lenofthis2 = bufr[currPos];
					int pack2 = atoi(getString((char*)bufr, (currPos + 1), lenofthis2).c_str());
					int fullpacklen2 = (pack2 + lenofthis2 + 1);

					if((currPos + fullpacklen2) > len2)
					{
						u->cp.eraseme();
						u->cp.waiting = true;

						Fragment fr2 = Fragment();
						fr2.buf = new unsigned char[len2 - currPos];
						memcpy(fr2.buf, bufr + currPos, len2 - currPos);
						fr2.len = len2 - currPos;

						u->cp.frag.push_back(fr2);

						u->cp.iwant = fullpacklen2 - (len2 - currPos);

						if(created)
							delete[] bufr;

						return 1;
					}

					if(fullpacklen2 > 2)
					{
						ProcessPacket(u, bufr + lenofthis2 + 1 + currPos, pack2);

						if(worldPackets >= 50)
						{
							RenderWorld();
							worldPackets = 0;
						}

						worldPackets++;
					}

					else
						break;

					currPos += fullpacklen2;
				}
			}

			else
			{
				if(fullpacklen == len2)
				{
					ProcessPacket(u, bufr + lenofthis + 1, pack);

					if(worldPackets >= 50)
					{
						RenderWorld();
						worldPackets = 0;
					}

					worldPackets++;
				}
			}
		}
	}

	if(created)
		delete[] bufr;

	return 0;
}

void initfd()
{
	FD_ZERO(&f_readset);
	FD_ZERO(&f_writeset);
	FD_ZERO(&f_exset);

	FD_SET(server, &f_readset);
	FD_SET(server, &f_exset);

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(!ac.at(i)->disconnect)
		{
			FD_SET(ac.at(i)->GetSocket(), &f_readset);
			FD_SET(ac.at(i)->GetSocket(), &f_exset);

			if(ac.at(i)->sps.size() > 0)
				FD_SET(ac.at(i)->GetSocket(), &f_writeset);
		}
	}
}

void packetmain()
{
	int worldPackets = 0;

	unsigned char *buf = new unsigned char[1000000];

	while(true)
	{
		initfd();

		TIMEVAL timeOut;

		timeOut.tv_sec = 0;
		timeOut.tv_usec = 1;

		if(select(0, &f_readset, &f_writeset, &f_exset, &timeOut) > 0)
		{
			if(FD_ISSET(server, &f_readset))
			{
				if(worldPackets >= 50)
				{
					RenderWorld();
					worldPackets = 0;
				}

				u_long nob = 1;
				int tcp = 1;
				sockaddr_in clientaddr;
				int len = sizeof(clientaddr);
				SOCKET clients = accept(server, (sockaddr*)&clientaddr, &len);

				if(strcmp(inet_ntoa(clientaddr.sin_addr), "127.0.0.1") != 0 && enableClientText)
					log(INFO, "Client connected [IP: %s] [SOCKET: %d].\n", inet_ntoa(clientaddr.sin_addr), clients);

				setsockopt(clients, IPPROTO_TCP, TCP_NODELAY, (char*)&tcp, sizeof(int));

				Client *client = new Client();
				client->SetSocket(clients);
				client->SetIP(inet_ntoa(clientaddr.sin_addr));

				ac.push_back(client);

				char buff[10] = "";
				std::string programString = "Kobla - ";

				_itoa_s(ac.size(), buff, 10, 10);

				programString += buff;

				SetConsoleTitleA(programString.c_str());

				worldPackets++;

				currentClient = client;

				if(strcmp(inet_ntoa(clientaddr.sin_addr), "127.0.0.1") != 0)
					PlayerLog("[Client connected] [%d]\n", clients);

				currentClient = NULL;
			}

			if(FD_ISSET(server, &f_exset))
			{
				log(ERR, "[void packetmain()] [Error detected at master socket]\n");

				continue;
			}

			for(unsigned int i = 0; i < ac.size(); i++)
			{
				Client *tryClient = ac.at(i);

				if(!tryClient->disconnect)
				{
					if(FD_ISSET(tryClient->GetSocket(), &f_readset))
					{
						int bufsize = 1000000;

						if(tryClient->cp.waiting)
							bufsize = tryClient->cp.iwant;

						int by = recv(tryClient->GetSocket(), (char*)buf, bufsize, 0);

						if(by == 0 || by == SOCKET_ERROR)
						{
							tryClient->disconnect = true;
							continue;
						}

						if(debugPacket)
						{
							log(RECV, "[%d] ", by);

							for(int i = 0; i < by; i++)
								printf("%02X ", buf[i]);

							printf("\n");
						}

						currentClient = tryClient;
						int ret = HandlePackets(buf, by, tryClient, worldPackets);
						currentClient = NULL;

						if(ret == 0)
							tryClient->cp.eraseme();
					}

					if(FD_ISSET(tryClient->GetSocket(), &f_writeset))
					{
						if(tryClient->donecsp)
						{
							if(tryClient->sps.size() > 0)
							{
								tryClient->csp.bu = tryClient->sps.at(0).bu;
								tryClient->csp.cp = tryClient->sps.at(0).cp;
								tryClient->csp.forward = tryClient->sps.at(0).forward;
								tryClient->csp.len = tryClient->sps.at(0).len;

								tryClient->donecsp = false;
								tryClient->sps.erase(tryClient->sps.begin());
							}
						}

						if(!tryClient->donecsp)
						{
							SendPacket *sp = &tryClient->csp;

							if(sp->len == 0)
							{
								log(DEBUG, "Sp->len == 0\n");
								
								delete[] sp->bu;
								tryClient->donecsp = true;
							}

							else if(sp->cp == sp->len)
							{
								log(DEBUG, "Sp->cp == sp->len\n");
								
								delete[] sp->bu;
								tryClient->donecsp = true;
							}

							else
							{
								int sendlen = (sp->len - sp->cp);

								int s = send(tryClient->GetSocket(), (const char*)sp->bu + sp->cp, sendlen, 0);

								if(worldPackets >= 50)
								{
									RenderWorld();
									worldPackets = 0;
								}

								if(s == 0)
								{
									delete[] sp->bu;
									tryClient->donecsp = true;
									tryClient->disconnect = true;
								}

								else if(s < 0)
								{
									log(DEBUG, "Senddata error %d\n", WSAGetLastError());
									delete[] sp->bu;
									tryClient->donecsp = true;
									tryClient->disconnect = true;
								}

								else if(s > 0)
								{
									sp->cp += s;

									if(sp->cp >= sp->len)
									{
										delete[] sp->bu;
										tryClient->donecsp = true;
									}
								}
							}
						}
					}

					if(FD_ISSET(tryClient->GetSocket(), &f_exset))
					{
						currentClient = tryClient;
						log(ERR, "[void packetmain()] [Client-socket error] [%d]\n", tryClient->GetSocket());
						currentClient = NULL;

						tryClient->disconnect = true;
					}
				}
			}
		}

		RenderWorld();
	}
}

void Uptime(bool eText)
{
	unsigned long upTime = ((GetTickCount() - starttime) / 1000);

    int hours = (upTime / 3600);
    upTime %= 3600;

    int minutes = (upTime / 60);
    int seconds = (upTime % 60);

	log(INFO, "[Uptime: %02d:%02d:%02d] [Players: %d] [Errors: %d].\n", hours, minutes, seconds, PlayersOnline(), serverError);
}

void QuitKobla()
{
	try
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			if(ac.at(i)->online)
				ac.at(i)->Logout();

			delete ac.at(i);
		}

		ac.clear();
	}

	catch(std::exception e)
	{
		return;
	}
}