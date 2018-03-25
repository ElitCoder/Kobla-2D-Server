#ifndef COMMAND_H
#define COMMAND_H

#define EXP 1
#define STAT 2
#define HP 3
#define HPPROCENT 4
#define PKPOINTS 5
#define MANA 6
#define ENERGY 7

class WorldCommand
{
public:
	WorldCommand()
	{
	}

	void sendDamage(Player *pl, Monster *m, int critType, int damage);
	void sendPlayerFollow(Client *follower, Client *target, bool status);
	void sendFollow(Monster *m, Client *c);
	void sendUpdateMProcent(Monster *m);

	int killPlayer(Client *c);
	int killMonster(Client *attacker, Monster *target, bool smite);

	void createMonster(int mId, int mapId, float x, float y, bool aggro, Monster *struc2);
};

#endif