#ifndef ITEM_H
#define ITEM_H

#define DELETEITEM 1
#define MOVEITEM 2

class Item
{
public:
	Item()
	{
		this->id = 0;
		this->effect = 0;
		this->value = 0;
		this->maxstack = 0;
		this->stack = 1;

		this->price = 0;
		this->minlvl = 0;

		this->itemName = "";

		this->job = 0;
	}

	Item(const Item &c)
	{
		this->id = c.id;
		this->effect = c.effect;
		this->value = c.value;
		this->maxstack = c.maxstack;
		this->stack = c.stack;
		this->job = c.job;

		this->price = c.price;
		this->minlvl = c.minlvl;
		
		this->itemName = c.itemName;

		this->evalue = c.evalue;
	}

	int id;
	int effect;
	int value;
	int maxstack;
	int stack;

	int price;
	int minlvl;

	std::string itemName;

	std::vector<TwoInt> evalue;

	int job;
};

class Drop
{
public:
	Drop()
	{
		this->drop = false;

		this->owner = 0;
		this->autodelete = 0;

		this->map = 0;
		this->x = 0;
		this->y = 0;

		this->id = 0;

		this->loottimer = 0;
		this->lootEnable = false;
		this->noChangePenya = false;
	}

	Item *drop;

	int owner;
	long autodelete;

	int map;
	int x;
	int y;

	int id;

	long loottimer;
	bool lootEnable;
	bool noChangePenya;
};

class StartItem
{
public:
	StartItem()
	{
		this->id = 0;
		this->eq = false;
		this->stack = 0;
		this->slot = 0;
	}

	int id;
	bool eq;
	int stack;
	int slot;
};

class Class
{
public:
	Class()
	{
		this->id = 1;

		this->needweap = true;
		this->mpgot = true;
		this->egot = false;
		this->mpplus = 0;
		this->eplus = 0;
		this->hpplus = 0;

		this->normalShot = 1;
		this->based = STRENGTH;

		this->bulletSpeed = 900;
		this->bulletRotate = 0;

		this->disabled = false;

		this->shootType = 0;
		this->shootCost = 0;

		this->ranged = false;

		this->bulletDis = 650;

		this->baseMult = 1;
	}

	int id;

	std::vector<Skill*> skills;
	bool needweap;
	bool mpgot;
	bool egot;
	int mpplus;
	int eplus;
	int hpplus;

	int normalShot;
	int based;

	int bulletSpeed;
	int bulletRotate;

	std::vector<StartItem> items;

	bool disabled;

	int shootType;
	int shootCost;

	bool ranged;

	int bulletDis;

	std::vector<TwoInt> effects;

	int baseMult;
};

Item *GetItemById(int id);

void DropItem(Item *item, int owner, int map, int x, int y, bool noChangePenya);

std::vector<Drop*> DropPacket(Monster *th);

#endif