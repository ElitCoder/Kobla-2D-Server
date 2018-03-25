#include <fstream>
#include <sys/stat.h>

#include "Handle.h"
#include "Main.h"
#include "World.h"
#include "Item.h"
#include "Command.h"
#include "BufferCheck.h"

using namespace std;

extern vector<Player*> pvector;
extern vector<Monster*> mobs;
extern vector<Party*> parties;
extern vector<Drop*> drops;
vector<NPC*> npcs;
vector<Map*> maps;
vector<Skill*> skills;
vector<SkillName> skillName;
vector<Buff*> buffdata;
vector<SkillCoolDown> skillCooldown;
vector<Class*> classes;

extern bool enableAttack;
extern int maxMaps;

extern long saveDelay;
extern long refreshDelay;
extern long onlineCheckDelay;
extern long reviveDelay;
extern long monsterRespawnDelay;
extern long monsterAttackDelay;
extern long moveDelay;
extern long foodDefCooldown;
long pickDropDelay = 60;

long spawnPlayerDelay = 600;
long spawnMonsterDelay = 1600;
long spawnNPCDelay = 1900;
long monsterFreeDelay = 3000;

int defSpeed = 8;
bool debugs = false;

extern std::vector<Client*> ac;

int expRate = 1;
int maxLvl = 10;
vector<int> expTable;

int expRateParty = 1;
int maxPartyLvl = 30;

int skillexprate = 1;
int skillmaxlvl = 10;
int skillexptable[10] = {5, 7, 13, 22, 31, 54, 80, 103, 132, 194};

int buff = 1;
int buffs[10] = {62};
int buffsLvl[10] = {5};
int buffTime[10] = {3600};

bool enableKS = false;
bool enableSkill = false;

int blockChance = 8;

int penyaRate = 1;
int dropRate = 1;

char serverName[100] = "LOL";

WorldCommand *wc = new WorldCommand();

bool mPvPEnablePoints = false;

extern int monsterMaxWalkY;
extern int monsterMaxWalkX;

extern long playerAttackTimer;

extern int serverError;

extern int randomInt;

void Client::Spawn(char *b)
{
	if(this->online && !this->ingame)
	{
		Player *tp = this->GetThisPlayer((int)b[3]);

		if(tp == NULL)
			log(ERR, "[void Client::Spawn(char*)] [Can not find player] [%s] [%d]\n", this->username.c_str(), (int)b[3]);

		else
		{
			this->p = tp;

			this->ingame = true;
			PlayerLog("[Player spawned (%d, %d, %d)]\n", tp->mapId->id, (int)tp->x, (int)tp->y);
			this->ingame = false;

			this->com->setPlayer(this->p);

			this->p->equipped = false;
			this->p->dead = false;
			this->p->moving = false;

			if(this->p->pt != NULL)
				log(ERR, "[void Client::Spawn(char*)] [In party]\n");

			LD(GAME, "Spawning player: [%s] [%s] [Name: %s] [Level: %d] [Map: %d].\n", this->username.c_str(), this->password.c_str(), tp->name, tp->level, tp->mapId->id);
			
			this->p->pt = NULL;

			this->GetBuffsOnline();

			this->p->meW = (int)b[4];
			this->p->meH = (int)b[5];
			this->p->wW = (int)b[6];
			this->p->wH = (int)b[7];

			this->p->hp = CalculateHP(this);
			this->p->chp = (this->p->hp / 2.0f);

			this->p->maxmana = CalculateMana(this);
			this->p->mana = (this->p->maxmana / 2.0f);

			this->p->maxenergy = CalculateEnergy(this);
			this->p->energy = (this->p->maxenergy / 2.0f);

			if(this->p->mapId->noUse)
			{
				this->p->mapId = GetMap(mPlayerStartMap);

				this->p->x = (float)mPlayerStartX;
				this->p->y = (float)mPlayerStartY;
			}

			if(tp->x < 0)
				tp->x = 0;

			if(tp->y < 0)
				tp->y = 0;

			if(tp->x > (tp->mapId->xSize - tp->meW))
				tp->x = (float)(tp->mapId->xSize - tp->meW);

			if(tp->y > (tp->mapId->ySize - tp->meH))
				tp->y = (float)(tp->mapId->ySize - tp->meH);

			Packet pak = Packet();

			pak.addHeader(0x5);
			pak.addInt(0);
			pak.addInt(1);
			pak.addInt(tp->level);
			pak.addInt(0);
			pak.addInt(tp->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
			pak.addBool(false);
			pak.addBool(false);
			pak.addInt(RIGHT);

			pak.addString(tp->name);
			pak.addLongInt(tp->getId());
			pak.addBool(false);

			pak.addLongInt((int)tp->x);
			pak.addLongInt((int)tp->y);

			pak.addBool(false);
			pak.addInt(0);

			int eqToUserItems = 0;

			for(int i = 0; i < 4; i++)
			{
				if(this->p->eq[i] != NULL)
					eqToUserItems++;
			}

			pak.addInt(eqToUserItems);

			for(int t = 0; t < 4; t++)
			{
				if(this->p->eq[t] != NULL)
				{
					pak.addInt((t + 1));
					pak.addLongInt(this->p->eq[t]->id);
				}
			}

			pak.addInt(GetPlayerProcentHP(tp));
			pak.addBool(false);
			pak.addLongInt(0);
			pak.addInt(tp->job);

			pak.ready();

			int cc = GetPlayersForMapNumber(tp->mapId->id, tp->getId());

			Packet mes = Packet();

			mes.addHeader(0x5);
			mes.addInt(1);
			mes.addInt(tp->job);
			mes.addInt(tp->level);
			mes.addInt(tp->mapId->id);
			mes.addBool(enableAttack);
			mes.addInt(tp->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
			mes.addBool(tp->mapId->pk);
			mes.addLongInt((int)this->p->hp);
			mes.addLongInt((int)this->p->chp);

			mes.addLongInt(tp->power + this->GetBonus(STRENGTH));
			mes.addLongInt(tp->magic + this->GetBonus(STAMINA));
			mes.addLongInt(tp->powerpoints);
			mes.addLongInt(tp->agility + this->GetBonus(AGILITY));
			mes.addLongInt(tp->intt + this->GetBonus(INTT));

			mes.addString(tp->name);
			mes.addLongInt(tp->getId());

			mes.addLongInt((int)tp->x);
			mes.addLongInt((int)tp->y);
			mes.addLongInt(monsterAttackDelay);
			mes.addLongInt(tp->gold);
			mes.addFloat(this->CalculateExp());

			int invItems = 0;
			int eqItems = 0;
			int tskItems = 0;

			for(int i = 0; i < 25; i++)
			{
				if(this->p->inv[i] != NULL)
					invItems++;
			}

			mes.addInt(invItems);

			for(int t = 0; t < 25; t++)
			{
				if(this->p->inv[t] != NULL)
				{
					mes.addInt((t + 1));
					mes.addLongInt(this->p->inv[t]->value);
					mes.addLongInt(this->p->inv[t]->effect);
					mes.addLongInt(this->p->inv[t]->stack);
					mes.addLongInt(this->p->inv[t]->maxstack);
					mes.addLongInt(this->p->inv[t]->id);
				}
			}

			for(int i = 0; i < 4; i++)
			{
				if(this->p->eq[i] != NULL)
					eqItems++;
			}

			mes.addInt(eqItems);

			for(int t = 0; t < 4; t++)
			{
				if(this->p->eq[t] != NULL)
				{
					mes.addInt((t + 1));
					mes.addLongInt(this->p->eq[t]->id);
					mes.addLongInt(this->p->eq[t]->value);
				}
			}

			mes.addLongInt(this->p->buffs.size());

			for(unsigned int i = 0; i < this->p->buffs.size(); i++)
			{
				mes.addLongInt(this->p->buffs.at(i)->effect);
				mes.addLongInt(this->p->buffs.at(i)->changeEffect);
				mes.addLongInt(this->p->buffs.at(i)->value);
				mes.addLongInt(((this->p->buffs.at(i)->endTime - GetTickCount()) / 1000));
			}

			mes.addLongInt(this->GetBonus(STRENGTH));
			mes.addLongInt(this->GetBonus(STAMINA));
			mes.addLongInt(this->GetBonus(AGILITY));
			mes.addLongInt(this->GetBonus(INTT));

			for(int i = 0; i < 9; i++)
			{
				if(this->p->task[i]->enable)
					tskItems++;
			}

			mes.addLongInt(tskItems);

			for(int i = 0; i < 9; i++)
			{
				if(this->p->task[i]->enable)
				{
					mes.addInt((i + 1));
					mes.addInt(this->p->task[i]->type);
					mes.addInt(this->p->task[i]->slot);
					mes.addBool(this->p->task[i]->eqinv);
					mes.addLongInt(this->p->task[i]->id);
				}
			}

			mes.addInt(this->p->skills.size());

			for(unsigned int i = 0; i < this->p->skills.size(); i++)
			{
				mes.addInt(this->p->skills.at(i)->lvl);
				mes.addInt(this->p->skills.at(i)->id);
			}

			/*
			mes.addInt(skills.size());

			for(unsigned int i = 0; i < skills.size(); i++)
			{
				Skill *gets = this->GetSkill(skills.at(i)->id);

				if(gets == NULL)
					mes.addInt(1);

				else
					mes.addInt(gets->lvl);

				mes.addInt(skills.at(i)->id);
			}
			*/

			mes.addLongInt(pickDropDelay);
			mes.addLongInt(this->p->pkpoints);
			mes.addString(serverName);
			mes.addLongInt((reviveDelay / 1000));
			mes.addLongInt(this->GetAttackDelay());

			mes.addBool(this->p->mapId->weaponAllowed);
			mes.addLongInt(this->p->mapId->xSize);
			mes.addLongInt(this->p->mapId->ySize);

			mes.addLongInt(this->p->mapId->pkArea.size());

			for(unsigned int i = 0; i < this->p->mapId->pkArea.size(); i++)
			{
				mes.addLongInt(this->p->mapId->pkArea.at(i).minX);
				mes.addLongInt(this->p->mapId->pkArea.at(i).maxX);
				mes.addLongInt(this->p->mapId->pkArea.at(i).minY);
				mes.addLongInt(this->p->mapId->pkArea.at(i).maxY);
			}

			mes.addLongInt(GetPlayersForMapNumber(this->p->mapId->id, this->p->getId()));

			mes.addLongInt(this->p->friends.size());

			this->UpdateFriends(true);

			for(unsigned int i = 0; i < this->p->friends.size(); i++)
			{
				bool online = false;

				Client *c = GetClientByPlayerName((char*)this->p->friends.at(i).name.c_str());

				if(c == NULL)
					online = false;

				else
					online = true;

				mes.addString((char*)this->p->friends.at(i).name.c_str());
				mes.addLongInt(this->p->friends.at(i).level);
				mes.addLongInt(this->p->friends.at(i).job);
				mes.addLongInt(this->p->friends.at(i).id);
				mes.addBool(online);

				this->p->friends.at(i).status = online;
				this->p->friends.at(i).c = c;
			}

			mes.addLongInt((int)this->p->mana);
			mes.addLongInt((int)this->p->maxmana);

			mes.addLongInt((int)this->p->energy);
			mes.addLongInt((int)this->p->maxenergy);

			mes.addInt((int)this->p->job);
			mes.addLongInt(this->GetBonus(ATTACKSPEED));

			mes.ready();
			this->AddPacket(mes, 0);

			if(debugs)
			{
				log(DEBUG, "Spawn packet: [%d] ", mes.getPos());

				for(int i = 0; i < mes.getPos(); i++)
					printf("%02X ", mes.getPacket()[i]);

				printf("\n");
			}

			if(cc > 0)
			{
				for(unsigned int i = 0; i < ac.size(); i++)
				{
					Client *c = ac.at(i);

					if(c->ingame)
					{
						if(c->p->mapId->id == tp->mapId->id)
						{
							Packet ps = Packet();

							ps.addHeader(0x5);
							ps.addInt(0);
							ps.addInt(1);
							ps.addInt(c->p->level);
							ps.addInt(c->p->power);
							ps.addInt(c->p->speed + (c->GetBonus(SPEED) - c->GetBonus(SPEEDMINUS)));
							ps.addBool(c->p->equipped);
							ps.addBool(c->p->dead);
							ps.addInt(c->p->weaponWay);
							ps.addString(c->p->name);
							ps.addLongInt(c->p->getId());
							ps.addBool(false);
							ps.addLongInt((int)c->p->x);
							ps.addLongInt((int)c->p->y);
							ps.addBool(c->p->moving);
							ps.addInt(c->p->cMove);

							int eqToUserItems2 = 0;

							for(int i = 0; i < 4; i++)
							{
								if(c->p->eq[i] != NULL)
									eqToUserItems2++;
							}

							ps.addInt(eqToUserItems2);

							for(int t = 0; t < 4; t++)
							{
								if(c->p->eq[t] != NULL)
								{
									ps.addInt((t + 1));
									ps.addLongInt(c->p->eq[t]->id);
								}
							}

							ps.addInt(GetPlayerProcentHP(c->p));
							ps.addBool(c->p->follow);
							ps.addLongInt(c->p->followId);
							ps.addInt(c->p->job);

							ps.ready();
							this->AddPacket(ps, 0);

							c->AddPacket(pak, 0);
						}
					}
				}
			}

			std::vector<Monster*> mob = GetMonstersForMap(tp->mapId->id);

			if(mob.size() > 0)
			{
				Packet pm = Packet();
				pm.addHeader(0x5);
				pm.addInt(3);
				pm.addLongInt(mob.size());

				for(unsigned int i = 0; i < mob.size(); i++)
				{
					pm.addInt(mob.at(i)->level);
					pm.addInt(mob.at(i)->attack);
					pm.addInt(mob.at(i)->id);
					pm.addInt(0);
					pm.addInt(0);
					pm.addLongInt((int)mob.at(i)->x);
					pm.addLongInt((int)mob.at(i)->y);
					pm.addLongInt(mob.at(i)->pid);
					pm.addLongInt(mob.at(i)->followId);
					pm.addBool(mob.at(i)->goAfter);
					pm.addLongInt((int)mob.at(i)->bx);
					pm.addLongInt((int)mob.at(i)->by);
					pm.addBool(mob.at(i)->aggro);
					pm.addLongInt((mob.at(i)->speed + ((mob.at(i)->GetMonsterBonus(SPEED) * 20) - (mob.at(i)->GetMonsterBonus(SPEEDMINUS) * 20))));
					pm.addBool(mob.at(i)->moving);
					pm.addInt(mob.at(i)->goDir);
					pm.addInt(GetMonsterProcentHP(mob.at(i)));
				}

				pm.ready();
				this->AddPacket(pm, 0);
			}

			std::vector<NPC*> npc = GetNPCForMap(tp->mapId->id);

			if(npc.size() > 0)
			{
				for(unsigned int i = 0; i < npc.size(); i++)
				{
					Packet pn = Packet();

					pn.addHeader(0x5);
					pn.addInt(10);
					pn.addLongInt(1);
					pn.addLongInt(npc.at(i)->x);
					pn.addLongInt(npc.at(i)->y);
					pn.addLongInt(npc.at(i)->id);
					pn.addString(npc.at(i)->name);
					pn.addInt(npc.at(i)->menu.size());

					for(unsigned int y = 0; y < npc.at(i)->menu.size(); y++)
						pn.addString(npc.at(i)->menu.at(y).text);

					pn.ready();
					this->AddPacket(pn, 0);
				}
			}

			std::vector<Drop*> dro = GetDropsForMap(tp->mapId->id);

			if(dro.size() > 0)
			{
				Packet pd = Packet();
				pd.addHeader(0x39);
				pd.addInt(1);
				pd.addLongInt(dro.size());

				for(unsigned int d = 0; d < dro.size(); d++)
				{
					pd.addInt(dro.at(d)->drop->id);
					pd.addInt(dro.at(d)->drop->value);
					pd.addLongInt(dro.at(d)->id);
					pd.addLongInt(dro.at(d)->x);
					pd.addLongInt(dro.at(d)->y);
				}

				pd.ready();
				this->AddPacket(pd, 0);
			}

			this->p->nextRefresh = (GetTickCount() + (refreshDelay * 2));
			this->p->nextSave = (GetTickCount() + saveDelay);

			if(this->p->access <= 50)
			{
				string welcomeNotice2 = this->p->name;
				welcomeNotice2 += " logged in to the game.";

				for(unsigned int i = 0; i < ac.size(); i++)
				{
					if(ac.at(i)->ingame)
						SystemChat(ac.at(i), CUSTOM, NULL, (char*)welcomeNotice2.c_str());
				}
			}

			this->ingame = true;

			int pOnline = PlayersOnline();

			string welcomeNotice = "Welcome to Kobla, ";
			welcomeNotice += this->p->name;
			welcomeNotice += ".";

			this->AddPacket(CreatePacketChat(welcomeNotice.c_str(), "Notice", false, false, 0), 0);

			welcomeNotice = "Players Online: ";
			welcomeNotice += IntToString(pOnline);
			welcomeNotice += ".";

			for(unsigned int i = 0; i < ac.size(); i++)
			{
				if(ac.at(i)->ingame && ac.at(i) != this)
				{
					if(ac.at(i)->p->access <= 50)
					{
						std::string onlineString = ac.at(i)->p->name;
						onlineString += " is currently online!";

						SystemChat(this, CUSTOM, NULL, (char*)onlineString.c_str());
					}
				}
			}

			SystemChat(this, CUSTOM, NULL, (char*)welcomeNotice.c_str());

			if(debugs)
				log(DEBUG, "Player spawned: %s %d %d. Mobs: %d NPCs: %d Drops: %d.\n", this->p->name, this->p->level, this->p->getId(), mob.size(), npc.size(), dro.size());

		}
	}
}

void Client::Move(char *b)
{
	if(!this->p->moving && !this->p->dead)
	{
		int pos = 4;
		this->p->cMove = b[3];

		if(this->p->cMove == LEFT)
			this->p->weaponWay = LEFT;
		else if(this->p->cMove == RIGHT)
			this->p->weaponWay = RIGHT;

		this->p->x = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
		pos += ((int)b[pos] + 1);

		this->p->y = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

		if(this->p->follow)
			wc->sendPlayerFollow(this, this, false);

		if(this->p->trading)
			this->CancelTrade(true);

		this->p->moving = true;
		this->p->move.start(false);

		Packet pak = Packet();

		pak.addHeader(0x8);
		pak.addInt(this->p->cMove);
		pak.addInt(1);
		pak.addLongInt(this->p->getId());
		pak.addLongInt((int)this->p->x);
		pak.addLongInt((int)this->p->y);

		pak.ready();
		SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);
	}
}

void Client::StopMove(char *b)
{
	if(this->p->moving && !this->p->dead)
	{
		int pos = 3;
		this->p->cMove = 0;

		this->p->x = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
		pos += ((int)b[pos] + 1);

		this->p->y = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

		if(this->p->follow)
			wc->sendPlayerFollow(this, this, false);

		if(this->p->trading)
			this->CancelTrade(true);

		this->p->moving = false;

		Packet pak = Packet();

		pak.addHeader(0x9);
		pak.addInt(1);
		pak.addLongInt(this->p->getId());
		pak.addLongInt((int)this->p->x);
		pak.addLongInt((int)this->p->y);

		pak.ready();
		SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);
	}
}

void Client::ChangeMap(char *b, bool serversided, int map, int x, int y)
{
	if(serversided)
	{
		if(this->p->follow)
			wc->sendPlayerFollow(this, this, false);

		if(this->p->trading)
			this->CancelTrade(true);

		this->RemoveAllFollowers();
		this->Unspawn(0);

		if(debugs)
			log(DEBUG, "Teleport: %d %d %d.\n", map, x, y);

		LD(GAME, "Teleport: [Name: %s] [From: %d] [To: %d].\n", this->p->name, this->p->mapId->id, map);

		this->p->mapId = GetMap(map);

		this->p->x = (float)x;
		this->p->y = (float)y;

		Packet borr = Packet();

		borr.addHeader(0x10);
		borr.addInt(map);
		borr.addBool(this->p->mapId->pk);
		borr.addBool(this->p->mapId->weaponAllowed);
		borr.addLongInt(x);
		borr.addLongInt(y);
		borr.addLongInt(this->p->mapId->xSize);
		borr.addLongInt(this->p->mapId->ySize);

		borr.addLongInt(this->p->mapId->pkArea.size());

		for(unsigned int i = 0; i < this->p->mapId->pkArea.size(); i++)
		{
			borr.addLongInt(this->p->mapId->pkArea.at(i).minX);
			borr.addLongInt(this->p->mapId->pkArea.at(i).maxX);
			borr.addLongInt(this->p->mapId->pkArea.at(i).minY);
			borr.addLongInt(this->p->mapId->pkArea.at(i).maxY);
		}

		borr.addLongInt(GetPlayersForMapNumber(this->p->mapId->id, this->p->getId()));

		borr.ready();
		this->AddPacket(borr, 0);

		if(!this->p->mapId->weaponAllowed)
			this->p->equipped = false;

		if(this->p->moving)
		{
			this->p->moving = false;
			this->p->cMove = 0;
		}

		this->SpawnForMap(this->p->mapId->id);

		this->ingame = true;
		this->p->botActive = false;
	}

	else
	{
		int pos = 3;

		/*
		this->p->x = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
		pos += ((int)b[pos] + 1);

		this->p->y = (float)atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
		*/

		Map *m = GetMap(this->p->mapId->id);
		if(m == NULL)
			log(ERR, "[void Client::ChangeMap(char*,bool,int,int,int)] [Map is NULL] [%d]\n", this->p->mapId->id);
		else
		{
			for(unsigned int i = 0; i < m->tzone.size(); i++)
			{
				if(this->InArea(m->tzone.at(i).xFrom, m->tzone.at(i).yFrom, (m->tzone.at(i).xTo - m->tzone.at(i).xFrom), (m->tzone.at(i).yTo - m->tzone.at(i).yFrom)))
				{
					if(this->p->follow)
						wc->sendPlayerFollow(this, this, false);

					if(this->p->trading)
						this->CancelTrade(true);

					this->RemoveAllFollowers();
					this->Unspawn(0);

					int oldMap = this->p->mapId->id;

					this->p->mapId = GetMap(m->tzone.at(i).toMap);

					this->p->x = (float)m->tzone.at(i).toMapX;

					if(m->tzone.at(i).toMapY != 1337)
						this->p->y = (float)m->tzone.at(i).toMapY;

					else
					{
						if(this->p->y >= (this->p->mapId->resY - this->p->meH))
							this->p->y = (float)(this->p->mapId->resY - this->p->meH);
					}

					Packet borr2 = Packet();

					borr2.addHeader(0x10);
					borr2.addInt(m->tzone.at(i).toMap);
					borr2.addBool(this->p->mapId->pk);
					borr2.addBool(this->p->mapId->weaponAllowed);
					borr2.addLongInt(m->tzone.at(i).toMapX);
					
					if(m->tzone.at(i).toMapY != 1337)
						borr2.addLongInt(m->tzone.at(i).toMapY);
					else
						borr2.addLongInt((int)this->p->y);

					borr2.addLongInt(this->p->mapId->xSize);
					borr2.addLongInt(this->p->mapId->ySize);

					borr2.addLongInt(this->p->mapId->pkArea.size());

					for(unsigned int u = 0; u < this->p->mapId->pkArea.size(); u++)
					{
						borr2.addLongInt(this->p->mapId->pkArea.at(u).minX);
						borr2.addLongInt(this->p->mapId->pkArea.at(u).maxX);
						borr2.addLongInt(this->p->mapId->pkArea.at(u).minY);
						borr2.addLongInt(this->p->mapId->pkArea.at(u).maxY);
					}

					borr2.addLongInt(GetPlayersForMapNumber(this->p->mapId->id, this->p->getId()));

					borr2.ready();
					this->AddPacket(borr2, 0);

					if(!this->p->mapId->weaponAllowed)
						this->p->equipped = false;

					if(this->p->moving)
					{
						this->p->moving = false;
						this->p->cMove = 0;
					}

					this->SpawnForMap(this->p->mapId->id);

					this->ingame = true;
					this->p->botActive = false;

					if(debugs)
						log(DEBUG, "Teleport: %d %d %d.\n", m->tzone.at(i).toMap, m->tzone.at(i).toMapX, m->tzone.at(i).toMapY);

					LD(GAME, "Change world: [Name: %s] [From: %d] [To: %d].\n", this->p->name, oldMap, this->p->mapId->id);

					break;
				}
			}
		}
	}
}

void Client::PlayerPosition(float x, float y, int param)
{
	this->p->x = x;
	this->p->y = y;

	Packet pak = Packet();
	
	pak.addHeader(0x11);
	pak.addLongInt(this->p->getId());
	pak.addLongInt((int)x);
	pak.addLongInt((int)y);

	pak.ready();

	SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);

	if(param != OTHER)
		this->AddPacket(pak, 0);
}

void Client::Unspawn(int param)
{
	if(param == ALL)
	{
		Packet pak = Packet();

		pak.addHeader(0x12);
		pak.addInt(0);
		pak.addInt(0);
		
		pak.ready();
		this->AddPacket(pak, 0);
	}

	else
	{
		Packet pak = Packet();

		pak.addHeader(0x12);
		pak.addInt(1);
		pak.addLongInt(this->p->getId());

		pak.ready();
		SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);

		if(this->ingame)
			this->ingame = false;

		if(debugs)
			log(DEBUG, "Unspawned player: %s %d.\n", this->p->name, this->p->getId());
	}
}

void Client::SpawnForMap(int map)
{
	Packet pak = Packet();

	pak.addHeader(0x5);
	pak.addInt(0);
	pak.addInt(1);
	pak.addInt(this->p->level);
	pak.addInt(this->p->power);
	pak.addInt(this->p->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
	pak.addBool(this->p->equipped);
	pak.addBool(false);
	pak.addInt(this->p->weaponWay);

	pak.addString(this->p->name);
	pak.addLongInt(this->p->getId());
	pak.addBool(false);

	pak.addLongInt((int)this->p->x);
	pak.addLongInt((int)this->p->y);

	pak.addBool(this->p->moving);
	pak.addInt(this->p->cMove);

	int eqToUserItems = 0;

	for(int i = 0; i < 4; i++)
	{
		if(this->p->eq[i] != NULL)
			eqToUserItems++;
	}

	pak.addInt(eqToUserItems);

	for(int t = 0; t < 4; t++)
	{
		if(this->p->eq[t] != NULL)
		{
			pak.addInt((t + 1));
			pak.addLongInt(this->p->eq[t]->id);
		}
	}

	pak.addInt(GetPlayerProcentHP(this->p));
	pak.addBool(false);
	pak.addLongInt(0);
	pak.addInt(this->p->job);

	pak.ready();

	if(this->p->pt == NULL)
		SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);

	else
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			Client *c = ac.at(i);

			if(c->ingame)
			{
				if(c->p != NULL)
				{
					if(c->p->mapId != NULL)
					{
						if(this->p->mapId->id == c->p->mapId->id)
						{
							if(c->p->pt != this->p->pt)
							{
								if(this->sock != c->GetSocket())
									c->AddPacket(pak, 0);
							}
						}
					}
				}
			}
		}
	}

	if(this->p->pt != NULL)
	{
		Packet pak2 = Packet();

		pak2.addHeader(0x5);
		pak2.addInt(0);
		pak2.addInt(1);
		pak2.addInt(this->p->level);
		pak2.addInt(this->p->power);
		pak2.addInt(this->p->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
		pak2.addBool(this->p->equipped);
		pak2.addBool(false);
		pak2.addInt(this->p->weaponWay);

		pak2.addString(this->p->name);
		pak2.addLongInt(this->p->getId());
		pak2.addBool(true);

		pak2.addLongInt((int)this->p->x);
		pak2.addLongInt((int)this->p->y);

		pak2.addBool(this->p->moving);
		pak2.addInt(this->p->cMove);

		int eqToUserItems = 0;

		for(int i = 0; i < 4; i++)
		{
			if(this->p->eq[i] != NULL)
				eqToUserItems++;
		}

		pak2.addInt(eqToUserItems);

		for(int t = 0; t < 4; t++)
		{
			if(this->p->eq[t] != NULL)
			{
				pak2.addInt((t + 1));
				pak2.addLongInt(this->p->eq[t]->id);
			}
		}

		pak2.addInt(GetPlayerProcentHP(this->p));
		pak2.addBool(this->p->follow);
		pak2.addLongInt(this->p->followId);
		pak2.addLongInt(this->p->job);

		pak2.ready();

		for(unsigned int i = 0; i < ac.size(); i++)
		{
			Client *c = ac.at(i);

			if(c->ingame)
			{
				if(c->p != NULL)
				{
					if(c->p->mapId != NULL)
					{
						if(this->p->mapId->id == c->p->mapId->id)
						{
							if(c->p->pt == this->p->pt)
							{
								if(this->sock != c->GetSocket())
									c->AddPacket(pak2, 0);
							}
						}
					}
				}
			}
		}
	}

	int pNum = GetPlayersForMapNumber(this->p->mapId->id, this->p->getId());

	if(pNum > 0)
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			Client *c = ac.at(i);

			if(c->ingame)
			{
				if(c->p->mapId->id == map && c->p->getId() != this->p->getId())
				{
					Packet s = Packet();

					s.addHeader(0x5);
					s.addInt(0);
					s.addInt(1);
					s.addInt(c->p->level);
					s.addInt(c->p->power);
					s.addInt(c->p->speed + (c->GetBonus(SPEED) - c->GetBonus(SPEEDMINUS)));
					s.addBool(c->p->equipped);
					s.addBool(c->p->dead);
					s.addInt(c->p->weaponWay);

					s.addString(c->p->name);
					s.addLongInt(c->p->getId());

					if(c->p->pt == this->p->pt && this->p->pt != NULL)
						s.addBool(true);
					else
						s.addBool(false);

					s.addLongInt((int)c->p->x);
					s.addLongInt((int)c->p->y);

					s.addBool(c->p->moving);
					s.addInt(c->p->cMove);

					int eqToUserItems = 0;

					for(int i = 0; i < 4; i++)
					{
						if(c->p->eq[i] != NULL)
							eqToUserItems++;
					}

					s.addInt(eqToUserItems);

					for(int t = 0; t < 4; t++)
					{
						if(c->p->eq[t] != NULL)
						{
							s.addInt((t + 1));
							s.addLongInt(c->p->eq[t]->id);
						}
					}

					s.addInt(GetPlayerProcentHP(c->p));
					s.addBool(c->p->follow);
					s.addLongInt(c->p->followId);
					s.addInt(c->p->job);

					s.ready();
					this->AddPacket(s, 0);
				}
			}
		}
	}

	std::vector<Monster*> mob = GetMonstersForMap(map);

	if(mob.size() > 0)
	{
		Packet m = Packet();
		m.addHeader(0x5);
		m.addInt(3);
		m.addLongInt(mob.size());

		for(unsigned int i = 0; i < mob.size(); i++)
		{
			m.addInt(mob.at(i)->level);
			m.addInt(mob.at(i)->attack);
			m.addInt(mob.at(i)->id);
			m.addInt(0);
			m.addInt(0);
			m.addLongInt((int)mob.at(i)->x);
			m.addLongInt((int)mob.at(i)->y);
			m.addLongInt(mob.at(i)->pid);
			m.addLongInt(mob.at(i)->followId);
			m.addBool(mob.at(i)->goAfter);
			m.addLongInt((int)mob.at(i)->bx);
			m.addLongInt((int)mob.at(i)->by);
			m.addBool(mob.at(i)->aggro);
			m.addLongInt((mob.at(i)->speed + ((mob.at(i)->GetMonsterBonus(SPEED) * 20) - (mob.at(i)->GetMonsterBonus(SPEEDMINUS) * 20))));
			m.addBool(mob.at(i)->moving);
			m.addInt(mob.at(i)->goDir);
			m.addInt(GetMonsterProcentHP(mob.at(i)));
		}

		m.ready();
		this->AddPacket(m, 0);
	}

	std::vector<NPC*> npc = GetNPCForMap(map);

	if(npc.size() > 0)
	{
		for(unsigned int i = 0; i < npc.size(); i++)
		{
			Packet pakn = Packet();

			pakn.addHeader(0x5);
			pakn.addInt(10);
			pakn.addLongInt(1);
			pakn.addLongInt(npc.at(i)->x);
			pakn.addLongInt(npc.at(i)->y);
			pakn.addLongInt(npc.at(i)->id);
			pakn.addString(npc.at(i)->name);

			pakn.addInt(npc.at(i)->menu.size());
			
			for(unsigned int y = 0; y < npc.at(i)->menu.size(); y++)
				pakn.addString(npc.at(i)->menu.at(y).text);

			pakn.ready();
			this->AddPacket(pakn, 0);
		}
	}

	std::vector<Drop*> dro = GetDropsForMap(map);

	if(dro.size() > 0)
	{
		Packet dr = Packet();
		dr.addHeader(0x39);
		dr.addInt(1);
		dr.addLongInt(dro.size());

		for(unsigned int d = 0; d < dro.size(); d++)
		{
			dr.addInt(dro.at(d)->drop->id);
			dr.addInt(dro.at(d)->drop->value);
			dr.addLongInt(dro.at(d)->id);
			dr.addLongInt(dro.at(d)->x);
			dr.addLongInt(dro.at(d)->y);
		}

		dr.ready();
		this->AddPacket(dr, 0);
	}

	if(debugs)
		log(DEBUG, "Spawned for map: %s %d.\n", this->p->name, this->p->getId());
}

float CalculateHP(Client *c)
{
	float temphp = ((((float)c->p->magic + (float)c->GetBonus(STAMINA)) * 5.0f) + ((float)c->p->level * 1.5f) + 18.0f);

	temphp += ((float)c->GetBonus(HPPLUS) - (float)c->GetBonus(HPMINUS));
	temphp += GetClass(c->p->job)->hpplus;

	if(temphp <= 0)
		temphp = 1.0f;

	temphp *= (1.0f + (((float)c->GetBonus(HPPROCENTPLUS) - (float)c->GetBonus(HPPROCENTMINUS)) / 100.0f));

	int correct = (int)(temphp + 0.5f);
	float returncorrect = (float)correct;

	return returncorrect;
}

float CalculateEnergy(Client *c)
{
	Class *gc = GetClass(c->p->job);
	float energy = 0.0f;

	if(gc->egot)
	{
		energy += gc->eplus;
	}

	return energy;
}

float CalculateMana(Client *c)
{
	Class *gc = GetClass(c->p->job);

	if(!gc->mpgot)
		return 0.0f;

	float tempmana = ((float)c->p->intt + (float)c->GetBonus(INTT));
	tempmana *= 8.0f;
	tempmana += ((float)c->p->level * 1.5f);
	tempmana += 30.0f;
	tempmana += ((float)c->GetBonus(MANAPLUS) - (float)c->GetBonus(MANAMINUS));
	tempmana += gc->mpplus;

	if(tempmana <= 0)
		tempmana = 1.0f;

	tempmana *= (1.0f + (((float)c->GetBonus(MANAPROCENTPLUS) - (float)c->GetBonus(MANAPROCENTMINUS)) / 100.0f));

	int correct = (int)(tempmana + 0.5f);
	float returncorrect = (float)correct;

	/*
	float temphp = ((((float)c->p->magic + (float)c->GetBonus(STAMINA)) * 5.0f) + ((float)c->p->level * 1.5f) + 18.0f);

	temphp += ((float)c->GetBonus(HPPLUS) - (float)c->GetBonus(HPMINUS));

	if(temphp <= 0)
		temphp = 1.0f;

	temphp *= (1.0f + (((float)c->GetBonus(HPPROCENTPLUS) - (float)c->GetBonus(HPPROCENTMINUS)) / 100.0f));

	int correct = (int)temphp;
	float returncorrect = (float)correct;
	*/

	return returncorrect;
}

void Client::SavePlayer(int PARAM)
{
	char slots[10];
	_itoa_s(this->p->slot, slots, 10);

	string charp = "Data/Character/";
	charp += this->p->owner;
	charp += slots;
	charp += ".txt";

	if(PARAM == QUITSAVE)
	{
		if(this->p->dead)
		{
			if(this->p->mapId->id != mPlayerStartMap && !this->p->mapId->pk)
				this->p->mapId = GetMap(mPlayerStartMap);

			if(!this->p->mapId->pk)
				this->RemoveExp(3);

			this->p->x = (float)mPlayerStartX;
			this->p->y = (float)mPlayerStartY;

			this->p->dead = false;

			this->p->reviveTime = 0;
		}

		if(this->p->pt != NULL)
		{								
			for(unsigned int r = 0; r < this->p->pt->members.size(); r++)
			{
				if(this->p->pt->members.at(r)->mb->p->getId() == this->p->getId())
				{
					this->p->pt->members.at(r)->online = false;

					break;
				}
			}

			this->p->pt = NULL;
		}

		this->p->moving = false;
		this->p->equipped = false;

		this->p->godMode = false;
		this->p->botActive = false;

		this->p->follow = false;
		this->p->followId = 0;

		if(this->p->trading)
			this->CancelTrade(true);

		this->p->trading = false;
		this->p->tradeWith = 0;
		this->p->doneTrade = false;
		this->p->tradePenya = 0;

		this->p->tradeInventory.clear();

		this->p->attacked = false;
		this->p->atkTimer = 0;

		this->p->weaponWay = RIGHT;
		this->p->cMove = 0;
	}

	int itm = 0;
	int itmeq = 0;
	int tsk = 0;

	ofstream o(charp.c_str());

	o << this->p->name << endl << this->p->level << endl << this->p->power << endl << this->p->agility << endl << (int)this->p->x << endl << (int)this->p->y << endl << this->p->mapId->id << endl << this->p->exp << endl << this->p->magic << endl << this->p->powerpoints << endl << this->p->pkpoints << endl << this->p->access << endl;
	
	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
			itm++;
	}

	o << (itm + 1) << endl;

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
			o << this->p->inv[i]->id << " " << (i + 1) << " " << this->p->inv[i]->stack << endl;
	}

	o << 2 << " " << this->p->gold << " " << 0 << endl;

	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
	{
		this->p->buffs.at(i)->endTime = (this->p->buffs.at(i)->endTime - GetTickCount());

		if(this->p->buffs.at(i)->endTime <= 0)
			this->p->buffs.at(i)->endTime = 0;
	}

	o << this->p->buffs.size() << endl;
	
	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
		o << this->p->buffs.at(i)->effect << " " << this->p->buffs.at(i)->value << " " << this->p->buffs.at(i)->endTime << endl;

	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
		this->p->buffs.at(i)->endTime += GetTickCount();

	for(int i = 0; i < 9; i++)
	{
		if(this->p->task[i]->enable)
			tsk++;
	}

	o << tsk << endl;

	for(int i = 0; i < 9; i++)
	{
		if(this->p->task[i]->enable)
		{
			int el = 0;

			if(this->p->task[i]->eqinv)
				el = 1;
			else
				el = 0;

			o << (i + 1) << " " << this->p->task[i]->type << " " << this->p->task[i]->slot << " " << el << " " << this->p->task[i]->id << endl;;
		}
	}

	o << this->p->skills.size() << endl;

	for(unsigned int i = 0; i < this->p->skills.size(); i++)
		o << this->p->skills.at(i)->id << " " << this->p->skills.at(i)->type << " " << this->p->skills.at(i)->lvl << " " << this->p->skills.at(i)->exp << endl;

	for(int i = 0; i < 4; i++)
	{
		if(this->p->eq[i] != NULL)
			itmeq++;
	}

	o << itmeq << endl;

	for(int i = 0; i < 4; i++)
	{
		if(this->p->eq[i] != NULL)
			o << this->p->eq[i]->id << " " << (i + 1) << endl;
	}

	o << this->p->intt << endl;

	o << this->p->friends.size() << endl;

	for(unsigned int i = 0; i < this->p->friends.size(); i++)
	{
		o << this->p->friends.at(i).name.c_str() << endl;
	}

	o << this->p->job << endl;

	o.close();

	if(PARAM == QUITSAVE)
		this->OptimizeBuffs();

	if(debugs)
		log(DEBUG, "Saved player: %s %d.\n", this->p->name, this->p->getId());
}

std::vector<Monster*> GetMonstersForMap(int mapId)
{
	std::vector<Monster*> retType;

	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		if(mobs.at(i)->mapId->id == mapId && mobs.at(i)->spawned)
			retType.push_back(mobs.at(i));
	}

	return retType;
}

int GetMonsterForMapNumber(int mapId)
{
	int n = 0;

	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		if(mobs.at(i)->mapId->id == mapId)
			n++;
	}

	return n;
}

std::vector<NPC*> GetNPCForMap(int mapId)
{
	std::vector<NPC*> retType;

	for(unsigned int i = 0; i < npcs.size(); i++)
	{
		if(npcs.at(i)->mapId->id == mapId)
			retType.push_back(npcs.at(i));
	}

	return retType;
}

int GetPlayersForMapNumber(int mapId, int myId)
{
	int n = 0;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p->mapId->id == mapId && ac.at(i)->p->getId() != myId)
				n++;
		}
	}

	return n;
}

std::vector<Drop*> GetDropsForMap(int mapId)
{
	std::vector<Drop*> retType;

	for(unsigned int i = 0; i < drops.size(); i++)
	{
		if(drops.at(i)->map == mapId)
			retType.push_back(drops.at(i));
	}

	return retType;
}

Monster *GetMonster(int id)
{
	Monster *ret = NULL;

	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		if(mobs.at(i)->pid == id)
		{
			ret = mobs.at(i);

			break;
		}
	}

	return ret;
}

void Client::MonsterAttack(int id)
{
	Monster *m = GetMonster(id);
	if(m == NULL)
		log(ERR, "[void Client::MonsterAttack(int)] [Monster is NULL] [%d]\n", id);

	else
	{
		if(m->mapId->id == this->p->mapId->id)
		{
			int type = 0;
			float bHp = this->p->chp;

			float mAttack = ((float)m->attack * 1.3f);

			float pDefence = (float)(this->p->magic + this->GetBonus(STAMINA));
			pDefence /= 2.5f;

			if(pDefence < 0)
				pDefence = 0;

			pDefence += ((float)this->p->level / 3.2f);
			pDefence += (float)this->GetEquipBonus(DEFENCE);
			pDefence += (float)this->GetBonus(DEFENCE);

			int intDam = 0;
			float fullDamage = 0;

			mAttack -= pDefence;

			if(mAttack > 0)
			{
				float atkMin = (mAttack - 1);
				float atkMax = (mAttack + 1);

				if(atkMin <= 0)
					atkMin = 0;

				mAttack = (float)Random((int)atkMin, (int)atkMax);

				float critChance = (100.0f / (float)m->critChance);

				if(critChance <= 1)
					critChance = 1.0f;

				float critornot = RandomFloat(1.0f, critChance);

				float blocks = (100.0f / (((float)blockChance + ((float)this->p->agility + (float)this->GetBonus(AGILITY))) + this->GetBonus(BLOCK)));

				if(blocks <= 1)
					blocks = 1.0f;

				float blockornot = RandomFloat(1.0f, blocks);
				
				if((int)(critornot + 0.5f) == 1)
				{
					mAttack *= RandomFloat(1.5f, 2.5f);

					type = 1;
				}

				if((int)(blockornot + 0.5f) == 1)
				{
					mAttack /= 4.0f;

					if(type == 1)
						type = 3;

					else
						type = 2;
				}

				fullDamage = mAttack;

				if(fullDamage < 1)
					fullDamage = 0;

				intDam = (int)fullDamage;

				if(debugs)
					log(DEBUG, "Monster attack: %d Player defence: %d Final attack: %d.\n", (int)mAttack, (int)pDefence, (int)fullDamage);

				if(this->p->godMode && this->p->access >= GM)
				{
					intDam = 0;
					fullDamage = 0;
				}
			}

			else
			{
				float critChance2 = (100.0f / (float)m->critChance);

				if(critChance2 <= 1)
					critChance2 = 1.0f;

				float critornot2 = RandomFloat(1.0f, critChance2);

				float blocks2 = (100.0f / (((float)blockChance + ((float)this->p->agility + (float)this->GetBonus(AGILITY))) + this->GetBonus(BLOCK)));

				if(blocks2 <= 1)
					blocks2 = 1.0f;

				float blockornot2 = RandomFloat(1.0f, blocks2);
				
				if((int)(critornot2 + 0.5f) == 1)
					type = 1;

				if((int)(blockornot2 + 0.5f) == 1)
				{
					if(type == 1)
						type = 3;

					else
						type = 2;
				}
			}

			if(this->p->trading && !this->p->dead)
				this->CancelTrade(true);

			if((bHp - fullDamage) > 0)
			{
				this->p->chp -= fullDamage;

				this->com->setAttackTimer(playerAttackTimer);

				if(m->followId == this->p->getId() && m->follows)
				{
					if(m->buffs.size() > 0)
					{
						for(unsigned int i = 0; i < m->buffs.size(); i++)
						{
							if(Random(1, (int)((100.0f / m->buffs.at(i).chance))) == 1)
								this->AddBuff(m->buffs.at(i).effect, m->buffs.at(i).value, m->buffs.at(i).dur, 0);
						}
					}
				}

				wc->sendDamage(this->p, NULL, type, intDam);
			}

			else
			{
				if(!this->p->dead)
				{
					fullDamage = (float)this->p->chp;

					wc->sendDamage(this->p, NULL, type, (int)fullDamage);
					wc->killPlayer(this);
				}
			}

			string bhpstring = FloatToString(bHp);
			string newstring = FloatToString(this->p->chp);

			if(strcmp(bhpstring.c_str(), newstring.c_str()) != 0 && intDam != 0)
				this->com->sendUpdate(HP);
		}

		else
			log(ERR, "[void Client::MonsterAttack(int)] [Monster attacks from another map] [%d] [%d]\n", this->p->mapId->id, m->mapId->id);
	}
}

void Client::UpdatePosition(char *p)
{
	int pos = 3;

	int x = atoi(getString(p, (pos + 1), (int)p[pos]).c_str());
	pos += ((int)p[pos] + 1);

	int y = atoi(getString(p, (pos + 1), (int)p[pos]).c_str());

	Packet pak = Packet();

	pak.addHeader(0x14);
	pak.addLongInt(this->p->getId());
	pak.addLongInt((int)this->p->x);
	pak.addLongInt((int)this->p->y);

	pak.ready();
	SendAllOnMap(pak, 0, this->p->mapId->id, this->sock);
}

void Client::IsOnline(bool status)
{
	if(!this->onlineCheck)
		this->onlineCheck = true;
}

void Client::Quit()
{
	Player *seeme = this->p;

	if(this->p->pt != NULL)
	{
		for(unsigned int r = 0; r < this->p->pt->members.size(); r++)
		{
			if(this->p->pt->members.at(r)->mb->p->getId() == this->p->getId())
			{
				this->p->pt->members.at(r)->online = false;

				break;
			}
		}
	}
	
	this->RemoveAllFollowers();

	this->ingame = false;

	this->Logout();

	this->Unspawn(0);
	this->SavePlayer(QUITSAVE);

	if(seeme->mapId != NULL)
	{
		if(seeme->access <= 50)
		{
			string welcomeNotice2 = seeme->name;
			welcomeNotice2 += " logged out.";

			for(unsigned int i = 0; i < ac.size(); i++)
			{
				if(ac.at(i)->ingame)
					SystemChat(ac.at(i), CUSTOM, NULL, (char*)welcomeNotice2.c_str());
			}
		}
	}

	this->UpdateFriends(false);
}

void Client::Chat(char *msg)
{
	if(this->p->access >= GM)
	{
		char command[200] = "";

		sscanf_s(msg, "%s", &command, 200);

		if(strcmp(command, "/tele") == 0)
		{
			int mapId = this->p->mapId->id;

			sscanf_s(msg, "/tele %d", &mapId);

			if(GetMap(mapId) != NULL && this->p->mapId->id != mapId)
			{
				this->ChangeMap(NULL, true, mapId, (int)this->p->x, (int)this->p->y);

				SystemChat(this, TELEPORTED, NULL, NULL);
			}
		}

		else if(strcmp(command, "/playersonline") == 0)
		{
			char mOnlineChar[10] = "1";
			char mSystemMessage[30] = "Players online: ";

			_itoa_s(PlayersOnline(), mOnlineChar, 10, 10);
			strcat_s(mSystemMessage, mOnlineChar);
			strcat_s(mSystemMessage, ".");

			vector<Client*> cPlayersOnline;

			for(unsigned int i = 0; i < ac.size(); i++)
			{
				if(ac.at(i)->ingame)
					cPlayersOnline.push_back(ac.at(i));
			}

			for(unsigned int i = 0; i < cPlayersOnline.size(); i++)
			{
				_itoa_s((i + 1), mOnlineChar, 10, 10);

				string esMsg = mOnlineChar;

				esMsg += ". ";
				esMsg += "Name: ";
				esMsg += cPlayersOnline.at(i)->p->name;
				esMsg += ". Level: ";

				_itoa_s(cPlayersOnline.at(i)->p->level, mOnlineChar, 10, 10);

				esMsg += mOnlineChar;
				esMsg += ". IP: ";
				esMsg += cPlayersOnline.at(i)->GetIP();
				esMsg += ". Ping: ";
				esMsg += IntToString(cPlayersOnline.at(i)->lastPing);
				esMsg += ".";

				SystemChat(this, CUSTOM, NULL, (char*)esMsg.c_str());
			}

			SystemChat(this, CUSTOM, NULL, mSystemMessage);
		}

		else if(strcmp(command, "/errors") == 0)
		{
			string errorMsg = "Errors: ";
			errorMsg += IntToString(serverError);
			errorMsg += ".";
			SystemChat(this, CUSTOM, NULL, (char*)errorMsg.c_str());
		}

		else if(strcmp(command, "/n") == 0)
		{
			char msgNotice[100] = "";

			for(unsigned int i = 3; i < strlen(msg); i++)
				msgNotice[(i - 3)] = msg[i];

			SendAllOnline(CreatePacketChat(msgNotice, "Notice", false, false, 0), 0);
		}

		else if(strcmp(command, "/addgold") == 0)
		{
			int amount = 0;

			sscanf_s(msg, "/addgold %d", &amount);

			this->AddItem(2, amount, false, NULL, amount);
		}

		else if(strcmp(command, "/god") == 0)
		{
			if(this->p->godMode)
			{
				this->p->godMode = false;

				SystemChat(this, CUSTOM, NULL, "God-mode disabled.");
			}

			else
			{
				this->p->godMode = true;

				SystemChat(this, CUSTOM, NULL, "God-mode enabled.");
			}
		}

		else if(strcmp(command, "/exprate") == 0)
		{
			char systemMsg[100] = "Exprate changed to ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/exprate %d", &amount);

			if(amount > 0)
			{
				expRate = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, "x.");

				SystemChat(this, CUSTOM, NULL, systemMsg);
			}
		}

		else if(strcmp(command, "/droprate") == 0)
		{
			char systemMsg[100] = "Droprate changed to ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/droprate %d", &amount);

			if(amount > 0)
			{
				dropRate = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, "x.");

				SystemChat(this, CUSTOM, NULL, systemMsg);
			}
		}

		else if(strcmp(command, "/goldrate") == 0)
		{
			char systemMsg[100] = "Goldrate changed to ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/goldrate %d", &amount);

			if(amount > 0)
			{
				penyaRate = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, "x.");

				SystemChat(this, CUSTOM, NULL, systemMsg);
			}
		}

		else if(strcmp(command, "/seta") == 0)
		{
			char names[40] = "";
			int accessLvl = 0;

			sscanf_s(msg, "/seta %s %d", &names, 40, &accessLvl);

			if(accessLvl >= 50 && accessLvl <= 52)
			{
				Player *p = GetPlayerByName(names);

				if(p == NULL)
					SystemChat(this, CUSTOM, NULL, "Player dosen't exist.");
				else
				{
					p->access = accessLvl;

					SystemChat(this, CUSTOM, NULL, "Players access level modified.");
				}
			}

			else
				SystemChat(this, CUSTOM, NULL, "Invalid access param.");
		}

		else if(strcmp(command, "/setstr") == 0)
		{
			char systemMsg[100] = "Your strength is now ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/setstr %d", &amount);

			if(amount > 0)
			{
				this->p->power = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(this, CUSTOM, NULL, systemMsg);

				this->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't have 0 strength!");
		}

		else if(strcmp(command, "!setstr") == 0)
		{
			char systemMsg[100] = "Your strength is now ";
			char tempe[10] = "";
			char name[40] = "";
			int amount = 0;

			sscanf_s(msg, "!setstr %d %s", &amount, &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				SystemChat(this, CUSTOM, NULL, "Could not find player.");
				return;
			}

			if(amount > 0)
			{
				c->p->power = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(c, CUSTOM, NULL, systemMsg);

				c->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't set 0 strength!");
		}

		else if(strcmp(command, "/setint") == 0)
		{
			char systemMsg[100] = "Your intelligence is now ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/setint %d", &amount);

			if(amount > 0)
			{
				this->p->intt = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(this, CUSTOM, NULL, systemMsg);

				this->com->sendUpdate(STAT);

				this->p->maxmana = CalculateMana(this);

				if(this->p->mana > this->p->maxmana)
					this->p->mana = this->p->maxmana;

				this->com->sendUpdate(MANA);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't have 0 strength!");
		}

		else if(strcmp(command, "!setint") == 0)
		{
			char systemMsg[100] = "Your intelligence is now ";
			char tempe[10] = "";
			char name[40] = "";
			int amount = 0;

			sscanf_s(msg, "!setint %d %s", &amount, &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				SystemChat(this, CUSTOM, NULL, "Could not find player.");
				return;
			}

			if(amount > 0)
			{
				c->p->intt = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(c, CUSTOM, NULL, systemMsg);

				c->com->sendUpdate(STAT);

				c->p->maxmana = CalculateMana(c);

				if(c->p->mana > c->p->maxmana)
					c->p->mana = c->p->maxmana;

				c->com->sendUpdate(MANA);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't set 0 intelligence!");
		}

		else if(strcmp(command, "/setagi") == 0)
		{
			char systemMsg[100] = "Your agility is now ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/setagi %d", &amount);

			if(amount > 0)
			{
				this->p->agility = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(this, CUSTOM, NULL, systemMsg);

				this->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't have 0 agility!");
		}

		else if(strcmp(command, "!setagi") == 0)
		{
			char systemMsg[100] = "Your agility is now ";
			char tempe[10] = "";
			int amount = 0;
			char name[40] = "";

			sscanf_s(msg, "!setagi %d %s", &amount, &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				SystemChat(this, CUSTOM, NULL, "Could not find player.");
				return;
			}

			if(amount > 0)
			{
				c->p->agility = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(c, CUSTOM, NULL, systemMsg);

				c->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't set 0 agility!");
		}

		else if(strcmp(command, "/setsta") == 0)
		{
			char systemMsg[100] = "Your stamina is now ";
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/setsta %d", &amount);

			if(amount > 0)
			{
				this->p->magic = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(this, CUSTOM, NULL, systemMsg);

				this->p->hp = CalculateHP(this);

				if(this->p->chp > this->p->hp)
					this->p->chp = this->p->hp;

				this->com->sendUpdate(HP);
				this->com->sendUpdate(HPPROCENT);
				this->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't have 0 stamina!");
		}

		else if(strcmp(command, "!setsta") == 0)
		{
			char systemMsg[100] = "Your stamina is now ";
			char tempe[10] = "";
			int amount = 0;
			char name[40] = "";

			sscanf_s(msg, "!setsta %d %s", &amount, &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				SystemChat(this, CUSTOM, NULL, "Could not find player.");
				return;
			}

			if(amount > 0)
			{
				c->p->magic = amount;

				_itoa_s(amount, tempe, 10, 10);

				strcat_s(systemMsg, tempe);
				strcat_s(systemMsg, ".");

				SystemChat(c, CUSTOM, NULL, systemMsg);

				c->p->hp = CalculateHP(c);

				if(c->p->chp > c->p->hp)
					c->p->chp = c->p->hp;

				c->com->sendUpdate(HP);
				c->com->sendUpdate(HPPROCENT);
				c->com->sendUpdate(STAT);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You can't set 0 stamina!");
		}

		else if(strcmp(command, "/setlvl") == 0)
		{
			char tempe[10] = "";
			int amount = 0;

			sscanf_s(msg, "/setlvl %d", &amount);

			if(amount > 0 && amount <= maxLvl)
			{
				if(amount == this->p->level)
					SystemChat(this, CUSTOM, NULL, "You are already that level!");
				else
				{
					this->SetLevel(amount);
				}
			}

			else
				SystemChat(this, CUSTOM, NULL, "Invalid level command.");
		}

		else if(strcmp(command, "/dc") == 0)
		{
			char name[20] = "";

			sscanf_s(msg, "/dc %s", &name, 20);

			if(strcmp(name, this->p->name) == 0)
				SystemChat(this, CUSTOM, NULL, "You cannot disconnect yourself!");
			
			else
			{
				Client *c = GetClientByPlayerName(name);
				if(c == NULL)
					SystemChat(this, CUSTOM, NULL, "Player is not online.");

				else
					c->disconnect = true;
			}
		}

		else if(strcmp(command, "/bot") == 0)
		{
			if(this->p->botActive)
			{
				this->p->botActive = false;

				SystemChat(this, CUSTOM, NULL, "Bot deactivated.");
			}

			else
			{
				this->p->botActive = true;

				SystemChat(this, CUSTOM, NULL, "Bot activated.");
			}

			Packet pak = Packet();

			pak.addHeader(0x57);
			pak.addBool(this->p->botActive);
			
			if(this->p->mapId->pk)
				pak.addInt(2);

			else
				pak.addInt(1);

			pak.ready();
			this->AddPacket(pak, 0);
		}

		else if(strcmp(command, "/additem") == 0)
		{
			int amount = 0;
			int id = 0;

			sscanf_s(msg, "/additem %d %d", &id, &amount);

			vector<TwoInt> checkVector;

			TwoInt tw;

			tw.int1 = id;
			tw.int2 = amount;

			checkVector.push_back(tw);

			if(GetItemById(id) == NULL)
				SystemChat(this, CUSTOM, NULL, "An item with that ID does not exist.");

			else
			{
				if(this->CheckInventorySpace(checkVector))
				{
					if(!this->AddItem(id, amount, false, NULL, amount))
						SystemChat(this, CUSTOM, NULL, "No inventory space left.");
				}

				else
					SystemChat(this, CUSTOM, NULL, "No inventory space left.");
			}
		}

		else if(strcmp(command, "/teletome") == 0)
		{
			char name[40] = "";

			sscanf_s(msg, "/teletome %s", &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				string errorMsg = name;

				errorMsg += " does not exist in-game.";

				SystemChat(this, CUSTOM, NULL, (char*)errorMsg.c_str());
			}

			else
			{
				if(c->p->mapId->id == this->p->mapId->id)
					c->PlayerPosition(this->p->x, this->p->y, 0);

				else
				{
					c->ChangeMap(NULL, true, this->p->mapId->id, (int)this->p->x, (int)this->p->y);

					SystemChat(c, TELEPORTED, NULL, NULL);
				}
			}
		}

		else if(strcmp(command, "/telemeto") == 0)
		{
			char name[40] = "";

			sscanf_s(msg, "/telemeto %s", &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				string errorMsg = name;

				errorMsg += " does not exist in-game.";

				SystemChat(this, CUSTOM, NULL, (char*)errorMsg.c_str());
			}

			else
			{
				if(c->p->mapId->id == this->p->mapId->id)
					this->PlayerPosition(c->p->x, c->p->y, 0);

				else
				{
					this->ChangeMap(NULL, true, c->p->mapId->id, (int)c->p->x, (int)c->p->y);

					SystemChat(this, TELEPORTED, NULL, NULL);
				}
			}
		}

		else if(strcmp(command, "/telealltome") == 0)
		{
			for(unsigned int i = 0; i < ac.size(); i++)
			{
				if(ac.at(i)->ingame)
				{
					if(ac.at(i)->p->getId() != this->p->getId())
					{
						if(ac.at(i)->p->mapId->id == this->p->mapId->id)
							ac.at(i)->PlayerPosition(this->p->x, this->p->y, 0);

						else
						{
							ac.at(i)->ChangeMap(NULL, true, this->p->mapId->id, (int)this->p->x, (int)this->p->y);

							SystemChat(ac.at(i), TELEPORTED, NULL, NULL);
						}
					}
				}
			}
		}

		else if(strcmp(command, "/addmonster") == 0)
		{
			int name = 0;

			int aggroChance = 0;
			int nMobs = 0;

			bool aggro = false;

			sscanf_s(msg, "/addmonster %d %d %d", &name, &nMobs, &aggroChance);

			if(debugs)
				log(DEBUG, "Spawning mobs: %d %d %d.\n", name, nMobs, aggroChance);

			if(aggroChance == 1)
				aggro = true;

			Monster *struc = NULL;

			for(unsigned int i = 0; i < mobs.size(); i++)
			{
				if(mobs.at(i)->id == name)
				{
					struc = mobs.at(i);

					break;
				}
			}

			if(struc != NULL)
			{
				Packet pakm = Packet();
				pakm.addHeader(0x5);
				pakm.addInt(3);
				pakm.addLongInt(nMobs);

				for(int i = 0; i < nMobs; i++)
				{
					wc->createMonster(name, this->p->mapId->id, (float)Random((int)(this->p->x - 50), (int)(this->p->x + this->p->meW + 50)), (float)Random((int)(this->p->y - 50), (int)(this->p->y + this->p->meH + 50)), aggro, struc);

					Monster *m = mobs.back();

					pakm.addInt(m->level);
					pakm.addInt(m->attack);
					pakm.addInt(m->id);
					pakm.addInt(0);
					pakm.addInt(0);
					pakm.addLongInt((int)m->x);
					pakm.addLongInt((int)m->y);

					pakm.addLongInt(m->pid);
					pakm.addLongInt(0);
					pakm.addBool(false);
					pakm.addLongInt(0);
					pakm.addLongInt(0);
					pakm.addBool(m->aggro);
					pakm.addLongInt((m->speed + ((m->GetMonsterBonus(SPEED) * 20) - (m->GetMonsterBonus(SPEEDMINUS) * 20))));
					pakm.addBool(m->moving);
					pakm.addInt(m->goDir);
					pakm.addInt(GetMonsterProcentHP(m));
				}

				pakm.ready();
				SendAllOnMap(pakm, 0, this->p->mapId->id, 0);
			}

			else
				SystemChat(this, CUSTOM, NULL, "Monster with that ID does not exist.");
		}

		else if(strcmp(command, "/aroundkill") == 0)
		{
			bool lolborr = this->p->godMode;

			if(!this->p->godMode)
				this->p->godMode = true;

			this->SkillAttackAoE(&GetMonstersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, this->p->mapId->xSize, this->p->mapId->ySize), 5, 1, STRENGTH);

			this->p->godMode = lolborr;
		}

		else if(strcmp(command, "/pvpdmg") == 0)
		{
			int lol = 0;

			sscanf_s(msg, "/pvpdmg %d", &lol);

			if(lol == 0)
				mPvPEnablePoints = false;

			else if(lol == 1)
				mPvPEnablePoints = true;

			string msgs = "PvP point-damage is ";

			if(mPvPEnablePoints)
				msgs += "enabled.";

			else
				msgs += "disabled.";

			SystemChat(this, CUSTOM, NULL, (char*)msgs.c_str());
		}

		else if(strcmp(command, "/ip") == 0)
		{
			char name[40] = "";

			sscanf_s(msg, "/ip %s", &name, 40);

			Client *cP = GetClientByPlayerName(name);

			if(cP != NULL)
			{
				string fullMsg = name;

				fullMsg += "'s IP: ";
				fullMsg += cP->GetIP();
				fullMsg += ".";

				SystemChat(this, CUSTOM, NULL, (char*)fullMsg.c_str());
			}

			else
				SystemChat(this, CUSTOM, NULL, "That player is not online.");
		}

		else if(strcmp(command, "/ping") == 0)
		{
			char name[40] = "";

			sscanf_s(msg, "/ping %s", &name, 40);

			Client *cP = GetClientByPlayerName(name);

			if(cP != NULL)
			{
				string fullMsg = name;

				fullMsg += "'s ping: ";
				fullMsg += IntToString(cP->lastPing);
				fullMsg += ".";

				SystemChat(this, CUSTOM, NULL, (char*)fullMsg.c_str());
			}

			else
				SystemChat(this, CUSTOM, NULL, "That player is not online.");
		}

		else if(strcmp(command, "/clearinv") == 0)
		{
			for(int i = 0; i < 25; i++)
			{
				if(this->p->inv[i] != NULL)
					this->RemoveItem((i + 1), this->p->inv[i]->stack, false, this->p->inv[i]->id);
			}

			SystemChat(this, CUSTOM, NULL, "Your inventory was cleaned.");
		}

		else if(strcmp(command, "!clearinv") == 0)
		{
			char name[40] = "";

			sscanf_s(msg, "!clearinv %s", &name, 40);

			Client *c = GetClientByPlayerName(name);

			if(c == NULL)
			{
				SystemChat(this, CUSTOM, NULL, "Could not find player.");
				return;
			}

			for(int i = 0; i < 25; i++)
			{
				if(c->p->inv[i] != NULL)
					c->RemoveItem((i + 1), c->p->inv[i]->stack, false, c->p->inv[i]->id);
			}

			SystemChat(c, CUSTOM, NULL, "Your inventory was cleaned.");
		}

		else if(strcmp(command, "/s") == 0)
		{
			char msgNotice[400] = "";

			for(unsigned int i = 3; i < strlen(msg); i++)
				msgNotice[(i - 3)] = msg[i];

			SendAllOnline(CreatePacketChat(msgNotice, this->p->name, false, false, 3), 0);
		}

		else if(strcmp(command, "/g") == 0)
		{
			if(this->p->pt != NULL)
			{
				char msgNotice[400] = "";

				for(unsigned int i = 3; i < strlen(msg); i++)
					msgNotice[(i - 3)] = msg[i];

				SendAllInParty(CreatePacketChat(msgNotice, this->p->name, false, false, 1), this->p->pt, NULL, 0);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You are not in a party.");
		}

		else if(strcmp(command, "/p") == 0)
		{
			if(this->p->pt != NULL)
			{
				char msgNotice[400] = "";

				for(unsigned int i = 3; i < strlen(msg); i++)
					msgNotice[(i - 3)] = msg[i];

				SendAllInParty(CreatePacketChat(msgNotice, this->p->name, false, false, 1), this->p->pt, NULL, 0);
			}

			else
				SystemChat(this, CUSTOM, NULL, "You are not in a party.");
		}

		else if(strcmp(command, "/pos") == 0)
		{
			string posMsg = "Position: ";
			posMsg += IntToString((int)(this->p->x + 0.5f));
			posMsg += " ";
			posMsg += IntToString((int)(this->p->y + 0.5f));
			posMsg += ".";

			SystemChat(this, CUSTOM, NULL, (char*)posMsg.c_str());
		}

		else if(strcmp(command, "/pm") == 0)
		{
			char msgSayName[100] = "";
			string msgSay = "";

			sscanf_s(msg, "/pm %s", &msgSayName, 100);

			for(unsigned int i = 5 + strlen(msgSayName); i < strlen(msg); i++)
				msgSay += msg[i];

			this->SendPM(msgSayName, msgSay.c_str());
		}

		else
		{
			SendAllOnMap(CreatePacketChat(msg, this->p->name, false, false, 0), 0, this->p->mapId->id, 0);
		}
	}

	else
	{
		if((unsigned)this->p->chatCoolDown <= GetTickCount())
		{
			char coms[200] = "";
			sscanf_s(msg, "%s", &coms, 200);

			if(strcmp(coms, "/s") == 0)
			{
				char msgNotice[400] = "";

				for(unsigned int i = 3; i < strlen(msg); i++)
					msgNotice[(i - 3)] = msg[i];

				SendAllOnline(CreatePacketChat(msgNotice, this->p->name, false, false, 3), 0);
			}

			else if(strcmp(coms, "/g") == 0)
			{
				if(this->p->pt != NULL)
				{
					char msgNotice[400] = "";

					for(unsigned int i = 3; i < strlen(msg); i++)
						msgNotice[(i - 3)] = msg[i];

					SendAllInParty(CreatePacketChat(msgNotice, this->p->name, false, false, 1), this->p->pt, NULL, 0);
				}

				else
					SystemChat(this, CUSTOM, NULL, "You are not in a party.");
			}

			else if(strcmp(coms, "/p") == 0)
			{
				if(this->p->pt != NULL)
				{
					char msgNotice[400] = "";

					for(unsigned int i = 3; i < strlen(msg); i++)
						msgNotice[(i - 3)] = msg[i];

					SendAllInParty(CreatePacketChat(msgNotice, this->p->name, false, false, 1), this->p->pt, NULL, 0);
				}

				else
					SystemChat(this, CUSTOM, NULL, "You are not in a party.");
			}

			else if(strcmp(coms, "/pos") == 0)
			{
				string posMsg = "Position: ";
				posMsg += IntToString((int)(this->p->x + 0.5f));
				posMsg += " ";
				posMsg += IntToString((int)(this->p->y + 0.5f));
				posMsg += ".";

				SystemChat(this, CUSTOM, NULL, (char*)posMsg.c_str());
			}

			else if(strcmp(coms, "/pm") == 0)
			{
				char msgSayName[100] = "";
				string msgSay = "";

				sscanf_s(msg, "/pm %s", &msgSayName, 100);

				for(unsigned int i = 5 + strlen(msgSayName); i < strlen(msg); i++)
					msgSay += msg[i];

				this->SendPM(msgSayName, msgSay.c_str());
			}

			else
			{
				SendAllOnMap(CreatePacketChat(msg, this->p->name, false, false, 0), 0, this->p->mapId->id, 0);
			}

			this->com->setChatCoolDown(600);
		}

		else
			SystemChat(this, CUSTOM, NULL, "You have to wait one second between each message.");
	}
}

int PlayersOnline()
{
	int r = 0;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
			r++;
	}

	return r;
}

void Client::Attack(int id, int type, int param, int hits, char *b, bool serv, int plus, bool poss, int based, bool lifesteal)
{
	if(enableAttack)
	{
		if(this->p->dead)
			return;

		if(!serv)
		{
			int pos = 3;

			id = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
			pos += ((int)b[pos] + 1);

			type = b[pos];
			param = b[(pos + 1)];
			pos += 2;

			hits = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
		}

		if(IsSkill(param))
		{
			this->SpecialAttack(id, type, param);

			return;
		}

		if(type == 0)
		{
			this->PKAttack(id, type, param, hits, plus, poss, based, lifesteal);

			return;
		}

		Monster *m = GetMonster(id);
		if(m == NULL)
		{
			Packet p22 = Packet();

			p22.addHeader(0x12);
			p22.addInt(2);
			p22.addLongInt(id);

			p22.ready();
			SendAllOnMap(p22, 0, this->p->mapId->id, 0);
		}

		else
		{
			if(m->mapId->id == this->p->mapId->id)
			{
				if(m->spawned)
				{
					if((m->attackerId == 0 || this->p->getId() == m->attackerId || enableKS || this->AttackInParty(m->attackerId)) && !m->goAfter)
					{
						int type = 0;
						int typeBased = 0;

						switch(based)
						{
							case STRENGTH: typeBased = this->p->power;
								break;

							case STAMINA: typeBased = this->p->magic;
								break;

							case AGILITY: typeBased = this->p->agility;
								break;

							case INTT: typeBased = this->p->intt;
								break;
						}

						int pStr = (typeBased + this->GetBonus(based));

						if(pStr < 0)
							pStr = 0;

						Class *gc = GetClass(this->p->job);

						if(gc == NULL)
							log(ERR, "[void Client::Attack(int,int,int,int,char*,bool,int,bool,int,bool)] [Class is NULL] [%d]\n", this->p->job);

						float pAttack = ((float)pStr * gc->baseMult);

						pAttack += (float)this->GetEquipBonus(ATTACK);
						pAttack += (float)this->GetBonus(ATTACK);
						pAttack += ((float)this->p->level / 2.5f);
						pAttack *= (1.0f + ((float)this->GetBonus(TOTALDMG) / 100.0f));
						pAttack += 4.0f;
						pAttack += plus;

						if(hits > 1)
							pAttack /= (float)hits;

						float pAtk = pAttack;

						if(pAtk > 0)
						{		
							float atkMin = (pAtk - 4.0f);
							float atkMax = (pAtk + 4.0f);

							if(atkMin <= 0)
								atkMin = 0;

							pAtk = RandomFloat(atkMin, atkMax);

							float critChance = (100.0f / (((float)this->p->critChance + (float)(this->p->agility + this->GetBonus(AGILITY))) + (float)this->GetBonus(CRIT)));

							if(critChance <= 1)
								critChance = 1;

							float blocks = (100.0f / (float)blockChance);

							if(blocks <= 1)
								blocks = 1;

							float critornot = RandomFloat(1.0f, critChance);
							float blockornot = RandomFloat(1.0f, blocks);

							if((int)(critornot + 0.5f) == 1 && poss)
							{
								pAtk *= RandomFloat(1.5f, 2.5f);

								type = 1;
							}

							if((int)(blockornot + 0.5f) == 1 && poss)
							{
								pAtk /= 4.0f;

								if(type == 1)
									type = 3;

								else
									type = 2;
							}

							if(pAtk > 0)
							{
								float lvlDiff = (float)(m->level - this->p->level);

								if(lvlDiff > 0)
								{
									float multi = 1.0f;

									multi -= (lvlDiff * 0.03f);

									if(multi <= 0)
									{
										if(pAtk > 0)
											pAtk = 1;

										else
											pAtk = 0;
									}

									else
										pAtk *= multi;
								}
							}

							if(pAtk < 1)
								pAtk = 0;
						}

						else
						{
							pAtk = 0;

							float critChance2 = (100.0f / (((float)this->p->critChance + (float)(this->p->agility + this->GetBonus(AGILITY))) + (float)this->GetBonus(CRIT)));

							if(critChance2 <= 1)
								critChance2 = 1;

							float blocks2 = (100.0f / (float)blockChance);

							if(blocks2 <= 1)
								blocks2 = 1;

							float critornot2 = RandomFloat(1.0f, critChance2);
							float blockornot2 = RandomFloat(1.0f, blocks2);

							if((int)(critornot2 + 0.5f) == 1 && poss)
								type = 1;

							if((int)(blockornot2 + 0.5f) == 1 && poss)
							{
								if(type == 1)
									type = 3;

								else
									type = 2;
							}
						}

						if(this->p->godMode && this->p->access >= GM)
							pAtk = m->chp;

						if((m->chp - pAtk) <= 0)
						{
							pAtk = m->chp;

							wc->sendDamage(NULL, m, type, (int)pAtk);
							wc->killMonster(this, m, false);

							if(lifesteal)
							{
								if(this->GetBonus(LIFESTEAL) > 0)
									this->AddHP((pAtk * ((float)this->GetBonus(LIFESTEAL) / 100.0f)));
							}
						}

						else
						{
							m->chp -= pAtk;

							m->attacked = true;
							m->freeAttack = (GetTickCount() + monsterFreeDelay);

							m->attackerId = this->p->getId();

							if(m->followId == 0)
								wc->sendFollow(m, this);

							wc->sendDamage(NULL, m, type, (int)pAtk);

							if(lifesteal)
							{
								if(this->GetBonus(LIFESTEAL) > 0)
									this->AddHP((pAtk * ((float)this->GetBonus(LIFESTEAL) / 100.0f)));
							}
						}
					}

					else
					{
					}
				}

				else
				{
					Packet p2 = Packet();

					p2.addHeader(0x12);
					p2.addInt(2);
					p2.addLongInt(m->pid);

					p2.ready();
					this->AddPacket(p2, 0);
				}
			}

			else
				log(ERR, "[void Client::Attack(char*)] [Player attacks from another map] [%d] [%d]\n", this->p->mapId->id, m->mapId->id);
		}
	}
}

void Client::Shoot(char *b, bool ss, int sId, int target, bool allowSkill, int type)
{
	Class *gc = GetClass(this->p->job);

	bool needEq = gc->needweap;

	if(needEq)
	{
		if(this->p->equipped)
			needEq = true;

		else
			needEq = false;
	}

	else
		needEq = true;

	if(enableAttack && (needEq) && this->p->mapId->weaponAllowed)
	{
		if(this->p->trading)
			this->CancelTrade(true);

		if(!ss)
		{
			int pos = 5;
			int PARAM = b[3];
			int cWay = b[4];

			long lastTime = atoi(getString(b, pos + 1, (int)b[pos]).c_str());
			pos += (int)b[pos] + 1;

			int cXClient = atoi(getString(b, pos + 1, (int)b[pos]).c_str());
			pos += (int)b[pos] + 1;
			int cYClient = atoi(getString(b, pos + 1, (int)b[pos]).c_str());
			pos += (int)b[pos] + 1;

			int cX = cXClient;
			int cY = cYClient;

			/*
			switch(this->p->weaponWay)
			{
				case LEFT: cX = ((int)this->p->x - 40);
					break;

				case RIGHT: cX = ((int)this->p->x + (this->p->meW + 21));
					break;

				default: cX = ((int)this->p->x + (this->p->meW + 21));
					break;
			}
			*/

			if(IsSkill(PARAM))
			{
				this->CheckSkill(PARAM);

				if((unsigned)this->GetSkill(PARAM)->nextUsage <= GetTickCount() || allowSkill)
				{
					/*
					if(PARAM == 6 && target == 0)
					{
						this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
						this->com->setAttackTimer(playerAttackTimer);
						this->SetSkillCoolDown(PARAM);

						return;
					}
					*/

					Packet pak = Packet();

					pak.addHeader(0x16);
					pak.addInt(PARAM);
					pak.addInt(cWay);
					pak.addLongInt(cX);
					pak.addLongInt((cY + 1));
					pak.addLongInt(this->p->getId());
					pak.addLongInt(target);
					pak.addInt(type);
					pak.addLongInt(this->GetBonus(IGENOM));
					pak.addLongInt(gc->bulletSpeed);
					pak.addLongInt(gc->bulletRotate);
					pak.addLongInt(gc->bulletDis);

					pak.ready();
					SendAllOnMap(pak, 0, this->p->mapId->id, 0);

					/*
					if(PARAM != 5)
						this->AddSkillExp(PARAM, 5);

					this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
					this->com->setAttackTimer(playerAttackTimer);
					this->SetSkillCoolDown(PARAM);
					*/
				}
			}

			else
			{
				if(lastTime == this->p->bulletTimeStamp)
					return;

				this->p->bulletTimeStamp = lastTime;

				//if((unsigned)this->p->bulletCoolDown > getTime(false))
				//	return;

				this->com->setAttackTimer(playerAttackTimer);
				this->com->setBulletCoolDown((this->GetAttackDelay() - 3 - this->lastPing));

				Packet pak = Packet();

				pak.addHeader(0x16);
				pak.addInt(gc->normalShot);
				pak.addInt(cWay);
				pak.addLongInt(cX);
				pak.addLongInt((cY + 1));
				pak.addLongInt(this->p->getId());
				pak.addLongInt(target);
				pak.addInt(type);
				pak.addLongInt(this->GetBonus(IGENOM));
				pak.addLongInt(gc->bulletSpeed);
				pak.addLongInt(gc->bulletRotate);
				pak.addLongInt(gc->bulletDis);

				pak.ready();
				SendAllOnMap(pak, 0, this->p->mapId->id, 0);
			}
		}

		else
		{
			int PARAM = sId;
			int cWay = this->p->weaponWay;

			if(IsSkill(PARAM))
			{
				if((unsigned)this->GetSkill(PARAM)->nextUsage <= GetTickCount() || allowSkill)
				{
					/*
					if(PARAM == 6 && target == 0)
					{
						this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
						this->com->setAttackTimer(playerAttackTimer);
						this->SetSkillCoolDown(PARAM);

						return;
					}
					*/

					int xAxel = 0;
					int yAxel = (((int)this->p->y + (this->p->meH / 2)) + 1);

					if(this->p->weaponWay == LEFT)
						xAxel = ((int)this->p->x - ((this->p->wW + 1) + 49));

					else if(this->p->weaponWay == RIGHT)
						xAxel = ((int)this->p->x + ((this->p->wW + 1) + 49));

					Packet pak = Packet();

					pak.addHeader(0x16);
					pak.addInt(PARAM);
					pak.addInt(cWay);
					pak.addLongInt(xAxel);
					pak.addLongInt(yAxel);
					pak.addLongInt(this->p->getId());
					pak.addLongInt(target);
					pak.addInt(type);
					pak.addLongInt(this->GetBonus(IGENOM));
					pak.addLongInt(gc->bulletSpeed);
					pak.addLongInt(gc->bulletRotate);
					pak.addLongInt(gc->bulletDis);

					pak.ready();
					SendAllOnMap(pak, 0, this->p->mapId->id, 0);

					/*
					if(PARAM != 5)
						this->AddSkillExp(PARAM, 5);

					this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
					this->com->setAttackTimer(playerAttackTimer);
					this->SetSkillCoolDown(PARAM);
					*/
				}
			}

			else
			{
				int xAxel = 0;
				int yAxel = (((int)this->p->y + (this->p->meH / 2)) + 1);

				if(this->p->weaponWay == LEFT)
					xAxel = ((int)this->p->x - ((this->p->wW + 1) + 18));

				else if(this->p->weaponWay == RIGHT)
					xAxel = ((int)this->p->x + ((this->p->wW + 1) + 18));

				this->com->setAttackTimer(playerAttackTimer);

				Packet pak = Packet();

				pak.addHeader(0x16);
				pak.addInt(gc->normalShot);
				pak.addInt(cWay);
				pak.addLongInt(xAxel);
				pak.addLongInt(yAxel);
				pak.addLongInt(this->p->getId());
				pak.addLongInt(target);
				pak.addInt(type);
				pak.addLongInt(this->GetBonus(IGENOM));
				pak.addLongInt(gc->bulletSpeed);
				pak.addLongInt(gc->bulletRotate);
				pak.addLongInt(gc->bulletDis);

				pak.ready();
				SendAllOnMap(pak, 0, this->p->mapId->id, 0);
			}
		}
	}

	if(!this->p->equipped && this->p->mapId->weaponAllowed && ss && gc->needweap)
	{
		if(sId > 1)
			SystemChat(this, CUSTOM, NULL, "You must have a weapon equipped to use that skill.");
	}
}

Map *GetMap(int id)
{
	Map *ret = NULL;

	for(unsigned int i = 0; i < maps.size(); i++)
	{
		if(id == maps.at(i)->id)
		{
			ret = maps.at(i);

			break;
		}
	}

	return ret;
}

void Client::EquipWeapon(bool art)
{
	if(!art)
	{
		if(this->p->equipped)
		{
			this->p->equipped = false;

			Packet pak = Packet();

			pak.addHeader(0x17);
			pak.addBool(false);
			pak.addLongInt(this->p->getId());

			pak.ready();
			SendAllOnMap(pak, 0, this->p->mapId->id, 0);
		}
	}

	else
	{
		if(this->p->mapId->weaponAllowed)
		{
			this->p->equipped = true;

			Packet pak = Packet();

			pak.addHeader(0x17);
			pak.addBool(true);
			pak.addLongInt(this->p->getId());

			pak.ready();
			SendAllOnMap(pak, 0, this->p->mapId->id, 0);
		}

		else
		{
			Packet pak = Packet();

			pak.addHeader(0x17);
			pak.addInt(2);

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}

	if(debugs)
		log(DEBUG, "Player equip packet: %s %d.\n", this->p->name, this->p->getId());
}

void Client::AddExp(long amount)
{
	if(this->p->level < maxLvl)
	{
		bool lvled = false;
		amount = (amount * expRate);

		std::string bExp = FloatToString(this->CalculateExp());

		while(this->p->level < maxLvl)
		{
			long cLvlExp = expTable[(this->p->level - 1)];

			if((cLvlExp - this->p->exp) <= amount)
			{
				this->p->level++;

				this->p->powerpoints += 1;

				amount -= (cLvlExp - this->p->exp);

				this->p->exp = 0;

				if(!lvled)
					lvled = true;

				this->AddPacket(CreatePacketChat("Congratulations! Level up!", "Notice", false, false, 0), 0);

				if(this->p->pt != NULL)
				{
					std::string lvlMsg = this->p->name;
					lvlMsg += " leveled up!";

					for(unsigned int i = 0; i < ac.size(); i++)
					{
						Client *c = ac.at(i);

						if(c->ingame)
						{
							if(c->p != NULL)
							{
								if(c->p->pt != NULL)
								{
									if(c->p->pt == this->p->pt)
									{
										if(c->p->mapId->id == this->p->mapId->id)
										{
											if(this->sock != c->GetSocket())
												SystemChat(c, CUSTOM, NULL, (char*)lvlMsg.c_str());
										}
									}
								}
							}
						}
					}
				}
			}

			else
			{
				this->p->exp += amount;

				break;
			}
		}

		if(this->p->level == maxLvl && this->p->exp != 0)
			this->p->exp = 0;

		if(lvled)
		{
			this->p->hp = CalculateHP(this);
			this->p->chp = this->p->hp;

			this->p->maxmana = CalculateMana(this);
			this->p->mana = this->p->maxmana;

			this->p->energy = this->p->maxenergy;

			this->com->sendUpdate(HP);
			this->com->sendUpdate(HPPROCENT);
			this->com->sendUpdate(EXP);
			this->com->sendUpdate(MANA);
			this->com->sendUpdate(ENERGY);

			if(this->p->pt != NULL)
			{
				Packet lv = Packet();

				lv.addHeader(0x25);
				lv.addInt(this->p->level);
				lv.addString(this->p->name);

				lv.ready();
				SendAllInParty(lv, this->p->pt, NULL, 0);
			}

			this->UpdateFriends(true);
		}

		if(strcmp(bExp.c_str(), FloatToString(this->CalculateExp()).c_str()) != 0)
			this->com->sendUpdate(EXP);
	}
}

void Client::SetLevel(int level)
{
	this->p->level = level;
	this->p->exp = 0;

	this->com->sendUpdate(EXP);

	this->p->hp = CalculateHP(this);
	this->p->chp = this->p->hp;

	this->p->maxmana = CalculateMana(this);
	this->p->mana = this->p->maxmana;

	this->p->energy = this->p->maxenergy;

	this->com->sendUpdate(HP);
	this->com->sendUpdate(HPPROCENT);
	this->com->sendUpdate(MANA);
	this->com->sendUpdate(ENERGY);
}

long AmountExp(Player *p, int proc)
{
	long lvlTable = expTable[(p->level - 1)];
	float expt = (float)lvlTable;

	float expRemove = ((expt / 100.f) * proc);

	return (long)expRemove;
}

void Client::RemoveExp(int procent)
{
	if(this->p->level < maxLvl && this->p->level > 9)
	{
		float lastExp = this->CalculateExp();
		long expRemove = AmountExp(this->p, procent);

		if((this->p->exp - expRemove) <= 0)
			this->p->exp = 0;
		else
			this->p->exp -= expRemove;

		if(lastExp != this->CalculateExp())
			this->com->sendUpdate(EXP);
	}
}

float Client::CalculateExp()
{
	if(this->p->level < maxLvl)
	{
		if(this->p->exp > 0)
		{
			float cExp = (float)expTable[(this->p->level - 1)];
			float myExp = (float)this->p->exp;

			float doneExp = ((myExp / cExp) * 100.0f);

			return doneExp;
		}

		else
			return 0.0f;
	}

	else
		return 0.0f;
}

int GetMonsterProcentHP(Monster *t)
{
	float max = t->hp;
	float now = t->chp;

	if(now > 0)
	{
		float retType = ((now / max) * 100.f);

		return (int)retType;
	}

	else
		return 0;
}

int GetPlayerProcentHP(Player *p)
{
	float max = p->hp;
	float now = p->chp;

	if(now > 0)
	{
		float retType = ((now / max) * 100.f);

		return (int)retType;
	}

	else
		return 0;
}

int Client::GetEquipBonus(int TYPE)
{
	int retType = 0;

	if(TYPE == DEFENCE)
	{
		for(int i = 1; i < 4; i++)
		{
			if(this->p->eq[i] != NULL)
				retType += this->p->eq[i]->value;
		}
	}

	else if(TYPE == ATTACK)
	{
		if(this->p->eq[0] != NULL)
			retType += this->p->eq[0]->value;
	}

	return retType;
}

void Client::PKAttack(int id, int type, int param, int hits, int plus, bool poss, int based, bool lifesteal)
{
	if(enableAttack)
	{
		if(this->p->mapId->pk)
		{
			Client *target = GetClientByPlayerId(id);

			if(target == NULL)
				log(ERR, "[void Client::PKAttack(int)] [Player is NULL] [%d]\n", id);

			else
			{
				if(target->p->mapId->id == this->p->mapId->id)
				{
					if(target->p->trading)
						target->CancelTrade(true);

					if(!target->PvPArea() || !this->PvPArea())
					{
						SystemChat(this, CUSTOM, NULL, "You can not attack in safezone.");

						return;
					}

					int type = 0;
					int typeBased = 0;

					switch(based)
					{
						case STRENGTH: typeBased = this->p->power;
							break;

						case STAMINA: typeBased = this->p->magic;
							break;

						case AGILITY: typeBased = this->p->agility;
							break;

						case INTT: typeBased = this->p->intt;
							break;
					}

					int pStr = (typeBased + this->GetBonus(based));

					if(pStr < 0)
						pStr = 0;

					float pAttack = ((float)pStr * 3.1f);

					pAttack += (float)this->GetEquipBonus(ATTACK);
					pAttack += (float)this->GetBonus(ATTACK);
					pAttack += ((float)this->p->level / 4.5f);

					if(mPvPEnablePoints)
					{
						float pLevel = ((float)this->p->level / 200.0f);

						pLevel *= (float)this->p->pkpoints;
						pAttack += pLevel;
					}

					pAttack *= (((float)this->GetBonus(TOTALDMG) / 100.0f) + 1.0f);
					pAttack += 4.0f;
					pAttack += plus;

					int tSta = (target->p->magic + target->GetBonus(STAMINA));

					float tDefence = ((float)tSta * 1.4f);

					tDefence += (float)target->GetEquipBonus(DEFENCE);
					tDefence += (float)target->GetBonus(DEFENCE);
					tDefence += ((float)target->p->level / 4.5f);
					tDefence += 4.0f;

					if(tDefence < 0)
						tDefence = 0;

					float pAtk = (pAttack - tDefence);

					if(debugs)
						log(DEBUG, "Player attack: %4.2f Target defence: %4.2f Final attack: %4.2f.\n", pAttack, tDefence, pAtk);

					if(pAtk > 0)
					{		
						float atkMin = (pAtk - 4.0f);
						float atkMax = (pAtk + 4.0f);

						if(atkMin <= 0)
							atkMin = 0;

						pAtk = RandomFloat(atkMin, atkMax);

						float critChance = (100.0f / (((float)this->p->critChance + (float)(this->p->agility + this->GetBonus(AGILITY))) + (float)this->GetBonus(CRIT)));
						float blocks = (100.0f / (((float)blockChance + ((float)target->p->agility + (float)target->GetBonus(AGILITY))) + (float)target->GetBonus(BLOCK)));

						if(critChance <= 1)
							critChance = 1.0f;

						if(blocks <= 1)
							blocks = 1.0f;

						float critornot = RandomFloat(1.0f, critChance);
						float blockornot = RandomFloat(1.0f, blocks);

						if((int)(critornot + 0.5f) == 1 && poss)
						{
							pAtk *= RandomFloat(1.5f, 2.5f);

							type = 1;
						}

						if(blockornot == 1 && poss)
						{
							pAtk /= 4.0f;

							if(type == 1)
								type = 3;
							
							else
								type = 2;
						}

						if(pAtk > 0)
						{
							pAtk *= 0.6f;

							if(pAtk <= 0)
								pAtk = 0;
						}

						if(pAtk > 0)
						{
							float lvlDiff = (float)(target->p->level - this->p->level);

							if(lvlDiff > 0)
							{
								float multi = 1.0f;

								multi -= (lvlDiff * 0.01f);

								if(multi <= 0)
								{
									if(pAtk > 0)
										pAtk = 0;

									else
										pAtk = 0;
								}

								else
									pAtk *= multi;
							}
						}

						else
							pAtk = 0;

						if(pAtk > 0)
							pAtk /= 2.0f;

						if(pAtk < 1)
							pAtk = 0;
					}

					else
					{
						float critChance2 = (100.0f / (((float)this->p->critChance + (float)(this->p->agility + this->GetBonus(AGILITY))) + (float)this->GetBonus(CRIT)));
						float blocks2 = (100.0f / (((float)blockChance + ((float)target->p->agility + (float)target->GetBonus(AGILITY))) + (float)target->GetBonus(BLOCK)));

						if(critChance2 <= 1)
							critChance2 = 1;

						if(blocks2 <= 1)
							blocks2 = 1;

						float critornot2 = RandomFloat(1.0f, critChance2);
						float blockornot2 = RandomFloat(1.0f, blocks2);

						if((int)(critornot2 + 0.5f) == 1 && poss)
							type = 1;

						if((int)(blockornot2 + 0.5f) == 1 && poss)
						{
							if(type == 1)
								type = 3;
							
							else
								type = 2;
						}

						pAtk = 0;
					}

					if(this->p->godMode && this->p->access >= GM)
						pAtk = target->p->chp;

					if(target->p->godMode && target->p->access >= GM)
						pAtk = 0;

					if((target->p->chp - pAtk) <= 0)
					{
						if(!target->p->dead)
						{
							pAtk = target->p->chp;

							wc->sendDamage(target->p, NULL, type, (int)pAtk);
							wc->killPlayer(target);

							SystemChat(this, PKKILLED, target, NULL);
							SystemChat(target, PKKILLEDBY, this, NULL);

							if(this->p->level < maxLvl)
								this->AddExp(target->p->level);

							if(target->p->pkpoints > 0)
							{
								target->p->pkpoints--;
								target->com->sendUpdate(PKPOINTS);

								if(target->p->gold > 0)
								{
									int goldAdd = target->p->level * 100;

									if(target->p->gold < goldAdd)
										goldAdd = target->p->gold;

									target->RemoveItem(2, goldAdd, false, 2);
									this->AddItem(2, goldAdd, true, NULL, randomInt);

									string penyaString = "You lost ";
									penyaString += IntToString(goldAdd);
									penyaString += " dinero to ";
									penyaString += this->p->name;
									penyaString += ".";

									SystemChat(target, CUSTOM, NULL, (char*)penyaString.c_str());

									penyaString = "You gained ";
									penyaString += IntToString(goldAdd);
									penyaString += " dinero from killing ";
									penyaString += target->p->name;
									penyaString += ".";

									SystemChat(this, CUSTOM, NULL, (char*)penyaString.c_str());
								}
							}

							this->p->pkpoints++;
							this->com->sendUpdate(PKPOINTS);

							string pwndString = this->p->name;
							pwndString += " pwned ";
							pwndString += target->p->name;
							pwndString += "!";

							Packet pwndPacket = CreatePacketChat(pwndString.c_str(), "Notice", false, false, 0);

							for(unsigned int l = 0; l < ac.size(); l++)
							{
								if(ac.at(l)->ingame)
									ac.at(l)->AddPacket(pwndPacket, 0);
							}

							if(lifesteal)
							{
								if(this->GetBonus(LIFESTEAL) > 0)
									this->AddHP((pAtk * ((float)this->GetBonus(LIFESTEAL) / 100.0f)));
							}
						}

						else
						{
							SystemChat(this, PKALREADYDEAD, NULL, NULL);
						}
					}

					else
					{
						target->com->setAttackTimer(playerAttackTimer);

						target->p->chp -= pAtk;

						if(pAtk > 0)
							target->com->sendUpdate(HP);

						wc->sendDamage(target->p, NULL, type, (int)pAtk);

						if(lifesteal)
						{
							if(this->GetBonus(LIFESTEAL) > 0)
								this->AddHP((pAtk * ((float)this->GetBonus(LIFESTEAL) / 100.0f)));
						}
					}
				}

				else
					log(ERR, "[void Client::PKAttack(int)] [Player attacks from another map] [%d] [%d]\n", this->p->mapId->id, target->p->mapId->id);
			}
		}
	}
}

Player *GetPlayerById(int id)
{
	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p != NULL)
			{
				if(ac.at(i)->p->getId() == id)
					return ac.at(i)->p;
			}
		}
	}

	return NULL;
}

Client *GetClientByPlayerId(int id)
{
	Client *retPointer = NULL;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p->getId() == id)
			{
				retPointer = ac.at(i);

				break;
			}
		}
	}

	return retPointer;
}

Client *GetClientByPlayerIdInMap(int id, int mapid)
{
	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p->mapId != NULL)
			{
				if(ac.at(i)->p->mapId->id == mapid)
				{
					if(ac.at(i)->p->getId() == id)
						return ac.at(i);
				}
			}
		}
	}

	return NULL;
}


void SystemChat(Client *_Client, int _Param, Client *_Client2, char *_Custom)
{
	char msg[1000] = "";

	Packet p = Packet();

	p.addHeader(0x15);

	switch(_Param)
	{
		case PKALREADYDEAD:
			{
				strcpy_s(msg, "That player is already dead.");
			break;
			}

		case PKATTACKSME:
			{
				strcpy_s(msg, _Client2->p->name);
				strcat_s(msg, " attacks you!");
			break;
			}

		case PKATTACKS:
			{
				char tHp[10] = "";

				strcpy_s(msg, "Attacked ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, " HP left: ");

				_itoa_s((int)_Client2->p->chp, tHp, 10, 10);

				strcat_s(msg, tHp);
			break;
			}

		case PKKILLED:
			{
				strcpy_s(msg, "You killed ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, ".");
			break;
			}

		case PKKILLEDBY:
			{
				strcpy_s(msg, "You were killed by ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, ".");
			break;
			}

		case PKINFECTPOISON:
			{
				strcpy_s(msg, "You poisoned ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, "!");
			break;
			}

		case WASPKINFECTPOISON:
			{
				strcpy_s(msg, _Client2->p->name);
				strcat_s(msg, " poisoned you!");
			break;
			}

		case PKSLOWSPEED:
			{
				strcpy_s(msg, "You slowed down ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, "!");
			break;
			}

		case WASPKSLOWSPEED:
			{
				strcpy_s(msg, "You were slowed down by ");
				strcat_s(msg, _Client2->p->name);
				strcat_s(msg, "!");
			break;
			}

		case CUSTOM: strcpy_s(msg, _Custom);
			break;

		case TELEPORTED:
			{
				char mCharId[10] = "1";

				_itoa_s(_Client->p->mapId->id, mCharId, 10, 10);

				strcpy_s(msg, "Teleported to world ");
				strcat_s(msg, mCharId);
				strcat_s(msg, ".");
			break;
			}

		case REVIVE:
			{
				char mMapId[10] = "1";

				_itoa_s(_Client->p->mapId->id, mMapId, 10, 10);

				strcpy_s(msg, "Resurrected at world ");
				strcat_s(msg, mMapId);
				strcat_s(msg, ".");
			break;
			}

		default: log(ERR, "[void SystemChat(Client*,int,Client*,char*)] [Invalid param] [%d]\n", _Param);
			break;
	}

	p.addString(msg);
	p.addString("System");
	p.addBool(false);
	p.addBool(false);
	p.addInt(0);

	p.ready();
	_Client->AddPacket(p, 0);
}

void Client::AddStat(char *b)
{
	if(this->p->powerpoints > 0)
	{
		int type = b[3];

		if(type == 20)
			this->p->power++;

		else if(type == 21)
		{
			this->p->magic++;

			this->p->hp = CalculateHP(this);

			if(this->p->chp > this->p->hp)
				this->p->chp = this->p->hp;

			this->com->sendUpdate(HP);
			this->com->sendUpdate(HPPROCENT);
		}

		else if(type == 22)
			this->p->agility++;

		else if(type == 23)
		{
			this->p->intt++;

			this->p->maxmana = CalculateMana(this);

			if(this->p->mana > this->p->maxmana)
				this->p->mana = this->p->maxmana;

			this->com->sendUpdate(MANA);
		}

		else
		{
			log(ERR, "[void Client::AddStat(char*)] [Invalid param] [%d]\n", type);
			
			return;
		}

		this->p->powerpoints -= 1;
		this->com->sendUpdate(STAT);
	}
}

Client *GetClientByPlayerIdStatus(int id, int &res)
{
	Client *ret = NULL;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p->getId() == id)
			{
				ret = ac.at(i);

				break;
			}
		}
	}

	if(ret == NULL)
	{
		for(unsigned int i = 0; i < pvector.size(); i++)
		{
			if(pvector.at(i)->getId() == id)
			{
				res = 1;

				break;
			}
		}
	}

	return ret;
}

Client *GetClientByName(char *name, int &result)
{
	Client *ret = NULL;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(strcmp(name, ac.at(i)->p->name) == 0)
			{
				ret = ac.at(i);

				break;
			}
		}
	}

	if(ret == NULL)
	{
		for(unsigned int i = 0; i < pvector.size(); i++)
		{
			if(strcmp(name, pvector.at(i)->name) == 0)
			{
				result = 1;

				break;
			}
		}
	}

	return ret;
}

Client *GetClientByPlayerName(char *name)
{
	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(strcmp(name, ac.at(i)->p->name) == 0)
				return ac.at(i);
		}
	}

	return NULL;
}

void Client::SendFriendInvite(int charId, const char *name)
{
	if(charId == this->p->getId() || strcmp(name, this->p->name) == 0)
		SystemChat(this, CUSTOM, NULL, "You can not add yourself as a friend?");

	else
	{
		for(unsigned int i = 0; i < this->p->friends.size(); i++)
		{
			if(charId == this->p->friends.at(i).id)
			{
				string errorString = "You already have ";
				errorString += name;
				errorString += " as you friend.";

				SystemChat(this, CUSTOM, NULL, (char*)errorString.c_str());
				return;
			}
		}

		int result = 0;

		Client *tar = GetClientByPlayerIdStatus(charId, result);

		if(tar == NULL)
		{
			string errorString = "The player '";
			errorString += name;
			errorString += "' does not exist.";

			string errorString2 = name;
			errorString2 += " is offline.";

			if(result == 0)
				SystemChat(this, CUSTOM, NULL, (char*)errorString.c_str());

			else if(result == 1)
				SystemChat(this, CUSTOM, NULL, (char*)errorString2.c_str());
		}

		else
		{
			Packet pak = Packet();

			pak.addHeader(0x0C);
			pak.addLongInt(this->p->getId());
			pak.addString(this->p->name);

			pak.ready();
			tar->AddPacket(pak, 0);

			SystemChat(this, CUSTOM, NULL, "Friend invitation sent.");
		}
	}
}

void Client::SendTradeInvite(int charId)
{
	if(charId == this->p->getId())
		SystemChat(this, CUSTOM, NULL, "You can not trade with yourself?");

	else
	{
		int result = 0;

		Client *tar = GetClientByPlayerIdStatus(charId, result);

		if(tar == NULL)
		{
			if(result == 0)
				log(ERR, "[void Client::SendTradeInvite(int)] [Could not find player] [%d]\n", charId);

			else if(result == 1)
				SystemChat(this, CUSTOM, NULL, "Your buddy is offline.");
		}

		else
		{
			if(tar->p->dead)
				SystemChat(this, CUSTOM, NULL, "You can not trade a player that's dead.");

			else if(tar->p->trading)
			{
				string tradeMsg = tar->p->name;

				tradeMsg += " is already trading someone.";

				SystemChat(this, CUSTOM, NULL, (char*)tradeMsg.c_str());
			}

			else
			{
				Packet pak = Packet();

				pak.addHeader(0x72);
				pak.addLongInt(this->p->getId());
				pak.addString(this->p->name);

				pak.ready();
				tar->AddPacket(pak, 0);
			}
		}
	}
}

void Client::SendInvite(char *name)
{
	if(strcmp(name, this->p->name) == 0)
	{
		SystemChat(this, CUSTOM, NULL, "You can not invite yourself.");
	}

	else
	{
		int result = 0;

		Client *target = GetClientByName(name, result);
		if(target == NULL)
		{
			if(result == 0)
			{
				char error[100] = "";

				strcpy_s(error, name);
				strcat_s(error, " does not exist.");

				SystemChat(this, CUSTOM, NULL, error);
			}

			else if(result == 1)
			{
				char error[100] = "";

				strcpy_s(error, name);
				strcat_s(error, " is not online.");

				SystemChat(this, CUSTOM, NULL, error);
			}
		}

		else
		{
			if(target->p->pt == NULL)
			{
				if(this->p->pt == NULL)
				{
					Packet pak = Packet();

					pak.addHeader(0x27);
					pak.addString(this->p->name);

					pak.ready();
					target->AddPacket(pak, 0);
				}

				else
				{
					if(this->p->pt->members.at(0)->mb->p->getId() == this->p->getId())
					{
						if(this->p->pt->members.size() < 6)
						{
							Packet pak = Packet();

							pak.addHeader(0x27);
							pak.addString(this->p->name);

							pak.ready();
							target->AddPacket(pak, 0);
						}

						else
							SystemChat(this, CUSTOM, NULL, "The party is full.");
					}

					else
						SystemChat(this, CUSTOM, NULL, "You are not the party leader.");
				}
			}

			else if(target->p->pt == this->p->pt)
			{
				char error[100] = "";

				strcpy_s(error, name);
				strcat_s(error, " is already in your party.");

				SystemChat(this, CUSTOM, NULL, error);
			}

			else
			{
				char error[100] = "";

				strcpy_s(error, name);
				strcat_s(error, " is in another party.");

				SystemChat(this, CUSTOM, NULL, error);
			}
		}
	}
}

void Client::AcceptFriendInvite(char *b)
{
	int res = 0;
	int clientaccept = b[3];
	int clientid = atoi(getString(b, 5, (int)b[4]).c_str());

	Client *inviteFrom = GetClientByPlayerIdStatus(clientid, res);

	if(inviteFrom == NULL)
	{
		if(res == 0)
			log(ERR, "[void Client::AcceptFriendInvite(char*)] [Friend inviter could not be found] [%d]\n", clientid);
	}

	else
	{
		if(clientaccept == 1)
		{
			inviteFrom->Friend(this->p->getId());
		}

		else if(clientaccept == 2)
		{
			char full[100] = "";

			strcpy_s(full, this->p->name);
			strcat_s(full, " refused to be your friend.");

			SystemChat(inviteFrom, CUSTOM, NULL, full);
		}
	}
}

void Client::AcceptTradeInvite(char *b)
{
	int res = 0;
	int clientaccept = b[3];
	int clientid = atoi(getString(b, 5, (int)b[4]).c_str());

	Client *inviteFrom = GetClientByPlayerIdStatus(clientid, res);

	if(inviteFrom == NULL)
	{
		if(res == 0)
			log(ERR, "[void Client::AcceptTradeInvite(char*)] [Trade inviter could not be found] [%d]\n", clientid);

		else if(res == 1)
			SystemChat(this, CUSTOM, NULL, "Your trade offer could not be found.");
	}

	else
	{
		if(clientaccept == 1)
		{
			if(inviteFrom->p->dead)
			{
				SystemChat(this, CUSTOM, NULL, "Your trade inviter is dead.");

				return;
			}

			if(this->p->dead)
			{
				SystemChat(inviteFrom, CUSTOM, NULL, "The player you asked to trade with is dead.");

				return;
			}

			if(inviteFrom->p->trading)
			{
				SystemChat(inviteFrom, CUSTOM, NULL, "You are already trading someone.");

				return;
			}

			if(this->p->trading)
			{
				string tradeMsg = this->p->name;

				tradeMsg += " is already trading someone.";

				SystemChat(inviteFrom, CUSTOM, NULL, (char*)tradeMsg.c_str());

				return;
			}

			if(this->p->mapId->id == inviteFrom->p->mapId->id)
			{
				if(this->InArea(((int)inviteFrom->p->x - 1024), ((int)inviteFrom->p->y - 1024), 2048, 2048))
					inviteFrom->Trade(this->p->getId());

				else
				{
					SystemChat(this, CUSTOM, NULL, "You are too far away to trade this player.");
					SystemChat(inviteFrom, CUSTOM, NULL, "You are too far away to trade this player.");
				}
			}

			else
			{
				SystemChat(this, CUSTOM, NULL, "You can not trade from another map.");
				SystemChat(inviteFrom, CUSTOM, NULL, "You can not trade from another map.");
			}
		}

		else if(clientaccept == 2)
		{
			char full[100] = "";

			strcpy_s(full, this->p->name);
			strcat_s(full, " refused to trade with you.");

			SystemChat(inviteFrom, CUSTOM, NULL, full);
		}
	}
}

void Client::AcceptInvite(char *b)
{
	int res = 0;
	int sta = b[3];
	char name[40] = "";

	strcpy_s(name, getString(b, 5, (int)b[4]).c_str());

	Client *inviteFrom = GetClientByName(name, res);

	if(inviteFrom == NULL)
	{
		if(res == 0)
			log(ERR, "[void Client::AcceptInvite(char*)] [Party inviter could not be found] [%s]\n", name);

		else if(res == 1)
			SystemChat(this, CUSTOM, NULL, "The inviter is offline.");
	}

	else
	{
		if(sta == 1)
			inviteFrom->InviteToParty(this->p->name);

		else if(sta == 2)
		{
			char full[100] = "";

			strcpy_s(full, this->p->name);
			strcat_s(full, " refused to join your party.");

			SystemChat(inviteFrom, CUSTOM, NULL, full);
		}
	}		
}

void Client::Friend(int id)
{
	Client *tar = GetClientByPlayerId(id);

	if(tar == NULL)
		log(ERR, "[void Client::Friend(int)] [Could not find player to be friends with] [%d]\n", id);

	else
	{
		for(unsigned int i = 0; i < this->p->friends.size(); i++)
		{
			if(this->p->friends.at(i).id == id)
				return;
		}

		Partner fri = Partner();

		fri.name = tar->p->name;
		fri.level = tar->p->level;
		fri.status = true;
		fri.job = 1;
		fri.id = tar->p->getId();

		Partner fri2 = Partner();

		fri2.name = this->p->name;
		fri2.level = this->p->level;
		fri2.status = true;
		fri2.job = 1;
		fri2.id = this->p->getId();

		this->p->friends.push_back(fri);
		tar->p->friends.push_back(fri2);

		Packet pak = Packet();

		pak.addHeader(0x0D);
		pak.addInt(1);
		pak.addString(tar->p->name);
		pak.addLongInt(tar->p->level);
		pak.addLongInt(1);
		pak.addLongInt(tar->p->getId());
		pak.addBool(true);

		pak.ready();

		Packet pak2 = Packet();

		pak2.addHeader(0x0D);
		pak2.addInt(1);
		pak2.addString(this->p->name);
		pak2.addLongInt(this->p->level);
		pak2.addLongInt(1);
		pak2.addLongInt(this->p->getId());
		pak2.addBool(true);

		pak2.ready();

		this->AddPacket(pak, 0);
		tar->AddPacket(pak2, 0);

		SystemChat(this, CUSTOM, NULL, "Friend invitation accepted.");

		LD(GAME, "Add friend: [Name: %s] [Name: %s].\n", this->p->name, tar->p->name);
	}
}

void Client::Trade(int id)
{
	Client *tar = GetClientByPlayerId(id);

	if(tar == NULL)
		log(ERR, "[void Client::Trade(int)] [Could not find player to open trade with] [%d]\n", id);

	else
	{
		this->p->trading = true;
		this->p->tradeWith = tar->p->getId();

		tar->p->trading = true;
		tar->p->tradeWith = this->p->getId();

		Packet pak = Packet();

		pak.addHeader(0x74);
		pak.addLongInt(tar->p->getId());
		pak.addString(tar->p->name);
		pak.addLongInt(this->p->getId());
		pak.addString(this->p->name);

		pak.ready();
		this->AddPacket(pak, 0);
		tar->AddPacket(pak, 0);
	}
}

void Client::CancelTrade(bool error)
{
	Client *tar = GetClientByPlayerId(this->p->tradeWith);

	this->p->trading = false;
	this->p->tradeWith = 0;
	this->p->doneTrade = false;

	this->p->tradeInventory.clear();
	this->p->tradePenya = 0;

	Packet pak = Packet();

	pak.addHeader(0x73);

	pak.ready();
	
	this->AddPacket(pak, 0);

	if(error)
		SystemChat(this, CUSTOM, NULL, "Trade cancelled.");

	if(tar == NULL)
		log(ERR, "[void Client::CancelTrade(bool)] [Tar is NULL] [%d]\n", this->p->getId());

	else
	{
		tar->p->trading = false;
		tar->p->tradeWith = 0;
		tar->p->doneTrade = false;

		tar->p->tradeInventory.clear();
		tar->p->tradePenya = 0;

		tar->AddPacket(pak, 0);

		if(error)
			SystemChat(tar, CUSTOM, NULL, "Trade cancelled.");
	}
}

void Client::InviteToParty(char *name)
{
	int none = 0;
	Client *target = GetClientByName(name, none);
	if(target == NULL)
		log(ERR, "[void Client::InviteToParty(char*)] [Player is NULL] [%s]\n", name);

	else
	{
		if(target->p->pt == NULL)
		{
			if(this->p->pt == NULL)
			{
				Party *p = new Party();

				PartyMember *pm = new PartyMember();
				PartyMember *pm2 = new PartyMember();

				pm->mb = this;
				pm->online = true;
				pm->setTimer = false;

				pm2->mb = target;
				pm2->online = true;
				pm2->setTimer = false;

				strcpy_s(pm->name, this->p->name);
				strcpy_s(pm2->name, target->p->name);

				p->members.push_back(pm);
				p->members.push_back(pm2);

				parties.push_back(p);

				this->p->pt = p;
				target->p->pt = p;

				if(debugs)
					log(DEBUG, "Party created! %s and %s.\n", this->p->name, target->p->name);

				Packet pak = Packet();

				pak.addHeader(0x24);
				pak.addInt(1);
				pak.addInt(1);
				pak.addInt(2);
				pak.addString(this->p->name);
				pak.addInt(this->p->level);

				pak.ready();
				target->AddPacket(pak, 0);

				Packet pak2 = Packet();

				pak2.addHeader(0x24);
				pak2.addInt(1);
				pak2.addInt(1);
				pak2.addInt(1);
				pak2.addString(target->p->name);
				pak2.addInt(target->p->level);
				
				pak2.ready();
				this->AddPacket(pak2, 0);

				SystemChat(this, CUSTOM, NULL, "A party has been created.");
				SystemChat(target, CUSTOM, NULL, "A party has been created.");
			}

			else
			{
				if(this->p->pt->members.at(0)->mb->p->getId() == this->p->getId())
				{
					if(this->p->pt->members.size() < 6)
					{
						PartyMember *pm = new PartyMember();
						
						pm->mb = target;
						pm->online = true;
						pm->setTimer = false;

						strcpy_s(pm->name, target->p->name);

						target->p->pt = this->p->pt;

						Packet pak = Packet();

						pak.addHeader(0x24);
						pak.addInt(1);
						pak.addInt(1);
						pak.addInt(2);
						pak.addString(target->p->name);
						pak.addInt(target->p->level);

						pak.ready();

						char full[100] = "";

						strcpy_s(full, target->p->name);
						strcat_s(full, " has joined the party.");

						for(unsigned int i = 0; i < this->p->pt->members.size(); i++)
						{
							this->p->pt->members.at(i)->mb->AddPacket(pak, 40);

							SystemChat(this->p->pt->members.at(i)->mb, CUSTOM, NULL, full);
						}

						Packet p2 = Packet();

						p2.addHeader(0x24);
						p2.addInt(this->p->pt->members.size());
						p2.addInt(1);
						p2.addInt(2);

						for(unsigned int i = 0; i < this->p->pt->members.size(); i++)
						{
							p2.addString(this->p->pt->members.at(i)->mb->p->name);
							p2.addInt(this->p->pt->members.at(i)->mb->p->level);
						}

						p2.ready();
						target->AddPacket(p2, 40);

						this->p->pt->members.push_back(pm);

						SystemChat(target, CUSTOM, NULL, full);

						if(debugs)
							log(DEBUG, "Invited %s to party.\n", target->p->name);
					}

					else
					{
						SystemChat(this, CUSTOM, NULL, "The party is full.\n");
					}
				}

				else
				{
					SystemChat(this, CUSTOM, NULL, "You are not the party leader.");
				}
			}
		}

		else if(target->p->pt == this->p->pt)
		{
			char error[100] = "";

			strcpy_s(error, name);
			strcat_s(error, " is already in your party.");

			SystemChat(this, CUSTOM, NULL, error);
		}

		else
		{
			char error[100] = "";

			strcpy_s(error, name);
			strcat_s(error, " is in another party.");

			SystemChat(this, CUSTOM, NULL, error);
		}
	}
}

void Client::LeaveParty()
{
	if(this->p->pt == NULL)
		SystemChat(this, CUSTOM, NULL, "You are currently not in a party.");

	else
	{
		Party *cpt = this->p->pt;

		for(unsigned int i = 0; i < cpt->members.size(); i++)
		{
			if(cpt->members.at(i)->mb->p->getId() == this->p->getId())
			{
				Packet pak = Packet();

				pak.addHeader(0x24);
				pak.addInt(1);
				pak.addInt(2);
				pak.addInt(2);
				pak.addString(this->p->name);

				pak.ready();

				char otherSystemChat[100] = "";

				strcpy_s(otherSystemChat, this->p->name);
				strcat_s(otherSystemChat, " has left the party.");

				for(unsigned int z = 0; z < cpt->members.size(); z++)
					cpt->members.at(z)->mb->AddPacket(pak, 0);

				SystemChat(cpt->members.at(i)->mb, CUSTOM, NULL, "You have left the party.");

				delete cpt->members.at(i);
				cpt->members.erase(cpt->members.begin() + i);

				for(unsigned int y = 0; y < cpt->members.size(); y++)
					SystemChat(cpt->members.at(y)->mb, CUSTOM, NULL, otherSystemChat);

				if(debugs)
					log(DEBUG, "Member left from party.\n");

				break;
			}
		}

		this->p->pt = NULL;
	}
}

void Client::KickFromParty(char *name)
{
	if(this->p->pt == NULL)
		SystemChat(this, CUSTOM, NULL, "You are currently not in a party.");

	else
	{
		if(this->p->pt->members.at(0)->mb->p->getId() == this->p->getId())
		{
			if(strcmp(this->p->name, name) == 0)
			{
				SystemChat(this, CUSTOM, NULL, "You can not kick yourself.");
			}

			else
			{
				bool exist = false;

				Party *p = this->p->pt;

				for(unsigned int i = 0; i < p->members.size(); i++)
				{
					if(strcmp(p->members.at(i)->mb->p->name, name) == 0)
					{
						Packet pak = Packet();

						pak.addHeader(0x24);
						pak.addInt(1);
						pak.addInt(2);
						pak.addInt(2);
						pak.addString(name);

						pak.ready();

						char otherSystemChat[100] = "";

						strcpy_s(otherSystemChat, name);
						strcat_s(otherSystemChat, " has left the party.");

						for(unsigned int z = 0; z < p->members.size(); z++)
							p->members.at(z)->mb->AddPacket(pak, 20);

						SystemChat(p->members.at(i)->mb, CUSTOM, NULL, "You have left the party.");

						p->members.at(i)->mb->p->pt = NULL;

						delete p->members.at(i);
						p->members.erase(p->members.begin() + i);

						for(unsigned int y = 0; y < p->members.size(); y++)
							SystemChat(p->members.at(y)->mb, CUSTOM, NULL, otherSystemChat);

						if(debugs)
							log(DEBUG, "Kicked member.\n");

						exist = true;
					}
				}

				if(!exist)
					SystemChat(this, CUSTOM, NULL, "That player is not in your party.");
			}
		}

		else
		{
			SystemChat(this, CUSTOM, NULL, "You are not the party leader.");
		}
	}
}

Buff *getBuffData(int PARAM)
{
	for(unsigned int i = 0; i < buffdata.size(); i++)
	{
		if(buffdata.at(i)->effect == PARAM)
			return buffdata.at(i);
	}

	return NULL;
}

int Client::GetBonus(int type)
{
	float totBonus = 0;

	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
	{
		if(this->p->buffs.at(i)->changeEffect == type)
			totBonus += (float)this->p->buffs.at(i)->value;
	}

	for(int i = 0; i < 4; i++)
	{
		if(this->p->eq[i] != NULL)
		{
			for(unsigned int y = 0; y < this->p->eq[i]->evalue.size(); y++)
			{
				if(this->p->eq[i]->evalue.at(y).int1 == type)
					totBonus += (float)this->p->eq[i]->evalue.at(y).int2;
			}
		}
	}

	Class *gc = GetClass(this->p->job);

	if(gc == NULL)
		log(ERR, "[int Client::GetBonus(int)] [Class is NULL] [%d]\n", this->p->job);

	else
	{
		for(unsigned int i = 0; i < gc->effects.size(); i++)
		{
			if(gc->effects.at(i).int1 == type)
				totBonus += gc->effects.at(i).int2;
		}
	}

	return (int)totBonus;
}

int Monster::GetMonsterBonus(int type)
{
	int totBonus = 0;

	for(unsigned int i = 0; i < this->mbuffs.size(); i++)
	{
		if(this->mbuffs.at(i)->changeEffect == type)
			totBonus += this->mbuffs.at(i)->value;
	}

	return totBonus;
}

void Client::AddBuff(int BUFF, int LVL, int DURSEC, int PLAYERPUT)
{
	if(!this->p->dead)
	{
		Buff *thisbuf = this->GetBuff(BUFF);
		Buff *buffd = getBuffData(BUFF);

		bool sendUpdate = true;

		if(buffd == NULL)
			log(ERR, "[void Client::AddBuff(int,int,int)] [Buff does not exist] [%d]\n", BUFF);

		else
		{
			Packet pak = Packet();

			pak.addHeader(0x28);
			pak.addInt(1);
			pak.addLongInt(BUFF);
			pak.addLongInt(buffd->changeEffect);
			pak.addLongInt(LVL);
			pak.addLongInt(DURSEC);

			if(thisbuf != NULL)
			{
				if(LVL > thisbuf->value)
					thisbuf->value = LVL;
				else
					sendUpdate = false;

				thisbuf->endTime = (GetTickCount() + (DURSEC * 1000));

				if(thisbuf->changeEffect == HPDRAIN)
					sendUpdate = false;

				thisbuf->userPut = PLAYERPUT;
			}

			else
			{
				Buff *b = new Buff();

				*b = *buffd;

				b->value = LVL;
				b->endTime = (GetTickCount() + (DURSEC * 1000));

				if(b->changeEffect == HPDRAIN)
				{
					b->nextUse = (GetTickCount() + 2000);
					sendUpdate = false;
				}

				b->userPut = PLAYERPUT;

				this->p->buffs.push_back(b);
			}

			if(sendUpdate)
			{
				Buff *updateBuff = this->GetBuff(BUFF);
				int sendUpdate2 = 1;

				if(this->GetBonus(updateBuff->changeEffect) == 0)
					sendUpdate2 = 2;

				Packet pak2 = Packet();

				pak2.addHeader(0x29);
				pak2.addInt(sendUpdate2);
				pak2.addInt(updateBuff->changeEffect);

				if(updateBuff->changeEffect == STAMINA)
				{
					this->p->hp = CalculateHP(this);

					if(this->p->chp > this->p->hp)
						this->p->chp = this->p->hp;

					this->com->sendUpdate(HP);
					this->com->sendUpdate(HPPROCENT);
				}

				if(updateBuff->changeEffect == INTT)
				{
					this->p->maxmana = CalculateMana(this);

					if(this->p->mana > this->p->maxmana)
						this->p->mana = this->p->maxmana;

					this->com->sendUpdate(MANA);
				}

				switch(updateBuff->changeEffect)
				{
					case SPEED: case SPEEDMINUS: pak2.addLongInt(this->p->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
						break;

					case STAMINA: pak2.addLongInt((int)this->p->hp);
						break;

					case STRENGTH: pak2.addLongInt((this->p->power + this->GetBonus(STRENGTH)));
						break;

					case AGILITY: pak2.addLongInt((this->p->agility + this->GetBonus(AGILITY)));
						break;

					case INTT: pak2.addLongInt((this->p->intt + this->GetBonus(INTT)));
						break;

					default: log(ERR, "[void Client::AddBuff(int,int,int)] [Usage param was not found] [%d]\n", updateBuff->changeEffect);
						break;
				}

				pak2.addLongInt(this->p->getId());

				if(updateBuff->changeEffect == STAMINA)
					pak2.addLongInt((this->p->magic + this->GetBonus(STAMINA)));

				pak2.ready();
				SendAllOnMap(pak2, 0, this->p->mapId->id, 0);

				//this->com->sendUpdate(STAT);
			}

			pak.ready();
			this->AddPacket(pak, 0);

			if(debugs)
				log(DEBUG, "Added buff: %d level: %d changeEffect: %d.\n", BUFF, LVL * 4, buffd->changeEffect);
		}
	}
}

void Monster::AddMonsterBuff(int BUFF, int LVL, int DURSEC, int userMade)
{
	if(this->spawned)
	{
		Buff *thisbuf = this->GetMonsterBuff(BUFF);
		Buff *buffd = getBuffData(BUFF);

		if(buffd == NULL)
			log(ERR, "[void Client::AddMonsterBuff(int,int,int,int)] [Buff does not exist] [%d]\n", BUFF);

		else
		{
			if(thisbuf != NULL)
			{
				if(LVL > thisbuf->value)
					thisbuf->value = LVL;

				thisbuf->endTime = (GetTickCount() + (DURSEC * 1000));
				thisbuf->userPut = userMade;
			}

			else
			{
				Buff *b = new Buff();

				*b = *buffd;

				b->value = LVL;
				b->endTime = (GetTickCount() + (DURSEC * 1000));
				b->userPut = userMade;

				if(b->changeEffect == HPDRAIN)
					b->nextUse = (GetTickCount() + 2000);

				this->mbuffs.push_back(b);
			}

			Packet pak = Packet();

			pak.addHeader(0x60);
			pak.addLongInt(this->pid);
			pak.addLongInt(buffd->changeEffect);

			switch(buffd->changeEffect)
			{
				case SPEEDMINUS: pak.addLongInt((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))));
					break;

				case HPDRAIN: pak.addLongInt(0);
					break;

				default: log(ERR, "[void Monster::AddMonsterBuff(int,int,int,int)] [Unknown usage param] [%d]\n", buffd->changeEffect);
					break;
			}

			pak.ready();
			SendAllOnMap(pak, 0, this->mapId->id, 0);

			if(debugs)
				log(DEBUG, "Added monster buff: %d level: %d changeEffect: %d.\n", BUFF, LVL, buffd->changeEffect);
		}
	}
}

void Client::RemoveBuff(int pos)
{
	Buff *rb = this->p->buffs.at(pos);
	if(rb == NULL)
		log(DEBUG, "Could not find buff at vector<T>[%d].\n", pos);

	else
	{
		Packet pak = Packet();

		pak.addHeader(0x28);
		pak.addInt(2);
		pak.addLongInt(rb->effect);

		this->p->buffs.erase(this->p->buffs.begin() + pos);

		if(rb->changeEffect != HPDRAIN)
		{
			int sendUpdate2 = 2;

			if(this->GetBonus(rb->changeEffect) > 0)
				sendUpdate2 = 1;

			Packet pak2 = Packet();

			pak2.addHeader(0x29);
			pak2.addInt(sendUpdate2);
			pak2.addInt(rb->changeEffect);

			if(rb->changeEffect == STAMINA)
			{
				this->p->hp = CalculateHP(this);

				if(this->p->chp > this->p->hp)
					this->p->chp = this->p->hp;

				this->com->sendUpdate(HP);
				this->com->sendUpdate(HPPROCENT);
			}

			if(rb->changeEffect == INTT)
			{
				this->p->maxmana = CalculateMana(this);

				if(this->p->mana > this->p->maxmana)
					this->p->mana = this->p->maxmana;

				this->com->sendUpdate(MANA);
			}

			switch(rb->changeEffect)
			{
				case SPEED: case SPEEDMINUS: pak2.addLongInt(this->p->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
					break;

				case STAMINA: pak2.addLongInt((int)this->p->hp);
					break;

				case STRENGTH: pak2.addLongInt((this->p->power + this->GetBonus(STRENGTH)));
					break;

				case AGILITY: pak2.addLongInt((this->p->agility + this->GetBonus(AGILITY)));
					break;

				case INTT: pak2.addLongInt((this->p->intt + this->GetBonus(INTT)));
					break;

				default: log(ERR, "[void Client::RemoveBuff(int)] [Usage param not found] [%d]\n", rb->changeEffect);
					break;
			}

			pak2.addLongInt(this->p->getId());

			if(rb->changeEffect == STAMINA)
				pak2.addLongInt((this->p->magic + GetBonus(STAMINA)));

			pak2.ready();
			SendAllOnMap(pak2, 0, this->p->mapId->id, 0);
		}

		this->com->sendUpdate(STAT);

		delete rb;

		pak.ready();
		this->AddPacket(pak, 0);

		if(debugs)
			log(DEBUG, "Removed buff for player %d.\n", this->p->getId());
	}
}

void Monster::RemoveMonsterBuff(int pos)
{
	Buff *rb = this->mbuffs.at(pos);
	if(rb == NULL)
		log(DEBUG, "Could not find monster buff at vector<T>[%d].\n", pos);
	else
	{
		this->mbuffs.erase(this->mbuffs.begin() + pos);

		Packet pak = Packet();

		pak.addHeader(0x60);
		pak.addLongInt(this->pid);
		pak.addLongInt(rb->changeEffect);

		switch(rb->changeEffect)
		{
			case SPEEDMINUS: pak.addLongInt((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))));
				break;

			case HPDRAIN: pak.addLongInt(0);
				break;

			default: log(ERR, "[void Monster::RemoveMonsterBuff(int)] [Unknown usage param] [%d]\n", rb->changeEffect);
				break;
		}

		pak.ready();
		SendAllOnMap(pak, 0, this->mapId->id, 0);

		delete rb;

		if(debugs)
			log(DEBUG, "Removed monster buff for monster %d.\n", this->pid);
	}
}

bool Client::BuffExist(int BUFF)
{
	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
	{
		if(this->p->buffs.at(i)->effect == BUFF)
			return true;
	}

	return false;
}

NPC *GetNPCById(int id)
{
	NPC *ret = NULL;

	for(unsigned int i = 0; i < npcs.size(); i++)
	{
		if(npcs.at(i)->id == id)
		{
			ret = npcs.at(i);

			break;
		}
	}

	return ret;
}

void Client::HitNPCMenuButton(char *b)
{
	int btn = b[3];
	int npcId = atoi(getString(b, 5, (int)b[4]).c_str());

	NPC *np = GetNPCById(npcId);
	if(np == NULL)
		log(ERR, "[void Client::HitNPCMenuButton(char*)] [Could not find NPC] [%d]\n", npcId);

	else
	{
		if(this->p->mapId->id == np->mapId->id)
		{
			if((unsigned)btn <= np->menu.size() && btn > 0)
			{
				if(this->InArea((np->x - 200 - this->p->meW), (np->y - 200 - this->p->meH), (400 + np->meW), (400 + np->meH)))
				{
					if(strcmp(np->menu.at((btn - 1)).text, "Get buffed!") == 0)
					{
						for(int i = 0; i < buff; i++)
							this->AddBuff(buffs[i], buffsLvl[i], buffTime[i], 0);
					}

					else if(strcmp(np->menu.at((btn - 1)).text, "Dialog") == 0)
					{
						Packet pak = Packet();

						pak.addHeader(0x31);
						pak.addLongInt(np->id);
						pak.addLongString(np->dialog);
						pak.addInt(np->diab.size());

						for(unsigned int i = 0; i < np->diab.size(); i++)
							pak.addString(np->diab.at(i).text);

						pak.ready();
						this->AddPacket(pak, 0);
					}

					else if(strcmp(np->menu.at((btn - 1)).text, "Trade") == 0)
					{
						Packet pak = Packet();

						pak.addHeader(0x64);
						pak.addLongInt(np->id);
						pak.addLongInt(np->sellingItems.size());

						for(unsigned int o = 0; o < np->sellingItems.size(); o++)
							pak.addLongInt(np->sellingItems.at(o));

						pak.ready();
						this->AddPacket(pak, 0);
					}

					else if(strcmp(np->menu.at((btn - 1)).text, "Teleportation") == 0)
					{
						Packet pak = Packet();

						pak.addHeader(0x78);
						pak.addLongInt(np->id);
						pak.addLongInt(np->tps.size());

						for(unsigned int i = 0; i < np->tps.size(); i++)
						{
							pak.addString((char*)np->tps.at(i).world.c_str());
							pak.addLongInt(np->tps.at(i).price);
						}

						pak.ready();
						this->AddPacket(pak, 0);
					}

					else
						log(ERR, "[void Client::HitNPCMenuButton(char*)] [Unknown NPC param] [%s] [%s] [%d]\n", np->menu.at((btn - 1)).text, np->name, np->id);
				}

				else
					SystemChat(this, CUSTOM, NULL, "You need to get closer to that NPC.");
			}
		}

		else
			log(ERR, "[void Client::HitNPCMenuButton(char*)] [Player trying to access a NPC from another map] [%d] [%d]\n", this->p->mapId->id, np->mapId->id);
	}
}

void Client::Teleport(char *b)
{
	if(this->p->dead)
		return;

	int pos = 3;

	int id = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	int i = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	NPC *tar = GetNPCById(id);

	if(tar == NULL)
	{
		log(ERR, "[void Client::Teleport(char*)] [NPC is NULL] [%d] [%d]\n", id, i);
		return;
	}

	if(this->p->mapId->id != tar->mapId->id)
	{
		log(ERR, "[void Client::Teleport(char*)] [Player trying to access a NPC from another map] [%d] [%d] [%d] [%d]\n", id, i, this->p->mapId->id, tar->mapId->id);
		return;
	}

	if((unsigned)i > tar->tps.size() || i <= 0)
		return;

	if(this->p->gold >= tar->tps.at((i - 1)).price)
	{
		this->ChangeMap(NULL, true, tar->tps.at((i - 1)).id, tar->tps.at((i - 1)).x, tar->tps.at((i - 1)).y);
		this->RemoveItem(0, tar->tps.at((i - 1)).price, false, 2);
	}

	else
		SystemChat(this, CUSTOM, NULL, "You need more dinero to teleport.");
}

void Client::RemoveAllBuffs()
{
	bool sendStat = false;

	if(this->p->buffs.size() > 0)
		sendStat = true;

	while(this->p->buffs.size() > 0)
	{
		if(this->p->buffs.size() > 0)
		{
			for(unsigned int i = 0; i < this->p->buffs.size(); i++)
			{
				Buff *rb = this->p->buffs.at(i);
				if(rb == NULL)
					log(DEBUG, "Could not find buff at vector<T>[%d].\n", i);

				else
				{
					Packet pak = Packet();

					pak.addHeader(0x28);
					pak.addInt(2);
					pak.addLongInt(rb->effect);

					this->p->buffs.erase(this->p->buffs.begin() + i);

					if(rb->changeEffect != HPDRAIN)
					{
						int sendUpdate2 = 2;

						if(this->GetBonus(rb->changeEffect) > 0)
							sendUpdate2 = 1;

						Packet pak2 = Packet();

						pak2.addHeader(0x29);
						pak2.addInt(sendUpdate2);
						pak2.addInt(rb->changeEffect);

						if(rb->changeEffect == STAMINA)
						{
							this->p->hp = CalculateHP(this);

							if(this->p->chp > this->p->hp)
								this->p->chp = this->p->hp;

							this->com->sendUpdate(HP);
							this->com->sendUpdate(HPPROCENT);
						}

						if(rb->changeEffect == INTT)
						{
							this->p->maxmana = CalculateMana(this);

							if(this->p->mana > this->p->maxmana)
								this->p->mana = this->p->maxmana;

							this->com->sendUpdate(MANA);
						}

						switch(rb->changeEffect)
						{
							case SPEED: case SPEEDMINUS: pak2.addLongInt(this->p->speed + (this->GetBonus(SPEED) - this->GetBonus(SPEEDMINUS)));
								break;

							case STAMINA: pak2.addLongInt((int)this->p->hp);
								break;

							case STRENGTH: pak2.addLongInt((this->p->power + this->GetBonus(STRENGTH)));
								break;

							case AGILITY: pak2.addLongInt((this->p->agility + this->GetBonus(AGILITY)));
								break;

							case INTT: pak2.addLongInt((this->p->intt + this->GetBonus(INTT)));
								break;

							default: log(ERR, "[void Client::RemoveBuff(int)] [Usage param not found] [%d]\n", rb->changeEffect);
								break;
						}

						pak2.addLongInt(this->p->getId());

						if(rb->changeEffect == STAMINA)
							pak2.addLongInt((this->p->magic + GetBonus(STAMINA)));

						pak2.ready();
						SendAllOnMap(pak2, 0, this->p->mapId->id, 0);

						//this->com->sendUpdate(STAT);
					}

					delete rb;

					pak.ready();
					this->AddPacket(pak, 0);

					if(debugs)
						log(DEBUG, "Removed buff for player %d.\n", this->p->getId());
				}
			/*
			bool redoHit = false;
			for(unsigned int i = 0; i < this->p->buffs.size(); i++)
			{
				if(this->p->buffs.at(i)->changeEffect == STAMINA)
					redoHit = true;

				delete this->p->buffs.at(i);
			}

			this->p->buffs.clear();

			this->p->hp = CalculateHP(this);

			if(this->p->chp > this->p->hp)
				this->p->chp = this->p->hp;

			this->p->maxmana = CalculateMana(this);

			if(this->p->mana > this->p->maxmana)
				this->p->mana = this->p->maxmana;

			this->com->sendUpdate(HP);
			this->com->sendUpdate(HPPROCENT);
			this->com->sendUpdate(MANA);
			*/
			}
		}
	}

	if(sendStat)
		this->com->sendUpdate(STAT);
}

void Monster::RemoveAllMonsterBuffs()
{
	if(this->mbuffs.size() > 0)
	{
		for(unsigned int i = 0; i < this->mbuffs.size(); i++)
			delete this->mbuffs.at(i);

		this->mbuffs.clear();
	}
}

Buff *Client::GetBuff(int BUFF)
{
	if(this->p->buffs.size() > 0)
	{
		for(unsigned int i = 0; i < this->p->buffs.size(); i++)
		{
			if(BUFF == this->p->buffs.at(i)->effect)
				return this->p->buffs.at(i);
		}
	}

	return NULL;
}

Buff *Monster::GetMonsterBuff(int BUFF)
{
	if(this->mbuffs.size() > 0)
	{
		for(unsigned int i = 0; i < this->mbuffs.size(); i++)
		{
			if(BUFF == this->mbuffs.at(i)->effect)
				return this->mbuffs.at(i);
		}
	}

	return NULL;
}

Monster *GetMonsterById(int id)
{
	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		if(mobs.at(i)->pid == id)
			return mobs.at(i);
	}

	return NULL;
}

void Client::SpecialAttack(int id, int type, int param)
{
	if(type == 0)
	{
		Client *c = GetClientByPlayerId(id);
		if(c == NULL)
			log(ERR, "[void Client::SpecialAttack(int,int,int)] [Could not find player] [%d]\n", id);

		else
		{
			if(this->p->mapId->id == c->p->mapId->id)
			{
				if(c->p->trading)
					c->CancelTrade(true);

				if(!c->p->dead)
				{
					if(!c->PvPArea())
					{
						SystemChat(this, CUSTOM, NULL, "You can not attack in safezone.");

						return;
					}

					Skill *aoeskill = this->GetSkill(param);

					if(aoeskill == NULL)
					{
						Skill *newskill = GetMainSkill(param);
						Skill *clientskill = new Skill();

						*clientskill = *newskill;

						clientskill->lvl = 1;
						clientskill->exp = 0;

						this->p->skills.push_back(clientskill);
					}

					c->com->setAttackTimer(playerAttackTimer);

					Skill *mains = GetMainSkill(param);
					Skill *myskill = this->GetSkill(param);

					if(mains->addbuffs.size() > 0)
					{
						for(unsigned int y = 0; y < mains->addbuffs.size(); y++)
						{
							if(mains->addbuffs.at(y).fi.int1 == 1)
							{
								int multLvl = mains->addbuffs.at(y).fi.int3;
								int multDur = mains->addbuffs.at(y).fi.int4;

								if(mains->addbuffs.at(y).scaleLvl)
									multLvl = myskill->lvl * mains->addbuffs.at(y).scaleMult;

								if(mains->addbuffs.at(y).scaleDur)
									multDur += myskill->lvl * mains->addbuffs.at(y).scaleDurMult;

								c->AddBuff(mains->addbuffs.at(y).fi.int2, multLvl, multDur, 0);
							}
						}
					}

					if(mains->aoe)
					{
						int plusdmg = 0;

						if(mains->scalebasedmg)
						{
							plusdmg += myskill->lvl * mains->multbasedmg;
						}

						if(this->p->mapId->pk)
							this->SkillAttackAoEPK(&GetPlayersInRange(c->p->mapId->id, (int)c->p->x, (int)c->p->y, c->p->meW, c->p->meH, mains->aoerange, mains->aoerange, this->p->getId()), mains->basedmg * 2 + plusdmg, mains->id, mains->basedOn);

						this->SkillAttackAoE(&GetMonstersInRange(c->p->mapId->id, (int)c->p->x, (int)c->p->y, c->p->meW, c->p->meH, mains->aoerange, mains->aoerange), mains->basedmg + plusdmg, mains->id, mains->basedOn);
					}

					else
					{
						if(mains->basedmg != 0)
						{
							bool mult = false;
							float pAttack = (float)mains->basedmg;/* + this->p->level);*/
							float plusDmg = 0;

							if(mains->scalebasedmg)
							{
								if(mains->scalebasetype == 1)
								{
									pAttack += this->p->level;
									float skillLevel = ((float)myskill->lvl / 10.0f);
									plusDmg = (skillLevel + 1.0f);
									mult = true;
								}

								else if(mains->scalebasetype == 2)
								{
									plusDmg += myskill->lvl;
								}
							}

							if(mult)
								pAttack *= plusDmg;

							else
								pAttack += plusDmg;

							this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, mains->basedOn, false);

							/*
							float skillLevel = ((float)this->GetSkill(param)->lvl / 10.0f);

							float pAttack = (25.0f + this->p->level);
							pAttack *= (skillLevel + 1.0f);

							this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, STRENGTH, false);
							*/
						}
					}
					/*
					switch(param)
					{
						case 2: 
							{
								c->AddBuff(POISON, this->GetSkill(param)->lvl, ((this->GetSkill(param)->lvl * 2) + 10), this->p->getId());
							break;
							}

						case 3: 
							{
								c->AddBuff(SLOWSPEED, this->GetSkill(param)->lvl, ((this->GetSkill(param)->lvl * 2) + 10), this->p->getId());
							break;
							}

						case 4:
							{
								float skillLevel = ((float)this->GetSkill(param)->lvl / 10.0f);

								float pAttack = (25.0f + this->p->level);
								pAttack *= (skillLevel + 1.0f);

								this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, STRENGTH, false);
							break;
							}

						case 5:
							{
								if(this->p->mapId->pk)
									this->Attack(id, type, 1, 1, NULL, true, 2 + (this->GetSkill(param)->lvl / 2), false, INTT, false);
							break;
							}

						case 6:
							{
								if(this->p->mapId->pk)
									this->SkillAttackAoEPK(&GetPlayersInRange(c->p->mapId->id, (int)c->p->x, (int)c->p->y, c->p->meW, c->p->meH, 50, 50, this->p->getId()), 10 + (this->GetSkill(param)->lvl / 2), 6, INTT);

								this->SkillAttackAoE(&GetMonstersInRange(c->p->mapId->id, (int)c->p->x, (int)c->p->y, c->p->meW, c->p->meH, 50, 50), 5 + (this->GetSkill(param)->lvl / 2), 6, INTT);
							break;
							}

						default: log(ERR, "[void Client::SpecialAttack(int,int,int)] [Unknown usage param] [%d]\n", param);
							break;
					}
					*/
				}

				else
					SystemChat(this, PKALREADYDEAD, c, NULL);
			}
		}
	}

	else if(type == 1)
	{
		Monster *m = GetMonsterById(id);
		if(m == NULL)
			log(ERR, "[void Client::SpecialAttack(int,int,int)] [Could not find monster] [%d]\n", id);

		else
		{
			if(this->p->mapId->id == m->mapId->id)
			{
				if(m->spawned && (m->attackerId == 0 || this->p->getId() == m->attackerId || enableKS || this->AttackInParty(m->attackerId)) && !m->goAfter)
				{
					Skill *aoeskill = this->GetSkill(param);

					if(aoeskill == NULL)
					{
						Skill *newskill = GetMainSkill(param);
						Skill *clientskill = new Skill();

						*clientskill = *newskill;

						clientskill->lvl = 1;
						clientskill->exp = 0;

						this->p->skills.push_back(clientskill);
					}

					Skill *mains = GetMainSkill(param);
					Skill *myskill = this->GetSkill(param);

					if(mains->addbuffs.size() > 0)
					{
						for(unsigned int y = 0; y < mains->addbuffs.size(); y++)
						{
							if(mains->addbuffs.at(y).fi.int1 == 1)
							{
								int multLvl = mains->addbuffs.at(y).fi.int3;
								int multDur = mains->addbuffs.at(y).fi.int4;

								if(mains->addbuffs.at(y).scaleLvl)
									multLvl = myskill->lvl * mains->addbuffs.at(y).scaleMult;

								if(mains->addbuffs.at(y).scaleDur)
									multDur += myskill->lvl * mains->addbuffs.at(y).scaleDurMult;

								m->AddMonsterBuff(mains->addbuffs.at(y).fi.int2, multLvl, multDur, this->p->getId());
							}
						}
					}

					if(mains->aoe)
					{
						int plusdmg = 0;

						if(mains->scalebasedmg)
						{
							plusdmg += myskill->lvl * mains->multbasedmg;
						}

						if(this->p->mapId->pk)
							this->SkillAttackAoEPK(&GetPlayersInRange(m->mapId->id, (int)m->x, (int)m->y, m->monW, m->monH, mains->aoerange, mains->aoerange, this->p->getId()), mains->basedmg * 2 + plusdmg, mains->id, mains->basedOn);

						this->SkillAttackAoE(&GetMonstersInRange(m->mapId->id, (int)m->x, (int)m->y, m->monW, m->monH, mains->aoerange, mains->aoerange), mains->basedmg + plusdmg, mains->id, mains->basedOn);
					}

					else
					{
						if(mains->basedmg != 0)
						{
							bool mult = false;
							float pAttack = ((float)mains->basedmg / 2.0f);/* + this->p->level);*/
							float plusDmg = 0;

							if(mains->scalebasedmg)
							{
								if(mains->scalebasetype == 1)
								{
									pAttack += this->p->level;
									float skillLevel = ((float)myskill->lvl / 10.0f);
									plusDmg = (skillLevel + 1.0f);
									mult = true;
								}

								else if(mains->scalebasetype == 2)
								{
									plusDmg += myskill->lvl;
								}
							}

							if(mult)
								pAttack *= plusDmg;

							else
								pAttack += plusDmg;

							this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, mains->basedOn, false);

							/*
							float skillLevel = ((float)this->GetSkill(param)->lvl / 10.0f);

							float pAttack = (25.0f + this->p->level);
							pAttack *= (skillLevel + 1.0f);

							this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, STRENGTH, false);
							*/
						}
					}

					/*
					switch(param)
					{
						case 2: 
							{
								m->AddMonsterBuff(POISON, this->GetSkill(param)->lvl, ((this->GetSkill(param)->lvl * 2) + 10), this->p->getId());
							break;
							}

						case 3: 
							{
								m->AddMonsterBuff(SLOWSPEED, this->GetSkill(param)->lvl, ((this->GetSkill(param)->lvl * 2) + 10), this->p->getId());
							break;
							}

						case 4:
							{
								float skillLevel = ((float)this->GetSkill(param)->lvl / 10.0f);

								float pAttack = (40.0f + this->p->level);
								pAttack *= (skillLevel + 1.0f);

								this->Attack(id, type, 1, 1, NULL, true, (int)pAttack, false, STRENGTH, false);
							break;
							}

						case 5:
							{
								this->Attack(id, type, 1, 1, NULL, true, 1 + (this->GetSkill(param)->lvl / 2), false, INTT, false);
							break;
							}

						case 6:
							{
								if(this->p->mapId->pk)
									this->SkillAttackAoEPK(&GetPlayersInRange(m->mapId->id, (int)m->x, (int)m->y, m->monW, m->monH, 50, 50, this->p->getId()), 10 + (this->GetSkill(param)->lvl / 2), 6, INTT);

								this->SkillAttackAoE(&GetMonstersInRange(m->mapId->id, (int)m->x, (int)m->y, m->monW, m->monH, 50, 50), 5 + (this->GetSkill(param)->lvl / 2), 6, INTT);
							break;
							}

						default: log(ERR, "[void Client::SpecialAttack(int,int,int)] [Unknown usage param] [%d]\n", param);
							break;
					}
					*/

					m->attacked = true;
					m->freeAttack = (GetTickCount() + monsterFreeDelay);

					m->attackerId = this->p->getId();

					if(m->followId == 0)
						wc->sendFollow(m, this);
				}

				else
				{
				}
			}
		}
	}

	else
		log(ERR, "[void Client::SpecialAttack(int,int,int)] [Unknown mover param] [%d]\n", type);
}

void Client::HitNPCDialogButton(char *b)
{
	int button = b[3];
	int npcId = atoi(getString(b, 5, (int)b[4]).c_str());

	NPC *tar = GetNPCById(npcId);
	if(tar == NULL)
		log(ERR, "[void Client::HitNPCDialogButton(char*)] [Could not find NPC] [%d]\n", npcId);

	else
	{
		if(tar->diab.size() >= (unsigned)button)
		{
			if(this->p->mapId->id == tar->mapId->id)
			{
				char *target = tar->diab.at((button - 1)).text;

				if(strcmp(target, "Bye!") == 0)
				{
					Packet pak = Packet();

					pak.addHeader(0x33);
					pak.addInt(3);

					pak.ready();
					this->AddPacket(pak, 0);
				}

				else
					log(ERR, "[void Client::HitNPCDialogButton(char*)] [Unknown usage param] [%s]\n", target);
			}
		}
	}
}

bool Client::AttackInParty(int mobId)
{
	if(this->p->pt == NULL)
		return false;

	else
	{
		for(unsigned int i = 0; i < this->p->pt->members.size(); i++)
		{
			if(this->p->pt->members.at(i)->mb->p->getId() == mobId)
				return true;
		}
	}

	return false;
}

bool Client::InArea(int buttonx, int buttony, int buttonw, int buttonh)
{
	if((this->p->x >= buttonx) && (this->p->x <= (buttonx + buttonw)) && (this->p->y >= buttony) && (this->p->y <= buttony + buttonh))
		return true;

	else
		return false;
}

bool Area(int myx, int myy, int myw, int myh, int buttonx, int buttony, int buttonw, int buttonh)
{
	if((myx >= buttonx && (myx - myw) <= (buttonx + buttonw)) && (myy >= buttony && (myy - myh) <= (buttony + buttonh)))
		return true;

	else
		return false;
}

void Client::RemoveFollower(int id)
{
	for(unsigned int t = 0; t < mobs.size(); t++)
	{
		if(mobs.at(t)->pid == id)
		{
			if(mobs.at(t)->follows && mobs.at(t)->followId == this->p->getId())
			{
				int w = Random((moveDelay / 2), (moveDelay * 2));

				mobs.at(t)->followId = 0;
				mobs.at(t)->attackerId = 0;
				mobs.at(t)->follows = false;
				mobs.at(t)->attacked = false;
				mobs.at(t)->goAfter = true;

				mobs.at(t)->nextMove = (GetTickCount() + w + 4000);

				mobs.at(t)->move.start(true);

				Packet pf = Packet();
				pf.addHeader(0x44);
				pf.addLongInt(1);
				pf.addLongInt(mobs.at(t)->pid);

				pf.ready();
				SendAllOnMap(pf, 0, this->p->mapId->id, 0);
			}

			else
				log(ERR, "[void Client::RemoveFollower(int)] [Follower dosen't belong to the player] [%d]\n", id);

			break;
		}
	}
}

void Client::RemoveAllFollowers()
{
	int mos = 0;
	int idMap = 0;

	if(debugs)
		log(DEBUG, "Removing followers, %d.\n", this->p->getId());

	for(unsigned int t = 0; t < mobs.size(); t++)
	{
		if(mobs.at(t)->follows)
		{
			if(mobs.at(t)->followId == this->p->getId())
				mos++;
		}
	}

	if(debugs)
		log(DEBUG, "Removing %d follower(s).\n", mos);

	if(mos > 0)
	{
		Packet pf = Packet();
		pf.addHeader(0x44);
		pf.addLongInt(mos);

		for(unsigned int r = 0; r < mobs.size(); r++)
		{
			if(mobs.at(r)->follows)
			{
				if(mobs.at(r)->followId == this->p->getId())
				{
					int w = Random((moveDelay / 2), (moveDelay * 2));

					mobs.at(r)->followId = 0;
					mobs.at(r)->attackerId = 0;
					mobs.at(r)->follows = false;
					mobs.at(r)->attacked = false;
					mobs.at(r)->goAfter = true;

					idMap = mobs.at(r)->mapId->id;

					mobs.at(r)->move.start(true);
				
					pf.addLongInt(mobs.at(r)->pid);

					if(debugs)
						log(DEBUG, "Removed follower %d.\n", mobs.at(r)->pid);
				}
			}
		}

		pf.ready();
		SendAllOnMap(pf, 0, idMap, 0);
	}
}

void Client::SkillAttackAoE(std::vector<Monster*> *vec, int baseDamage, int skillId, int basedOn)
{
	if(enableAttack && vec->size() > 0)
	{
		std::vector<Drop*> droDro;
		std::vector<ThreeInt> dmgDmg;
		std::vector<FourInt> folFol;
		std::vector<int> unsUns;

		long totalExpAdd = 0;
		long totalMobs = 0;

		for(unsigned int i = 0; i < vec->size(); i++)
		{
			Monster *m = vec->at(i);
			if(m == NULL)
				log(ERR, "[void Client::SkillAttackAoE(vector<Monster*>*,int,int)] [Could not find monster]\n");

			else
			{
				if((m->attackerId == 0 || this->p->getId() == m->attackerId || enableKS || this->AttackInParty(m->attackerId)) && !m->goAfter)
				{
					Skill *aoeskill = this->GetSkill(skillId);

					if(aoeskill == NULL)
					{
						Skill *newskill = GetMainSkill(skillId);
						Skill *clientskill = new Skill();

						*clientskill = *newskill;

						clientskill->lvl = 1;
						clientskill->exp = 0;
						clientskill->nextUsage = 0;

						this->p->skills.push_back(clientskill);
					}

					int type = 0;
					int typeBased = 0;

					switch(basedOn)
					{
						case STRENGTH: typeBased = this->p->power;
							break;

						case STAMINA: typeBased = this->p->magic;
							break;

						case AGILITY: typeBased = this->p->agility;
							break;

						case INTT: typeBased = this->p->intt;
							break;
					}

					int pStr = (typeBased + this->GetBonus(basedOn));

					float pAttack = ((float)pStr * 3.5f);
					float skillLevel = (this->GetSkill(skillId)->lvl / 10.0f);

					pAttack += baseDamage;
					pAttack += (float)this->GetEquipBonus(ATTACK);
					pAttack += (float)this->GetBonus(ATTACK);
					pAttack += ((float)this->p->level / 1.5f);
					pAttack *= (skillLevel + 1.0f + ((float)this->GetBonus(TOTALDMG) / 100.0f));
					pAttack += 4.0f;

					if(pAttack > 0)
					{
						float atkMin = (pAttack - 3.0f);
						float atkMax = (pAttack + 3.0f);

						if(atkMin <= 0)
							atkMin = 0;

						pAttack = RandomFloat(atkMin, atkMax);

						if(pAttack > 0)
						{
							float lvlDiff = (float)(m->level - this->p->level);

							if(lvlDiff > 0)
							{
								float multi = 1.0f;

								multi -= (lvlDiff * 0.03f);

								if(multi <= 0)
								{
									if(pAttack > 0)
										pAttack = 0;

									else
										pAttack = 0;
								}

								else
									pAttack *= multi;
							}
						}

						else
							pAttack = 0;

						if(pAttack > 0)
						{
							pAttack /= 4.0f;

							if(pAttack < 0)
								pAttack = 0;
						}
					}

					float pAtk = pAttack;

					if(pAtk < 1)
						pAtk = 0;

					if(this->p->godMode && this->p->access >= GM)
						pAtk = m->chp;

					if((m->chp - pAtk) <= 0)
					{
						pAtk = m->chp;

						m->chp = 0;
						m->spawned = false;
						m->respawnTime = (GetTickCount() + m->respawnDelayMonster);

						m->attackerId = this->p->getId();

						std::vector<Drop*> temp = DropPacket(m);

						m->attackerId = 0;
						m->followId = 0;
						m->follows = false;
						m->attacked = false;
						m->goAfter = false;
						m->moveDone = true;

						m->RemoveAllMonsterBuffs();

						totalMobs++;

						ThreeInt tw;

						tw.int1 = m->pid;
						tw.int2 = (int)pAtk;
						tw.int3 = GetMonsterProcentHP(m);

						dmgDmg.push_back(tw);

						totalExpAdd += m->expamount;

						unsUns.push_back(m->pid);

						for(unsigned int t = 0; t < temp.size(); t++)
							droDro.push_back(temp.at(t));

						if(m->removeOnDeath)
						{
							for(unsigned int u = 0; u < mobs.size(); u++)
							{
								if(m->pid == mobs.at(u)->pid)
								{
									delete m;
									mobs.erase(mobs.begin() + u);

									break;
								}
							}
						}
					}

					else
					{
						m->chp -= pAtk;

						m->attacked = true;
						m->freeAttack = (GetTickCount() + monsterFreeDelay);

						m->attackerId = this->p->getId();

						if(m->followId == 0)
						{
							m->followId = this->p->getId();
							m->follows = true;
							m->goAfter = false;

							m->moving = false;
							m->goDir = 0;

							m->bx = m->x;
							m->by = m->y;

							m->move.start(true);

							FourInt tw2;

							tw2.int1 = m->pid;
							tw2.int2 = this->p->getId();
							tw2.int3 = (int)m->x;
							tw2.int4 = (int)m->y;

							folFol.push_back(tw2);
						}

						ThreeInt tw;

						tw.int1 = m->pid;
						tw.int2 = (int)pAtk;
						tw.int3 = GetMonsterProcentHP(m);

						dmgDmg.push_back(tw);
					}
				}

				else
				{
				}
			}
		}

		if(dmgDmg.size() > 0)
		{
			Packet pakDmg = Packet();

			pakDmg.addHeader(0x42);
			pakDmg.addLongInt(dmgDmg.size());

			for(unsigned int i = 0; i < dmgDmg.size(); i++)
			{
				pakDmg.addInt(2);
				pakDmg.addInt(0);
				pakDmg.addLongInt(dmgDmg.at(i).int1);
				pakDmg.addLongInt(dmgDmg.at(i).int2);
				pakDmg.addInt(dmgDmg.at(i).int3);
			}

			pakDmg.ready();
			SendAllOnMap(pakDmg, 0, this->p->mapId->id, 0);

			dmgDmg.clear();
		}

		if(unsUns.size() > 0)
		{
			Packet pakUns = Packet();

			pakUns.addHeader(0x46);
			pakUns.addLongInt(unsUns.size());
			
			for(unsigned int i = 0; i < unsUns.size(); i++)
				pakUns.addLongInt(unsUns.at(i));

			pakUns.ready();
			SendAllOnMap(pakUns, 0, this->p->mapId->id, 0);

			unsUns.clear();
		}

		if(folFol.size() > 0)
		{
			Packet pakFol = Packet();

			pakFol.addHeader(0x47);
			pakFol.addLongInt(folFol.size());
			
			for(unsigned int i = 0; i < folFol.size(); i++)
			{
				pakFol.addLongInt(folFol.at(i).int1);
				pakFol.addLongInt(folFol.at(i).int2);
				pakFol.addLongInt(folFol.at(i).int3);
				pakFol.addLongInt(folFol.at(i).int4);
			}

			pakFol.ready();
			SendAllOnMap(pakFol, 0, this->p->mapId->id, 0);

			folFol.clear();
		}

		if(droDro.size() > 0)
		{
			Packet pakDro = Packet();

			pakDro.addHeader(0x39);
			pakDro.addInt(1);
			pakDro.addLongInt(droDro.size());

			for(unsigned int i = 0; i < droDro.size(); i++)
			{
				pakDro.addInt(droDro.at(i)->drop->id);
				pakDro.addInt(droDro.at(i)->drop->value);
				pakDro.addLongInt(droDro.at(i)->id);
				pakDro.addLongInt(droDro.at(i)->x);
				pakDro.addLongInt(droDro.at(i)->y);
			}

			pakDro.ready();
			SendAllOnMap(pakDro, 0, this->p->mapId->id, 0);

			droDro.clear();
		}

		if(totalExpAdd > 0)
			this->AddKillExp(totalExpAdd, totalMobs * 15);
	}
}

void Client::SkillAttackAoEPK(std::vector<Player*> *vec, int baseDamage, int skillId, int basedOn)
{
	if(enableAttack && vec->size() > 0)
	{
		if(this->p->mapId->pk)
		{
			for(unsigned int i = 0; i < vec->size(); i++)
			{
				Client *target = GetClientByPlayerId(vec->at(i)->getId());

				if(target == NULL)
					log(ERR, "[void Client::SkillAttackAoEPK(vector<Player*>*,int,int)] [Could not find player] [%d]\n", vec->at(i)->getId());

				else
				{
					if(target->p->mapId->id == this->p->mapId->id)
					{
						if(target->p->trading)
							target->CancelTrade(true);

						if(!target->PvPArea() || !this->PvPArea())
						{
							SystemChat(this, CUSTOM, NULL, "You can not attack in safezone.");

							continue;
						}

						Skill *aoeskill = this->GetSkill(skillId);

						if(aoeskill == NULL)
						{
							Skill *newskill = GetMainSkill(skillId);
							Skill *clientskill = new Skill();

							*clientskill = *newskill;

							clientskill->lvl = 1;
							clientskill->exp = 0;
							clientskill->nextUsage = 0;

							this->p->skills.push_back(clientskill);
						}

						int type = 0;
						int typeBased = 0;

						switch(basedOn)
						{
							case STRENGTH: typeBased = this->p->power;
								break;

							case STAMINA: typeBased = this->p->magic;
								break;

							case AGILITY: typeBased = this->p->agility;
								break;

							case INTT: typeBased = this->p->intt;
								break;
						}

						int pStr = (typeBased + this->GetBonus(basedOn));

						float pAttack = ((float)pStr * 3.5f);
						float skillLevel = (this->GetSkill(skillId)->lvl / 10.0f);

						pAttack += baseDamage;
						pAttack += (float)this->GetEquipBonus(ATTACK);
						pAttack += (float)this->GetBonus(ATTACK);
						pAttack += ((float)this->p->level / 1.5f);

						if(mPvPEnablePoints)
						{
							float pLevel = ((float)this->p->level / 200.0f);

							pLevel *= (float)this->p->pkpoints;
							pAttack += pLevel;
						}

						pAttack *= (skillLevel + 1.0f + ((float)this->GetBonus(TOTALDMG) / 100.0f));
						pAttack += 4.0f;

						int tSta = (target->p->magic + target->GetBonus(STAMINA));

						float tDefence = ((float)tSta * 2.0f);

						tDefence += (float)target->GetEquipBonus(DEFENCE);
						tDefence += (float)target->GetBonus(DEFENCE);
						tDefence += ((float)target->p->level / 4.5f);
						tDefence += 4.0f;

						pAttack -= tDefence;

						if(pAttack > 0)
						{
							float atkMin = (pAttack - 3.0f);
							float atkMax = (pAttack + 3.0f);

							if(atkMin <= 0)
								atkMin = 0;

							pAttack = RandomFloat(atkMin, atkMax);

							if(pAttack > 0)
							{
								float lvlDiff = (float)(target->p->level - this->p->level);

								if(lvlDiff > 0)
								{
									float multi = 1.0f;

									multi -= (lvlDiff * 0.01f);

									if(multi <= 0)
									{
										if(pAttack > 0)
											pAttack = 0;

										else
											pAttack = 0;
									}

									else
										pAttack *= multi;
								}
							}

							else
								pAttack = 0;

							if(pAttack > 0)
							{
								pAttack /= 12.0f;

								if(pAttack < 0)
									pAttack = 0;
							}
						}

						else
							pAttack = 0;

						float pAtk = pAttack;

						if(pAtk < 1)
							pAtk = 0;

						if(this->p->godMode && this->p->access >= GM)
							pAtk = target->p->chp;

						if(target->p->godMode && target->p->access >= GM)
							pAtk = 0;

						if((target->p->chp - pAtk) <= 0)
						{
							if(!target->p->dead)
							{
								pAtk = target->p->chp;

								wc->sendDamage(target->p, NULL, type, (int)pAtk);
								wc->killPlayer(target);

								if(this->p->level < maxLvl)
									this->AddExp(target->p->level);

								if(target->p->pkpoints > 0)
								{
									target->p->pkpoints--;
									target->com->sendUpdate(PKPOINTS);

									if(target->p->gold > 0)
									{
										int goldAdd = target->p->level * 100;

										if(target->p->gold < goldAdd)
											goldAdd = target->p->gold;

										target->RemoveItem(2, goldAdd, false, 2);
										this->AddItem(2, goldAdd, true, NULL, randomInt);

										string penyaString = "You lost ";
										penyaString += IntToString(goldAdd);
										penyaString += " dinero to ";
										penyaString += this->p->name;
										penyaString += ".";

										SystemChat(target, CUSTOM, NULL, (char*)penyaString.c_str());

										penyaString = "You gained ";
										penyaString += IntToString(goldAdd);
										penyaString += " dinero from killing ";
										penyaString += target->p->name;
										penyaString += ".";

										SystemChat(this, CUSTOM, NULL, (char*)penyaString.c_str());
									}
								}

								this->p->pkpoints++;
								this->com->sendUpdate(PKPOINTS);

								string pwndString = this->p->name;
								pwndString += " pwned ";
								pwndString += target->p->name;
								pwndString += "!";

								Packet pwndPacket = CreatePacketChat(pwndString.c_str(), "Notice", false, false, 0);

								for(unsigned int l = 0; l < ac.size(); l++)
								{
									if(ac.at(l)->ingame)
										ac.at(l)->AddPacket(pwndPacket, 0);
								}
							}

							else
							{
								SystemChat(this, PKALREADYDEAD, NULL, NULL);
							}
						}

						else
						{
							target->com->setAttackTimer(playerAttackTimer);

							target->p->chp -= pAtk;

							if(pAtk > 0)
								target->com->sendUpdate(HP);

							wc->sendDamage(target->p, NULL, type, (int)pAtk);
						}
					}

					else
						log(ERR, "[void Client::SkillAttackAoEPK(vector<Player*>*,int,int)] [Player attacks from another map] [%d] [%d]\n", this->p->mapId->id, target->p->mapId->id);
				}
			}
		}
	}
}

void Client::UseSkill(int id)
{
	bool doit = GetMainSkill(id)->useWeap;

	if(doit)
	{
		if(!this->p->equipped)
			doit = false;
	}

	else
		doit = true;

	if(enableSkill && doit)
	{
		this->CheckSkill(id);

		if((unsigned)this->GetSkill(id)->nextUsage > GetTickCount())
			return;

		Skill *mains = GetMainSkill(id);
		Skill *myskill = this->GetSkill(id);
		bool doSkill = false;

		switch(mains->cost)
		{
			case HPUPDATE:
				{
					if(this->p->chp > (float)mains->costamount)
					{
						doSkill = true;
						this->AddHP(0.0f - (float)mains->costamount);
					}
				break;
				}

			case MANAUPDATE:
				{
					if(this->p->mana > (float)mains->costamount)
					{
						doSkill = true;
						this->AddMana(0.0f - (float)mains->costamount);
					}
				break;
				}

			case ENERGYUPDATE:
				{
					if(this->p->energy > (float)mains->costamount)
					{
						doSkill = true;
						this->AddEnergy(0.0f - (float)mains->costamount);
					}
				break;
				}

			default: log(ERR, "[void Client::UseSkill(int)] [Unknown update param] [%d] [%d]\n", mains->cost, mains->costamount);
		}

		if(!doSkill)
		{
			return;
		}

		this->com->setAttackTimer(playerAttackTimer);

		int target = 0;
		int type = 0;

		if(mains->addbuffs.size() > 0)
		{
			for(unsigned int y = 0; y < mains->addbuffs.size(); y++)
			{
				if(mains->addbuffs.at(y).fi.int1 == 0)
				{
					int multLvl = mains->addbuffs.at(y).fi.int3;
					int multDur = mains->addbuffs.at(y).fi.int4;

					if(mains->addbuffs.at(y).scaleLvl)
						multLvl = myskill->lvl * mains->addbuffs.at(y).scaleMult;

					if(mains->addbuffs.at(y).scaleDur)
						multDur += myskill->lvl * mains->addbuffs.at(y).scaleDurMult;

					this->AddBuff(mains->addbuffs.at(y).fi.int2, multLvl, multDur, 0);
				}
			}
		}

		if(mains->special)
		{
			if(id == 6)
			{
				target = GetClosestMover(this->p->x, this->p->y, this->p->mapId->id, this->p->getId(), type);

				if(type == 0)
				{
					Client *getClient = GetClientByPlayerId(target);

					if(getClient == NULL)
					{
						target = 0;
					}

					else
					{
						if(!getClient->PvPArea() || !this->PvPArea())
						{
							target = 0;
						}
					}
				}

				if(target != 0)
					this->Shoot(NULL, true, id, target, false, type);

				/*
					if(PARAM == 6 && target == 0)
					{
						this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
						this->com->setAttackTimer(playerAttackTimer);
						this->SetSkillCoolDown(PARAM);

						return;
					}
					*/

				/*
					if(PARAM != 5)
						this->AddSkillExp(PARAM, 5);

					this->GetSkill(PARAM)->nextUsage = (GetTickCount() + GetSkillCooldown(PARAM));
					this->com->setAttackTimer(playerAttackTimer);
					this->SetSkillCoolDown(PARAM);
					*/

				//this->Shoot(NULL, true, id, target, false, type);

				return;
			}

			else if(id == 5)
			{
				vector<Monster*> inRange = GetMonstersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, 250, 250);
				vector<Player*> inPlay = GetPlayersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, 250, 250, this->p->getId());

				for(unsigned int i = 0; i < inRange.size(); i++)
				{
					if(inRange.at(i)->spawned)
						this->Shoot(NULL, true, 5, inRange.at(i)->pid, true, 1);
				}

				for(unsigned int i = 0; i < inPlay.size(); i++)
				{
					if(!inPlay.at(i)->dead && GetClientByPlayerId(inPlay.at(i)->getId())->PvPArea() && this->PvPArea())
						this->Shoot(NULL, true, 5, inPlay.at(i)->getId(), true, 0);
				}
			}

			/*
			switch(id)
			{
				case 2: case 3: case 4: case 6: this->Shoot(NULL, true, id, target, false, type); return;
					break;
			}

			int w = 100;
			int h = 100;

			long dis = 1000;

			switch(id)
			{
				case 1:
					{
						w = 100;
						h = 100;

						if(this->p->mapId->pk)
							this->SkillAttackAoEPK(&GetPlayersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, w, h, this->p->getId()), 14, 1, INTT);

						this->SkillAttackAoE(&GetMonstersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, w, h), 7, 1, INTT);

						dis = 450;
					break;
					}

				case 5:
					{
						vector<Monster*> inRange = GetMonstersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, 250, 250);
						vector<Player*> inPlay = GetPlayersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, 250, 250, this->p->getId());

						for(unsigned int i = 0; i < inRange.size(); i++)
						{
							if(inRange.at(i)->spawned)
								this->Shoot(NULL, true, 5, inRange.at(i)->pid, true, 1);
						}

						for(unsigned int i = 0; i < inPlay.size(); i++)
						{
							if(!inPlay.at(i)->dead && GetClientByPlayerId(inPlay.at(i)->getId())->PvPArea() && this->PvPArea())
								this->Shoot(NULL, true, 5, inPlay.at(i)->getId(), true, 0);
						}
					break;
					}
			}
			*/
		}

		else
		{
			if(mains->shoot)
			{
				this->Shoot(NULL, true, id, target, false, type);
			}

			else if(mains->aoe)
			{
				if(this->p->mapId->pk)
					this->SkillAttackAoEPK(&GetPlayersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, mains->aoerange, mains->aoerange, this->p->getId()), mains->basedmg * 2, mains->id, mains->basedOn);

				this->SkillAttackAoE(&GetMonstersInRange(this->p->mapId->id, (int)this->p->x, (int)this->p->y, this->p->meW, this->p->meH, mains->aoerange, mains->aoerange), mains->basedmg, mains->id, mains->basedOn);
			}
		}

		this->com->setAttackTimer(playerAttackTimer);
		this->AddSkillExp(mains->id, mains->expamount);

		if(mains->animation)
		{
			Packet pak = Packet();

			pak.addHeader(0x45);
			pak.addLongInt(mains->id);
			pak.addLongInt(450);
			pak.addLongInt((((int)this->p->x - mains->aoerange) + (this->p->meW / 2)));
			pak.addLongInt((((int)this->p->y - mains->aoerange) + (this->p->meH / 2)));

			pak.ready();
			SendAllOnMap(pak, 0, this->p->mapId->id, 0);
		}

		this->SetSkillCoolDown(id);
		this->GetSkill(id)->nextUsage = (GetTickCount() + GetSkillCooldown(id));
	}

	else
		SystemChat(this, CUSTOM, NULL, "You must have a weapon equipped to use that skill.");
}

std::vector<Monster*> GetMonstersInRange(int mapid, int x, int y, int w, int h, int xRange, int yRange)
{
	std::vector<Monster*> retType;

	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		if(mobs.at(i)->spawned)
		{
			if(mobs.at(i)->mapId->id == mapid)
			{
				if((mobs.at(i)->x + mobs.at(i)->monW) > ((x + (w / 2)) - xRange) && mobs.at(i)->x < ((x + (w / 2)) + xRange) && (mobs.at(i)->y + mobs.at(i)->monH) > ((y + (h / 2)) - yRange) && mobs.at(i)->y < ((y + (h / 2)) + yRange))
					retType.push_back(mobs.at(i));
			}
		}
	}

	return retType;
}

std::vector<Player*> GetPlayersInRange(int mapid, int x, int y, int w, int h, int xRange, int yRange, int self)
{
	std::vector<Player*> retType;

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		if(ac.at(i)->ingame)
		{
			if(ac.at(i)->p->mapId->id == mapid && ac.at(i)->p->getId() != self)
			{
				if((ac.at(i)->p->x + ac.at(i)->p->meW) > ((x + (w / 2)) - xRange) && ac.at(i)->p->x < ((x + (w/ 2)) + xRange) && (ac.at(i)->p->y + ac.at(i)->p->meH) > ((y + (h / 2)) - yRange) && ac.at(i)->p->y < ((y + (h / 2)) + yRange))
					retType.push_back(ac.at(i)->p);
			}
		}
	}

	return retType;
}

bool Monster::GoBack()
{
	float nor = ((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))) * (this->move.get_ticks(true) / 1000.f));

	float changeX = 0;
	float changeY = 0;

	bool returned = false;

	if(this->bx > this->x)
		changeX = (this->bx - this->x);

	else if(this->bx < this->x)
		changeX = (this->x - this->bx);

	if(this->by > this->y)
		changeY = (this->by - this->y);

	else if(this->by < this->y)
		changeY = (this->y - this->by);

	if(changeX <= nor && changeY <= nor)
	{
		returned = true;

		this->x = this->bx;
		this->y = this->by;
	}

	else
	{	
		if(this->bx > this->x)
			this->x += nor;

		else if(this->bx < this->x)
			this->x -= nor;

		if(this->by > this->y)
			this->y += nor;

		else if(this->by < this->y)
			this->y -= nor;
	}

	this->move.start(true);

	if(this->bx == this->x && this->by == this->y)
		return true;
	else
		return false;
}

void Monster::Follow()
{
	if(this->followId != 0)
	{
		Player *temp = GetPlayerById(this->followId);
		if(temp == NULL)
			log(ERR, "[void Monster::Follow()] [Could not find player] [%d]\n", this->followId);

		else
		{
			float pX = temp->x;
			float pY = temp->y;

			if(!this->InsidePlayer(temp))
			{
				float cX = 0;
				float cY = 0;

				bool yOver = false;
				bool xOver = false;

				if(pX < this->x)
				{
					xOver = true;

					cX = (this->x - pX);
				}

				else
					cX = (pX - this->x);

				if(pY < this->y)
				{
					yOver = true;

					cY = (this->y - pY);
				}

				else
					cY = (pY - this->y);

				if(cX > cY)
				{
					float xTemp = ((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))) * (this->move.get_ticks(true) / 1000.f));

					if(xOver)
						this->x -= xTemp;
					else
						this->x += xTemp;
				}

				else if(cY > cX)
				{
					float xTemp = ((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))) * (this->move.get_ticks(true) / 1000.f));

					if(yOver)
						this->y -= xTemp;
					else
						this->y += xTemp;
				}

				else
				{
					float xTemp = (((this->speed + ((this->GetMonsterBonus(SPEED) * 20) - (this->GetMonsterBonus(SPEEDMINUS) * 20))) / 2) * (this->move.get_ticks(true) / 1000.f));

					if(xOver)
						this->x -= xTemp;
					else
						this->x += xTemp;

					if(yOver)
						this->y -= xTemp;
					else
						this->y += xTemp;
				}
			}

			this->move.start(true);
		}
	}
}

bool Monster::InsidePlayer(Player *p)
{
	if(p->weaponWay == LEFT && p->equipped)
	{
		if(((p->x - this->monW - p->wW - 3) <= this->x && (p->x + p->meW) >= this->x) && ((p->y - this->monH) <= this->y && (p->y + p->meH) >= this->y))
			return true;
		else
			return false;
	}

	else if(p->weaponWay == RIGHT && p->equipped)
	{
		if(((p->x - this->monW) <= this->x && (p->x + p->meW + p->wW + 3) >= this->x) && ((p->y -p->meH) <= this->y && (p->y + p->meH) >= this->y))
			return true;
		else
			return false;
	}

	else if(!p->equipped)
	{
		if(((p->x - this->monW) <= this->x && (p->x + p->meW) >= this->x) && ((p->y -p->meH) <= this->y && (p->y + p->meH) >= this->y))
			return true;
		else
			return false;
	}

	return false;
}

bool Client::NotInsidePKPlayer(Player *m)
{
	Player *c = this->p;

	if(c->weaponWay == LEFT && c->equipped)
	{
		if(((c->x - m->meW - c->wW - 3) <= m->x && (c->x + c->meW) >= m->x) && ((c->y - m->meH) <= m->y && (c->y + c->meH) >= m->y))
			return false;
		else
			return true;
	}

	else if(c->weaponWay == RIGHT && c->equipped)
	{
		if(((c->x - m->meW) <= m->x && (c->x + c->meW + c->wW + 3) >= m->x) && ((c->y - m->meH) <= m->y && (c->y + c->meH) >= m->y))
			return false;
		else
			return true;
	}

	else if(!c->equipped)
	{
		if(((c->x - m->meW) <= m->x && (c->x + c->meW) >= m->x) && ((c->y - m->meH) <= m->y && (c->y + c->meH) >= m->y))
			return false;
		else
			return true;
	}

	return true;
}

bool Monster::WalkBack()
{
	float pX = this->bx;
	float pY = this->by;

	float cX = 0;
	float cY = 0;

	bool yOver = false;
	bool xOver = false;

	if(pX < this->x)
	{
		xOver = true;

		cX = (this->x - pX);
	}

	else
		cX = (pX - this->x);

	if(pY < this->y)
	{
		yOver = true;

		cY = (this->y - pY);
	}

	else
		cY = (pY - this->y);

	if(cX >= monsterMaxWalkX || cY >= monsterMaxWalkY)
		return true;
	else
		return false;
}

void Client::OptimizeBuffs()
{
	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
	{
		this->p->buffs.at(i)->endTime = (this->p->buffs.at(i)->endTime - GetTickCount());

		if(this->p->buffs.at(i)->endTime <= 0)
			this->p->buffs.at(i)->endTime = 0;

		this->p->buffs.at(i)->nextUse = 0;
	}
}

void Client::GetBuffsOnline()
{
	for(unsigned int i = 0; i < this->p->buffs.size(); i++)
	{
		this->p->buffs.at(i)->endTime = (GetTickCount() + this->p->buffs.at(i)->endTime);
		this->p->buffs.at(i)->nextUse = (GetTickCount() + 2000);
	}
}

Skill *Client::GetSkill(int id)
{
	for(unsigned int i = 0; i < this->p->skills.size(); i++)
	{
		if(this->p->skills.at(i)->id == id)
			return this->p->skills.at(i);
	}

	return NULL;
}

string GetSkillName(int id)
{
	for(unsigned int i = 0; i < skillName.size(); i++)
	{
		if(id == skillName.at(i).id)
			return skillName.at(i).name;
	}

	return "";
}

void Client::AddSkillExp(int id, long amount)
{
	Skill *aoeskill = this->GetSkill(id);

	if(aoeskill == NULL)
	{
		Skill *newskill = GetMainSkill(id);
		Skill *clientskill = new Skill();

		*clientskill = *newskill;

		clientskill->lvl = 1;
		clientskill->exp = 0;
		clientskill->nextUsage = 0;

		this->p->skills.push_back(clientskill);

		aoeskill = this->GetSkill(id);
	}

	if(aoeskill->lvl < skillmaxlvl)
	{
		bool lvled = false;
		amount *= skillexprate;

		int lastLvl = aoeskill->lvl;

		while(aoeskill->lvl < skillmaxlvl)
		{
			long cLvlExp = skillexptable[(aoeskill->lvl - 1)];

			if((cLvlExp - aoeskill->exp) <= amount)
			{
				aoeskill->lvl++;

				amount -= (cLvlExp - aoeskill->exp);

				aoeskill->exp = 0;

				if(!lvled)
					lvled = true;

				string systemString = "The skill ";

				systemString += GetSkillName(aoeskill->id);
				systemString += " leveled up!";

				SystemChat(this, CUSTOM, NULL, (char*)systemString.c_str());
			}

			else
			{
				aoeskill->exp += amount;

				break;
			}
		}

		if(aoeskill->lvl == skillmaxlvl && aoeskill->exp != 0)
			aoeskill->exp = 0;

		if(lastLvl != aoeskill->lvl)
		{
			Packet pak = Packet();

			pak.addHeader(0x51);
			pak.addInt(aoeskill->type);
			pak.addInt(aoeskill->id);
			pak.addInt(aoeskill->lvl);

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}
}

Skill *GetMainSkill(int id)
{
	for(unsigned int i = 0; i < skills.size(); i++)
	{
		if(skills.at(i)->id == id)
			return skills.at(i);
	}

	log(ERR, "[Skill *GetMainSkill(int)] [Could not find main skill] [%d]\n", id);

	return NULL;
}

long GetSkillCooldown(int id)
{
	for(unsigned int i = 0; i < skillCooldown.size(); i++)
	{
		if(skillCooldown.at(i).id == id)
			return skillCooldown.at(i).coolDown;
	}

	log(ERR, "[long GetSkillCooldown(int)] [Could not find skill cooldown] [%d]\n", id);

	return 0;
}

Player *GetPlayerByName(const char *name)
{
	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(pvector.at(i)->name, name) == 0)
			return pvector.at(i);
	}

	return NULL;
}

void Client::Ping(char *b)
{
	Packet pak = Packet();

	pak.addHeader(0x59);
	
	pak.ready();
	this->AddPacket(pak, 0);

	int ping = atoi(getString(b, 4, (int)b[3]).c_str());
	this->lastPing = ping;
}

void Client::ShopCartAdd(char *b)
{
	int pos = 3;

	int npcId = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	int place = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	int qty = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

	NPC *npc = GetNPCById(npcId);

	if(npc == NULL)
		log(ERR, "[void Client::ShopCartAdd(char*)] [Could not find NPC] [%d]\n", npcId);

	else
	{
		if((unsigned)place <= npc->sellingItems.size() && place > 0)
		{
			Packet pak = Packet();

			pak.addHeader(0x65);
			pak.addLongInt(npc->sellingItems.at((place - 1)));
			pak.addLongInt(qty);

			pak.ready();
			this->AddPacket(pak, 0);
		}

		else
			log(ERR, "[void Client::ShopCartAdd(char*)] [Out of memory vector<T>] [%d] [%d]\n", place, npc->sellingItems.size());
	}
}

void Client::ShopCartBuy(char *b)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not buy any items from the shops now.");

		return;
	}

	int pos = 3;

	int npcid = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	int amountItem = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	if(debugs)
		log(DEBUG, "Buying cart: %d %d.\n", npcid, amountItem);

	NPC *npc = GetNPCById(npcid);

	if(npc == NULL)
		log(ERR, "[void Client::ShopCartBuy(char*)] [Could not find NPC] [%d]\n", npcid);

	else
	{
		if(npc->mapId->id == this->p->mapId->id)
		{
			vector<TwoInt> items;

			for(int i = 0; i < amountItem; i++)
			{
				int itemid = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
				pos += ((int)b[pos] + 1);

				int itemq = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
				pos += ((int)b[pos] + 1);

				TwoInt tw;

				tw.int1 = itemid;
				tw.int2 = itemq;

				if(GetItemById(itemid) == NULL)
					log(ERR, "[void Client::ShopCartBuy(char*)] [Could not find item] [%d]\n", itemid);

				items.push_back(tw);

				if(debugs)
					log(DEBUG, "Push back: %d %d %d/%d.\n", itemid, itemq, (i + 1), items.size());
			}

			if(items.size() > 0)
			{
				int fullPrice = 0;

				for(unsigned int ii = 0; ii < items.size(); ii++)
					fullPrice += (GetItemById(items.at(ii).int1)->price * items.at(ii).int2);

				if(this->p->gold >= fullPrice)
				{
					bool hoff = this->CheckInventorySpace(items);

					if(hoff)
					{
						this->p->gold -= fullPrice;

						Packet pak = Packet();

						pak.addHeader(0x40);
						pak.addLongInt(this->p->gold);
						pak.addLongInt(0);
						pak.addBool(false);

						pak.ready();
						this->AddPacket(pak, 0);

						for(unsigned int i = 0; i < items.size(); i++)
							this->AddItem(items.at(i).int1, items.at(i).int2, false, NULL, npcid);

						LD(GAME, "NPC trade: [Name: %s] [NPC: %s] [Items: %d] [Price: %d].\n", this->p->name, npc->name, items.size(), fullPrice);
					}

					else
						SystemChat(this, CUSTOM, NULL, "Your inventory is full!");
				}

				else
					SystemChat(this, CUSTOM, NULL, "You do not have enough dinero.");
			}
		}

		else
			log(ERR, "[void Client::ShopCartBuy(char*)] [Player is trying to access a NPC from an another map] [%d] [%d]\n", this->p->mapId->id, npc->mapId->id);
	}
}

bool Client::CheckInventorySpace(vector<TwoInt> its)
{
	Item *in[25];
	int gold = this->p->gold;

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
		{
			in[i] = new Item();

			*in[i] = *this->p->inv[i];
		}

		else
			in[i] = NULL;
	}

	bool ret = true;

	for(unsigned int o = 0; o < its.size(); o++)
	{
		if(!ret)
			break;

		if(its.at(o).int1 != 2)
		{
			if(its.at(o).int2 == 1)
			{
				int slotAdd = 0;

				for(int i = 0; i < 25; i++)
				{
					if(in[i] != NULL)
					{
						if(in[i]->id == its.at(o).int1 && in[i]->maxstack >= (in[i]->stack + its.at(o).int2))
						{
							slotAdd = (i + 1);

							break;
						}
					}
				}

				for(int i = 0; i < 25; i++)
				{
					if(in[i] == NULL)
					{
						slotAdd = (i + 1);

						break;
					}
				}

				if(slotAdd == 0)
					ret = false;

				else
				{
					Item *stru = GetItemById(its.at(o).int1);

					if(stru == NULL)
						log(ERR, "[void Client::CheckInventorySpace(vector<TwoInt>)] [Could not find item] [%d]\n", its.at(o).int1);

					else
					{
						if(in[(slotAdd - 1)] != NULL)
						{
							if(in[(slotAdd - 1)]->maxstack >= (in[(slotAdd - 1)]->stack + its.at(o).int2))
								in[(slotAdd - 1)]->stack += its.at(o).int2;

							else
							{
								in[(slotAdd - 1)] = new Item();

								Item *item = in[(slotAdd - 1)];

								item->effect = stru->effect;
								item->maxstack = stru->maxstack;
								item->value = stru->value;

								item->stack = its.at(o).int2;
								item->id = its.at(o).int1;
							}
						}

						else
						{
							Item *item = new Item();

							item->effect = stru->effect;
							item->maxstack = stru->maxstack;
							item->value = stru->value;

							item->stack = its.at(o).int2;
							item->id = its.at(o).int1;

							in[(slotAdd - 1)] = item;
						}
					}
				}
			}

			else if(its.at(o).int2 > 1)
			{
				bool borr = true;

				int am = its.at(o).int2;
				int temp = 0;

				while(am > 0)
				{
					temp = 0;

					for(int i = 0; i < 25; i++)
					{
						if(in[i] != NULL)
						{
							if(in[i]->id == its.at(o).int1 && in[i]->maxstack > in[i]->stack)
							{
								temp = (i + 1);

								break;
							}
						}
					}

					if(temp > 0)
					{
						if(am > (in[(temp - 1)]->maxstack - in[(temp - 1)]->stack))
						{
							am -= (in[(temp - 1)]->maxstack - in[(temp - 1)]->stack);
							in[(temp - 1)]->stack = in[(temp - 1)]->maxstack;
						}

						else
						{
							in[(temp - 1)]->stack += am;
							am = 0;

							break;
						}
					}

					else
					{
						temp = 0;

						for(int ii = 0; ii < 25; ii++)
						{
							if(in[ii] == NULL)
							{
								temp = (ii + 1);

								break;
							}
						}

						if(temp > 0)
						{
							int t = GetItemById(its.at(o).int1)->maxstack;

							if(t > am)
							{
								Item *stru = GetItemById(its.at(o).int1);
								Item *item = new Item();

								item->effect = stru->effect;
								item->maxstack = stru->maxstack;
								item->value = stru->value;

								item->stack = am;
								item->id = its.at(o).int1;

								in[(temp - 1)] = item;

								am = 0;

								break;
							}

							else
							{
								Item *stru = GetItemById(its.at(o).int1);
								Item *item = new Item();

								item->effect = stru->effect;
								item->maxstack = stru->maxstack;
								item->value = stru->value;

								item->stack = t;
								item->id = its.at(o).int1;

								in[(temp - 1)] = item;

								am -= t;
							}
						}

						else
						{
							borr = false;

							am = 0;

							break;
						}
					}
				}

				ret = borr;
			}
		}

		else
		{
			if((gold + its.at(o).int2) <= 200000000)
				gold += its.at(o).int2;

			else
				ret = false;
		}
	}

	for(int i = 0; i < 25; i++)
	{
		if(in[i] != NULL)
			delete in[i];
	}

	return ret;
}

void Client::CheckSkill(int id)
{
	Skill *aoeskill = this->GetSkill(id);

	if(aoeskill == NULL)
	{
		Skill *newskill = GetMainSkill(id);
		Skill *clientskill = new Skill();

		*clientskill = *newskill;

		clientskill->lvl = 1;
		clientskill->exp = 0;
		clientskill->nextUsage = 0;

		this->p->skills.push_back(clientskill);
	}
}

void Client::ClientErrorMessage(char *b)
{
	int pos = 3;

	int len = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	string text = getString(b, pos, len);

	SystemChat(this, CUSTOM, NULL, (char*)text.c_str());
}

void Client::ShopCartSell(int place, int qty, int itemid)
{
	if(itemid == 2)
	{
		SystemChat(this, CUSTOM, NULL, "You can not sell dinero.");

		return;
	}

	if(this->p->inv[(place - 1)] != NULL)
	{
		if(qty > this->p->inv[(place - 1)]->stack || qty <= 0)
			return;

		if(this->p->trading)
			SystemChat(this, CUSTOM, NULL, "You can not sell an item while trading.");

		else
		{
			int price = ((GetItemById(this->p->inv[(place - 1)]->id)->price / 2) * qty);

			this->RemoveItem(place, qty, false, this->p->inv[(place - 1)]->id);
			this->AddItem(2, price, false, NULL, price);
		}
	}
}

void Client::ShopCartRemove(char *b)
{
	int place = atoi(getString(b, 4, (int)b[3]).c_str());

	if(place <= 20 && place > 0)
	{
		Packet pak = Packet();

		pak.addHeader(0x69);
		pak.addLongInt(place);

		pak.ready();
		this->AddPacket(pak, 0);
	}
}

int Client::GetAttackDelay()
{
	long delay = 406;

	int agi = ((this->p->agility + this->GetBonus(AGILITY)) * 2);
	agi += this->GetBonus(ATTACKSPEED);

	if((delay - agi) <= 10)
		return 10;

	else
		return (delay - agi);
}

void Client::PlayerMenu(char *b)
{
	int pos = 3;

	int charId = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	string strMen = getString(b, (pos + 1), (int)b[pos]);

	Client *c = GetClientByPlayerId(charId);

	if(c == NULL)
		log(ERR, "[void Client::PlayerMenu(char*)] [Could not find player] [%d]\n", charId);

	else
	{
		if(strcmp(strMen.c_str(), "Trade") == 0)
		{
			if(!this->p->trading && !this->p->dead)
				this->SendTradeInvite(charId);

			else
				SystemChat(this, CUSTOM, NULL, "You are already trading someone.");
		}

		else if(strcmp(strMen.c_str(), "Follow") == 0)
		{
			wc->sendPlayerFollow(this, c, true);
		}

		else if(strcmp(strMen.c_str(), "Party") == 0)
		{
			this->SendInvite(c->p->name);
		}

		else if(strcmp(strMen.c_str(), "Add as friend") == 0)
		{
			this->SendFriendInvite(charId, c->p->name);
		}

		else
			log(ERR, "[void Client::PlayerMenu(char*)] [Unknown usage param] [%s]\n", strMen.c_str());
	}
}

void Client::FriendMenu(char *b)
{
	int pos = 3;

	string charName = getString(b, (pos + 1), (int)b[pos]);
	pos += ((int)b[pos] + 1);

	string strMen = getString(b, (pos + 1), (int)b[pos]);

	if(strcmp(strMen.c_str(), "Remove friend") == 0)
	{
		this->RemoveFriend(charName.c_str());

		string errorString = charName;
		errorString += " removed from Friends.";

		SystemChat(this, CUSTOM, NULL, (char*)errorString.c_str());

		return;
	}

	else if(strcmp(strMen.c_str(), "Invite to party") == 0)
	{
		this->SendInvite((char*)charName.c_str());
	}

	else if(strcmp(strMen.c_str(), "Send message") == 0)
	{
		this->SendPM(charName.c_str(), "");
	}

	else
		log(ERR, "[void Client::FriendMenu(char*)] [Unknown usage param] [%s]\n", strMen.c_str());
}

void MoveToPlayer(Client *c1, Client *c2)
{
	float bufLvl = (float)(c1->p->speed + (c1->GetBonus(SPEED) - c1->GetBonus(SPEEDMINUS)));
	float Vel = ((bufLvl * 62.0f) * ((float)c1->p->move.get_ticks(false) / 1000.0f));

	Player *fol = c1->p;

	float pX = c2->p->x;
	float pY = c2->p->y;

	float cX = 0;
	float cY = 0;

	bool yOver = false;
	bool xOver = false;

	if(pX < fol->x)
	{
		xOver = true;

		cX = (fol->x - pX);
	}

	else
	{
		cX = (pX - fol->x);
	}

	if(pY < fol->y)
	{
		yOver = true;

		cY = (fol->y - pY);
	}

	else
	{
		cY = (pY - fol->y);
	}

	if(cX == 0)
	{
		if(cY != 0)
		{
			if(yOver)
				fol->y -= Vel;
			else
				fol->y += Vel;
		}
	}

	else if(cY == 0)
	{
		if(cX != 0)
		{
			if(xOver)
			{
				if(fol->weaponWay != LEFT)
					fol->weaponWay = LEFT;

				fol->x -= Vel;
			}

			else
			{
				if(fol->weaponWay != RIGHT)
					fol->weaponWay = RIGHT;

				fol->x += Vel;
			}
		}
	}

	else
	{
		if(cX > cY)
		{
			if(xOver)
			{
				if(fol->weaponWay != LEFT)
					fol->weaponWay = LEFT;

				fol->x -= Vel;
			}

			else
			{
				if(fol->weaponWay != RIGHT)
					fol->weaponWay = RIGHT;

				fol->x += Vel;
			}
		}

		else if(cY > cX)
		{
			if(yOver)
				fol->y -= Vel;
			else
				fol->y += Vel;
		}

		else
		{
			if(xOver)
			{
				if(fol->weaponWay != LEFT)
					fol->weaponWay = LEFT;

				fol->x -= Vel;
			}

			else
			{
				if(fol->weaponWay != RIGHT)
					fol->weaponWay = RIGHT;

				fol->x += Vel;
			}
		}
	}
}

void Client::AddTradePenya(int pen)
{
	if(this->p->trading)
	{
		if(pen <= 0 || pen > 2000000000)
			return;

		if(this->p->tradePenya != 0)
		{
			SystemChat(this, CUSTOM, NULL, "You have already added dinero.");

			return;
		}

		Client *cp = GetClientByPlayerId(this->p->tradeWith);

		if(cp == NULL)
		{
			this->CancelTrade(true);

			return;
		}

		this->p->tradePenya = pen;

		Packet pak = Packet();

		pak.addHeader(0x77);
		pak.addLongInt(this->p->getId());
		pak.addLongInt(pen);

		pak.ready();
		this->AddPacket(pak, 0);
		cp->AddPacket(pak, 0);
	}
}

void Client::AddTradeItem(char *b)
{
	if(this->p->trading)
	{
		int pos = 4;
		int mShopAdd = b[3];
		int mShopQty = atoi(getString(b, 5, (int)b[4]).c_str());
		pos += ((int)b[4] + 1);
		int mShopItemId = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

		if(mShopItemId == 2)
		{
			this->AddTradePenya(mShopQty);

			return;
		}

		if(mShopAdd <= 0 || mShopAdd > 25)
			return;

		if(this->p->inv[(mShopAdd - 1)] == NULL)
			log(ERR, "[void Client::AddTradeItem(char*)] [Inventory place is NULL] [%d]\n", mShopAdd);

		else
		{
			if(this->p->tradeInventory.size() >= 16)
			{
				SystemChat(this, CUSTOM, NULL, "You can not add more items.");

				return;
			}

			bool exist = false;
			int itemsInSlot = 0;

			for(unsigned int i = 0; i < this->p->tradeInventory.size(); i++)
			{
				if(this->p->tradeInventory.at(i).int1 == mShopAdd)
				{
					exist = true;
					itemsInSlot = this->p->inv[(this->p->tradeInventory.at(i).int1 - 1)]->stack;

					break;
				}
			}

			if(exist)
			{
				if(itemsInSlot > 1)
					SystemChat(this, CUSTOM, NULL, "You have already added items from that slot.");

				else
					SystemChat(this, CUSTOM, NULL, "You have already added that item.");

				return;
			}

			Client *cp = GetClientByPlayerId(this->p->tradeWith);

			if(cp == NULL)
				this->CancelTrade(true);

			else
			{
				if(this->p->doneTrade || cp->p->doneTrade)
				{
					SystemChat(this, CUSTOM, NULL, "You can not add an item when the player has accepted the trade.");

					return;
				}

				if(mShopQty > 0 && this->p->inv[(mShopAdd - 1)]->stack >= mShopQty)
				{
					TwoInt tw;

					tw.int1 = mShopAdd;
					tw.int2 = mShopQty;

					this->p->tradeInventory.push_back(tw);

					Packet pak = Packet();

					pak.addHeader(0x75);
					pak.addLongInt(this->p->getId());
					pak.addInt(this->p->tradeInventory.size());
					pak.addLongInt(this->p->inv[(mShopAdd - 1)]->id);
					pak.addLongInt(mShopQty);

					pak.ready();
					this->AddPacket(pak, 0);
					cp->AddPacket(pak, 0);
				}
			}
		}
	}
}

void Client::DoneTrade(int s)
{
	if(this->p->trading)
	{
		if(s == 1)
		{
			Client *cp = GetClientByPlayerId(this->p->tradeWith);

			if(cp == NULL)
				this->CancelTrade(true);

			else
			{
				this->p->doneTrade = true;

				Packet pak = Packet();

				pak.addHeader(0x76);
				pak.addLongInt(this->p->getId());
				pak.addInt(s);

				pak.ready();
				this->AddPacket(pak, 0);
				cp->AddPacket(pak, 0);

				if(this->p->doneTrade && cp->p->doneTrade)
					this->FinishTrade(cp);
			}
		}
		
		else if(s == 2)
			this->CancelTrade(true);
	}
}

void Client::FinishTrade(Client *tar)
{
	Item *origMe[25];
	Item *origHis[25];

	int origPenyaMe = this->p->gold;
	int origPenyaHis = tar->p->gold;

	vector<TwoInt> thisTrade;
	vector<TwoInt> tarTrade;

	bool cancelTrade = false;

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
		{
			origMe[i] = new Item();
			*origMe[i] = *this->p->inv[i];
		}

		else
			origMe[i] = NULL;

		if(tar->p->inv[i] != NULL)
		{
			origHis[i] = new Item();
			*origHis[i] = *tar->p->inv[i];
		}

		else
			origHis[i] = NULL;
	}

	for(unsigned int i = 0; i < this->p->tradeInventory.size(); i++)
	{
		int cSlot = (this->p->tradeInventory.at(i).int1 - 1);

		if(this->p->inv[cSlot] != NULL)
		{
			TwoInt tw;

			tw.int1 = this->p->inv[cSlot]->id;
			tw.int2 = this->p->tradeInventory.at(i).int2;

			thisTrade.push_back(tw);

			if(this->p->inv[cSlot]->stack == this->p->tradeInventory.at(i).int2 || this->p->inv[cSlot]->maxstack == 1)
			{
				delete this->p->inv[cSlot];
				this->p->inv[cSlot] = NULL;
			}

			else if(this->p->inv[cSlot]->stack > this->p->tradeInventory.at(i).int2)
				this->p->inv[cSlot]->stack -= this->p->tradeInventory.at(i).int2;

			else
			{
				log(ERR, "[void Client::FinishTrade(Client*)] [Exception *this->inv[cSlot]*] [%d] [%d]\n", this->p->inv[cSlot]->stack, this->p->tradeInventory.at(i).int2);

				cancelTrade = true;
				break;
			}
		}

		else
		{
			log(ERR, "[void Client::FinishTrade(Client*)] [Exception *this->inv*] [%d]\n", cSlot);

			cancelTrade = true;
			break;
		}
	}

	for(unsigned int i = 0; i < tar->p->tradeInventory.size(); i++)
	{
		int cSlot = (tar->p->tradeInventory.at(i).int1 - 1);

		if(tar->p->inv[cSlot] != NULL)
		{
			TwoInt tw;

			tw.int1 = tar->p->inv[cSlot]->id;
			tw.int2 = tar->p->tradeInventory.at(i).int2;

			tarTrade.push_back(tw);

			if(tar->p->inv[cSlot]->stack == tar->p->tradeInventory.at(i).int2 || tar->p->inv[cSlot]->maxstack == 1)
			{
				delete tar->p->inv[cSlot];
				tar->p->inv[cSlot] = NULL;
			}

			else if(tar->p->inv[cSlot]->stack > tar->p->tradeInventory.at(i).int2)
				tar->p->inv[cSlot]->stack -= tar->p->tradeInventory.at(i).int2;

			else
			{
				log(ERR, "[void Client::FinishTrade(Client*)] [Exception *tar->inv[cSlot]*] [%d] [%d]\n", tar->p->inv[cSlot]->stack, tar->p->tradeInventory.at(i).int2);

				cancelTrade = true;
				break;
			}
		}

		else
		{
			log(ERR, "[void Client::FinishTrade(Client*)] [Exception *tar->inv*] [%d]\n", cSlot);

			cancelTrade = true;
			break;
		}
	}

	if(this->p->tradePenya > 0)
	{
		if(this->p->tradePenya > this->p->gold)
			cancelTrade = true;

		else
		{
			TwoInt tw;

			tw.int1 = 2;
			tw.int2 = this->p->tradePenya;

			thisTrade.push_back(tw);
		}
	}

	if(tar->p->tradePenya > 0)
	{
		if(tar->p->tradePenya > tar->p->gold)
			cancelTrade = true;

		else
		{
			TwoInt tw;

			tw.int1 = 2;
			tw.int2 = tar->p->tradePenya;

			tarTrade.push_back(tw);
		}
	}

	bool checkThis = this->CheckInventorySpace(tarTrade);
	bool checkTar = tar->CheckInventorySpace(thisTrade);

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
			delete this->p->inv[i];

		if(tar->p->inv[i] != NULL)
			delete tar->p->inv[i];

		if(origMe[i] != NULL)
		{
			this->p->inv[i] = new Item();
			*this->p->inv[i] = *origMe[i];

			delete origMe[i];
		}

		if(origHis[i] != NULL)
		{
			tar->p->inv[i] = new Item();
			*tar->p->inv[i] = *origHis[i];

			delete origHis[i];
		}
	}

	if(cancelTrade || !checkThis || !checkTar)
	{
		if(!checkThis)
		{
			string errMsg = this->p->name;

			errMsg += " does not have enough inventory space.";

			SystemChat(this, CUSTOM, NULL, "You need more inventory space to complete the trade.");
			SystemChat(tar, CUSTOM, NULL, (char*)errMsg.c_str());
		}

		if(!checkTar)
		{
			string errMsg = tar->p->name;

			errMsg += " does not have enough inventory space.";

			SystemChat(tar, CUSTOM, NULL, "You need more inventory space to complete the trade.");
			SystemChat(this, CUSTOM, NULL, (char*)errMsg.c_str());
		}

		if(cancelTrade)
		{
			SystemChat(this, CUSTOM, NULL, "Server exception caught.");
			SystemChat(tar, CUSTOM, NULL, "Server exception caught.");
		}

		this->p->gold = origPenyaMe;
		tar->p->gold = origPenyaHis;

		this->CancelTrade(false);

		return;
	}

	Drop *safeFunction = new Drop();
	safeFunction->drop = new Item();
	int safeInt = 0;

	if(this->p->tradePenya > 0)
		this->RemoveItem(0, this->p->tradePenya, true, 2);

	if(tar->p->tradePenya > 0)
		tar->RemoveItem(0, tar->p->tradePenya, true, 2);

	for(unsigned int i = 0; i < this->p->tradeInventory.size(); i++)
		this->RemoveItem(this->p->tradeInventory.at(i).int1, this->p->tradeInventory.at(i).int2, true, 1);

	for(unsigned int i = 0; i < tar->p->tradeInventory.size(); i++)
		tar->RemoveItem(tar->p->tradeInventory.at(i).int1, tar->p->tradeInventory.at(i).int2, true, 1);

	for(unsigned int i = 0; i < tarTrade.size(); i++)
		this->AddItem(tarTrade.at(i).int1, tarTrade.at(i).int2, false, safeFunction, safeInt);

	for(unsigned int i = 0; i < thisTrade.size(); i++)
		tar->AddItem(thisTrade.at(i).int1, thisTrade.at(i).int2, false, safeFunction, safeInt);

	delete safeFunction->drop;
	delete safeFunction;

	this->CancelTrade(false);

	SystemChat(tar, CUSTOM, NULL, "Trade completed.");
	SystemChat(this, CUSTOM, NULL, "Trade completed.");

	LD(GAME, "Player trade: [Name: %s] [Name: %s] [Items: %d] [Dinero: %d].\n", this->p->name, tar->p->name, tarTrade.size() + thisTrade.size(), this->p->tradePenya + tar->p->tradePenya);
}

void Client::AddPartyExp(long am)
{
	Party *pt = this->p->pt;

	if(pt == NULL)
		return;

	if(pt->level < maxPartyLvl)
	{
		bool lvled = false;
		long amount = (am * expRateParty);

		while(pt->level < maxPartyLvl)
		{
			long cLvlExp = expTable[(pt->level - 1)];

			if((cLvlExp - pt->exp) <= amount)
			{
				pt->level++;

				amount -= (cLvlExp - pt->exp);

				pt->exp = 0;

				if(!lvled)
					lvled = true;

				SendAllInParty(CreatePacketChat("Party level up!", "System", false, false, 0), pt, NULL, 0);
			}

			else
			{
				pt->exp += amount;

				break;
			}
		}

		if(pt->level == maxPartyLvl && pt->exp != 0)
			pt->exp = 0;

		if(lvled)
		{
			Packet pk = Packet();

			pk.addHeader(0x79);
			pk.addLongInt(pt->level);

			pk.ready();
			SendAllInParty(pk, pt, NULL, 0);
		}
	}
}

void Client::AddKillExp(long am, long toparty)
{
	if(this->p->pt != NULL)
	{
		int pInParty2 = 0;

		for(unsigned int i2 = 0; i2 < this->p->pt->members.size(); i2++)
		{
			Client *c = this->p->pt->members.at(i2)->mb;

			if(c->p->mapId->id == this->p->mapId->id && c->p->getId() != this->p->getId() && !c->p->dead)
				pInParty2++;
		}

		if(pInParty2 > 0)
			this->AddPartyExp(toparty);

		float multi = (0.9f + ((float)this->p->pt->level / 100.0f)) - 0.01f;

		float expChange = 0.0f;
		long expDone = 0;

		int pInParty = 0;
		
		for(unsigned int i = 0; i < this->p->pt->members.size(); i++)
		{
			Client *c = this->p->pt->members.at(i)->mb;

			if(c->p->mapId->id == this->p->mapId->id && c->p->getId() != this->p->getId() && !c->p->dead)
			{
				if((multi - 0.1f) > 0)
				{
					multi -= 0.1f;

					pInParty++;
				}

				else
					break;
			}
		}

		expChange = ((float)am * multi);
		expDone = (long)expChange;

		for(unsigned int i = 0; i < this->p->pt->members.size(); i++)
		{
			Client *c = this->p->pt->members.at(i)->mb;

			if(c->p->mapId->id == this->p->mapId->id && c->p->level < maxLvl && c->p->getId() != this->p->getId() && !c->p->dead)
				c->AddExp(expDone);
		}

		expChange = ((float)am * (multi + 1.0f));
		expDone = (long)expChange;

		if(pInParty == 0)
			this->AddExp(am);

		else
			this->AddExp(expDone);
	}

	else
		this->AddExp(am);
}

bool Client::PvPArea()
{
	if(this->p->mapId != NULL)
	{
		for(unsigned int i = 0; i < this->p->mapId->pkArea.size(); i++)
		{
			if(this->InArea(this->p->mapId->pkArea.at(i).minX, this->p->mapId->pkArea.at(i).minY, (this->p->mapId->pkArea.at(i).maxX - this->p->mapId->pkArea.at(i).minX), (this->p->mapId->pkArea.at(i).maxY - this->p->mapId->pkArea.at(i).minY)))
				return true;
		}
	}

	return false;
}

int Client::GetInventoryStack(int id)
{
	int stack = 0;

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
		{
			if(this->p->inv[i]->id == id)
				stack += this->p->inv[i]->stack;
		}
	}

	return stack;
}

void Client::UpdateTaskbar(int p, int slot)
{
	if(p == DELETEITEM)
	{
		for(int i = 0; i < 9; i++)
		{
			if(this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && this->p->task[i]->slot == slot && !this->p->task[i]->eqinv)
			{
				if(GetItemById(this->p->task[i]->id)->maxstack > 1)
				{
					if(this->GetInventoryStack(this->p->task[i]->id) <= 0)
						this->p->task[i]->enable = false;
				}

				else
					this->p->task[i]->enable = false;
			}
		}
	}

	for(int i = 0; i < 9; i++)
	{
		if(this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && !this->p->task[i]->eqinv)
		{
			if(GetItemById(this->p->task[i]->id)->maxstack > 1)
			{
				if(this->GetInventoryStack(this->p->task[i]->id) <= 0)
					this->p->task[i]->enable = false;
			}
		}
	}
}

void Client::AddHP(float hp)
{
	this->p->chp += hp;

	if(this->p->chp > this->p->hp)
		this->p->chp = this->p->hp;

	if(this->p->chp <= 0)
		wc->killPlayer(this);

	this->com->sendUpdate(HP);
	this->com->sendUpdate(HPPROCENT);
}

void Client::AddMana(float hp)
{
	this->p->mana += hp;

	if(this->p->mana > this->p->maxmana)
		this->p->mana = this->p->maxmana;

	if(this->p->mana <= 0.0f)
		this->p->mana = 0;

	this->com->sendUpdate(MANA);
}

void Client::AddEnergy(float hp)
{
	this->p->energy += hp;

	if(this->p->energy > this->p->maxenergy)
		this->p->energy = this->p->maxenergy;

	if(this->p->energy <= 0.0f)
		this->p->energy = 0;

	this->com->sendUpdate(ENERGY);
}

Packet CreatePacketChat(const char *msg, const char *name, bool ispro, bool isnpc, int col)
{
	Packet pak = Packet();

	pak.addHeader(0x15);
	pak.addString((char*)msg);
	pak.addString((char*)name);
	pak.addBool(ispro);
	pak.addBool(isnpc);
	pak.addInt(col);

	pak.ready();
	return pak;
}

NPC *GetClosestGuardNPC(int mapid, float x, float y)
{
	NPC *clos = NULL;

	float cYY = 0;
	float cXX = 0;

	for(unsigned int i = 0; i < npcs.size(); i++)
	{
		if(npcs.at(i)->mapId->id == mapid && npcs.at(i)->mapGuard == mapid)
		{
			NPC *c = npcs.at(i);

			float pX = (float)c->x;
			float pY = (float)c->y;

			float cX = 0;
			float cY = 0;

			bool yOver = false;
			bool xOver = false;

			if(pX < x)
			{
				xOver = true;

				cX = (x - pX);
			}

			else
				cX = (pX - x);

			if(pY < y)
			{
				yOver = true;

				cY = (y - pY);
			}

			else
				cY = (pY - y);

			if(((cY + cX) < (cYY + cXX)) || clos == 0)
			{
				clos = c;

				cYY = cY;
				cXX = cX;
			}
		}
	}

	return clos;
}

void Monster::Smite()
{
	wc->sendDamage(NULL, this, 0, (int)this->chp);
	wc->killMonster(NULL, this, true);

	NPC *clo = GetClosestGuardNPC(this->mapId->id, this->x, this->y);

	if(clo != NULL)
		SendAllOnMap(CreatePacketChat("Watch out!", clo->name, false, true, 0), 0, this->mapId->id, 0);
}

bool Monster::ValidArea()
{
	for(unsigned int i = 0; i < this->mapId->nowalk.size(); i++)
	{
		NoWalkZone *nw = &this->mapId->nowalk.at(i);

		if(Area((int)this->x, (int)this->y, this->monW, this->monH, nw->xFrom - this->monW, nw->yFrom - this->monH, (nw->xTo - nw->xFrom), (nw->yTo - nw->yFrom)))
		{
			return false;
		}
	}

	return true;
}

void Client::SetSkillCoolDown(int id)
{
	for(int i = 0; i < 9; i++)
	{
		if(this->p->task[i]->enable)
		{
			if(this->p->task[i]->type == SKILL)
			{
				if(this->p->task[i]->slot == id)
				{
					Packet pak = Packet();

					pak.addHeader(0x0B);
					pak.addInt(i);
					pak.addLongInt(GetSkillCooldown(id));

					pak.ready();
					this->AddPacket(pak, 0);
				}
			}
		}
	}
}

void Client::SetFoodCoolDown(int id, long cool)
{
	Item *findItem = GetItemById(id);
	int eff = 0;

	if(findItem == NULL)
		eff = 0;

	else
		eff = findItem->effect;

	for(int i = 0; i < 9; i++)
	{
		if(this->p->task[i]->enable)
		{
			if(this->p->task[i]->type == INVENTORYITEM)
			{
				int newEffect = 0;
				Item *getNewItem = GetItemById(this->p->task[i]->id);

				if(getNewItem == NULL)
					newEffect = 0;

				else
					newEffect = getNewItem->effect;

				if(this->p->task[i]->id == id || (eff == FOOD && newEffect == FOOD))
				{
					Packet pak = Packet();

					pak.addHeader(0x0B);
					pak.addInt(i);
					pak.addLongInt(cool);

					pak.ready();
					this->AddPacket(pak, 0);
				}
			}
		}
	}
}

int GetClosestMover(float x, float y, int mapid, int id, int &type)
{
	int clos = 0;

	float cYY = 0;
	float cXX = 0;

	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		Monster *t = mobs.at(i);

		if(t->mapId->id != mapid)
			continue;

		if(!t->spawned)
			continue;

		float pX = t->x;
		float pY = t->y;

		float cX = 0;
		float cY = 0;

		bool yOver = false;
		bool xOver = false;

		if(pX < x)
		{
			xOver = true;

			cX = (x - pX);
		}

		else
			cX = (pX - x);

		if(pY < y)
		{
			yOver = true;

			cY = (y - pY);
		}

		else
			cY = (pY - y);

		if(((cY + cX) < (cYY + cXX)) || clos == 0)
		{
			clos = t->pid;

			cYY = cY;
			cXX = cX;
		}
	}

	if(clos == 0)
	{
		for(unsigned int i = 0; i < ac.size(); i++)
		{
			if(ac.at(i)->ingame)
			{
				if(ac.at(i)->p->mapId->id == mapid && !ac.at(i)->p->dead)
				{
					if(ac.at(i)->p->getId() == id)
						continue;

					Player *c = ac.at(i)->p;

					float pX = c->x;
					float pY = c->y;

					float cX = 0;
					float cY = 0;

					bool yOver = false;
					bool xOver = false;

					if(pX < y)
					{
						xOver = true;

						cX = (x - pX);
					}

					else
						cX = (pX - x);

					if(pY < y)
					{
						yOver = true;

						cY = (y - pY);
					}

					else
						cY = (pY - y);

					if(((cY + cX) < (cYY + cXX)) || clos == 0)
					{
						clos = c->getId();

						cYY = cY;
						cXX = cX;
					}
				}
			}
		}

		type = 0;
		return clos;
	}

	else
	{
		type = 1;
		return clos;
	}
}


void Client::UpdateFriends(bool on)
{
	Packet pak = Packet();

	pak.addHeader(0x0E);
	pak.addString(this->p->name);
	pak.addLongInt(this->p->level);
	pak.addLongInt(0);
	pak.addLongInt(this->p->getId());
	pak.addBool(on);
	
	pak.ready();

	for(unsigned int i = 0; i < this->p->friends.size(); i++)
	{
		Client *cli = GetClientByPlayerName((char*)this->p->friends.at(i).name.c_str());

		if(cli != NULL)
		{
			if(cli->ingame)
			{
				cli->AddPacket(pak, 0);

				this->p->friends.at(i).status = true;
			}
		}

		else
			this->p->friends.at(i).status = false;

		Player *pl = GetPlayerByName(this->p->friends.at(i).name.c_str());

		if(pl != NULL)
		{
			for(unsigned int y = 0; y < pl->friends.size(); y++)
			{
				if(strcmp(pl->friends.at(y).name.c_str(), this->p->name) == 0)
				{
					pl->friends.at(y).level = this->p->level;
					pl->friends.at(y).job = 0;
					pl->friends.at(y).id = this->p->getId();
					pl->friends.at(y).status = on;
				}
			}

			this->p->friends.at(i).level = pl->level;
			this->p->friends.at(i).job = 0;
			this->p->friends.at(i).id = pl->getId();
		}

		this->p->friends.at(i).c = cli;
	}
}

void Client::RemoveFriend(const char *name)
{
	for(unsigned int i = 0; i < this->p->friends.size(); i++)
	{
		Partner *ps = &this->p->friends.at(i);

		if(strcmp(ps->name.c_str(), name) == 0)
		{
			Client *plas = GetClientByPlayerName((char*)name);

			if(plas == NULL)
			{
				for(unsigned int y = 0; y < pvector.size(); y++)
				{
					if(strcmp(pvector.at(y)->name, name) == 0)
					{
						for(unsigned int z = 0; z < pvector.at(y)->friends.size(); z++)
						{
							if(strcmp(pvector.at(y)->friends.at(z).name.c_str(), this->p->name) == 0)
							{
								pvector.at(y)->friends.erase(pvector.at(y)->friends.begin() + z);

								break;
							}
						}

						break;
					}
				}
			}

			else
			{
				for(unsigned int y = 0; y < plas->p->friends.size(); y++)
				{
					if(strcmp(plas->p->friends.at(y).name.c_str(), this->p->name) == 0)
					{
						plas->p->friends.erase(plas->p->friends.begin() + y);

						Packet pak2 = Packet();

						pak2.addHeader(0x0D);
						pak2.addInt(2);
						pak2.addString(this->p->name);

						pak2.ready();
						plas->AddPacket(pak2, 0);

						plas->SavePlayer(AUTOSAVE);

						break;
					}
				}
			}

			this->p->friends.erase(this->p->friends.begin() + i);

			Packet pak = Packet();

			pak.addHeader(0x0D);
			pak.addInt(2);
			pak.addString((char*)name);

			pak.ready();
			this->AddPacket(pak, 0);
			
			LD(GAME, "Remove friend: [Name: %s] [Name: %s].\n", this->p->name, name);

			break;
		}
	}
}

void FixFriends()
{
	vector<int> removeId;
	vector<string> removeName;

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		for(unsigned int y = 0; y < pvector.at(i)->friends.size(); y++)
		{
			bool found = false;

			for(unsigned int z = 0; z < pvector.size(); z++)
			{
				if(strcmp(pvector.at(z)->name, pvector.at(i)->friends.at(y).name.c_str()) == 0)
				{
					for(unsigned int r = 0; r < pvector.at(z)->friends.size(); r++)
					{
						if(strcmp(pvector.at(i)->name, pvector.at(z)->friends.at(r).name.c_str()) == 0)
						{
							found = true;
							break;
						}
					}

					pvector.at(i)->friends.at(y).level = pvector.at(z)->level;
					pvector.at(i)->friends.at(y).id = pvector.at(z)->getId();
					pvector.at(i)->friends.at(y).job = 0;
					pvector.at(i)->friends.at(y).c = NULL;
					pvector.at(i)->friends.at(y).status = false;

					break;
				}
			}

			if(!found)
			{
				removeId.push_back(pvector.at(i)->getId());
				removeName.push_back(pvector.at(i)->friends.at(y).name);
			}

			found = false;
		}
	}

	for(unsigned int i = 0; i < removeId.size(); i++)
	{
		for(unsigned int y = 0; y < pvector.size(); y++)
		{
			if(pvector.at(y)->getId() == removeId.at(i))
			{
				for(unsigned int z = 0; z < pvector.at(y)->friends.size(); z++)
				{
					if(strcmp(removeName.at(i).c_str(), pvector.at(y)->friends.at(z).name.c_str()) == 0)
					{
						pvector.at(y)->friends.erase(pvector.at(y)->friends.begin() + z);

						break;
					}
				}

				break;
			}
		}
	}
}

void Client::SendPM(const char *name, const char *msg)
{
	if(strcmp(name, this->p->name) == 0)
	{
		SystemChat(this, CUSTOM, NULL, "You cannot send a message to yourself!");

		return;
	}

	Client *c = GetClientByPlayerName((char*)name);

	if(c == NULL)
	{
		string errorString = name;
		errorString += " is not available right now.";

		SystemChat(this, CUSTOM, NULL, (char*)errorString.c_str());
	}

	else
	{
		Packet pak = Packet();

		pak.addHeader(0x1A);
		pak.addString(this->p->name);
		pak.addString(this->p->name);
		pak.addString((char*)msg);

		pak.ready();
		c->AddPacket(pak, 0);

		Packet pak2 = Packet();

		pak2.addHeader(0x1A);
		pak2.addString(c->p->name);
		pak2.addString(this->p->name);
		pak2.addString((char*)msg);

		pak2.ready();
		this->AddPacket(pak2, 0);
	}
}

Class *GetClass(int id)
{
	Class *retVal = NULL;

	for(unsigned int i = 0; i < classes.size(); i++)
	{
		if(id == classes.at(i)->id)
		{
			retVal = classes.at(i);
			break;
		}
	}

	if(retVal == NULL)
	{
		log(DEBUG, "CLASS NOT FOUND %d %d\n", id, classes.size());
	}

	return retVal;
}

bool IsSkill(int param)
{
	for(unsigned i = 0; i < classes.size(); i++)
	{
		if(classes.at(i)->normalShot == param)
			return false;
	}

	return true;
}

void CheckClassSkills(Player *p)
{
	Class *gc = GetClass(p->job);
	vector<int> skillIdRemove;

	if(gc == NULL)
		log(ERR, "[void CheckSkills(Player*)] [Class is NULL] [%d]\n", p->job);

	else
	{
		for(unsigned int i = 0; i < gc->skills.size(); i++)
		{
			bool found = false;

			for(unsigned int y = 0; y < p->skills.size(); y++)
			{
				if(gc->skills.at(i)->id == p->skills.at(y)->id)
				{
					found = true;
					break;
				}
			}

			if(!found)
				skillIdRemove.push_back(gc->skills.at(i)->id);
		}

		for(unsigned int i = 0; i < skillIdRemove.size(); i++)
		{
			Skill *s = new Skill();
			Skill *stru = GetMainSkill(skillIdRemove.at(i));

			*s = *stru;

			s->lvl = 1;
			s->exp = 0;

			p->skills.push_back(s);
		}
	}
}

void CheckSkills(Player *p)
{
	Class *gc = GetClass(p->job);
	vector<int> skillIdRemove;

	if(gc == NULL)
		log(ERR, "[void CheckSkills(Player*)] [Class is NULL] [%d]\n", p->job);

	else
	{
		for(unsigned int i = 0; i < p->skills.size(); i++)
		{
			bool found = false;

			for(unsigned int y = 0; y < gc->skills.size(); y++)
			{
				if(gc->skills.at(y)->id == p->skills.at(i)->id)
				{
					found = true;
					break;
				}
			}

			if(!found)
				skillIdRemove.push_back(p->skills.at(i)->id);
		}

		for(unsigned int i = 0; i < skillIdRemove.size(); i++)
		{
			for(unsigned int y = 0; y < p->skills.size(); y++)
			{
				if(p->skills.at(y)->id == skillIdRemove.at(i))
				{
					delete p->skills.at(y);
					p->skills.erase(p->skills.begin() + y);

					break;
				}
			}
		}
	}
}