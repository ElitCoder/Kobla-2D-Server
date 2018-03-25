#include <string>
#include <fstream>

#include "Load.h"
#include "Handle.h"
#include "Item.h"
#include "World.h"
#include "Main.h"
#include "Item.h"

#define strcpy_s strcpy
#define sscanf_s sscanf

using namespace std;

extern vector<Player*> pvector;
extern vector<Monster*> mobs;
extern vector<NPC*> npcs;
extern vector<Map*> maps;
extern vector<Item*> items;
extern vector<Skill*> skills;
extern vector<SkillName> skillName;
extern vector<Buff*> buffdata;
extern vector<SkillCoolDown> skillCooldown;
extern vector<Class*> classes;

extern long saveDelay;
extern long moveDelay;
extern long refreshDelay;
extern long spawnPlayerDelay;
extern long spawnMonsterDelay;
extern long spawnNPCDelay;
extern long reviveDelay;
extern long monsterRespawnDelay;
extern long monsterFreeDelay;
extern long npcChatDelay;
extern long deleteDropDelay;
extern long pickDropDelay;

extern char ISCIP[16];
extern int ISCPort;
extern string PasswordISC;

extern bool enableAttack;
extern int maxMaps;

extern int defSpeed;
int defCritChance = 10;
extern bool debugs;

extern int expRate;
extern int maxLvl;
extern vector<int> expTable;

extern int skillexprate;
extern int skillmaxlvl;
extern int skillexptable[10];

extern int buff;
extern int buffs[10];
extern int buffsLvl[10];
extern int buffTime[10];

extern bool enableKS;
extern bool enableSkill;

extern bool logPacket;

int mPlayerStartX = 0;
int mPlayerStartY = 0;
int mPlayerStartMap = 0;

long onlineCheckDelay = 20000;
long monsterAttackDelay = 2500;

extern int blockChance;

extern int penyaRate;
extern int dropRate;

extern char serverName[100];

extern int playPuhId;

extern bool mPvPEnablePoints;

int monsterId = 1;

bool debugPacket = false;
bool enableClientText = true;
bool enableStatusCheck = false;

long statusDelay = 10000;

int monsterMaxWalkY = 1500;
int monsterMaxWalkX = 1500;

long playerAttackTimer = 5000;
long foodDefCooldown = 700;

bool disconnectWithOnlinePacket = false;

extern int expRateParty;
extern int maxPartyLvl;

extern bool enableLD;

int LoadMobs()
{
	string op = "Data/Monsters/Monster-Index.txt";
	string tchar = "Data/Monsters/";
	string t2 = "Data/Monsters/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadMobs()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadMobs()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				char name[100] = "";

				int level = 0;
				float hp = 0;

				int exp, attack, minX, maxX, minY, maxY, numMobs, map, drops, monW, monH, agg, noMove, respawnDelayMonster;
				char getdrops[100] = "";

				vector<MonsterDrop> sdro;
				vector<MonsterBuff> mbs;

				z.getline(name, 100);
				z >> level;
				z >> hp;
				z >> exp;
				z >> attack;
				z >> minX;
				z >> maxX;
				z >> minY;
				z >> maxY;
				z >> numMobs;
				z >> map;
				z >> agg;

				z >> monW;
				z >> monH;

				z >> drops;

				z.ignore();

				for(int i = 0; i < drops; i++)
				{
					int itemid = 0;
					int qty = 0;
					float chance = 0;

					z.getline(getdrops, 100);

					sscanf(getdrops, "%d %d %f", &itemid, &qty, &chance);

					MonsterDrop md = MonsterDrop();

					md.itemid = itemid;
					md.qty = qty;
					md.chance = chance;

					sdro.push_back(md);
				}

				int params = 0;

				z >> params;

				for(int i = 0; i < params; i++)
				{
					int dur = 0, eff = 0, val = 0;
					float chance = 0;

					z >> dur;
					z >> eff;
					z >> val;
					z >> chance;

					MonsterBuff mb = MonsterBuff();

					mb.dur = dur;
					mb.effect = eff;
					mb.value = val;
					mb.chance = chance;

					mbs.push_back(mb);
				}

				z >> noMove;
				z >> respawnDelayMonster;

				for(int i = 0; i < numMobs; i++)
				{
					int r = Random((moveDelay / 2), (moveDelay * 2));

					int dropornot = 0;

					if(agg > 0)
						dropornot = Random(1, (int)((float)(100 / agg)));
					else
						dropornot = 2;

					Monster *m = new Monster();

					number++;
					m->id = atoi(t);
					m->pid = monsterId;

					monsterId++;

					m->hp = hp;
					m->chp = (m->hp / 2);

					m->mapId = GetMap(map);

					while(true)
					{
						m->x = (float)Random(minX, maxX);
						m->y = (float)Random(minY, maxY);

						if(m->ValidArea())
							break;
					}

					m->nextMove = (getTimestamp() + r);
					m->moving = false;

					m->expamount = exp;
					m->attack = attack;

					m->attacked = false;
					
					m->level = level;

					m->spawned = true;
					m->respawnTime = (getTimestamp() + monsterRespawnDelay);

					m->minY = minY;
					m->maxY = maxY;
					m->minX = minX;
					m->maxX = maxX;
					m->monW = monW;
					m->monH = monH;
					m->aggChance = agg;

					if(dropornot == 1)
						m->aggro = true;

					m->drops = sdro;
					m->buffs = mbs;

					m->respawnDelayMonster = respawnDelayMonster;

					if(respawnDelayMonster == 1337)
						m->respawnDelayMonster = monsterRespawnDelay;
					
					if(noMove == 1)
						m->noMove = true;
					else
						m->noMove = false;

					mobs.push_back(m);
				}
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadChars()
{
	string op = "Data/Character/CharacterIndex.txt";
	string tchar = "Data/Character/";
	string t2 = "Data/Character/";
	ifstream r(op.c_str());

	char t[20];
	char slot[10];
	int number = 0;

	if(!r)
		log(ERR, "[int LoadChars()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			r >> slot;

			t2 += t;
			t2 += slot;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadChars()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				Player *p = new Player();

				int map = mPlayerStartMap;
				int ite = 0;

				z >> p->name;
				z >> p->level;
				z >> p->power;
				z >> p->agility;
				z >> p->x;
				z >> p->y;
				z >> map;
				z >> p->exp;
				z >> p->magic;
				z >> p->powerpoints;
				z >> p->pkpoints;
				z >> p->access;
				z >> ite;

				for(int i = 0; i < ite; i++)
				{
					int id = 0;
					int slot = 0;
					int qty = 0;

					z >> id;
					z >> slot;
					z >> qty;

					if(id != 2)
					{
						Item *stru = GetItemById(id);

						if(stru == NULL)
							log(ERR, "[int LoadChars()] [Could not find item] [%d]\n", id);

						else
						{
							Item *item = new Item();

							*item = *stru;

							item->stack = qty;
							item->id = id;

							p->inv[(slot - 1)] = item;
						}
					}

					else
					{
						p->gold = slot;
					}
				}

				int num = 0;

				z >> num;

				for(int i = 0; i < num; i++)
				{
					int x2 = 0;
					int y2 = 0;
					long z2 = 0;

					z >> x2;
					z >> y2;
					z >> z2;

					Buff *b = new Buff();

					b->effect = x2;
					b->value = y2;
					b->endTime = z2;
					b->nextUse = 0;
					b->changeEffect = getBuffData(b->effect)->changeEffect;

					p->buffs.push_back(b);
				}

				int tskItem = 0;

				z >> tskItem;

				for(int i = 0; i < tskItem; i++)
				{
					int tskSlot = 0;
					int tskType = 0;
					int tskPointSlot = 0;
					int tskEqInv = 0;
					int tskId = 0;

					z >> tskSlot;
					z >> tskType;
					z >> tskPointSlot;
					z >> tskEqInv;
					z >> tskId;

					p->task[(tskSlot - 1)]->slot = tskPointSlot;
					p->task[(tskSlot - 1)]->type = tskType;
					p->task[(tskSlot - 1)]->id = tskId;
					p->task[(tskSlot - 1)]->enable = true;
					
					if(tskEqInv == 1)
						p->task[(tskSlot - 1)]->eqinv = true;
					else
						p->task[(tskSlot - 1)]->eqinv = false;

					if(tskId == 0 && tskType == INVENTORYITEM && tskEqInv != 1)
						p->task[(tskSlot - 1)]->id = p->inv[(tskPointSlot - 1)]->id;
				}

				Client *cOmg = new Client();
				cOmg->p = p;

				cOmg->UpdateTaskbar(0, 0);

				delete cOmg;

				int skill = 0;

				z >> skill;

				for(int i = 0; i < skill; i++)
				{
					Skill *s = new Skill();

					z >> s->id;

					if(s->id == 12 || s->id == 13)
						s->id -= 10;

					Skill *stru = GetMainSkill(s->id);

					*s = *stru;

					z >> s->type;
					z >> s->lvl;
					z >> s->exp;

					p->skills.push_back(s);
				}

				int eqItem = 0;

				z >> eqItem;

				for(int i = 0; i < eqItem; i++)
				{
					int idd = 0;
					int slott = 0;

					z >> idd;
					z >> slott;

					Item *stru = GetItemById(idd);

					if(stru == NULL)
						log(ERR, "[int LoadChars()] [Could not find item] [%d]\n", idd);

					else
					{
						Item *item = new Item();
						*item = *stru;

						item->effect = stru->effect;
						item->maxstack = stru->maxstack;
						item->value = stru->value;

						item->stack = 1;
						item->id = idd;

						p->eq[(slott - 1)] = item;
					}
				}

				z >> p->intt;

				if(p->intt < 2)
					p->intt = 2;

				int numFriends = 0;
				z >> numFriends;

				for(int i = 0; i < numFriends; i++)
				{
					Partner ps = Partner();

					z >> ps.name;
					ps.status = false;
					ps.c = NULL;

					p->friends.push_back(ps);
				}

				z >> p->job;

				if(p->job < 1)
					p->job = 1;

				p->mapId = GetMap(map);

				if(p->mapId == NULL)
				{
					p->mapId = GetMap(mPlayerStartMap);

					p->x = (float)mPlayerStartX;
					p->y = (float)mPlayerStartY;
				}

				if(p->mapId->noUse)
				{
					p->mapId = GetMap(mPlayerStartMap);

					p->x = (float)mPlayerStartX;
					p->y = (float)mPlayerStartY;
				}

				if(p->x < 0)
					p->x = 0;

				if(p->y < 0)
					p->y = 0;

				if(p->x > (p->mapId->xSize - 64))
					p->x = (float)(p->mapId->xSize - 64);

				if(p->y > (p->mapId->ySize - 64))
					p->y = (float)(p->mapId->ySize - 64);

				p->slot = atoi(slot);
				p->owner = t;
				//strcpy_s(p->owner, t);

				CheckSkills(p);
				CheckClassSkills(p);

				number++;
				p->setId(playPuhId);
				p->trading = false;

				playPuhId += 1;

				pvector.push_back(p);
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadExtraSpawns()
{
	string op = "Data/ExtraSpawn/ExtraSpawn-Index.txt";
	string tchar = "Data/ExtraSpawn/";
	string t2 = "Data/ExtraSpawn/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadExtraSpawns()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadExtraSpawns()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				int id = 0;

				int minX, maxX, minY, maxY, numMobs, map, aggChance, noMove, respawnDelayMonster;

				z >> id;
				z >> minX;
				z >> maxX;
				z >> minY;
				z >> maxY;
				z >> numMobs;
				z >> map;
				z >> aggChance;
				z >> noMove;
				z >> respawnDelayMonster;

				Monster *struc = NULL;

				for(unsigned int i = 0; i < mobs.size(); i++)
				{
					if(mobs.at(i)->id == id)
					{
						struc = mobs.at(i);

						break;
					}
				}

				if(struc == NULL)
					log(ERR, "[int LoadExtraSpawns()] [Could not find monster] [%d]\n", id);

				else
				{

					for(int i = 0; i < numMobs; i++)
					{
						int r = Random((moveDelay / 2), (moveDelay * 2));

						int dropornot = 0;

						if(aggChance > 0)
							dropornot = Random(1, (int)((float)(100 / aggChance)));
						else
							dropornot = 2;

						Monster *m = new Monster();

						*m = *struc;

						m->pid = monsterId;
						monsterId++;

						m->attacked = false;
						m->mapId = GetMap(map);
						if(m->mapId == NULL)
							log(ERR, "[int LoadExtraSpawns()] [Could not find map] [%d]\n", map);
						m->moving = false;
						m->x = (float)Random(minX, maxX);
						m->y = (float)Random(minY, maxY);
						m->nextMove = (getTimestamp() + r);
						m->spawned = true;
						m->respawnTime = (getTimestamp() + monsterRespawnDelay);
						m->minY = minY;
						m->maxY = maxY;
						m->minX = minX;
						m->maxX = maxX;
						m->aggChance = aggChance;

						if(dropornot == 1)
							m->aggro = true;

						m->respawnDelayMonster = respawnDelayMonster;

						if(respawnDelayMonster == 1337)
							m->respawnDelayMonster = monsterRespawnDelay;
					
						if(noMove == 1)
							m->noMove = true;
						else
							m->noMove = false;

						number++;
						mobs.push_back(m);
					}
				}
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadNPC()
{
	string op = "Data/NPC/NPC-Index.txt";
	string tchar = "Data/NPC/";
	string t2 = "Data/NPC/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadNPC()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadNPC()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				char temp[100] = "";
				int mapId = 0;
				int menus = 0;

				NPC *npc = new NPC();

				z.getline(temp, 100);
				npc->name = temp;
				z.getline(temp, 100);

				sscanf(temp, "%d %d %d", &npc->x, &npc->y, &mapId);

				char dialog_buf[1000];
				z.getline(dialog_buf, 1000);
				npc->dialog = dialog_buf;
				
				number++;
				npc->id = number;
				npc->mapId = GetMap(mapId);

				z.getline(temp, 100);

				sscanf(temp, "%d", &menus);

				for(int i = 0; i < menus; i++)
				{
					MenuButton m = MenuButton();

					char buf[80];
					z.getline(buf, 80);
					m.text_ = buf;

					npc->menu.push_back(m);
				}

				z.getline(temp, 100);

				sscanf(temp, "%d", &menus);

				for(int i = 0; i < menus; i++)
				{
					MenuButton m = MenuButton();

					char buf[80];
					z.getline(buf, 80);
					m.text_ = buf;

					npc->diab.push_back(m);
				}

				z.getline(temp, 100);

				sscanf(temp, "%d", &menus);

				if(menus > 0)
				{
					for(int i = 0; i < menus; i++)
					{
						MenuButton m = MenuButton();

						char buf[80];
						z.getline(buf, 80);
						m.text_ = buf;

						npc->chat.push_back(m);
					}

					npc->currChat = 1;
					npc->enableChat = true;
					npc->nextChat = (getTimestamp() + npcChatDelay);
				}

				z.getline(temp, 100);

				sscanf(temp, "%d", &menus);

				if(menus > 0)
				{
					for(int i = 0; i < menus; i++)
					{
						int haha = 0;

						z.getline(temp, 100);

						haha = atoi(temp);

						npc->sellingItems.push_back(haha);
					}
				}

				z.getline(temp, 100);

				sscanf(temp, "%d", &menus);

				if(menus > 0)
				{
					for(int i = 0; i < menus; i++)
					{
						char name[40];
						int price;
						int idss;
						int xS;
						int yS;

						z.getline(name, 40);

						TeleportObject to = TeleportObject();

						to.world = name;

						z.getline(temp, 100);

						sscanf_s(temp, "%d %d %d %d", &price, &idss, &xS, &yS);

						to.price = price;
						to.id = idss;
						to.x = xS;
						to.y = yS;

						npc->tps.push_back(to);
					}
				}

				z.getline(temp, 100);

				sscanf_s(temp, "%d %d", &npc->meW, &npc->meH);

				z.getline(temp, 100);

				sscanf_s(temp, "%d", &npc->mapGuard);

				npcs.push_back(npc);
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadMaps()
{
	string op = "Data/Maps/Maps-Index.txt";
	string tchar = "Data/Maps/";
	string t2 = "Data/Maps/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadMaps()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadMaps()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				char temp[100] = "";
				char temp2[100] = "";
				char temp3[100] = "";
				int sizex = 0, sizey = 0;

				Map *map = new Map();

				z.getline(temp, 100);

				sscanf(temp, "%s %s", &temp2, &temp3);

				if(strcmp(temp2, "PK") == 0)
				{
					if(strcmp(temp3, "ENABLE") == 0)
						map->pk = true;
					else
						map->pk = false;
				}

				z.getline(temp, 100);

				sscanf(temp, "%s %s", &temp2, &temp3);

				if(strcmp(temp2, "WEAPON") == 0)
				{
					if(strcmp(temp3, "ENABLE") == 0)
						map->weaponAllowed = true;
					else
						map->weaponAllowed = false;
				}

				z.getline(temp, 100);

				sscanf(temp, "%d %d", &sizex, &sizey);

				map->xSize = sizex;
				map->ySize = sizey;

				map->id = atoi(t);

				while(z.getline(temp, 100))
				{
					char tt[100];
					int from = 0;
					int to = 0;
					int from2 = 0;
					int to2 = 0;

					sscanf(temp, "%s", tt);

					if(strcmp(tt, "NOWALK") == 0)
					{
						sscanf(temp, "NOWALK X %d %d Y %d %d", &from, &to, &from2, &to2);

						NoWalkZone nn = NoWalkZone();

						nn.xFrom = from;
						nn.xTo = to;
						nn.yFrom = from2;
						nn.yTo = to2;

						map->nowalk.push_back(nn);
					}

					else if(strcmp(tt, "TELEPORTER") == 0)
					{
						int mapid = 0;
						int mapX = 0;
						int mapY = 0;

						sscanf(temp, "TELEPORTER X %d %d Y %d %d %d %d %d", &from, &to, &from2, &to2, &mapid, &mapX, &mapY);

						TeleporterZone tz = TeleporterZone();

						tz.xFrom = from;
						tz.xTo = to;
						tz.yFrom = from2;
						tz.yTo = to2;
						tz.toMap = mapid;
						tz.toMapX = mapX;
						tz.toMapY = mapY;

						map->tzone.push_back(tz);
					}

					else if(strcmp(tt, "PKAREA") == 0)
					{
						PKAREA pkArea;

						sscanf(temp, "PKAREA X %d %d Y %d %d", &pkArea.minX, &pkArea.maxX, &pkArea.minY, &pkArea.maxY);

						map->pkArea.push_back(pkArea);
					}

					else if(strcmp(tt, "MAPNOUSE") == 0)
						map->noUse = true;

					else if(strcmp(tt, "RESSPOS") == 0)
					{
						sscanf(temp, "RESSPOS %d %d", &map->resX, &map->resY);
					}

					else if(strcmp(tt, "AGGRORANGE") == 0)
					{
						sscanf(temp, "AGGRORANGE %d %d", &map->aggX, &map->aggY);
					}
				}

				if(map->resX == 0)
					map->resX = mPlayerStartX;

				if(map->resY == 0)
					map->resY = mPlayerStartY;

				maps.push_back(map);
				number++;
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadItems()
{
	string op = "Data/Items/Items-Index.txt";
	string tchar = "Data/Items/";
	string t2 = "Data/Items/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadItems()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadItems()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				int numValues = 0;

				Item *item = new Item();

				z >> item->maxstack;
				z >> item->effect;
				z >> item->value;
				
				z.ignore();
				getline(z, item->itemName);

				z >> item->price;
				z >> item->minlvl;

				z >> numValues;

				for(int i = 0; i < numValues; i++)
				{
					TwoInt tw = TwoInt();

					z >> tw.int1;
					z >> tw.int2;

					item->evalue.push_back(tw);
				}

				item->id = atoi(t);

				z >> item->job;

				number++;

				items.push_back(item);
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadSkills()
{
	string op = "Data/Skill/Skill-Index.txt";
	string tchar = "Data/Skill/";
	string t2 = "Data/Skill/";
	ifstream r(op.c_str());

	char t[20];
	char test[2000];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadSkills()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadSkills()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				Skill *skill = new Skill();
				SkillCoolDown skillCool = SkillCoolDown();

				z >> skill->type;
				z >> skillCool.coolDown;
				z >> skill->job;
				z >> skill->cost;
				z >> skill->costamount;
				z.ignore();

				z.getline(test, 2000);

				int amountItem = 0;

				z >> amountItem;

				for(int i = 0; i < amountItem; i++)
				{
					SkillAddBuff sab = SkillAddBuff();
					FourInt fi = FourInt();

					int target, buffid, bufflvl, bufftime;
					int scaleLvl, scaleMult, scaleDur, scaleDurMult;

					z >> target;
					z >> buffid;
					z >> bufflvl;
					z >> bufftime;
					z >> scaleLvl;
					z >> scaleMult;
					z >> scaleDur;
					z >> scaleDurMult;

					fi.int1 = target;
					fi.int2 = buffid;
					fi.int3 = bufflvl;
					fi.int4 = bufftime;

					if(scaleLvl == 1)
						sab.scaleLvl = true;

					sab.scaleMult = scaleMult;

					if(scaleDur == 1)
						sab.scaleDur = true;

					sab.scaleDurMult = scaleDurMult;

					sab.fi = fi;

					skill->addbuffs.push_back(sab);
				}

				int special, doAnimation, shot, aoe, basedmg, dmgUpLvl, aoerange, basedOn, expamount, scaleBaseDmg, multBase, scaletype, useWeap;

				z >> special;
				z >> doAnimation;
				z >> shot;
				z >> aoe;
				z >> aoerange;
				z >> basedmg;
				z >> dmgUpLvl;
				z >> basedOn;
				z >> expamount;
				z >> scaleBaseDmg;
				z >> multBase;
				z >> scaletype;
				z >> useWeap;

				if(special == 1)
					skill->special = true;

				if(doAnimation == 1)
					skill->animation = true;

				if(shot == 1)
					skill->shoot = true;

				if(aoe == 1)
					skill->aoe = true;

				if(scaleBaseDmg == 1)
					skill->scalebasedmg = true;

				if(useWeap == 1)
					skill->useWeap = true;

				skill->multbasedmg = multBase;
				skill->aoerange = aoerange;
				skill->basedmg = basedmg;
				skill->basedOn = basedOn;
				skill->expamount = expamount;
				skill->scalebasetype = scaletype;

				if(dmgUpLvl == 1)
					skill->dmglvl = true;

				skill->id = atoi(t);
				skill->lvl = 1;
				skill->exp = 0;

				skillCool.id = skill->id;

				skills.push_back(skill);

				SkillName sn;

				sn.id = skill->id;
				sn.name = test;
				
				skillName.push_back(sn);

				skillCooldown.push_back(skillCool);

				number++;
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadClasses()
{
	string op = "Data/Class/Class-Index.txt";
	string tchar = "Data/Class/";
	string t2 = "Data/Class/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadClasses()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadClasses()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				Class *c = new Class();

				c->id = atoi(t);

				int num2 = 0;
				int num3 = 0;
				z >> num2;

				for(int i = 0; i < num2; i++)
				{
					z >> num3;

					Skill *skillmain = GetMainSkill(num3);

					if(skillmain == NULL)
						log(ERR, "[int LoadClasses()] [Could not find skill] [%d]\n", num3);

					c->skills.push_back(skillmain);
				}

				z >> num2;

				if(num2 == 1)
					c->needweap = true;

				else
					c->needweap = false;

				z >> num2;

				if(num2 == 1)
					c->mpgot = true;

				else
					c->mpgot = false;

				z >> num2;

				if(num2 == 1)
					c->egot = true;

				else
					c->egot = false;

				z >> c->mpplus;
				z >> c->eplus;
				z >> c->hpplus;
				z >> c->normalShot;
				z >> c->based;
				z >> c->bulletSpeed;
				z >> c->bulletRotate;

				int amountItem;
				z >> amountItem;

				for(int i = 0; i < amountItem; i++)
				{
					int id, eq, slot, stack;

					z >> id;
					z >> eq;
					z >> slot;
					z >> stack;

					StartItem si = StartItem();

					si.id = id;
					
					if(eq == 1)
						si.eq = true;

					else
						si.eq = false;

					si.slot = slot;
					si.stack = stack;

					c->items.push_back(si);
				}

				z >> amountItem;

				if(amountItem == 1)
					c->disabled = true;

				z >> c->shootType;
				z >> c->shootCost;
				
				z >> amountItem;

				if(amountItem == 1)
					c->ranged = true;

				z >> c->bulletDis;

				z >> amountItem;

				for(int i = 0; i < amountItem; i++)
				{
					TwoInt tw;

					z >> tw.int1;
					z >> tw.int2;

					c->effects.push_back(tw);
				}

				z >> c->baseMult;

				classes.push_back(c);

				/*
				b->effect = atoi(t);
				
				z >> b->changeEffect;

				buffdata.push_back(b);
				*/

				number++;
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

int LoadBuffs()
{
	string op = "Data/Buff/Buff-Index.txt";
	string tchar = "Data/Buff/";
	string t2 = "Data/Buff/";
	ifstream r(op.c_str());

	char t[20];

	int number = 0;

	if(!r)
		log(ERR, "[int LoadBuffs()] [Unable to open file] [%s]\n", op.c_str());
	else
	{
		while(r >> t)
		{
			t2 += t;
			t2 += ".txt";

			ifstream z(t2.c_str());

			if(!z)
				log(ERR, "[int LoadBuffs()] [Unable to open file] [%s]\n", t2.c_str());
			else
			{
				Buff *b = new Buff();

				b->effect = atoi(t);
				
				z >> b->changeEffect;

				buffdata.push_back(b);

				number++;
			}

			z.close();

			t2 = tchar;
		}
	}

	return number;
}

void Config(int &p)
{
	expTable.push_back(16);
	expTable.push_back(34);
	expTable.push_back(65);
	expTable.push_back(84);
	expTable.push_back(130);
	expTable.push_back(186);
	expTable.push_back(210);
	expTable.push_back(235);
	expTable.push_back(260);
	expTable.push_back(300);

	char lol[2048];
	std::ifstream o("Config.txt");

	if(!o)
		log(ERR, "[void Config(int&)] [Unable to open file] [%s]\n", "Config.txt");
	else
	{
		while(o >> lol)
		{
			if(strcmp(lol, "Port:") == 0)
				o >> p;

			else if(strcmp(lol, "Password:") == 0)
				o >> (char*)PasswordISC.c_str();

			else if(strcmp(lol, "Debug:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					debugs = true;
				else
					debugs = false;
			}

			else if(strcmp(lol, "ISCIP:") == 0)
				o >> ISCIP;

			else if(strcmp(lol, "MaxMaps:") == 0)
				o >> maxMaps;

			else if(strcmp(lol, "EnableAttack:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableAttack = true;
				else
					enableAttack = false;
			}

			else if(strcmp(lol, "EnableKS:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableKS = true;
				else
					enableKS = false;
			}

			else if(strcmp(lol, "SaveDelay:") == 0)
				o >> saveDelay;

			else if(strcmp(lol, "RefreshDelay:") == 0)
				o >> refreshDelay;

			else if(strcmp(lol, "MoveDelay:") == 0)
				o >> moveDelay;

			else if(strcmp(lol, "PlayerDelay:") == 0)
				o >> spawnPlayerDelay;
			
			else if(strcmp(lol, "MonsterDelay:") == 0)
				o >> spawnMonsterDelay;

			else if(strcmp(lol, "DefaultSpeed:") == 0)
				o >> defSpeed;

			else if(strcmp(lol, "OnlineDelay:") == 0)
				o >> onlineCheckDelay;

			else if(strcmp(lol, "NPCDelay:") == 0)
				o >> spawnNPCDelay;

			else if(strcmp(lol, "StartX:") == 0)
				o >> mPlayerStartX;
			
			else if(strcmp(lol, "StartY:") == 0)
				o >> mPlayerStartY;

			else if(strcmp(lol, "StartMap:") == 0)
				o >> mPlayerStartMap;

			else if(strcmp(lol, "ReviveDelay:") == 0)
				o >> reviveDelay;

			else if(strcmp(lol, "MonsterRespawnDelay:") == 0)
				o >> monsterRespawnDelay;

			else if(strcmp(lol, "MonsterFreeDelay:") == 0)
				o >> monsterFreeDelay;

			else if(strcmp(lol, "ISCPort:") == 0)
				o >> ISCPort;

			else if(strcmp(lol, "ExpRate:") == 0)
				o >> expRate;

			else if(strcmp(lol, "MaxLevel:") == 0)
				o >> maxLvl;

			else if(strcmp(lol, "ExpTable:") == 0)
			{
				int expTableUnit = 0;
				expTable.clear();

				for(int i = 0; i < maxLvl; i++)
				{
					o >> expTableUnit;

					expTable.push_back(expTableUnit);
				}
			}

			else if(strcmp(lol, "Buffs:") == 0)
				o >> buff;

			else if(strcmp(lol, "BuffID:") == 0)
			{
				for(int i = 0; i < buff; i++)
				{
					o >> buffs[i];
					o >> buffsLvl[i];
					o >> buffTime[i];
				}
			}

			else if(strcmp(lol, "NPCChatDelay:") == 0)
				o >> npcChatDelay;

			else if(strcmp(lol, "DeleteDropDelay:") == 0)
				o >> deleteDropDelay;

			else if(strcmp(lol, "AttackDelay:") == 0)
				o >> monsterAttackDelay;

			else if(strcmp(lol, "EnableSkill:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableSkill = true;
				else
					enableSkill = false;
			}

			else if(strcmp(lol, "StartCrit:") == 0)
				o >> defCritChance;

			else if(strcmp(lol, "StartBlock:") == 0)
				o >> blockChance;

			else if(strcmp(lol, "SkillExpRate:") == 0)
				o >> skillexprate;

			else if(strcmp(lol, "SkillMaxLevel:") == 0)
				o >> skillmaxlvl;

			else if(strcmp(lol, "SkillExpTable:") == 0)
			{
				for(int i = 0; i < skillmaxlvl; i++)
					o >> skillexptable[i];
			}

			else if(strcmp(lol, "PickDropDelay:") == 0)
				o >> pickDropDelay;

			else if(strcmp(lol, "DeniroRate:") == 0)
				o >> penyaRate;

			else if(strcmp(lol, "DropRate:") == 0)
				o >> dropRate;
			
			else if(strcmp(lol, "ServerName:") == 0)
				o >> serverName;

			else if(strcmp(lol, "EnablePvPPoints:") == 0)
			{
				int temps = 0;

				o >> temps;

				if(temps == 0)
					mPvPEnablePoints = false;

				else
					mPvPEnablePoints = true;
			}

			else if(strcmp(lol, "DebugPacket:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					debugPacket = true;

				else
					debugPacket = false;
			}

			else if(strcmp(lol, "EnableText:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableClientText = true;

				else
					enableClientText = false;
			}

			else if(strcmp(lol, "EnableStatusCheck:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableStatusCheck = true;

				else
					enableStatusCheck = false;
			}
			
			else if(strcmp(lol, "StatusCheckDelay:") == 0)
				o >> statusDelay;

			else if(strcmp(lol, "MonsterFollowMaxX:") == 0)
				o >> monsterMaxWalkX;

			else if(strcmp(lol, "MonsterFollowMaxY:") == 0)
				o >> monsterMaxWalkY;

			else if(strcmp(lol, "PlayerAttackTimer:") == 0)
				o >> playerAttackTimer;

			else if(strcmp(lol, "FoodCooldown:") == 0)
				o >> foodDefCooldown;

			else if(strcmp(lol, "OnlinePacketDisconnect:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					disconnectWithOnlinePacket = true;

				else
					disconnectWithOnlinePacket = false;
			}

			else if(strcmp(lol, "PartyExp:") == 0)
				o >> expRateParty;

			else if(strcmp(lol, "PartyMaxLevel:") == 0)
				o >> maxPartyLvl;

			else if(strcmp(lol, "HeavyDebug:") == 0)
			{
				o >> lol;

				if(atoi(lol) == 1)
					enableLD = true;

				else
					enableLD = false;
			}
		}
	}

	o.close();
}