#include <fstream>

#include "Main.h"
#include "Cluster.h"
#include "Handle.h"
#include "Item.h"
#include "BufferCheck.h"

using namespace std;

extern vector<Player*> pvector;
extern vector<NPC*> npcs;

extern bool debugs;

int playPuhId = 1;

void Client::CharacterList()
{
	if(this->online)
	{
		std::vector<Player*> te = this->GetCharacterVector();

		Packet pak = Packet();

		pak.addHeader(0x6);

		if(te.size() > 0)
		{
			pak.addInt(te.size());
			
			for(unsigned int i = 0; i < te.size(); i++)
			{
				pak.addLongInt(te.at(i)->level);
				pak.addLongInt(te.at(i)->power);
				pak.addLongInt(te.at(i)->magic);
				pak.addLongInt(te.at(i)->agility);
				pak.addLongInt(te.at(i)->intt);
				pak.addInt(te.at(i)->job);

				pak.addString(te.at(i)->name);

				pak.addInt(2);
				pak.addInt(te.at(i)->slot);
			}
		}

		else
			pak.addInt(0);

		pak.ready();
		this->AddPacket(pak, 0);

		if(debugs)
			log(DEBUG, "Character list: %s %d.\n", this->username.c_str(), te.size());

		LD(LOGIN, "Requesting character list: [%s] [%s] [%d].\n", this->username.c_str(), this->password.c_str(), te.size());
	}
}

bool Client::SlotFree(int slot)
{
	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(this->username.c_str(), pvector.at(i)->owner.c_str()) == 0)
		{
			if(pvector.at(i)->slot == slot)
				return false;
		}
	}

	return true;
}

int Client::GetFirstFreeCharacterSlot()
{
	int i1 = 0;
	int i2 = 0;
	int i3 = 0;

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(this->username.c_str(), pvector.at(i)->owner.c_str()) == 0)
		{
			switch(pvector.at(i)->slot)
			{
				case 1: i1 = 1;
					break;

				case 2: i2 = 1;
					break;
				
				case 3: i3 = 1;
					break;
			}
		}
	}

	if(i1 == 0)
		return 1;
	else if(i2 == 0)
		return 2;
	else if(i3 == 0)
		return 3;

	return 4;
}

std::vector<Player*> Client::GetCharacterVector()
{
	vector<Player*> cp;

	Player *p1 = new Player();
	Player *p2 = new Player();
	Player *p3 = new Player();

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(this->username.c_str(), pvector.at(i)->owner.c_str()) == 0)
		{
			switch(pvector.at(i)->slot)
			{
				case 1: p1 = pvector.at(i);
					break;

				case 2: p2 = pvector.at(i);
					break;

				case 3: p3 = pvector.at(i);
					break;
			}
		}
	}

	if(strcmp(p1->name.c_str(), "") != 0)
		cp.push_back(p1);
	else
		delete p1;

	if(strcmp(p2->name.c_str(), "") != 0)
		cp.push_back(p2);
	else
		delete p2;

	if(strcmp(p3->name.c_str(), "") != 0)
		cp.push_back(p3);
	else
		delete p3;

	return cp;
}

Player *Client::GetThisPlayer(int id)
{
	Player *ret = NULL;

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(this->username.c_str(), pvector.at(i)->owner.c_str()) == 0 && pvector.at(i)->slot == id)
		{
			ret = pvector.at(i);

			break;
		}
	}

	return ret;
}

void Client::EditCharacter(int b, char *paks)
{
	if(this->online)
	{
		if(b == 1)
		{
			int pos = 4;
			int res = 3;

			string cname = getString(paks, 5, (int)paks[4]);
			pos += ((int)paks[4] + 1);

			int slots = paks[pos];
			pos++;

			int job = paks[pos];
			pos++;

			Class *gc = GetClass(job);

			bool doClass = true;

			if(gc == NULL)
				doClass = false;

			else
			{
				if(gc->disabled)
					doClass = false;
			}

			if(doClass)
			{
				if(this->GetCharacterVector().size() < 3)
				{
					if(slots > 0 && slots <= 3)
					{
						if(CreateOK(cname))
						{
							if(AllowLetters(cname))
							{
								if(cname.length() <= 16)
								{
									if(cname.length() >= 3)
									{
										ofstream r;

										r.open("Data/Character/CharacterIndex.txt", fstream::app);

										if(!r)
										{
											log(ERR, "[void Client::EditCharacter(int,char*)] [Can not open file] [%s]\n", "Data/Character/CharacterIndex.txt");

											r.close();
										}

										else
										{
											int kslot = slots;

											if(!this->SlotFree(slots))
												kslot = this->GetFirstFreeCharacterSlot();

											r << this->username.c_str() << " " << kslot << endl;
											r.close();

											Player *play = new Player();

											play->power = 2;
											play->x = (float)mPlayerStartX;
											play->y = (float)mPlayerStartY;
											play->exp = 0;
											play->magic = 2;
											play->powerpoints = 0;
											play->pkpoints = 0;
											play->access = 50;
											play->mapId = GetMap(mPlayerStartMap);
											play->level = 1;
											play->slot = kslot;
											play->agility = 2;
											play->job = job;

											play->owner = this->username;
											play->name = cname;	
											//strcpy(play->owner, this->username.c_str());
											//strcpy(play->name, cname.c_str());

											play->setId(playPuhId);
											playPuhId += 1;

											if(play->x > play->mapId->xSize || play->x < 0)
												play->x = (float)(play->mapId->xSize / 2);

											if(play->y > play->mapId->ySize || play->y < 0)
												play->y = (float)(play->mapId->ySize / 2);

											for(unsigned int i = 0; i < gc->items.size(); i++)
											{
												Item *stru = GetItemById(gc->items.at(i).id);

												if(stru != NULL)
												{
													Item *item = new Item();

													*item = *stru;

													item->stack = gc->items.at(i).stack;
													item->id = gc->items.at(i).id;

													if(gc->items.at(i).eq)
														play->eq[gc->items.at(i).slot - 1] = item;
													
													else
														play->inv[gc->items.at(i).slot - 1] = item;
												}

												else
													log(ERR, "[void Client::EditCharacter(int,char*)] [Could not find item] [%d]\n", 3);
											}

											CheckSkills(play);
											CheckClassSkills(play);

											pvector.push_back(play);

											//char slots[10];
											//_itoa_s(play->slot, slots, 10);

											string charp = "Data/Character/";
											charp += play->owner;
											charp += to_string(play->slot);
											charp += ".txt";

											ofstream o(charp.c_str());
											o << play->name << endl << play->level << endl << play->power << endl << play->agility << endl << play->x << endl << play->y << endl << play->mapId->id << endl << play->exp << endl << play->magic << endl << play->powerpoints << endl << play->pkpoints << endl << play->access << endl;
											/*<< 1 << endl << "2 0 0" << endl << 0 << endl << 0 << endl << 3 << endl << "1 91 1 0" << endl << "12 92 1 0" << endl << "13 92 1 0" << endl;*//* << 1 << endl;*/
											
											int num = 1;

											for(int i = 0; i < 25; i++)
											{
												if(play->inv[i] != NULL)
													num++;
											}

											o << num << endl;
											o << "2 0 0" << endl;

											for(int i = 0; i < 25; i++)
											{
												if(play->inv[i] != NULL)
													o << play->inv[i]->id << " " << (i + 1) << " " << play->inv[i]->stack << endl;
											}

											o << 0 << endl << 0 << endl << 0 << endl;

											num = 0;

											for(int i = 0; i < 4; i++)
											{
												if(play->eq[i] != NULL)
													num++;
											}
											
											o << num << endl;

											for(int i = 0; i < 4; i++)
											{
												if(play->eq[i] != NULL)
													o << play->eq[i]->id << " " << (i + 1) << endl;
											}

											o << "2" << endl << "0" << endl << play->job << endl;

											/*"3 1" << endl << "2" << endl << "0" << endl;*/
											o.close();

											res = 1;

											LD(LOGIN, "Create character: [%s] [%s] [Name: %s].\n", this->username.c_str(), this->password.c_str(), play->name);
										}
									}

									else
										res = 6;
								}
								
								else
									res = 5;
							}

							else
								res = 4;
						}

						else
							res = 2;
					}
				}
			}

			else
				res = 7;

			switch(res)
			{
				case 1: PlayerLog("[Create character] [%s] [Accepted] [%d]\n", cname.c_str(), slots);
					break;

				case 2: PlayerLog("[Create character] [%s] [Name already taken]\n", cname.c_str());
					break;

				case 3: PlayerLog("[Create character] [%s] [Already got 3 characters]\n", cname.c_str());
					break;

				case 4: PlayerLog("[Create character] [%s] [Not allowed letters]\n", cname.c_str());
					break;

				case 5: PlayerLog("[Create character] [%s] [Name too long]\n", cname.c_str());
					break;

				case 6: PlayerLog("[Create character] [%s] [Name too short]\n", cname.c_str());
					break;

				case 7: PlayerLog("[Create character] [%s] [Class disabled] [%d]\n", cname.c_str(), job);
			}

			Packet pak = Packet();

			pak.addHeader(0x7);
			pak.addInt(res);

			pak.ready();
			this->AddPacket(pak, 0);
		}

		else if(b == 2)
		{
			int res = 2;
			int pos = 4;

			string dname = getString(paks, (pos + 1), (int)paks[pos]);
			pos += ((int)paks[pos] + 1);

			string passn = getString(paks, (pos + 1), (int)paks[pos]);

			if(this->GetCharacterVector().size() > 0)
			{
				if(strcmp(passn.c_str(), this->password.c_str()) == 0)
				{
					if(!CreateOK(dname))
					{
						string slots = to_string(DeleteSlot((char*)dname.c_str()));
						//char slots[10];
						//_itoa_s(DeleteSlot((char*)dname.c_str()), slots, 10, 10);

						for(unsigned int i = 0; i < pvector.size(); i++)
						{
							if(strcmp(pvector.at(i)->name.c_str(), dname.c_str()) == 0)
							{
								for(unsigned int y = 0; y < pvector.size(); y++)
								{
									if(pvector.at(y)->getId() == pvector.at(i)->getId())
										continue;

									for(unsigned int z = 0; z < pvector.at(y)->friends.size(); z++)
									{
										if(strcmp(pvector.at(y)->friends.at(z).name.c_str(), pvector.at(i)->name.c_str()) == 0)
										{
											Client *plas = GetClientByPlayerName(pvector.at(y)->name.c_str());

											if(plas == NULL)
												pvector.at(y)->friends.erase(pvector.at(y)->friends.begin() + z);

											else
											{
												plas->p->friends.erase(plas->p->friends.begin() + z);

												Packet pak2 = Packet();

												pak2.addHeader(0x0D);
												pak2.addInt(2);
												pak2.addString(pvector.at(i)->name);

												pak2.ready();
												plas->AddPacket(pak2, 0);
											}

											break;
										}
									}
								}

								delete pvector.at(i);
								pvector.erase((pvector.begin() + i));

								break;
							}
						}

						string ds = "Data/Character/CharacterIndex.txt";

						ofstream o(ds.c_str());
						if(!o)
							log(ERR, "[void Client::EditCharacter(int,char*)] [Unable to open file] [%s]\n", ds.c_str());
						else
						{
							for(unsigned int i = 0; i < pvector.size(); i++)
								o << pvector.at(i)->owner << " " << pvector.at(i)->slot << endl;
						}

						o.close();

						string charp = "Data/Character/";
						charp += this->username.c_str();
						charp += slots;
						charp += ".txt";

						remove(charp.c_str());

						res = 1;

						LD(LOGIN, "Remove character: [%s] [%s] [Name: %s].\n", this->username.c_str(), this->password.c_str(), dname.c_str());
					}

					else
						res = 3;
				}

				else
					res = 4;
			}

			switch(res)
			{
				case 1: PlayerLog("[Delete character] [%s] [Accepted]\n", dname.c_str());
					break;

				case 2: PlayerLog("[Delete character] [%s] [Access violation]\n", dname.c_str());
					break;

				case 3: PlayerLog("[Delete character] [%s] [Character does not exist]\n", dname.c_str());
					break;

				case 4: PlayerLog("[Delete character] [%s] [Password is wrong] [%s]\n", dname.c_str(), passn.c_str());
					break;
			}

			Packet pak = Packet();

			pak.addHeader(0x7);
			pak.addInt(res);

			pak.ready();
			this->AddPacket(pak, 0);
		}
	}
}

bool AllowLetters(std::string name)
{
	std::string al = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	unsigned int len = al.length();

	bool found = false;

	for(unsigned int i = 0; i < name.length(); i++)
	{
		found = false;

		for(unsigned int u = 0; u < len; u++)
		{
			if(name[i] == al[u])
			{
				found = true;

				break;
			}
		}

		if(!found)
			return false;
	}

	return true;
}

bool CreateOK(std::string name)
{
	bool val = true;

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(name.c_str(), pvector.at(i)->name.c_str()) == 0)
		{
			val = false;
			break;
		}
	}

	if(val)
	{
		for(unsigned int i = 0; i < npcs.size(); i++)
		{
			if(strcmp(name.c_str(), npcs.at(i)->name.c_str()) == 0)
			{
				val = false;
				break;
			}
		}
	}

	if(val)
	{
		if(strcmp("System", name.c_str()) == 0)
			val = false;

		else if(strcmp("Notice", name.c_str()) == 0)
			val = false;
	}

	return val;
}

int DeleteSlot(char *name)
{
	int slot = 0;

	for(unsigned int i = 0; i < pvector.size(); i++)
	{
		if(strcmp(name, pvector.at(i)->name.c_str()) == 0)
		{
			slot = pvector.at(i)->slot;
			break;
		}
	}

	return slot;
}