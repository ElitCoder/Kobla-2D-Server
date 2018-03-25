#include "Main.h"
#include "Handle.h"
#include "Item.h"
#include "Command.h"
#include "BufferCheck.h"

extern bool debugPacket;

void ProcessPacket(Client *p, unsigned char *b2, int len)
{
	if(len <= 0)
		return;

	char *b = (char*)b2;

	if(p->disconnect)
		return;

	if(!PacketBufferCheck(b, len))
	{
		log(ERR, "[void ProcessPacket(Client*,char*,int)] [Invalid packet buffer] [%d]\n", len);

		return;
	}

	if(len >= 3)
	{
		if(b[0] == 0x3)
		{
			if(p->validateClient || b[2] == 0x1)
			{
				switch(b[2])
				{
					case 0x1: p->STARTversion();
						break;

					case 0x2: p->Login(b, len);
						break;

					case 0x3: p->Register(b, len);
						break;

					case 0x4: 
						{
							p->Logout();

							if(p->ingame)
							{
								p->SavePlayer(QUITSAVE);
								p->Unspawn(0);
							}
						break;
						}

					case 0x5: p->Spawn(b);
						break;

					case 0x6: p->CharacterList();
						break;

					case 0x7: p->EditCharacter((int)b[3], b);
						break;

					case 0x55: p->IsOnline(true);
						break;

					case 0x59: p->Ping(b);
						break;
				}

				if(p->ingame)
				{
					switch(b[2])
					{
						case 0x8: p->Move(b);
							break;

						case 0x9: p->StopMove(b);
							break;

						case 0x10: p->ChangeMap(b, false, 0, 0, 0);
							break;

						case 0x14: p->UpdatePosition(b);
							break;

						case 0x15: p->Chat((char*)getString(b, 4, (int)b[3]).c_str());
							break;

						case 0x16: p->Shoot(b, false, 0, 0, false, 0);
							break;

						case 0x17:
							{
								if(p->p->eq[0] != NULL)
									p->EquipWeapon(!p->p->equipped);
							break;
							}

						case 0x18: p->Attack(0, 0, 0, 0, b, false, 0, true, GetClass(p->p->job)->based, true);
							break;

						case 0x22: p->AddStat(b);
							break;

						case 0x24: p->SendInvite((char*)getString(b, 4, (int)b[3]).c_str());
							break;

						case 0x25: p->LeaveParty();
							break;

						case 0x26: p->KickFromParty((char*)getString(b, 4, (int)b[3]).c_str());
							break;

						case 0x27: p->AcceptInvite(b);
							break;

						case 0x30: p->HitNPCMenuButton(b);
							break;

						case 0x33: p->HitNPCDialogButton(b);
							break;

						case 0x35: p->MoveItem(b[3], b[4]);
							break;

						case 0x37: p->UseItem(b[3]);
							break;

						case 0x38:
							{
								int pos = 4; int int1 = atoi(getString(b, (pos + 1), (int)b[pos]).c_str()); pos += ((int)b[pos] + 1); int int2 = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());

								p->RemoveItem(b[3], int1, false, int2);
							break;
							}

						case 0x39: p->PickUpDrop(atoi(getString(b, 4, (int)b[3]).c_str()));
							break;

						case 0x45: p->UseSkill(atoi(getString(b, 4, (int)b[3]).c_str()));
							break;

						case 0x48: p->RemoveTaskbarItem(atoi(getString(b, 4, (int)b[3]).c_str()));
							break;

						case 0x49: p->AddTaskbarItem(b);
							break;

						case 0x50: p->UseTaskbarItem(atoi(getString(b, 4, (int)b[3]).c_str()));
							break;

						case 0x53: p->DropInventoryItem(b);
							break;

						case 0x56: p->Quit();
							break;

						case 0x58: p->UnEquip(b[3]);
							break;

						case 0x65: p->ShopCartAdd(b);
							break;

						case 0x66: p->ShopCartBuy(b);
							break;

						case 0x67:
							{
								int pos = 4; int int1 = atoi(getString(b, (pos + 1), (int)b[pos]).c_str()); pos += ((int)b[pos] + 1); int int2 = atoi(getString(b, (pos + 1), (int)b[pos]).c_str());
								p->ShopCartSell(b[3], int1, int2);
							break;
							}

						case 0x68: p->ClientErrorMessage(b);
							break;

						case 0x69: p->ShopCartRemove(b);
							break;

						case 0x70: p->PlayerMenu(b);
							break;

						case 0x72: p->AcceptTradeInvite(b);
							break;

						case 0x75: p->AddTradeItem(b);
							break;

						case 0x76: p->DoneTrade(b[3]);
							break;

						case 0x77: p->AddTradePenya(atoi(getString(b, 4, (int)b[3]).c_str()));
							break;

						case 0x78: p->Teleport(b);
							break;

						case 0x0C: p->AcceptFriendInvite(b);
							break;

						case 0x0D: p->RemoveFriend(getString(b, 4, (int)b[3]).c_str());
							break;

						case 0x0E:
							{
								int pos = 4;
								int ids = 0;
								std::string names = getString(b, (pos + 1), (int)b[pos]);

								Player *pl = GetPlayerByName(names.c_str());

								if(pl != NULL)
									ids = pl->getId();

								if(b[3] == 1)
									p->SendFriendInvite(ids, names.c_str());

								else if(b[3] == 2)
									p->RemoveFriend(names.c_str());
							break;
							}

						case 0x0F: p->FriendMenu(b);
							break;

						case 0x1A:
							{
								int pos = 3;

								std::string name = getString(b, (pos + 1), (int)b[pos]);
								pos += ((int)b[pos] + 1);

								std::string msg = getString(b, (pos + 1), (int)b[pos]);
								pos += ((int)b[pos] + 1);

								p->SendPM(name.c_str(), msg.c_str());
							break;
							}

						case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7: case 0x55: case 0x59:
							break;

						default: log(ERR, "[void ProcessPacket(Client*,char*,int)] [Invalid command] [0x%X] [%d]\n", b[2], len);
							break;
					}
				}
			}
		}

		else if(b[0] == 0x4)
		{
			switch(b[2])
			{
				case 0x1: p->CheckISC(b);
					break;

				case 0x2: p->GetInfo();
					break;

				case 0x3: p->GetTopTen();
					break;

				case 0x4: p->GetTopTenPvP();
					break;

				case 0x5: p->GetCharactersInfo();
					break;

				case 0x6: p->GetAccountsInfo();
					break;

				default: log(ERR, "[void ProcessPacket(Client*,char*,int)] [Invalid website command] [0x%X] [%d]\n", b[1], len);
					break;
			}
		}

		else
		{
			log(ERR, "[void ProcessPacket(Client*,char*,int)] [Invalid packet] [0x%X] [0x%X] [%d]\n", b[0], b[2], len);

			p->disconnect = true;
		}
	}

	else
	{
		log(ERR, "[void ProcessPacket(Client*,char*,int)] [Packet is too small] [%d]\n", len);

		p->disconnect = true;
	}
}