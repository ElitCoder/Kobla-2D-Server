#ifndef WORLD_H
#define WORLD_H

#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4
#define JUMP 5

#define ALL 100
#define OTHER 250

#define PKATTACKSME 2000
#define PKATTACKS 2001
#define PKKILLED 2002
#define PKKILLEDBY 2003
#define PKALREADYDEAD 2004
#define CUSTOM 2005
#define TELEPORTED 2006
#define PKINFECTPOISON 2007
#define PKSLOWSPEED 2008
#define REVIVE 2009
#define WASPKINFECTPOISON 2010
#define WASPKSLOWSPEED 2011

#define FOOD 4000
#define EQUIPWEAPON 4001
#define EQUIPHAT 4002
#define EQUIPSUIT 4003
#define EQUIPBELT 4004
#define FOODPILL 4005
#define RESTAT 4006

#include "Timer.h"

class Monster;
class NPC;
class Map;
class Player;
class Client;
class Drop;
class Skill;
class Buff;
class Class;

class TwoInt
{
public:
	TwoInt()
	{
		this->int1 = 0;
		this->int2 = 0;
	}

	TwoInt(const TwoInt &c)
	{
		this->int1 = c.int1;
		this->int2 = c.int2;
	}

	int int1;
	int int2;
};

class ThreeInt
{
public:
	ThreeInt()
	{
		this->int1 = 0;
		this->int2 = 0;
		this->int3 = 0;
	}

	int int1;
	int int2;
	int int3;
};

struct FourInt
{
	int int1;
	int int2;
	int int3;
	int int4;
};

class SkillAddBuff
{
public:
	SkillAddBuff()
	{
		this->fi = FourInt();

		this->scaleLvl = false;
		this->scaleMult = 1;
		this->scaleDur = false;
		this->scaleDurMult = 1;
	}

	FourInt fi;

	bool scaleLvl;
	int scaleMult;
	bool scaleDur;
	int scaleDurMult;
};

struct SkillCoolDown
{
	int id;
	long coolDown;
};

float CalculateHP(Client *c);
float CalculateMana(Client *c);
float CalculateEnergy(Client *c);

std::vector<Monster*> GetMonstersForMap(int mapId);
int GetMonsterForMapNumber(int mapId);

int GetPlayersForMapNumber(int mapId, int myId);
Map *GetMap(int id);

std::vector<NPC*> GetNPCForMap(int mapId);

std::vector<Drop*> GetDropsForMap(int mapId);

int PlayersOnline();

Player *GetPlayerById(int id);
Player *GetPlayerByName(const char *name);
Client *GetClientByPlayerId(int id);
Client *GetClientByName(char *name, int &result);
Client *GetClientByPlayerName(char *name);
Monster *GetMonsterById(int id);
Client *GetClientByPlayerIdInMap(int id, int mapid);
Client *GetClientByPlayerIdStatus(int id, int &res);

NPC *GetNPCById(int id);

void SystemChat(Client *_Client, int _Param, Client *_Client2, char *_Custom);

std::vector<Monster*> GetMonstersInRange(int mapid, int x, int y, int w, int h, int xRange, int yRange);
std::vector<Player*> GetPlayersInRange(int mapid, int x, int y, int w, int h, int xRange, int yRange, int self);

Skill *GetMainSkill(int id);

Buff *getBuffData(int PARAM);

int GetMonsterProcentHP(Monster *t);
int GetPlayerProcentHP(Player *p);

std::string GetSkillName(int id);

long GetSkillCooldown(int id);

void MoveToPlayer(Client *c1, Client *c2);

Packet CreatePacketChat(const char *msg, const char *name, bool ispro, bool isnpc, int col);

bool Area(int myx, int myy, int myw, int myh, int buttonx, int buttony, int buttonw, int buttonh);

NPC *GetClosestGuardNPC(int mapid, float x, float y);

int getbulsizex(int pam);
int getbulsizey(int pam);

int GetClosestMover(float x, float y, int mapid, int id, int &type);

void FixFriends();

Class *GetClass(int id);

bool IsSkill(int param);

void CheckSkills(Player *p);
void CheckClassSkills(Player *p);

#endif