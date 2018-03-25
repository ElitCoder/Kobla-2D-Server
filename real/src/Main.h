#ifndef MAIN_H
#define MAIN_H

#define MAX_LEN 2048

#define INFO 1
#define LOAD 3
#define ERR 4
#define DEBUG 5 
#define NETWORK 7
#define SEND 8
#define RECV 9
#define LOGIN 10
#define GAME 11

#define NORMAL 50
#define GM 51
#define ADMIN 52

#define AUTOSAVE 80
#define QUITSAVE 81

#define SLOWSPEED 60
#define POISON 62

#define SPEED 30
#define STAMINA 31
#define STRENGTH 32
#define SPEEDMINUS 33
#define HPDRAIN 34
#define AGILITY 35
#define CRIT 36
#define TOTALDMG 37
#define DEFENCE 38
#define HPREGEN 39
#define HPPLUS 40
#define LIFESTEAL 41
#define ATTACK 42
#define BLOCK 43
#define HPPROCENTPLUS 44
#define HPPROCENTMINUS 45
#define HPMINUS 46
#define IGENOM 47
#define INTT 48
#define MANAPLUS 49
#define MANAMINUS 50
#define MANAPROCENTPLUS 51
#define MANAPROCENTMINUS 52
#define ATTACKSPEED 53

#define INVENTORYITEM 90
#define SKILL 91

#include <vector>
#include <string>

#include "Packet.h"
#include "World.h"
#include "Timer.h"
#include "Connection.h"

extern int defSpeed;
extern long onlineCheckDelay;

extern int mPlayerStartX;
extern int mPlayerStartY;
extern int mPlayerStartMap;

extern int defCritChance;

class Map;
class Party;
class Buff;
class Item;
class Skill;
class Player;
class Monster;

unsigned long long getTimestamp();

class Partner
{
public:
	Partner()
	{
		this->id = 0;
		this->name = "";
		this->status = false;
		this->level = 1;
		this->job = 0;

		this->c = NULL;
	}

	int id;
	std::string name;
	bool status;
	int level;
	int job;

	Client *c;
};

class Command
{
public:
	Command()
	{
		this->c = NULL;
		this->p = NULL;
	}

	void setClient(Client *c)
	{
		this->c = c;
	}

	Client *getClient()
	{
		return this->c;
	}

	void setPlayer(Player *p)
	{
		this->p = p;
	}

	Player *getPlayer()
	{
		return this->p;
	}

	bool checkInfo();

	void sendUpdate(int up);

	void setAttackTimer(int del);
	void setFoodCoolDown(int del);
	void setPillCoolDown(int del);
	void setBulletCoolDown(int del);
	void setPosCoolDown(int del);
	void setChatCoolDown(int del);
	void setExtraCoolDown(int del);

private:
	Client *c;
	Player *p;
};

class TaskItem
{
public:
	TaskItem()
	{
		this->enable = false;
		this->eqinv = false;

		this->type = 0;
		this->slot = 0;
		this->id = 0;
	}

	bool enable;
	bool eqinv;

	int type;
	int slot;
	int id;
};

class Player
{
public:
	Player()
	{
		this->level = 0;
		this->power = 2;

		this->slot = 1;

		this->critChance = defCritChance;

		name = "";
		owner = "";
		//strcpy_s(this->name, "");
		//strcpy_s(this->owner, "");

		this->x = (float)mPlayerStartX;
		this->y = (float)mPlayerStartY;

		this->cMove = 0;
		this->id = 0;
		this->mapId = GetMap(mPlayerStartMap);
		this->pt = NULL;

		this->nextRefresh = 0;
		this->nextSave = 0;

		this->hp = 0;
		this->chp = 0;

		this->speed = defSpeed;

		this->dead = false;
		this->reviveTime = 0;

		this->equipped = false;
		this->moving = false;

		this->exp = 0;
		this->magic = 2;

		this->powerpoints = 0;
		this->pkpoints = 0;

		this->weaponWay = RIGHT;

		this->access = NORMAL;

		for(int i = 0; i < 25; i++)
			inv[i] = NULL;

		for(int i = 0; i < 4; i++)
			eq[i] = NULL;

		this->nextItemPick = 0;

		this->gold = 0;

		this->move = Timer();

		this->meW = 64;
		this->meH = 64;
		this->wW = 21;
		this->wH = 11;

		this->godMode = false;

		for(int i = 0; i < 9; i++)
			task[i] = new TaskItem();

		this->botActive = false;

		this->agility = 2;

		this->followId = 0;
		this->follow = false;

		this->trading = false;
		this->tradeWith = 0;
		this->doneTrade = false;

		this->tradePenya = 0;

		this->attacked = false;
		this->atkTimer = 0;

		this->foodCoolDown = 0;
		this->pillCoolDown = 0;
		this->bulletCoolDown = 0;
		this->posCoolDown = 0;
		this->chatCoolDown = 0;
		this->extraCoolDown = 0;

		this->intt = 2;

		this->mana = 0.0f;
		this->maxmana = 0.0f;

		this->energy = 0.0f;
		this->maxenergy = 0.0f;

		this->job = 1;

		this->bulletTimeStamp = 0;
	}

	int level;
	int power;
	int slot;

	int critChance;

	long exp;
	int magic;

	int powerpoints;
	int pkpoints;

	int agility;
	int intt;

	std::string name;
	std::string owner;
	
	//char name[20];
	//char owner[20];

	Map *mapId;
	Party *pt;

	long nextRefresh;
	long nextSave;

	int speed;

	bool dead;
	long reviveTime;

	float hp;
	float chp;

	long nextItemPick;

	float x;
	float y;

	int cMove;

	bool equipped;
	bool moving;
	int weaponWay;

	int access;

	int getId();
	void setId(int id);

	std::vector<Buff*> buffs;
	std::vector<Skill*> skills;
	
	Item *inv[25];
	Item *eq[4];
	TaskItem *task[9];

	int gold;

	Timer move;

	int meW;
	int meH;
	int wW;
	int wH;

	bool godMode;

	bool botActive;

	int followId;
	bool follow;

	bool trading;
	int tradeWith;
	bool doneTrade;

	std::vector<TwoInt> tradeInventory;
	int tradePenya;

	bool attacked;
	long atkTimer;

	int foodCoolDown;
	int pillCoolDown;
	int bulletCoolDown;
	int posCoolDown;
	int chatCoolDown;
	int extraCoolDown;

	std::vector<Partner> friends;

	float mana;
	float maxmana;

	float energy;
	float maxenergy;

	int job;

	long bulletTimeStamp;

private:
	int id;
};

class Client
{
public:
	Client()
	{
		this->online = false;
		this->ingame = false;
		this->username = "";
		this->password = "";

		this->loginTries = 0;
		this->nextLogin = getTimestamp();

		this->onlineCheck = true;
		this->nextOnlineCheck = (getTimestamp() + onlineCheckDelay);

		this->gone = false;
		this->isc = false;

		this->onlineTries = 0;

		this->disconnect = false;

		this->p = NULL;

		this->com = new Command();
		this->com->setClient(this);

		this->validateClient = false;

		this->registerTries = 0;
		this->nextRegister = 0;

		this->lastPing = 0;

		this->donecsp = true;

		this->cp = CurrentPacket();
		this->csp = SendPacket();
	}

	~Client()
	{
		delete this->com;

		connection_.destroy();
		//closesocket(this->sock);
	}

	void AddPacket(Packet pak, long delay);
	bool Send();

	void SetIP(char *_ip);
	char *GetIP();

	void setConnection(const Connection& connection);
	Connection& getConnection();
	const Connection& getConnection() const;
	
	//void SetSocket(SOCKET s);
	//SOCKET GetSocket();

	Command *com;

	bool online;
	bool ingame;
	Player *p;
	std::string username;
	std::string password;

	void STARTversion();

	void Login(char *b, int len);
	void Register(char *b, int len);
	void Logout();

	void CharacterList();
	std::vector<Player*> GetCharacterVector();
	Player *GetThisPlayer(int id);
	int GetFirstFreeCharacterSlot();
	bool SlotFree(int slot);
	void EditCharacter(int b, char *paks);

	void Move(char *b);
	void StopMove(char *b);
	void UpdatePosition(char *p);

	void ChangeMap(char *b, bool serversided, int map, int x, int y);
	void PlayerPosition(float x, float y, int param);

	void Spawn(char *b);
	void SpawnForMap(int map);
	void Unspawn(int param);

	void SavePlayer(int PARAM);

	void MonsterAttack(int id);
	void Attack(int id, int type, int param, int hits, char *b, bool serv, int plus, bool poss, int based, bool lifesteal);
	void PKAttack(int id, int type, int param, int hits, int plus, bool poss, int based, bool lifesteal);

	void IsOnline(bool status);
	void Quit();

	void Chat(char *msg);

	void Shoot(char *b, bool ss, int sId, int target, bool allowSkill, int type);
	void EquipWeapon(bool art);

	void AddExp(long amount);
	void RemoveExp(int procent);
	float CalculateExp();

	void AddStat(char *b);

	void AcceptInvite(char *b);
	void SendInvite(const std::string& name_string);
	void InviteToParty(char *name);
	void LeaveParty();
	void KickFromParty(char *name);

	void SendFriendInvite(int charId, const char *name);
	void AcceptFriendInvite(char *b);
	void Friend(int id);
	void RemoveFriend(const char *name);

	void AddBuff(int BUFF, int LVL, int DURSEC, int PLAYERPUT);
	void RemoveBuff(int pos);
	void RemoveAllBuffs();

	bool BuffExist(int BUFF);
	Buff *GetBuff(int BUFF);
	int GetBonus(int type);

	void HitNPCMenuButton(char *b);
	void HitNPCDialogButton(char *b);

	void SpecialAttack(int id, int type, int param);

	bool AttackInParty(int mobId);

	bool InArea(int buttonx, int buttony, int buttonw, int buttonh);

	int GetFirstInventorySlot(int id, int amount);
	int GetFirstNotFullSlot(int id);
	int GetFirstCleanSlot();
	bool AddItem(int id, int amount, bool precv, Drop *it, int &slotAddedTo);
	void RemoveItem(int slot, int qty, bool tradeAdd, int itemid);
	void MoveItem(int slot, int toslot);
	bool UseItem(int slot);

	bool PickUpDrop(int id);

	void RemoveAllFollowers();
	void RemoveFollower(int id);

	void UseSkill(int id);
	void SkillAttackAoE(std::vector<Monster*> *vec, int baseDamage, int skillId, int basedOn);
	void SkillAttackAoEPK(std::vector<Player*> *vec, int baseDamage, int skillId, int basedOn);

	void OptimizeBuffs();
	void GetBuffsOnline();

	void RemoveTaskbarItem(int id);
	void AddTaskbarItem(char *b);
	void UseTaskbarItem(int puh);

	Skill *GetSkill(int id);
	void AddSkillExp(int id, long amount);

	void SetLevel(int level);

	void DropInventoryItem(char *b);
	bool UnEquip(int slot);

	int GetEquipBonus(int TYPE);

	void Ping(char *b);

	void ShopCartAdd(char *b);
	void ShopCartRemove(char *b);
	void ShopCartBuy(char *b);
	void ShopCartSell(int place, int qty, int itemid);

	bool CheckInventorySpace(std::vector<TwoInt> its);
	void CheckSkill(int id);

	void ClientErrorMessage(char *b);

	int GetAttackDelay();

	void PlayerMenu(char *b);

	bool NotInsidePKPlayer(Player *m);

	void SendTradeInvite(int charId);
	void AcceptTradeInvite(char *b);

	void Trade(int id);
	void CancelTrade(bool error);
	void AddTradeItem(char *b);
	void AddTradePenya(int pen);
	void DoneTrade(int s);
	void FinishTrade(Client *tar);

	void AddKillExp(long am, long toparty);

	bool PvPArea();

	void Teleport(char *b);

	int GetInventoryStack(int id);
	void UpdateTaskbar(int p, int slot);

	void AddHP(float hp);
	void AddPartyExp(long am);

	void AddMana(float hp);
	void AddEnergy(float hp);

	void SetSkillCoolDown(int id);
	void SetFoodCoolDown(int id, long cool);

	void UpdateFriends(bool on);

	void FriendMenu(char *b);

	void SendPM(const char *name, const char *msg);

	void CheckISC(char *b);
	void GetInfo();
	void GetTopTen();
	void GetTopTenPvP();
	void GetCharactersInfo();
	void GetAccountsInfo();

	int loginTries;
	long nextLogin;

	int registerTries;
	long nextRegister;

	bool gone;
	bool onlineCheck;
	long nextOnlineCheck;
	bool disconnect;

	bool isc;

	int onlineTries;
	int lastPing;

	bool validateClient;

	std::vector<SendPacket> sps;
	CurrentPacket cp;

	SendPacket csp;
	bool donecsp;

private:
	char ip[16];

	Connection connection_;
	//SOCKET sock;
};

void packetmain();

void log(int logmsg, const std::string& message, ...);
void Config(int &p);
void Uptime();

void ProcessPacket(Client *p, unsigned char *b2, int len);

void QuitKobla();

void LD(int logmsg, const std::string& message, ...);

#endif
