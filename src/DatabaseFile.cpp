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
		
		Action action;
		action.setID(id);
		action.addText(text);
		
		reference_actions.push_back(action);
	}
}

void DatabaseFile::parseNPCs(vector<NPC>& reference_npcs) {
	auto data = loadDataID("data/npcs/id");
	
	for (auto& peer : data) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		Config config;
		config.parse("data/npcs/" + name);
		
		auto real_name = config.get<string>("name", "");
		auto object_id = config.get<int>("object_id", -1);
		auto activate_ids = config.getAll<int>("actions", vector<int>());
		
		NPC npc;
		npc.setNPCID(id);
		npc.setName(real_name);
		npc.setObjectID(object_id);
		npc.setActions(activate_ids);
			
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
		
		auto real_name = config.get<string>("name", "");
		auto object_id = config.get<int>("object_id", -1);
		
		Monster monster;
		monster.setMonsterID(id);
		monster.setName(real_name);
		monster.setObjectID(object_id);
		
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
				auto npc_id = stoi(tokens.at(1));
				auto x = stoi(tokens.at(2));
				auto y = stoi(tokens.at(3));
				auto collision = stoi(tokens.at(4));
				
				NPC npc = Base::game().getReferenceNPC(npc_id);
				npc.setPosition(x, y);
				npc.setMapID(id);
				npc.setColliding(collision);
				// Otherwise the NPC will have the same ID as the reference
				npc.setValidID();
				
				// Set NPC AI type to Basic for now
				// TODO: Change this
				npc.setAI(AI_NPC_TYPE_KILL_CLOSE);
				
				map.addNPC(npc);
				
				//Log(DEBUG) << "Added NPC " << npc.getName() << " on map " << id << endl;
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