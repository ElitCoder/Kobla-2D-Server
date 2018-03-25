#ifndef HANDLE_H
#define HANDLE_H

#define HPUPDATE 100
#define MANAUPDATE 101
#define ENERGYUPDATE 102

#include <vector>

#include "Main.h"
#include "Timer.h"

class PartyMember
{
public:
	PartyMember()
	{
		this->mb = NULL;

		this->autoKick = 0;
		this->online = true;
		this->setTimer = false;

		strcpy_s(this->name, "");
	}

	Client *mb;

	long autoKick;
	bool online;
	bool setTimer;

	char name[20];
};

class Party
{
public:
	Party()
	{
		this->level = 1;
		this->id = 0;

		this->exp = 0;
	}

	int level;
	int id;

	long exp;

	std::vector<PartyMember*> members;
};

class NoWalkZone
{
public:
	NoWalkZone()
	{
		this->xFrom = 0;
		this->xTo = 0;
		this->yFrom = 0;
		this->yTo = 0;
	}

	int xFrom;
	int xTo;
	int yFrom;
	int yTo;
};

class TeleporterZone
{
public:
	TeleporterZone()
	{
		this->xFrom = 0;
		this->xTo = 0;
		this->yFrom = 0;
		this->yTo = 0;

		this->toMap = 0;
		this->toMapX = 0;
		this->toMapY = 0;
	}

	int xFrom;
	int xTo;
	int yFrom;
	int yTo;

	int toMap;
	int toMapX;
	int toMapY;
};

struct PKAREA
{
	int minX;
	int maxX;

	int minY;
	int maxY;
};


class Map
{
public:
	Map()
	{
		this->pk = false;
		this->weaponAllowed = true;

		this->id = 0;

		this->xSize = 0;
		this->ySize = 0;

		this->noUse = false;

		this->resX = 0;
		this->resY = 0;

		this->aggX = 1337;
		this->aggY = 1337;
	}

	bool pk;
	bool weaponAllowed;

	int id;

	int xSize;
	int ySize;

	bool noUse;

	int resX;
	int resY;

	std::vector<NoWalkZone> nowalk;
	std::vector<TeleporterZone> tzone;
	std::vector<PKAREA> pkArea;

	int aggX;
	int aggY;
};

class MenuButton
{
public:
	MenuButton()
	{
		strcpy_s(this->text, "");
	}

	char text[80];
};

class TeleportObject
{
public:
	TeleportObject()
	{
		this->id = 0;
		this->price = 0;
		this->world = "";

		this->x = 0;
		this->y = 0;
	}

	int id;
	int price;
	std::string world;

	int x;
	int y;
};

class NPC
{
public:
	NPC()
	{
		this->x = 0;
		this->y = 0;
		this->mapId = NULL;

		this->id = 0;

		strcpy_s(this->dialog, "");
		strcpy_s(this->name, "");

		this->nextChat = 0;
		this->enableChat = false;

		this->currChat = 0;

		this->meW = 64;
		this->meH = 64;

		this->mapGuard = 0;
	}

	int x;
	int y;
	Map *mapId;

	int id;

	char dialog[1000];
	char name[100];

	std::vector<MenuButton> menu;
	std::vector<MenuButton> diab;
	std::vector<MenuButton> chat;

	std::vector<int> sellingItems;
	std::vector<TeleportObject> tps;

	long nextChat;
	bool enableChat;

	int currChat;

	int meW;
	int meH;

	int mapGuard;
};

class MonsterDrop
{
public:
	MonsterDrop()
	{
		this->chance = 0;

		this->itemid = 0;
		this->qty = 1;
	}

	float chance;

	int itemid;
	int qty;
};

class Buff
{
public:
	Buff()
	{
		this->endTime = 0;
		this->nextUse = 0;

		this->effect = 0;
		this->value = 0;

		this->changeEffect = 0;

		this->userPut = 0;
	}

	long endTime;
	long nextUse;

	int effect;
	int value;

	int changeEffect;

	int userPut;
};

class MonsterBuff
{
public:
	MonsterBuff()
	{
		this->dur = 0;

		this->effect = 0;
		this->value = 0;

		this->chance = 0;
	}

	int dur;

	int effect;
	int value;

	float chance;
};

class Monster
{
public:
	Monster()
	{
		this->id = 0;
		this->pid = 0;
		
		this->hp = 0;
		this->chp = 0;

		this->critChance = 10;

		this->attacked = false;
		this->moving = false;

		this->x = 0;
		this->y = 0;

		this->nextMove = 0;

		this->goDir = 0;

		this->expamount = 0;
		this->attack = 0;

		this->mapId = NULL;

		this->respawnTime = (GetTickCount() + 30000);
		this->respawnDelayMonster = 14000;
		this->spawned = true;

		this->nextRefresh = (GetTickCount() + 3000);
		this->freeAttack = 0;
		this->nextAttack = 0;

		this->attackerId = 0;

		this->minY = 0;
		this->maxY = 0;
		this->minX = 0;
		this->maxX = 0;

		this->followId = 0;
		this->follows = false;

		this->goAfter = 0;

		this->bx = 0;
		this->by = 0;

		this->move = Timer();

		this->monW = 0;
		this->monH = 0;

		this->aggro = false;
		this->aggChance = 0;

		this->speed = 250;

		this->noMove = false;

		this->moveStartX = 0;
		this->moveStartY = 0;

		this->removeOnDeath = false;
		this->moveDone = true;
	}

	void Drop();
	bool GoBack();
	void Follow();
	bool InsidePlayer(Player *p);
	bool WalkBack();
	void Smite();
	bool ValidArea();

	void AddMonsterBuff(int BUFF, int LVL, int DURSEC, int userMade);
	void RemoveMonsterBuff(int pos);

	int GetMonsterBonus(int type);
	Buff *GetMonsterBuff(int BUFF);

	void RemoveAllMonsterBuffs();

	int id;
	int pid;

	float hp;
	float chp;

	bool attacked;
	int attackerId;

	int followId;
	bool follows;
	
	float x;
	float y;

	int goDir;
	long nextMove;
	bool moving;

	int level;

	int expamount;
	int attack;

	Map *mapId;

	long respawnTime;
	long respawnDelayMonster;
	bool spawned;

	long nextRefresh;
	long freeAttack;
	long nextAttack;

	int minY;
	int maxY;
	int minX;
	int maxX;

	std::vector<MonsterDrop> drops;
	std::vector<MonsterBuff> buffs;
	std::vector<Buff*> mbuffs;

	int critChance;

	bool goAfter;

	float bx;
	float by;

	Timer move;

	int monW;
	int monH;

	bool aggro;
	int aggChance;

	int speed;

	bool noMove;

	float moveStartX;
	float moveStartY;

	bool removeOnDeath;
	bool moveDone;
};

class Skill
{
public:
	Skill()
	{
		this->id = 0;
		this->type = 0;

		this->lvl = 0;
		this->exp = 0;

		this->nextUsage = 0;

		this->useWeap = false;

		this->job = 1;
		this->cost = ENERGYUPDATE;
		this->costamount = 50;

		this->special = false;
		this->animation = false;
		this->shoot = false;
		this->aoe = false;
		this->aoerange = 0;
		this->basedmg = 0;
		this->dmglvl = false;
		this->basedOn = STRENGTH;
		this->expamount = 2;
		this->scalebasedmg = false;
		this->multbasedmg = 1;
		this->scalebasetype = 1;
	}

	int id;
	int type;

	int lvl;
	long exp;

	long nextUsage;

	bool useWeap;

	int job, cost, costamount;

	std::vector<SkillAddBuff> addbuffs;

	bool special;
	bool animation;
	bool shoot;
	bool aoe;
	int aoerange;
	int basedmg;
	bool dmglvl;
	int basedOn;
	int expamount;
	bool scalebasedmg;
	int multbasedmg;
	int scalebasetype;
};

struct SkillName
{
	int id;

	std::string name;
};

DWORD WINAPI PLAYERHANDLE(LPVOID lpParam);

int Random(int lowest, int highest);
float RandomFloat(float a, float b);

unsigned int getTime(bool fast);

#endif