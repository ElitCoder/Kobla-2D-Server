#include "Main.h"
#include "Command.h"
#include "Handle.h"

extern int maxLvl;

extern long reviveDelay;
extern long moveDelay;
extern long monsterRespawnDelay;

extern int monsterId;

extern std::vector<Monster*> mobs;

bool Command::checkInfo()
{
	if(this->getPlayer() == NULL || this->getClient() == NULL)
		return false;

	return true;
}

void Command::setAttackTimer(int del)
{
	if(!this->checkInfo())
		return;

	this->p->attacked = true;
	this->p->atkTimer = (getTimestamp() + del);
}

void Command::setFoodCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	this->p->foodCoolDown = (getTimestamp() + del);
}

void Command::setPillCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	this->p->pillCoolDown = (getTimestamp() + del);
}

void Command::setBulletCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	if(del < 0)
		del = 0;

	this->p->bulletCoolDown = (getTime(false) + del);
}

void Command::setPosCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	this->p->posCoolDown = (getTimestamp() + del);
}

void Command::setChatCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	this->p->chatCoolDown = (getTimestamp() + del);
}

void Command::setExtraCoolDown(int del)
{
	if(!this->checkInfo())
		return;

	this->p->extraCoolDown = (getTimestamp() + del);
}

void Command::sendUpdate(int up)
{
	if(!this->checkInfo())
		return;

	Packet pak = Packet();

	switch(up)
	{
		case EXP:
			{
				pak.addHeader(0x19);
				pak.addLongInt(this->p->level);
				pak.addFloat(this->c->CalculateExp());
				pak.addLongInt(this->p->powerpoints);
			break;
			}

		case STAT:
			{
				pak.addHeader(0x22);
				pak.addLongInt((this->p->power + this->c->GetBonus(STRENGTH)));
				pak.addLongInt((this->p->magic + this->c->GetBonus(STAMINA)));
				pak.addLongInt((this->p->agility + this->c->GetBonus(AGILITY)));
				pak.addLongInt((this->p->intt + this->c->GetBonus(INTT)));
				pak.addLongInt(this->p->powerpoints);
				pak.addLongInt((int)this->p->hp);
				pak.addLongInt(this->c->GetBonus(ATTACKSPEED));
				pak.addLongInt(this->c->GetBonus(STRENGTH));
				pak.addLongInt(this->c->GetBonus(STAMINA));
				pak.addLongInt(this->c->GetBonus(AGILITY));
				pak.addLongInt(this->c->GetBonus(INTT));
			break;
			}

		case HP:
			{
				pak.addHeader(0x13);
				pak.addInt(HPUPDATE);
				pak.addBool(this->p->dead);
				pak.addLongInt((int)this->p->chp);
				pak.addLongInt((int)this->p->hp);
			break;
			}

		case MANA:
			{
				pak.addHeader(0x13);
				pak.addInt(MANAUPDATE);
				pak.addLongInt((int)this->p->mana);
				pak.addLongInt((int)this->p->maxmana);
			break;
			}

		case ENERGY:
			{
				pak.addHeader(0x13);
				pak.addInt(ENERGYUPDATE);
				pak.addLongInt((int)this->p->energy);
				pak.addLongInt((int)this->p->maxenergy);
			break;
			}

		case HPPROCENT:
			{
				pak.addHeader(0x62);
				pak.addLongInt(1);
				pak.addLongInt(this->p->getId());
				pak.addInt(GetPlayerProcentHP(this->p));
			break;
			}

		case PKPOINTS:
			{
				pak.addHeader(0x52);
				pak.addLongInt(this->p->pkpoints);
			break;
			}

		default: log(ERR, "[void Command::sendUpdate(int)] [Unknown param] [%d]\n", up);
			break;
	}

	pak.ready();

	if(up == HPPROCENT)
		SendAllOnMap(pak, 0, this->p->mapId->id, this->c->getConnection());
	else
		this->c->AddPacket(pak, 0);
}

void WorldCommand::sendUpdateMProcent(Monster *m)
{
	if(m == NULL)
	{
		log(ERR, "[void WorldCommand::sendUpdateMProcent(Monster*)] [Monster is NULL]\n");

		return;
	}

	Packet pak = Packet();

	pak.addHeader(0x61);
	pak.addLongInt(1);
	pak.addLongInt(m->pid);
	pak.addInt(GetMonsterProcentHP(m));

	pak.ready();
	SendAllOnMap(pak, 0, m->mapId->id, 0);
}

int WorldCommand::killPlayer(Client *c)
{
	if(c == NULL)
	{
		log(ERR, "[int WorldCommand::killPlayer(Client*)] [Client is NULL]\n");

		return 0;
	}

	this->sendPlayerFollow(c, c, false);

	c->p->chp = 0;
	c->p->mana = 0;
	c->p->energy = 0;

	c->p->dead = true;
	c->p->reviveTime = (getTimestamp() + reviveDelay);

	c->RemoveAllBuffs();
	c->RemoveAllFollowers();

	Packet b2 = Packet();

	b2.addHeader(0x29);
	b2.addInt(2);
	b2.addInt(SPEED);
	b2.addLongInt(c->p->speed);
	b2.addLongInt(c->p->getId());

	b2.ready();
	SendAllOnMap(b2, 0, c->p->mapId->id, 0);

	Packet p3 = Packet();

	p3.addHeader(0x21);
	p3.addBool(true);
	p3.addLongInt(c->p->getId());

	p3.ready();
	SendAllOnMap(p3, 0, c->p->mapId->id, c->getConnection());

	if(c->p->moving)
	{
		c->p->cMove = 0;
		c->p->moving = false;

		Packet pak = Packet();

		pak.addHeader(0x9);
		pak.addInt(1);
		pak.addLongInt(c->p->getId());
		pak.addLongInt((int)c->p->x);
		pak.addLongInt((int)c->p->y);

		pak.ready();
		SendAllOnMap(pak, 0, c->p->mapId->id, c->getConnection());
	}

	c->com->sendUpdate(HP);
	c->com->sendUpdate(HPPROCENT);

	c->com->sendUpdate(MANA);
	c->com->sendUpdate(ENERGY);

	return 1;
}

void WorldCommand::createMonster(int mId, int mapId, float x, float y, bool aggro, Monster *struc2)
{
	Monster *struc = NULL;

	if(struc2 != NULL)
		struc = struc2;

	else
	{
		for(unsigned int i = 0; i < mobs.size(); i++)
		{
			if(mobs.at(i)->id == mId)
			{
				struc = mobs.at(i);

				break;
			}
		}
	}

	if(struc == NULL)
		log(ERR, "[void WorldCommand::createMonster(int,int,float,float,bool)] [Could not find monster] [%d]\n", mId);

	else
	{
		int r = Random((moveDelay / 2), (moveDelay * 2));

		Monster *m = new Monster();

		*m = *struc;

		m->pid = monsterId;
		monsterId++;

		m->attacked = false;
		m->mapId = GetMap(mapId);
		if(m->mapId == NULL)
			log(ERR, "[void WorldCommand::createMonster(int,int,float,float,bool)] [Could not find map] [%d]\n", mapId);
		m->moving = false;
		m->x = x;
		m->y = y;
		m->nextMove = (getTimestamp() + r);
		m->spawned = true;
		m->respawnTime = (getTimestamp() + monsterRespawnDelay);
		m->minY = 16;
		m->maxY = (m->mapId->ySize - m->monH);
		m->minX = 0;
		m->maxX = (m->mapId->xSize - m->monW);
		m->aggChance = 0;

		if(m->x < 0)
			m->x = 0;

		if(m->y < 16)
			m->y = 16;

		if(m->x > (m->mapId->xSize - m->monW))
			m->x = (float)(m->mapId->xSize - m->monW);

		if(m->y > (m->mapId->ySize - m->monH))
			m->y = (float)(m->mapId->ySize - m->monH);

		m->aggro = aggro;

		m->respawnDelayMonster = monsterRespawnDelay;

		m->noMove = false;
		m->removeOnDeath = true;

		mobs.push_back(m);
	}
}

int WorldCommand::killMonster(Client *attacker, Monster *target, bool smite)
{
	if(target == NULL)
	{
		log(ERR, "[int WorldCommand::killMonster(Client*,Monster*)] [Is NULL]\n");

		return 0;
	}

	target->chp = 0;
	target->spawned = false;
	target->respawnTime = (getTimestamp() + target->respawnDelayMonster);

	if(attacker == NULL)
		target->attackerId = 0;
	else
		target->attackerId = attacker->p->getId();

	if(!smite)
		target->Drop();

	target->attackerId = 0;
	target->followId = 0;
	target->follows = false;
	target->attacked = false;
	target->goAfter = false;
	target->moveDone = true;

	target->RemoveAllMonsterBuffs();

	if(attacker == NULL)
	{
		if(!smite)
			log(ERR, "[int WorldCommand::killMonster(Client*,Client*)] [Could not find player for adding exp]\n");
	}

	else
	{
		if(!attacker->p->dead)
			attacker->AddKillExp(target->expamount, 15);
	}

	Packet pak = Packet();

	pak.addHeader(0x12);
	pak.addInt(2);
	pak.addLongInt(target->pid);

	pak.ready();
	SendAllOnMap(pak, 0, target->mapId->id, 0);

	if(target->removeOnDeath)
	{
		for(unsigned int i = 0; i < mobs.size(); i++)
		{
			if(target->pid == mobs.at(i)->pid)
			{
				delete target;
				mobs.erase(mobs.begin() + i);

				break;
			}
		}
	}

	return 1;
}

void WorldCommand::sendDamage(Player *pl, Monster *m, int critType, int damage)
{
	if(pl == NULL && m == NULL)
		return;

	int moverType = 1;
	int sendId = 0;
	int mapId = 0;
	int porc = 100;

	if(pl == NULL)
	{
		sendId = m->pid;
		mapId = m->mapId->id;
		moverType = 2;
		porc = GetMonsterProcentHP(m);
	}

	else
	{
		sendId = pl->getId();
		mapId = pl->mapId->id;
		porc = GetPlayerProcentHP(pl);
	}

	Packet pak = Packet();

	pak.addHeader(0x42);
	pak.addLongInt(1);
	pak.addInt(moverType);
	pak.addInt(critType);
	pak.addLongInt(sendId);
	pak.addLongInt(damage);
	pak.addInt(porc);

	pak.ready();
	SendAllOnMap(pak, 0, mapId, 0);
}

void WorldCommand::sendPlayerFollow(Client *follower, Client *target, bool status)
{
	if(follower == NULL || target == NULL)
	{
		log(ERR, "[void WorldCommand::sendPlayerFollow(Client*,Client*,bool)] [Is NULL]\n");

		return;
	}

	if(follower->p->dead)
	{
		SystemChat(follower, CUSTOM, NULL, "You can not follow a player while you are dead.");

		return;
	}

	if(status)
	{
		follower->p->followId = target->p->getId();
		follower->p->follow = true;

		follower->p->moving = false;
		follower->p->cMove = 0;

		follower->p->move.start(false);
	}

	else
	{
		follower->p->followId = 0;
		follower->p->follow = false;
	}

	Packet pak = Packet();

	pak.addHeader(0x71);
	pak.addBool(status);
	pak.addLongInt(follower->p->getId());
	pak.addLongInt(target->p->getId());
	pak.addLongInt((int)follower->p->x);
	pak.addLongInt((int)follower->p->y);
	pak.addLongInt((int)target->p->x);
	pak.addLongInt((int)target->p->y);

	pak.ready();
	SendAllOnMap(pak, 0, follower->p->mapId->id, 0);
}

void WorldCommand::sendFollow(Monster *m, Client *c)
{
	if(m == NULL || c == NULL)
	{
		log(ERR, "[void WorldCommand::sendFollow(Monster*,Client*)] [Is NULL]\n");

		return;
	}

	m->followId = c->p->getId();
	m->follows = true;
	m->goAfter = false;
	m->moveDone = true;

	m->moving = false;
	m->goDir = 0;

	m->bx = m->x;
	m->by = m->y;

	m->move.start(true);

	Packet pak = Packet();

	pak.addHeader(0x47);
	pak.addLongInt(1);
	pak.addLongInt(m->pid);
	pak.addLongInt(c->p->getId());
	pak.addLongInt((int)m->x);
	pak.addLongInt((int)m->y);

	pak.ready();
	SendAllOnMap(pak, 0, c->p->mapId->id, 0);
}