#include "Main.h"
#include "Handle.h"
#include "Command.h"
#include "Item.h"
#include "RenderWorld.h"
#include "BufferCheck.h"

using namespace std;

extern std::vector<Client*> ac;
extern bool enableClientText;
extern long refreshDelay;
extern WorldCommand *wc;
extern bool enableAttack;
extern std::vector<Monster*> mobs;
extern long monsterAttackDelay;
extern long saveDelay;
extern long moveDelay;
extern std::vector<Party*> parties;
extern bool debugs;
extern std::vector<NPC*> npcs;
extern long npcChatDelay;
extern std::vector<Drop*> drops;
extern bool enableStatusCheck;
extern long nextStatusCheck;
extern long statusDelay;
extern Client *currentClient;
extern long playerAttackTimer;
extern bool disconnectWithOnlinePacket;
extern int maxLvl;

int randomInt = 0;

void RenderWorld()
{
	for(unsigned int i = 0; i < mobs.size(); i++)
	{
		Monster *tm = mobs.at(i);

		if(!tm->attacked && !tm->follows && !tm->goAfter && tm->spawned)
		{
			if((unsigned)tm->nextRefresh <= getTimestamp())
			{
				if(tm->hp > tm->chp)
				{
					tm->chp += (tm->hp / 10);

					if(tm->chp > tm->hp)
						tm->chp = tm->hp;

					wc->sendUpdateMProcent(tm);
				}

				tm->nextRefresh = (getTimestamp() + refreshDelay);
			}

			if(tm->moving && !tm->noMove)
			{
				bool trues = false;

				switch(tm->goDir)
				{
					case UP: if((tm->moveStartY - tm->y) >= 250) { trues = true; tm->y = (tm->moveStartY - 250); }
						break;

					case DOWN: if((tm->y - tm->moveStartY) >= 250) { trues = true; tm->y = (tm->moveStartY + 250); }
						break;

					case LEFT: if((tm->moveStartX - tm->x) >= 250) { trues = true; tm->x = (tm->moveStartX - 250); }
						break;

					case RIGHT: if((tm->x - tm->moveStartX) >= 250) { trues = true; tm->x = (tm->moveStartX + 250); }
						break;
				}

				if(trues)
				{
					Packet pak = Packet();

					pak.addHeader(0x9);
					pak.addInt(2);
					pak.addLongInt(tm->pid);
					pak.addLongInt((int)tm->x);
					pak.addLongInt((int)tm->y);

					pak.ready();
					SendAllOnMap(pak, 0, tm->mapId->id, 0);

					tm->moveDone = true;
					tm->moving = false;
					tm->goDir = 0;

					tm->moveStartX = 0;
					tm->moveStartY = 0;

					int r = Random((moveDelay - (moveDelay / 3)), (moveDelay + (moveDelay / 3)));

					tm->nextMove = (getTimestamp() + r);
				}
			}

			if((unsigned)tm->nextMove <= getTimestamp() && !tm->moving && !tm->noMove && tm->moveDone)
			{
				bool bloop = true;

				while(bloop)
				{
					tm->goDir = Random(LEFT, DOWN);

					switch(tm->goDir)
					{
						case UP:
							{
								if(tm->y >= (tm->minY + ((tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20))) + 16)))
									bloop = false;
							break;
							}

						case DOWN:
							{
								if(tm->y <= (tm->maxY - (tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20)))))
									bloop = false;
							break;
							}

						case LEFT:
							{
								if(tm->x >= (tm->minX + (tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20)))))
									bloop = false;
							break;
							}

						case RIGHT:
							{
								if(tm->x <= (tm->maxX - (tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20)))))
									bloop = false;
							break;
							}
					}
				}

				tm->move.start(true);

				Packet pak = Packet();

				pak.addHeader(0x8);
				pak.addInt(tm->goDir);
				pak.addInt(2);
				pak.addLongInt(tm->pid);
				pak.addLongInt((int)tm->x);
				pak.addLongInt((int)tm->y);

				pak.ready();
				SendAllOnMap(pak, 0, tm->mapId->id, 0);

				tm->moving = true;
				tm->moveDone = false;
				tm->nextMove = getTimestamp();

				tm->moveStartX = tm->x;
				tm->moveStartY = tm->y;
			}

			if(tm->followId != 0)
				tm->followId = 0;

			if(tm->moving)
			{
				float xTemp = ((tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20))) * (tm->move.get_ticks(true) / 1000.f));

				switch(tm->goDir)
				{
					case UP: tm->y -= xTemp;
						break;

					case DOWN: tm->y += xTemp;
						break;

					case LEFT: tm->x -= xTemp;
						break;

					case RIGHT: tm->x += xTemp;
						break;
				}

				tm->move.start(true);
			}

			bool validArea2 = tm->ValidArea();

			if(!validArea2)
			{
				tm->Smite();
				continue;
			}
		}

		if(!tm->spawned && (unsigned)tm->respawnTime <= getTimestamp())
		{
			int r = Random((moveDelay / 2), (moveDelay * 2));

			int dropornot = 0;

			if(tm->aggChance > 0)
				dropornot = Random(1, (int)((float)(100 / tm->aggChance)));
			else
				dropornot = 2;

			tm->spawned = true;
			tm->attacked = false;
			tm->moving = false;
			tm->follows = false;
			tm->goAfter = false;
			tm->moveDone = true;

			tm->attackerId = 0;
			tm->followId = 0;

			tm->chp = tm->hp;

			tm->nextMove = (getTimestamp() + r);

			while(true)
			{
				tm->x = (float)Random(tm->minX, tm->maxX);
				tm->y = (float)Random(tm->minY, tm->maxY);

				if(tm->ValidArea())
					break;
			}

			tm->moveStartX = 0;
			tm->moveStartY = 0;

			if(dropornot == 1)
				tm->aggro = true;
			else
				tm->aggro = false;

			Packet pakm = Packet();

			pakm.addHeader(0x5);
			pakm.addInt(3);
			pakm.addLongInt(1);

			pakm.addInt(tm->level);
			pakm.addInt(tm->attack);
			pakm.addInt(tm->id);
			pakm.addInt(0);
			pakm.addInt(0);
			pakm.addLongInt((int)tm->x);
			pakm.addLongInt((int)tm->y);

			pakm.addLongInt(tm->pid);
			pakm.addLongInt(0);
			pakm.addBool(false);
			pakm.addLongInt(0);
			pakm.addLongInt(0);
			pakm.addBool(tm->aggro);
			pakm.addLongInt((tm->speed + ((tm->GetMonsterBonus(SPEED) * 20) - (tm->GetMonsterBonus(SPEEDMINUS) * 20))));
			pakm.addBool(tm->moving);
			pakm.addInt(tm->goDir);
			pakm.addInt(GetMonsterProcentHP(tm));

			pakm.ready();
			SendAllOnMap(pakm, 0, tm->mapId->id, 0);
		}

		if(tm->spawned)
		{
			if(tm->attacked && (unsigned)tm->freeAttack <= getTimestamp())
			{
				tm->attacked = false;
				tm->moving = false;
				tm->moveDone = true;

				tm->attackerId = 0;

				int r = Random((moveDelay / 2), (moveDelay * 2));

				tm->nextMove = (getTimestamp() + r);
			}

			if(tm->follows)
			{
				if(!tm->WalkBack() && tm->followId != 0)
					tm->Follow();

				else
				{
					tm->followId = 0;
					tm->attackerId = 0;
					tm->follows = false;
					tm->attacked = false;
					tm->goAfter = true;
					tm->moveDone = true;

					tm->move.start(true);

					Packet pf = Packet();
					pf.addHeader(0x44);
					pf.addLongInt(1);
					pf.addLongInt(tm->pid);

					pf.ready();
					SendAllOnMap(pf, 0, tm->mapId->id, 0);
				}
			}

			if(tm->goAfter)
			{
				if(tm->GoBack())
				{
					tm->goAfter = false;
					tm->moveDone = true;

					int r = Random((moveDelay / 2), (moveDelay * 2));

					tm->nextMove = (getTimestamp() + r);
				}
			}

			bool validArea = tm->ValidArea();

			if(!validArea)
			{
				tm->Smite();
				continue;
			}
		}

		if(tm->mbuffs.size() > 0)
		{
			for(unsigned int y = 0; y < tm->mbuffs.size(); y++)
			{
				if(tm->mbuffs.at(y)->changeEffect == HPDRAIN)
				{
					if((unsigned)tm->mbuffs.at(y)->nextUse <= getTimestamp())
					{
						float mobDamage = (float)tm->mbuffs.at(y)->value * 10;

						if((tm->chp - mobDamage) <= 0)
						{
							mobDamage = tm->chp;

							Client *c2 = GetClientByPlayerId(tm->mbuffs.at(y)->userPut);

							wc->sendDamage(NULL, tm, 2, (int)mobDamage);
							wc->killMonster(c2, tm, false);

							break;
						}

						else
						{
							tm->chp -= mobDamage;

							wc->sendDamage(NULL, tm, 2, (int)mobDamage);
							wc->sendUpdateMProcent(tm);
						}

						tm->mbuffs.at(y)->nextUse = (getTimestamp() + 2000);
					}
				}

				if((unsigned)tm->mbuffs.at(y)->endTime <= getTimestamp())
				{
					tm->RemoveMonsterBuff(y);

					break;
				}
			}
		}
	}

	for(unsigned int i = 0; i < ac.size(); i++)
	{
		Client *c = ac.at(i);

		if(ac.at(i)->disconnect)
		{
			if(strcmp(c->GetIP(), "127.0.0.1") != 0 && enableClientText)
				log(INFO, "Client disconnected [IP: %s] [SOCKET: %d].\n", c->GetIP(), c->getConnection().getSocket());

			currentClient = c;

			if(strcmp(c->GetIP(), "127.0.0.1") != 0)
				PlayerLog("[Client disconnected] [%d]\n", c->getConnection().getSocket());

			currentClient = NULL;

			if(c->online)
			{
				c->Logout();

				if(c->ingame)
				{
					Player *seeme = c->p;

					if(c->p->pt != NULL)
					{
						for(unsigned int r = 0; r < c->p->pt->members.size(); r++)
						{
							if(c->p->pt->members.at(r)->mb->p->getId() == c->p->getId())
							{
								c->p->pt->members.at(r)->online = false;

								break;
							}
						}

						c->p->pt = NULL;
					}

					c->UpdateFriends(false);

					c->RemoveAllFollowers();

					c->ingame = false;

					c->Unspawn(0);
					c->SavePlayer(QUITSAVE);

					if(seeme->mapId != NULL)
					{
						if(seeme->access <= 50)
						{
							std::string welcomeNotice2 = seeme->name;
							welcomeNotice2 += " logged out.";

							for(unsigned int i = 0; i < ac.size(); i++)
							{
								if(ac.at(i)->ingame)
									SystemChat(ac.at(i), CUSTOM, NULL, (char*)welcomeNotice2.c_str());
							}
						}
					}
				}
			}

			if(!c->donecsp)
				delete[] c->csp.bu;

			for(unsigned int t = 0; t < c->sps.size(); t++)
				delete[] c->sps.at(t).bu;

			delete ac.at(i);
			ac.erase(ac.begin() + i);

/*
			char buff[10] = "";
			std::string programString = "Kobla - ";

			_itoa_s(ac.size(), buff, 10, 10);

			programString += buff;

			SetConsoleTitleA(programString.c_str());
*/
			continue;
		}

		if(!c->isc)
		{
			if((unsigned)c->nextOnlineCheck <= getTimestamp())
			{
				if(c->onlineCheck)
				{
					Packet pak = Packet();

					pak.addHeader(0x55);

					pak.ready();
					c->AddPacket(pak, 0);

					c->onlineCheck = false;
					c->nextOnlineCheck = (getTimestamp() + onlineCheckDelay);
					c->onlineTries = 0;
				}

				else if(!c->onlineCheck && c->onlineTries >= 3)
				{
					if(!c->disconnect && disconnectWithOnlinePacket)
						c->disconnect = true;
				}

				else
				{
					Packet pak = Packet();

					pak.addHeader(0x55);

					pak.ready();
					c->AddPacket(pak, 0);

					c->onlineCheck = false;
					c->nextOnlineCheck = (getTimestamp() + onlineCheckDelay);
					c->onlineTries++;
				}
			}
		}

		if(c->ingame && !c->disconnect)
		{
			if(!c->p->dead)
			{
				if((unsigned)c->p->atkTimer <= getTimestamp() && c->p->attacked)
					c->p->attacked = false;

				if((unsigned)c->p->nextRefresh <= getTimestamp())
				{
					if(!c->p->attacked)
					{
						if((int)c->p->hp > (int)c->p->chp)
							c->AddHP((c->p->hp / 20.0f));

						if((int)c->p->maxmana > (int)c->p->mana)
							c->AddMana((c->p->maxmana / 20.0f));
					}

					if((int)c->p->maxenergy > (int)c->p->energy)
						c->AddEnergy((c->p->maxenergy / 10.0f));

					c->p->nextRefresh = (getTimestamp() + refreshDelay);
				}

				if((unsigned)c->p->extraCoolDown <= getTimestamp())
				{
					if(c->GetBonus(HPREGEN) > 0)
					{
						c->AddHP(1.0f);

						c->com->setExtraCoolDown((int)(1000.0f / (float)c->GetBonus(HPREGEN)));
					}

					else
						c->com->setExtraCoolDown(250);
				}

				if(c->p->follow)
				{
					Client *ctp = GetClientByPlayerIdInMap(c->p->followId, c->p->mapId->id);

					if(ctp == NULL)
						wc->sendPlayerFollow(c, c, false);

					else
					{
						if(c->NotInsidePKPlayer(ctp->p))
							MoveToPlayer(c, ctp);

						c->p->move.start(false);
					}
				}

				else if(c->p->moving)
				{
					int speed = (c->p->speed + (c->GetBonus(SPEED) - c->GetBonus(SPEEDMINUS)));

					if(speed <= 0)
						speed = 1;

					float bufLvl = (float)speed;
					float xTemp = ((bufLvl * 62.0f) * ((float)c->p->move.get_ticks(false) / 1000.0f));

					if(c->p->cMove == UP)
					{
						if((c->p->y - xTemp) < 26 && (int)c->p->y != 26 && (int)c->p->y > 26)
							xTemp = (c->p->y - 26.0f);

						if((c->p->y - xTemp) >= 26)
							c->p->y -= xTemp;
					}
					
					else if(c->p->cMove == DOWN)
					{
						if((c->p->y + xTemp) > (c->p->mapId->ySize - c->p->meH) && (int)c->p->y != (c->p->mapId->ySize - c->p->meH) && (int)c->p->y < (c->p->mapId->ySize - c->p->meH))
							xTemp = ((c->p->mapId->ySize - c->p->meH) - c->p->y);

						if((c->p->y + xTemp) <= (c->p->mapId->ySize - c->p->meH))
							c->p->y += xTemp;
					}

					else if(c->p->cMove == LEFT)
					{
						if((c->p->x - xTemp) < 0 && (int)c->p->x != 0 && (int)c->p->x > 0)
							xTemp = c->p->x;

						if((c->p->x - xTemp) >= 0)
							c->p->x -= xTemp;
					}

					else if(c->p->cMove == RIGHT)
					{
						if((c->p->x + xTemp) > (c->p->mapId->xSize - c->p->meW) && (int)c->p->x != (c->p->mapId->xSize - c->p->meW) && (int)c->p->x < (c->p->mapId->xSize - c->p->meW))
							xTemp = ((c->p->mapId->xSize - c->p->meW) - c->p->x);

						if((c->p->x + xTemp) <= (c->p->mapId->xSize - c->p->meW))
							c->p->x += xTemp;
					}

					c->p->move.start(false);
				}

				if(c->p->buffs.size() > 0)
				{
					for(unsigned int i = 0; i < c->p->buffs.size(); i++)
					{
						if(c->p->buffs.at(i)->changeEffect == HPDRAIN)
						{
							if((unsigned)c->p->buffs.at(i)->nextUse <= getTimestamp())
							{
								if(!c->p->godMode)
								{
									int dead = 0;
									int dmg = c->p->buffs.at(i)->value * 4;
									
									if((c->p->chp - dmg) > 0)
										c->p->chp -= dmg;

									else
									{
										dmg = (int)c->p->chp;

										if(c->p->buffs.at(i)->userPut != 0)
										{
											Client *c2 = GetClientByPlayerId(c->p->buffs.at(i)->userPut);

											if(c2 != NULL)
											{
												SystemChat(c2, PKKILLED, c, NULL);
												SystemChat(c, PKKILLEDBY, c2, NULL);

												if(c2->p->level < maxLvl)
													c2->AddExp(c->p->level);

												if(c->p->pkpoints > 0)
												{
													c->p->pkpoints--;
													c->com->sendUpdate(PKPOINTS);

													if(c->p->gold > 0)
													{
														int goldAdd = c->p->level * 100;

														if(c->p->gold < goldAdd)
															goldAdd = c->p->gold;

														c->RemoveItem(2, goldAdd, false, 2);
														c2->AddItem(2, goldAdd, true, NULL, randomInt);

														string penyaString = "You lost ";
														penyaString += IntToString(goldAdd);
														penyaString += " dinero to ";
														penyaString += c2->p->name;
														penyaString += ".";

														SystemChat(c, CUSTOM, NULL, (char*)penyaString.c_str());

														penyaString = "You gained ";
														penyaString += IntToString(goldAdd);
														penyaString += " dinero from killing ";
														penyaString += c->p->name;
														penyaString += ".";

														SystemChat(c2, CUSTOM, NULL, (char*)penyaString.c_str());
													}
												}

												c2->p->pkpoints++;
												c2->com->sendUpdate(PKPOINTS);

												string pwndString = c2->p->name;
												pwndString += " pwned ";
												pwndString += c->p->name;
												pwndString += "!";

												Packet pwndPacket = CreatePacketChat(pwndString.c_str(), "Notice", false, false, 0);

												for(unsigned int l = 0; l < ac.size(); l++)
												{
													if(ac.at(l)->ingame)
														ac.at(l)->AddPacket(pwndPacket, 0);
												}
											}
										}

										dead = wc->killPlayer(c);
									}

									c->com->sendUpdate(HP);
									c->com->sendUpdate(HPPROCENT);

									wc->sendDamage(c->p, NULL, 2, dmg);

									if(dead == 1)
										break;
								}

								else
									wc->sendDamage(c->p, NULL, 2, 0);

								c->com->setAttackTimer(playerAttackTimer);
								c->p->buffs.at(i)->nextUse = (getTimestamp() + 2000);
							}
						}

						if((unsigned)c->p->buffs.at(i)->endTime <= getTimestamp())
						{
							c->RemoveBuff(i);

							break;
						}
					}
				}

				if(enableAttack)
				{
					std::vector<Monster*> toFol;

					int aggX = 300;
					int aggY = 300;

					if(c->p->mapId->aggX != 1337)
						aggX = c->p->mapId->aggX;

					if(c->p->mapId->aggY != 1337)
						aggY = c->p->mapId->aggY;

					for(unsigned int q = 0; q < mobs.size(); q++)
					{
						if((unsigned)mobs.at(q)->nextAttack <= getTimestamp() && mobs.at(q)->mapId->id == c->p->mapId->id && mobs.at(q)->spawned)
						{
							if(mobs.at(q)->followId == c->p->getId() && mobs.at(q)->follows)
							{
								if(((c->p->x - mobs.at(q)->monW - c->p->wW - 20) <= mobs.at(q)->x && (c->p->x + c->p->meW + c->p->wW + 20) >= mobs.at(q)->x) && ((c->p->y - mobs.at(q)->monH - 20) <= mobs.at(q)->y && (c->p->y + c->p->meH + 20) >= mobs.at(q)->y))
								{
									mobs.at(q)->nextAttack = (getTimestamp() + (Random((monsterAttackDelay - (monsterAttackDelay / 3)), (monsterAttackDelay + (monsterAttackDelay / 3)))));

									c->MonsterAttack(mobs.at(q)->pid);
								}
							}

							else if(!mobs.at(q)->follows)
							{
								if(((c->p->x - mobs.at(q)->monW) <= mobs.at(q)->x && (c->p->x + c->p->meW) >= mobs.at(q)->x) && ((c->p->y - mobs.at(q)->monH) <= mobs.at(q)->y && (c->p->y + c->p->meH) >= mobs.at(q)->y))
								{
									mobs.at(q)->nextAttack = (getTimestamp() + (Random((monsterAttackDelay - (monsterAttackDelay / 3)), (monsterAttackDelay + (monsterAttackDelay / 3)))));

									c->MonsterAttack(mobs.at(q)->pid);
								}
							}

							if(mobs.at(q)->aggro && !mobs.at(q)->follows && !mobs.at(q)->goAfter && mobs.at(q)->followId == 0)
							{
								if(mobs.at(q)->x > (c->p->x - (aggX / 2)) && mobs.at(q)->x < (c->p->x + (aggX / 2)) && mobs.at(q)->y > (c->p->y - (aggY / 2)) && mobs.at(q)->y < (c->p->y + (aggY / 2)))
								{
									Monster *m = mobs.at(q);

									m->followId = c->p->getId();
									m->follows = true;
									m->goAfter = false;
									m->moveDone = true;

									m->moving = false;
									m->goDir = 0;

									m->bx = m->x;
									m->by = m->y;

									m->move.start(true);

									toFol.push_back(m);
								}
							}
						}
					}

					if(toFol.size() > 0)
					{
						Packet pak = Packet();
						pak.addHeader(0x47);
						pak.addLongInt(toFol.size());

						for(unsigned int l = 0; l < toFol.size(); l++)
						{
							pak.addLongInt(toFol.at(l)->pid);
							pak.addLongInt(c->p->getId());
							pak.addLongInt((int)toFol.at(l)->x);
							pak.addLongInt((int)toFol.at(l)->y);
						}

						pak.ready();
						SendAllOnMap(pak, 0, c->p->mapId->id, 0);

						toFol.clear();
					}
				}
			}

			if(c->p->dead && (unsigned)c->p->reviveTime <= getTimestamp())
			{
				c->p->dead = false;
				c->p->moving = false;
				c->p->reviveTime = 0;
				c->p->attacked = false;
				c->p->atkTimer = 0;

				if(c->p->mapId->id != mPlayerStartMap)
				{
					if(!c->p->mapId->pk)
						c->ChangeMap(NULL, true, mPlayerStartMap, mPlayerStartX, mPlayerStartY);
				}

				if(c->p->mapId->pk || c->p->mapId->id == mPlayerStartMap)
					c->PlayerPosition((float)c->p->mapId->resX, (float)c->p->mapId->resY, 0);

				c->p->chp = (c->p->hp / 2);

				if(!c->p->mapId->pk)
				{
					c->RemoveExp(3);

					if(c->p->equipped && !c->p->mapId->weaponAllowed)
					{
						c->p->equipped = false;

						Packet pak = Packet();

						pak.addHeader(0x17);
						pak.addBool(false);
						pak.addLongInt(c->p->getId());

						pak.ready();
						SendAllOnMap(pak, 0, c->p->mapId->id, 0);
					}
				}

				SystemChat(c, REVIVE, NULL, NULL);

				Packet p3 = Packet();

				p3.addHeader(0x21);
				p3.addBool(false);
				p3.addLongInt(c->p->getId());

				p3.ready();
				SendAllOnMap(p3, 0, c->p->mapId->id, c->getConnection());

				c->com->sendUpdate(HP);
				c->com->sendUpdate(HPPROCENT);
			}

			if((unsigned)c->p->nextSave <= getTimestamp())
			{
				c->SavePlayer(AUTOSAVE);

				c->p->nextSave = (getTimestamp() + saveDelay);
			}
		}
	}

	for(unsigned int i = 0; i < parties.size(); i++)
	{
		if(parties.at(i)->members.size() > 0)
		{
			if(parties.at(i)->members.size() > 1)
			{
				for(unsigned int z = 0; z < parties.at(i)->members.size(); z++)
				{
					if(!parties.at(i)->members.at(z)->online)
					{
						Packet pak = Packet();

						pak.addHeader(0x24);
						pak.addInt(1);
						pak.addInt(2);
						pak.addInt(2);
						pak.addString(parties.at(i)->members.at(z)->name_.c_str());

						pak.ready();

						//char otherSystemChat[100] = "";

						//strcpy_s(otherSystemChat, parties.at(i)->members.at(z)->name);
						//strcat_s(otherSystemChat, " has left the party.");

						string other_chat = parties.at(i)->members.at(z)->name_;
						other_chat += " has left the party.";

						for(unsigned int o = 0; o < parties.at(i)->members.size(); o++)
							parties.at(i)->members.at(o)->mb->AddPacket(pak, 0);

						parties.at(i)->members.at(z)->mb = NULL;

						delete parties.at(i)->members.at(z);
						parties.at(i)->members.erase(parties.at(i)->members.begin() + z);

						for(unsigned int y = 0; y < parties.at(i)->members.size(); y++)
							SystemChat(parties.at(i)->members.at(y)->mb, CUSTOM, NULL, other_chat.c_str());

						if(debugs)
							log(DEBUG, "Kicked member.\n");
					}
				}
			}

			else
			{
				for(unsigned int y = 0; y < parties.at(i)->members.size(); y++)
				{
					Packet pak = Packet();

					pak.addHeader(0x24);
					pak.addInt(1);
					pak.addInt(2);
					pak.addInt(2);
					pak.addString(parties.at(i)->members.at(y)->name_.c_str());

					pak.ready();
					parties.at(i)->members.at(y)->mb->AddPacket(pak, 0);

					Packet pak2 = Packet();

					pak2.addHeader(0x79);
					pak2.addLongInt(1);

					parties.at(i)->members.at(y)->mb->AddPacket(pak2, 0);

					SystemChat(parties.at(i)->members.at(y)->mb, CUSTOM, NULL, "The party has been disbanded.");

					parties.at(i)->members.at(y)->mb->p->pt = NULL;

					if(debugs)
						log(DEBUG, "Removed party.\n");

					parties.at(i)->members.at(y)->mb = NULL;

					delete parties.at(i)->members.at(y);
				}

				parties.at(i)->members.clear();
				
				delete parties.at(i);
				parties.erase(parties.begin() + i);
			}
		}

		else
		{
			parties.at(i)->members.clear();
			
			delete parties.at(i);
			parties.erase(parties.begin() + i);

			if(debugs)
				log(DEBUG, "Removed party.\n");
		}
	}

	for(unsigned int i = 0; i < npcs.size(); i++)
	{
		if(npcs.at(i)->enableChat)
		{
			if((unsigned)npcs.at(i)->nextChat <= getTimestamp())
			{
				SendAllOnMap(CreatePacketChat(npcs.at(i)->chat.at((npcs.at(i)->currChat - 1)).text_.c_str(), npcs.at(i)->name.c_str(), false, true, 0), 0, npcs.at(i)->mapId->id, 0);

				if(npcs.at(i)->currChat == npcs.at(i)->chat.size())
					npcs.at(i)->currChat = 1;
				else
					npcs.at(i)->currChat++;

				npcs.at(i)->nextChat = (getTimestamp() + Random((npcChatDelay - (npcChatDelay / 4)), (npcChatDelay + (npcChatDelay / 4))));
			}
		}
	}

	for(unsigned int i = 0; i < drops.size(); i++)
	{
		if(!drops.at(i)->lootEnable)
		{
			if((unsigned)drops.at(i)->loottimer <= getTimestamp())
				drops.at(i)->lootEnable = true;
		}

		if((unsigned)drops.at(i)->autodelete <= getTimestamp())
		{
			Packet pak = Packet();

			pak.addHeader(0x39);
			pak.addInt(2);
			pak.addLongInt(drops.at(i)->id);

			pak.ready();
			SendAllOnMap(pak, 0, drops.at(i)->map, 0);

			delete drops.at(i)->drop;
			delete drops.at(i);

			drops.erase(drops.begin() + i);
		}
	}

	if(enableStatusCheck)
	{
		if((unsigned)nextStatusCheck <= getTimestamp())
		{
			Uptime();

			nextStatusCheck += statusDelay;
		}
	}
}