#include <vector>

#include "Main.h"
#include "World.h"
#include "Item.h"
#include "Handle.h"
#include "Command.h"
#include "Connection.h"

using namespace std;

std::vector<Item*> items;
extern std::vector<Drop*> drops;

extern bool debugs;

extern int penyaRate;
extern int dropRate;

extern long foodDefCooldown;

long deleteDropDelay = 60000;

int dropCId = 1;

Item *GetItemById(int id)
{
	for(unsigned int i = 0; i < items.size(); i++)
	{
		if(items.at(i)->id == id)
			return items.at(i);
	}

	return NULL;
}

int Client::GetFirstInventorySlot(int id, int amount)
{
	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
		{
			if(this->p->inv[i]->id == id && this->p->inv[i]->maxstack >= (this->p->inv[i]->stack + amount))
				return (i + 1);
		}
	}

	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] == NULL)
			return (i + 1);
	}

	return 0;
}

int Client::GetFirstNotFullSlot(int id)
{
	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] != NULL)
		{
			if(this->p->inv[i]->id == id && this->p->inv[i]->maxstack > this->p->inv[i]->stack)
				return (i + 1);
		}
	}

	return 0;
}

int Client::GetFirstCleanSlot()
{
	for(int i = 0; i < 25; i++)
	{
		if(this->p->inv[i] == NULL)
			return (i + 1);
	}

	return 0;
}

bool Client::AddItem(int id, int amount, bool precv, Drop *it, int &slotAddedTo)
{
	bool ret = false;

	if(id != 2)
	{
		if(amount == 1)
		{
			int slotAdd = this->GetFirstInventorySlot(id, amount);
			slotAddedTo = slotAdd;

			if(slotAdd == 0)
			{
			}

			else
			{
				Item *stru = GetItemById(id);

				if(stru == NULL)
					log(ERR, "[bool Client::AddItem(int,int,bool,Drop*,int&)] [Could not find item] [%d]\n", id);

				else
				{
					if(this->p->inv[(slotAdd - 1)] != NULL)
					{
						if(this->p->inv[(slotAdd - 1)]->maxstack >= (this->p->inv[(slotAdd - 1)]->stack + amount))
						{
							this->p->inv[(slotAdd - 1)]->stack += amount;

							Packet pak = Packet();

							pak.addHeader(0x41);
							pak.addInt(slotAdd);
							pak.addLongInt(this->p->inv[(slotAdd - 1)]->stack);
							pak.addLongInt(amount);
							pak.addBool(precv);

							pak.ready();
							this->AddPacket(pak, 0);

							ret = true;
						}

						else
						{
							Item *item = new Item();

							*item = *stru;

							item->stack = amount;
							item->id = id;

							this->p->inv[(slotAdd - 1)] = item;

							Packet pak = Packet();

							pak.addHeader(0x34);
							pak.addInt(slotAdd);
							pak.addLongInt(item->stack);
							pak.addLongInt(item->id);
							pak.addLongInt(item->value);
							pak.addLongInt(item->effect);
							pak.addBool(precv);
							pak.addLongInt(item->maxstack);

							pak.ready();
							this->AddPacket(pak, 0);

							ret = true;
						}
					}

					else
					{
						Item *item = new Item();

						*item = *stru;

						item->stack = amount;
						item->id = id;

						this->p->inv[(slotAdd - 1)] = item;

						Packet pak = Packet();

						pak.addHeader(0x34);
						pak.addInt(slotAdd);
						pak.addLongInt(item->stack);
						pak.addLongInt(item->id);
						pak.addLongInt(item->value);
						pak.addLongInt(item->effect);
						pak.addBool(precv);
						pak.addLongInt(item->maxstack);

						pak.ready();
						this->AddPacket(pak, 0);

						ret = true;
					}
				}
			}
		}

		else if(amount > 1)
		{
			bool borr = true;

			int am = amount;
			int added = 0;
			int temp = 0;

			while(am > 0)
			{
				temp = this->GetFirstNotFullSlot(id);

				if(temp > 0)
				{
					if(am > (this->p->inv[(temp - 1)]->maxstack - this->p->inv[(temp - 1)]->stack))
					{
						am -= (this->p->inv[(temp - 1)]->maxstack - this->p->inv[(temp - 1)]->stack);
						added += (this->p->inv[(temp - 1)]->maxstack - this->p->inv[(temp - 1)]->stack);

						int adds = (this->p->inv[(temp - 1)]->maxstack - this->p->inv[(temp - 1)]->stack);

						this->p->inv[(temp - 1)]->stack = this->p->inv[(temp - 1)]->maxstack;

						Packet pak = Packet();

						pak.addHeader(0x41);
						pak.addInt(temp);
						pak.addLongInt(this->p->inv[(temp - 1)]->stack);
						pak.addLongInt(adds);
						pak.addBool(false);

						pak.ready();
						this->AddPacket(pak, 0);
					}

					else
					{
						this->p->inv[(temp - 1)]->stack += am;

						added += am;
						
						Packet pak = Packet();

						pak.addHeader(0x41);
						pak.addInt(temp);
						pak.addLongInt(this->p->inv[(temp - 1)]->stack);
						pak.addLongInt(am);
						pak.addBool(false);

						pak.ready();
						this->AddPacket(pak, 0);

						am = 0;
						break;
					}
				}

				else
				{
					temp = this->GetFirstCleanSlot();

					if(temp > 0)
					{
						int t = GetItemById(id)->maxstack;

						if(t > am)
						{
							Item *stru = GetItemById(id);
							Item *item = new Item();

							*item = *stru;

							item->stack = am;
							item->id = id;

							this->p->inv[(temp - 1)] = item;

							Packet pak = Packet();

							pak.addHeader(0x34);
							pak.addInt(temp);
							pak.addLongInt(item->stack);
							pak.addLongInt(item->id);
							pak.addLongInt(item->value);
							pak.addLongInt(item->effect);
							pak.addBool(false);
							pak.addLongInt(item->maxstack);

							pak.ready();
							this->AddPacket(pak, 0);


							added += am;
							am = 0;
							break;
						}

						else
						{
							Item *stru = GetItemById(id);
							Item *item = new Item();

							*item = *stru;

							item->stack = t;
							item->id = id;

							this->p->inv[(temp - 1)] = item;

							Packet pak = Packet();

							pak.addHeader(0x34);
							pak.addInt(temp);
							pak.addLongInt(item->stack);
							pak.addLongInt(item->id);
							pak.addLongInt(item->value);
							pak.addLongInt(item->effect);
							pak.addBool(false);
							pak.addLongInt(item->maxstack);

							pak.ready();
							this->AddPacket(pak, 0);

							am -= t;
							added += t;
						}
					}

					else
					{
						if(added > 0)
						{
							if(it != NULL)
								it->drop->stack -= added;
						}

						borr = false;
						break;
					}
				}
			}

			ret = borr;

			if(added > 0 && precv)
			{
				Packet pk = Packet();

				pk.addHeader(0x56);
				pk.addBool(false);
				pk.addLongInt(added);
				pk.addLongInt(0);
				pk.addString((char*)GetItemById(id)->itemName.c_str());

				pk.ready();
				this->AddPacket(pk, 0);
			}
		}
	}

	else
	{
		if(this->p->gold >= 2000000000)
		{
			ret = false;

			SystemChat(this, CUSTOM, NULL, "You have too much dinero.");
		}

		else
		{
			if((this->p->gold + amount) <= 2000000000)
			{
				this->p->gold += amount;

				Packet pak = Packet();

				pak.addHeader(0x40);
				pak.addLongInt(this->p->gold);
				pak.addLongInt(amount);
				pak.addBool(precv);

				pak.ready();
				this->AddPacket(pak, 0);

				ret = true;
			}

			else
			{
				if(it == NULL)
					ret = false;

				else
				{
					it->drop->stack -= (2000000000 - this->p->gold);
					this->p->gold = 2000000000;

					Packet pak = Packet();

					pak.addHeader(0x40);
					pak.addLongInt(this->p->gold);
					pak.addLongInt(amount);
					pak.addBool(precv);

					pak.ready();
					this->AddPacket(pak, 0);

					ret = false;
				}
			}
		}
	}

	return ret;
}

void Client::RemoveItem(int slot, int qty, bool tradeAdd, int itemid)
{
	if(this->p->trading && !tradeAdd)
	{
		SystemChat(this, CUSTOM, NULL, "You can not modify your inventory while trading.");

		return;
	}

	if(itemid == 2)
	{
		if(qty > this->p->gold || qty <= 0)
			return;

		this->p->gold -= qty;

		Packet pak = Packet();

		pak.addHeader(0x40);
		pak.addLongInt(this->p->gold);
		pak.addLongInt(0);
		pak.addBool(false);

		pak.ready();
		this->AddPacket(pak, 0);

		return;
	}

	Item *tar = this->p->inv[(slot - 1)];

	if(tar == NULL)
		log(ERR, "[void Client::RemoveItem(int,int,bool,int)] [Could not find inventory item to delete] [%d]\n", slot);

	else
	{
		if(qty > this->p->inv[(slot - 1)]->stack || qty <= 0 || this->p->inv[(slot - 1)]->stack <= 0)
			return;

		if(this->p->inv[(slot - 1)]->stack > 1)
		{
			if(qty >= this->p->inv[(slot - 1)]->stack)
			{
				delete tar;
				this->p->inv[(slot - 1)] = NULL;

				this->UpdateTaskbar(DELETEITEM, slot);

				Packet pak = Packet();

				pak.addHeader(0x36);
				pak.addInt(slot);

				pak.ready();
				this->AddPacket(pak, 0);
			}

			else
			{
				this->p->inv[(slot - 1)]->stack -= qty;

				this->UpdateTaskbar(DELETEITEM, slot);

				Packet pak = Packet();

				pak.addHeader(0x41);
				pak.addInt(slot);
				pak.addLongInt(this->p->inv[(slot - 1)]->stack);
				pak.addLongInt(1);
				pak.addBool(false);

				pak.ready();
				this->AddPacket(pak, 0);
			}
		}

		else
		{
			delete tar;
			this->p->inv[(slot - 1)] = NULL;

			this->UpdateTaskbar(DELETEITEM, slot);

			Packet pak = Packet();

			pak.addHeader(0x36);
			pak.addInt(slot);

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}
}

void Client::MoveItem(int slot, int toslot)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not modify your inventory while trading.");

		return;
	}

	if(this->p->inv[(slot - 1)] == NULL)
		log(ERR, "[void Client::MoveItem(int,int)] [Item is NULL] [%d]\n", slot);

	else
	{
		if(slot != toslot)
		{
			if(this->p->inv[(toslot - 1)] != NULL)
			{
				Item *temp = this->p->inv[(toslot - 1)];

				if(temp->id == this->p->inv[(slot - 1)]->id && this->p->inv[(slot - 1)]->maxstack > this->p->inv[(slot - 1)]->stack && temp->maxstack > temp->stack)
				{
					if(temp->maxstack >= (temp->stack + this->p->inv[(slot - 1)]->stack))
					{
						temp->stack += this->p->inv[(slot - 1)]->stack;

						this->RemoveItem(slot, this->p->inv[(slot - 1)]->stack, true, this->p->inv[(slot - 1)]->id);

						for(int i = 0; i < 9; i++)
						{
							if(this->p->task[i]->slot == slot && this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && !this->p->task[i]->eqinv)
								this->p->task[i]->slot = toslot;
						}

						UpdateTaskbar(MOVEITEM, 0);

						Packet pak = Packet();

						pak.addHeader(0x41);
						pak.addInt(toslot);
						pak.addLongInt(temp->stack);
						pak.addLongInt(1);
						pak.addBool(false);

						pak.ready();
						this->AddPacket(pak, 0);
					}

					else
					{
						int slotstack = this->p->inv[(slot - 1)]->stack;
						int toslotstack = temp->stack;

						temp->stack = temp->maxstack;
						this->p->inv[(slot - 1)]->stack = ((slotstack + toslotstack) - temp->maxstack);

						UpdateTaskbar(MOVEITEM, 0);

						Packet pak = Packet();

						pak.addHeader(0x41);
						pak.addInt(toslot);
						pak.addLongInt(temp->stack);
						pak.addLongInt(1);
						pak.addBool(false);

						pak.ready();
						this->AddPacket(pak, 0);

						Packet pak2 = Packet();

						pak2.addHeader(0x41);
						pak2.addInt(slot);
						pak2.addLongInt(this->p->inv[(slot - 1)]->stack);
						pak2.addLongInt(1);
						pak2.addBool(false);

						pak2.ready();
						this->AddPacket(pak2, 10);

						if(debugs)
							log(DEBUG, "temp->stack = %d this->p->inv->stack = %d. %d.\n", temp->stack, this->p->inv[(slot - 1)]->stack, ((slotstack + toslotstack) - temp->maxstack));
					}
				}

				else
				{
					for(int i = 0; i < 9; i++)
					{
						if(this->p->task[i]->slot == slot && this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && !this->p->task[i]->eqinv)
							this->p->task[i]->slot = toslot;

						else if(this->p->task[i]->slot == toslot && this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && !this->p->task[i]->eqinv)
							this->p->task[i]->slot = slot;
					}

					this->p->inv[(toslot - 1)] = this->p->inv[(slot - 1)];
					this->p->inv[(slot - 1)] = temp;

					UpdateTaskbar(MOVEITEM, 0);

					Packet pak = Packet();

					pak.addHeader(0x35);
					pak.addInt(2);
					pak.addInt(slot);
					pak.addInt(toslot);

					pak.ready();
					this->AddPacket(pak, 0);

					if(debugs)
						log(DEBUG, "Switched slot: %d and %d.\n", slot, toslot);
				}
			}

			else
			{
				for(int i = 0; i < 9; i++)
				{
					if(this->p->task[i]->slot == slot && this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && !this->p->task[i]->eqinv)
						this->p->task[i]->slot = toslot;
				}

				this->p->inv[(toslot - 1)] = this->p->inv[(slot - 1)];
				this->p->inv[(slot - 1)] = NULL;

				UpdateTaskbar(MOVEITEM, 0);

				Packet pak = Packet();

				pak.addHeader(0x35);
				pak.addInt(1);
				pak.addInt(slot);
				pak.addInt(toslot);

				pak.ready();
				this->AddPacket(pak, 0);

				if(debugs)
					log(DEBUG, "Moved item on slot %d to slot %d.\n", slot, toslot);
			}
		}
	}
}

bool Client::UseItem(int slot)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not use an item while trading.");

		return false;
	}

	if(!this->p->dead)
	{
		bool retType = false;

		Item *tar = this->p->inv[(slot - 1)];

		if(tar == NULL)
			return false;

		else
		{
			if(this->p->level < tar->minlvl)
			{
				std::string msg = "You need to be level ";
				msg += IntToString(tar->minlvl);
				msg += " to use this item.";

				SystemChat(this, CUSTOM, NULL, (char*)msg.c_str());

				return retType;
			}

			if(tar->job != 0)
			{
				if(this->p->job != tar->job)
				{
					SystemChat(this, CUSTOM, NULL, "You are not the required class for this item.");

					return retType;
				}
			}

			switch(tar->effect)
			{
				case FOOD:
					{
						if((unsigned)this->p->foodCoolDown <= getTimestamp())
						{
							this->p->chp += tar->value;

							if(this->p->chp > this->p->hp)
								this->p->chp = this->p->hp;

							int theId = tar->id;

							this->RemoveItem(slot, 1, false, tar->id);

							this->com->sendUpdate(HP);
							this->com->sendUpdate(HPPROCENT);

							this->com->setFoodCoolDown(foodDefCooldown);
							this->SetFoodCoolDown(theId, foodDefCooldown);

							if(debugs)
								log(DEBUG, "Player used food item.\n");
						}
					break;
					}

				case EQUIPWEAPON: case EQUIPHAT: case EQUIPSUIT: case EQUIPBELT:
					{
						bool goodToGo = true;

						int slotEquip = 1;

						switch(tar->effect)
						{
							case EQUIPWEAPON: slotEquip = 1;
								break;

							case EQUIPHAT: slotEquip = 2;
								break;

							case EQUIPSUIT: slotEquip = 3;
								break;

							case EQUIPBELT: slotEquip = 4;
								break;
						}

						if(this->p->eq[(slotEquip - 1)] != NULL)
						{
							if(!this->UnEquip(slotEquip))
								goodToGo = false;
						}

						if(goodToGo)
						{
							Item *ne = new Item();
							*ne = *tar;

							this->p->eq[(slotEquip - 1)] = ne;

							delete tar;
							this->p->inv[(slot - 1)] = NULL;

							for(int i = 0; i < 9; i++)
							{
								if(this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && this->p->task[i]->slot == slot && !this->p->task[i]->eqinv)
								{
									Packet pak99 = Packet();

									pak99.addHeader(0x63);
									pak99.addInt(1);
									pak99.addInt((i + 1));
									pak99.addInt(this->p->task[i]->slot);
									pak99.addInt(slotEquip);

									pak99.ready();
									this->AddPacket(pak99, 0);

									this->p->task[i]->eqinv = true;
									this->p->task[i]->slot = slotEquip;
								}
							}

							Packet pak88 = Packet();

							pak88.addHeader(0x36);
							pak88.addInt(slot);

							pak88.ready();
							this->AddPacket(pak88, 0);

							Packet pak = Packet();

							pak.addHeader(0x58);
							pak.addInt(1);
							pak.addInt(slotEquip);
							pak.addLongInt(ne->id);
							pak.addLongInt(ne->value);

							pak.ready();
							this->AddPacket(pak, 0);

							Packet pak2 = Packet();

							pak2.addHeader(0x58);
							pak2.addInt(3);
							pak2.addInt(slotEquip);
							pak2.addLongInt(ne->id);
							pak2.addLongInt(this->p->getId());

							pak2.ready();
							SendAllOnMap(pak2, 0, this->p->mapId->id, getConnection());

							if(slotEquip == 1 && !this->p->equipped)
								this->EquipWeapon(true);

							this->p->hp = CalculateHP(this);

							if(this->p->chp > this->p->hp)
								this->p->chp = this->p->hp;
							
							this->p->maxmana = CalculateMana(this);

							if(this->p->mana > this->p->maxmana)
								this->p->mana = this->p->maxmana;

							this->com->sendUpdate(HP);
							this->com->sendUpdate(HPPROCENT);
							this->com->sendUpdate(STAT);
							this->com->sendUpdate(MANA);

							retType = true;
						}
					break;
					}

				case FOODPILL:
					{
						if((unsigned)this->p->pillCoolDown <= getTimestamp())
						{
							this->p->chp += tar->value;

							if(this->p->chp > this->p->hp)
								this->p->chp = this->p->hp;

							int tarId = tar->id;

							this->RemoveItem(slot, 1, false, tar->id);

							this->com->sendUpdate(HP);
							this->com->sendUpdate(HPPROCENT);

							this->com->setPillCoolDown(15000);
							this->SetFoodCoolDown(tarId, 15000);

							if(debugs)
								log(DEBUG, "Player used pill item.\n");
						}
					break;
					}

				case RESTAT:
					{
						int allstat = 0;

						allstat += (this->p->power - 2);
						allstat += (this->p->magic - 2);
						allstat += (this->p->agility - 2);
						allstat += (this->p->intt - 2);

						this->p->power = 2;
						this->p->magic = 2;
						this->p->agility = 2;
						this->p->intt = 2;

						this->p->powerpoints += allstat;

						this->RemoveItem(slot, 1, false, tar->id);

						this->p->hp = CalculateHP(this);

						if(this->p->chp > this->p->hp)
							this->p->chp = this->p->hp;

						this->p->maxmana = CalculateMana(this);

						if(this->p->mana > this->p->maxmana)
							this->p->mana = this->p->maxmana;

						this->com->sendUpdate(HP);
						this->com->sendUpdate(HPPROCENT);
						this->com->sendUpdate(STAT);
						this->com->sendUpdate(MANA);

						SystemChat(this, CUSTOM, NULL, "Your stat points have been reset.");
					break;
					}

				case 0: SystemChat(this, CUSTOM, NULL, "That item can not be used.");
					break;

				default: log(ERR, "[bool Client::UseItem(int)] [Unknown usage param] [%d]\n", tar->effect);
					break;
			}
		}

		return retType;
	}

	return false;
}

bool Client::PickUpDrop(int id)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not pick up an item while trading.");

		return false;
	}

	bool status = false;

	for(unsigned int i = 0; i < drops.size(); i++)
	{
		if(id == drops.at(i)->id)
		{
			if(drops.at(i)->owner == this->p->getId() || this->AttackInParty(drops.at(i)->owner) || drops.at(i)->owner == 0 || drops.at(i)->lootEnable)
			{
				int lastStack = drops.at(i)->drop->stack;
				int slotAddedTo = 0;

				if(drops.at(i)->drop->id == 2)
				{
					if(drops.at(i)->noChangePenya)
					{
						bool addedPenya = this->AddItem(2, drops.at(i)->drop->stack, true, drops.at(i), slotAddedTo);

						if(addedPenya)
						{
							Packet pak = Packet();

							pak.addHeader(0x39);
							pak.addInt(2);
							pak.addLongInt(drops.at(i)->id);

							pak.ready();

							SendAllOnMap(pak, 15, drops.at(i)->map, 0);

							delete drops.at(i)->drop;
							delete drops.at(i);

							drops.erase(drops.begin() + i);

							status = true;
						}
					}

					else
					{
						float penya1min = (float)drops.at(i)->drop->stack;
						float penya2min = (float)drops.at(i)->drop->stack;

						penya1min = (penya1min - (penya1min / 4));
						penya2min = (penya2min + (penya2min / 4));
						
						drops.at(i)->drop->stack = Random((int)penya1min, (int)penya2min);
						drops.at(i)->drop->stack *= penyaRate;

						if(this->p->pt != NULL)
						{
							int inPt = 0;

							for(unsigned int e = 0; e < this->p->pt->members.size(); e++)
							{
								if(this->p->pt->members.at(e)->mb->p->mapId->id == this->p->mapId->id && !this->p->pt->members.at(e)->mb->p->dead && this->p->pt->members.at(e)->mb->p->getId() != this->p->getId())
									inPt++;
							}

							if(inPt > 0)
								drops.at(i)->drop->stack = (drops.at(i)->drop->stack / (inPt + 1));
						}

						if(this->AddItem(drops.at(i)->drop->id, drops.at(i)->drop->stack, true, drops.at(i), slotAddedTo))
						{
							if(this->p->pt != NULL)
							{
								for(unsigned int e = 0; e < this->p->pt->members.size(); e++)
								{
									if(this->p->pt->members.at(e)->mb->p->mapId->id == this->p->mapId->id && this->p->pt->members.at(e)->mb->p->getId() != this->p->getId() && !this->p->pt->members.at(e)->mb->p->dead)
										this->p->pt->members.at(e)->mb->AddItem(drops.at(i)->drop->id, drops.at(i)->drop->stack, true, drops.at(i), slotAddedTo);
								}
							}

							Packet pak = Packet();

							pak.addHeader(0x39);
							pak.addInt(2);
							pak.addLongInt(drops.at(i)->id);

							pak.ready();

							SendAllOnMap(pak, 15, drops.at(i)->map, 0);

							delete drops.at(i)->drop;
							delete drops.at(i);

							drops.erase(drops.begin() + i);

							status = true;
						}

						else
						{
							drops.at(i)->drop->stack = lastStack;
						}
					}
				}

				else
				{
					if(this->AddItem(drops.at(i)->drop->id, drops.at(i)->drop->stack, true, drops.at(i), slotAddedTo))
					{
						Packet pak = Packet();

						pak.addHeader(0x39);
						pak.addInt(2);
						pak.addLongInt(drops.at(i)->id);

						pak.ready();

						SendAllOnMap(pak, 15, drops.at(i)->map, 0);

						delete drops.at(i)->drop;
						delete drops.at(i);

						drops.erase(drops.begin() + i);

						status = true;
					}

					else
					{
						if((unsigned)this->p->nextItemPick <= getTimestamp())
						{
							SystemChat(this, CUSTOM, NULL, "Your inventory is full!");

							this->p->nextItemPick = (getTimestamp() + 3000);
						}
					}
				}
			}

			else
			{
				if((unsigned)this->p->nextItemPick <= getTimestamp())
				{
					SystemChat(this, CUSTOM, NULL, "That item belongs to another player.");

					if(debugs)
						log(DEBUG, "Drop owner: %d my ID: %d.\n", drops.at(i)->owner, this->p->getId());

					this->p->nextItemPick = (getTimestamp() + 3000);
				}
			}

			break;
		}
	}

	return status;
}

void Monster::Drop()
{
	if(this->drops.size() > 0)
	{
		for(unsigned int i = 0; i < this->drops.size(); i++)
		{
			float chances = (this->drops.at(i).chance * (float)dropRate);

			if(chances > 100)
				chances = 100.0f;

			float chanceDrop = (100.f / chances);

			int dropornot = Random(1, (int)(chanceDrop + 0.5f));

			if(dropornot == 1)
			{
				Item *stru = GetItemById(this->drops.at(i).itemid);

				if(stru == NULL)
					log(ERR, "[void Monster::Drop()] [Item is NULL] [%d]\n", this->drops.at(i).itemid);

				else
				{
					int dropx = Random(((int)this->x - 30), ((int)this->x + 30));
					int dropy = Random(((int)this->y - 30), ((int)this->y + 30));

					if(dropx < 0)
						dropx = 0;

					if((dropx + 28) > this->mapId->xSize)
						dropx = (this->mapId->xSize - 28);

					if(dropy < 0)
						dropy = 0;

					if((dropy + 28) > this->mapId->ySize)
						dropy = (this->mapId->ySize - 28);

					Item *item = new Item();

					*item = *stru;

					item->stack = this->drops.at(i).qty;
					item->id = this->drops.at(i).itemid;

					DropItem(item, this->attackerId, this->mapId->id, dropx, dropy, false);
				}
			}
		}
	}
}

std::vector<Drop*> DropPacket(Monster *th)
{
	std::vector<Drop*> retType;

	if(th->drops.size() > 0)
	{
		for(unsigned int i = 0; i < th->drops.size(); i++)
		{
			float chances = (th->drops.at(i).chance  * (float)dropRate);

			if(chances > 100)
				chances = 100.0f;

			float chanceDrop = (100.f / chances);

			int dropornot = Random(1, (int)(chanceDrop + 0.5f));

			if(dropornot == 1)
			{
				Item *stru = GetItemById(th->drops.at(i).itemid);

				if(stru == NULL)
					log(ERR, "[vector<Drop*> DropPacket(Monster*)] [Item is NULL] [%d]\n", th->drops.at(i).itemid);

				else
				{
					int dropx = Random(((int)th->x - 30), ((int)th->x + 30));
					int dropy = Random(((int)th->y - 30), ((int)th->y + 30));

					if(dropx < 0)
						dropx = 0;

					if((dropx + 28) > th->mapId->xSize)
						dropx = (th->mapId->xSize - 28);

					if(dropy < 0)
						dropy = 0;

					if((dropy + 28) > th->mapId->ySize)
						dropy = (th->mapId->ySize - 28);

					Item *item = new Item();

					*item = *stru;

					item->stack = th->drops.at(i).qty;
					item->id = th->drops.at(i).itemid;

					Drop *drop = new Drop();

					drop->drop = item;
					drop->owner = th->attackerId;
					drop->map = th->mapId->id;
					drop->x = dropx;
					drop->y = dropy;
					drop->autodelete = (getTimestamp() + deleteDropDelay);
					drop->loottimer = (getTimestamp() + 60000);
					drop->lootEnable = false;

					drop->id = dropCId;
					dropCId++;

					drops.push_back(drop);
					retType.push_back(drop);
				}
			}
		}
	}

	return retType;
}

void DropItem(Item *item, int owner, int map, int x, int y, bool noChangePenya)
{
	Drop *drop = new Drop();

	drop->drop = item;
	drop->owner = owner;
	drop->map = map;
	drop->x = x;
	drop->y = y;
	drop->autodelete = (getTimestamp() + deleteDropDelay);
	drop->loottimer = (getTimestamp() + 60000);
	drop->lootEnable = false;
	drop->noChangePenya = noChangePenya;

	drop->id = dropCId;
	dropCId++;

	drops.push_back(drop);

	Packet pak = Packet();

	pak.addHeader(0x39);
	pak.addInt(1);
	pak.addLongInt(1);
	pak.addInt(item->id);
	pak.addInt(item->value);
	pak.addLongInt(drop->id);
	pak.addLongInt(x);
	pak.addLongInt(y);

	pak.ready();
	SendAllOnMap(pak, 0, drop->map, Connection(0));
}

void Client::RemoveTaskbarItem(int id)
{
	this->p->task[(id - 1)]->enable = false;

	Packet pak = Packet();

	pak.addHeader(0x48);
	pak.addLongInt(id);

	pak.ready();
	this->AddPacket(pak, 0);

	if(debugs)
		log(DEBUG, "Removed taskbar item: %d.\n", id);
}

void Client::AddTaskbarItem(char *b)
{
	int pos = 3;

	int id = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
	pos += ((int)b[pos] + 1);

	int type = b[pos];
	int slot = b[(pos + 1)];

	this->p->task[(id - 1)]->type = type;
	this->p->task[(id - 1)]->slot = slot;
	this->p->task[(id - 1)]->enable = true;
	this->p->task[(id - 1)]->eqinv = false;

	if(type == INVENTORYITEM)
	{
		if(this->p->inv[(slot - 1)] != NULL)
			this->p->task[(id - 1)]->id = this->p->inv[(slot - 1)]->id;
	}

	Packet pak = Packet();

	pak.addHeader(0x49);
	pak.addLongInt(id);
	pak.addLongInt(type);
	pak.addLongInt(slot);

	pak.ready();
	this->AddPacket(pak, 0);

	if(debugs)
		log(DEBUG, "Added taskbar item: %d %d %d.\n", id, type, slot);
}

void Client::UseTaskbarItem(int puh)
{
	if(puh <= 0 || puh > 9)
		return;

	if(this->p->task[(puh - 1)]->enable)
	{
		switch(this->p->task[(puh - 1)]->type)
		{
			case INVENTORYITEM:
				{
					if(!this->p->task[(puh - 1)]->eqinv)
					{
						if(this->UseItem(this->p->task[(puh - 1)]->slot))
							this->p->task[(puh - 1)]->eqinv = true;
					}

					else
					{
						if(this->UnEquip(this->p->task[(puh - 1)]->slot))
							this->p->task[(puh - 1)]->eqinv = false;
					}
				break;
				}

			case SKILL: this->UseSkill(this->p->task[(puh - 1)]->slot);
				break;

			default: log(ERR, "[void Client::UseTaskbarItem(int)] [Unknown usage param] [%d]\n", this->p->task[(puh - 1)]->type);
				break;
		}
	}
}

void Client::DropInventoryItem(char *b)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not drop an item while trading.");

		return;
	}

	int pos = 4;

	int slot = b[3];

	int qty = atoi(getString(b, 5, (int)b[4]).c_str());
	pos += ((int)b[4] + 1);

	int itemid = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

	if(itemid == 2)
	{
		if(qty > this->p->gold || qty <= 0)
			return;

		this->RemoveItem(0, qty, false, 2);

		Item *it = new Item();

		it->id = 2;
		it->stack = qty;

		int xx = (int)(this->p->x - 40);
		int yy = (int)(this->p->y + (this->p->meH / 2) - 14);

		if(xx < 0)
			xx = (int)((this->p->x + 12) + this->p->meW);

		DropItem(it, 0, this->p->mapId->id, xx, yy, true);

		return;
	}

	if(this->p->inv[(slot - 1)] == NULL)
		log(ERR, "[void Client::DropInventoryItem(char*)] [Item to drop does not exist] [%d]\n", slot);
	else
	{
		if(qty > this->p->inv[(slot - 1)]->stack || qty <= 0)
			return;

		Item *it = new Item();

		*it = *this->p->inv[(slot - 1)];
		it->stack = qty;

		this->RemoveItem(slot, qty, false, it->id);

		int xx = (int)(this->p->x - 40);
		int yy = (int)(this->p->y + (this->p->meH / 2) - 14);

		if(xx < 0)
			xx = (int)((this->p->x + 12) + this->p->meW);

		if(debugs)
			log(DEBUG, "Dropping item: %d %d.\n", it->id, it->stack);

		LD(GAME, "Dropped item: [Name: %s] [ID: %d] [Stack: %d].\n", this->p->name, it->id, it->stack);

		DropItem(it, 0, this->p->mapId->id, xx, yy, true);
	}
}

bool Client::UnEquip(int slot)
{
	if(this->p->trading)
	{
		SystemChat(this, CUSTOM, NULL, "You can not modify your inventory while trading.");

		return false;
	}

	bool returnValue = false;
	int slotAddedTo = 0;

	if(this->p->eq[(slot - 1)] == NULL)
		log(ERR, "[bool Client::UnEquip(int)] [Equipment is NULL] [%d]\n", slot);
	else
	{
		if(this->AddItem(this->p->eq[(slot - 1)]->id, 1, false, NULL, slotAddedTo))
		{
			delete this->p->eq[(slot - 1)];
			this->p->eq[(slot - 1)] = NULL;

			Packet pak = Packet();

			pak.addHeader(0x58);
			pak.addInt(2);
			pak.addInt(slot);

			pak.ready();
			this->AddPacket(pak, 0);

			Packet pak2 = Packet();

			pak2.addHeader(0x58);
			pak2.addInt(4);
			pak2.addInt(slot);
			pak2.addLongInt(this->p->getId());

			pak2.ready();
			SendAllOnMap(pak2, 0, this->p->mapId->id, getConnection());

			this->p->hp = CalculateHP(this);

			if(this->p->chp > this->p->hp)
				this->p->chp = this->p->hp;

			this->p->maxmana = CalculateMana(this);

			if(this->p->mana > this->p->maxmana)
				this->p->mana = this->p->maxmana;

			this->com->sendUpdate(HP);
			this->com->sendUpdate(HPPROCENT);
			this->com->sendUpdate(STAT);
			this->com->sendUpdate(MANA);

			for(int i = 0; i < 9; i++)
			{
				if(this->p->task[i]->type == INVENTORYITEM && this->p->task[i]->enable && this->p->task[i]->slot == slot && this->p->task[i]->eqinv)
				{
					Packet pak99 = Packet();

					pak99.addHeader(0x63);
					pak99.addInt(2);
					pak99.addInt((i + 1));
					pak99.addInt(this->p->task[i]->slot);
					pak99.addInt(slotAddedTo);

					pak99.ready();
					this->AddPacket(pak99, 0);

					this->p->task[i]->eqinv = false;
					this->p->task[i]->slot = slotAddedTo;
				}
			}

			if(slot == 1)
				this->EquipWeapon(false);

			returnValue = true;
		}

		else
			SystemChat(this, CUSTOM, NULL, "No inventory space left.");
	}

	return returnValue;
}