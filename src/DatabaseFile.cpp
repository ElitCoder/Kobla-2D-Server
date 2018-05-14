#include "DatabaseFile.h"
#include "Config.h"
#include "Log.h"
#include "Base.h"
#include "NPC.h"
#include "Monster.h"
#include "Map.h"
#include "Game.h"
#include "TemporaryObject.h"

#include <deque>
#include <fstream>

using namespace std;

using DataContainer = vector<pair<int, deque<string>>>;

bool DatabaseFile::login(const string& username, const string& password) {
	// Check login from file
	
	// Remove warning for now
	if (username == password) {}
	
	return true;
}

/*
	Parsing stuff below
*/

// Generic data loader
static DataContainer loadDataID(const string& path) {
	DataContainer container;
	Config config;
	config.parse(path);
	
	for (auto& peer : config.internal())
		container.push_back({ stoi(peer.first), peer.second });
		
	return container;
}

template<class T>
static T flattenString(const vector<T>& container) {
	T value;
	
	for (auto& element : container) {
		value += element;
		value += " ";
	}
		
	return value;
}

void DatabaseFile::parseActions(vector<Action>& reference_actions) {
	auto data = loadDataID("data/actions/id");
	
	for (auto& peer : data) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		Config config;
		config.parse("data/actions/" + name);
		
		auto text = flattenString(config.getAll<string>("text", vector<string>()));
		auto send_all = config.get<string>("send", "") == "all";
		
		Action action;
		action.setID(id);
		action.addText(text);
		action.setTextResponse(send_all);
		
		reference_actions.push_back(action);
	}
}

static void parseCharacter(Character* character, Config& config) {
	auto real_name = config.get<string>("name", "");
	auto object_id = config.get<int>("object_id", -1);
	auto activate_ids = config.getAll<int>("actions", vector<int>());
	
	character->setName(real_name);
	character->setObjectID(object_id);
	character->setActions(activate_ids);
}

void DatabaseFile::parseNPCs(vector<NPC>& reference_npcs) {
	auto data = loadDataID("data/npcs/id");
	
	for (auto& peer : data) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		Config config;
		config.parse("data/npcs/" + name);
		
		NPC npc;
		npc.setNPCID(id);
		
		parseCharacter(&npc, config);
			
		reference_npcs.push_back(npc);
	}
}

void DatabaseFile::parseMonsters(vector<Monster>& reference_monsters) {
	auto monsters = loadDataID("data/monsters/id");
	
	for (auto& peer : monsters) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		Config config;
		config.parse("data/monsters/" + name);
		
		Monster monster;
		monster.setMonsterID(id);
		
		parseCharacter(&monster, config);
		
		reference_monsters.push_back(monster);
	}
}

static deque<string> getTokens(string input, char delimiter) {
	istringstream stream(input);
	deque<string> tokens;
	string token;
	
	while (getline(stream, token, delimiter))
		if (!token.empty())
			tokens.push_back(token);
	
	return tokens;
}

void DatabaseFile::parseMaps(vector<Map>& maps) {
	auto data = loadDataID("data/maps/id");
	
	for (auto& peer : data) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		ifstream file("data/maps/" + name);
		
		if (!file.is_open())
			Log(WARNING) << "Could not open Map file " << name << endl;
			
		Map map;
		map.setID(id);
		
		string tmp;
		
		while (getline(file, tmp)) {
			if (tmp.empty() || tmp.front() == '#')
				continue;

			auto tokens = getTokens(tmp, ' ');
			
			// Remove ':'
			tokens.front().pop_back();
			
			// Add NPC
			if (tokens.front() == "npc") {
				tokens.pop_front();
				
				vector<pair<double, double>> patrol;
				int patrol_wait_min = 10000;
				int patrol_wait_max = 20000;
				int npc_id = -1;
				double x = -1;
				double y = -1;
				bool collision = false;
				
				for (size_t i = 0; i < tokens.size(); i++) {
					auto& key = tokens.at(i);
					i += 1;
					
					if (key == "ID") {
						npc_id = stoi(tokens.at(i));
					} else if (key == "X") {
						x = stod(tokens.at(i));	
					} else if (key == "Y") {
						y = stod(tokens.at(i));
					} else if (key == "COLLISION") {
						collision = stoi(tokens.at(i));
					} else if (key == "PATROL") {
						auto patrol_x = stod(tokens.at(i));
						auto patrol_y = stod(tokens.at(i + 1));
						i++;
						
						patrol.push_back({ patrol_x, patrol_y });
					} else if (key == "PATROL_WAIT") {
						patrol_wait_min = stoi(tokens.at(i));
						patrol_wait_max = stoi(tokens.at(i + 1));
						i++;
					}
				}
				
				NPC npc = Base::game().getReferenceNPC(npc_id);
				npc.setPosition(x, y);
				npc.setMapID(id);
				npc.setColliding(collision);
				// Otherwise the NPC will have the same ID as the reference
				npc.setValidID();
				npc.setPatrol(patrol);
				npc.setPatrolWaitTime(patrol_wait_min, patrol_wait_max);
				
				npc.setAI(AI_NPC_TYPE_KILL_CLOSE);
				
				// Set NPC AI type to Basic for now
				// TODO: Change this
				if (!patrol.empty()) {
					// Add the position as first patrol position
					patrol.insert(patrol.begin(), { x, y });
					
					npc.setAI(AI_NPC_TYPE_PATROL);
				}
				
				map.addNPC(npc);
			} else if (tokens.front() == "player_spawn") {
				// Where the player can be spawned (random here)
				auto from_x = stoi(tokens.at(1));
				auto from_y = stoi(tokens.at(2));
				auto to_x = stoi(tokens.at(3));
				auto to_y = stoi(tokens.at(4));
				
				//Log(DEBUG) << "Adding spawning point from " << from_x << endl;
				
				map.addSpawnPoint(MapSpawnPoint({{ from_x, from_y }}, {{ to_x, to_y }}));
			} else if (tokens.front() == "monster") {
				auto monster_id = stoi(tokens.at(1));
				auto from_x = stoi(tokens.at(2));
				auto from_y = stoi(tokens.at(3));
				auto to_x = stoi(tokens.at(4));
				auto to_y = stoi(tokens.at(5));
				auto number = stoi(tokens.at(6));
				
				Monster monster = Base::game().getReferenceMonster(monster_id);
				monster.setMapID(id);
				
				map.addMonster(monster, number, MapSpawnPoint({{ from_x, from_y }}, {{ to_x, to_y }}));
			}
		}
		
		maps.push_back(map);
	}
}