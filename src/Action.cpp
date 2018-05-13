#include "Action.h"
#include "Object.h"
#include "Base.h"
#include "Player.h"
#include "NetworkCommunication.h"
#include "PacketCreator.h"
#include "Packet.h"

using namespace std;

void Action::setID(int id) {
	id_ = id;
}

int Action::getID() const {
	return id_;
}

void Action::activate(Object* object, Object* activater) const {
	if (!texts_.empty()) {
		auto& text = texts_.front();
		string actual;
		
		for (size_t i = 0; i < text.size(); i++) {
			if (text.at(i) == '$') {
				// Skip next $
				i += 2;
				
				string variable_name;
				
				for (; i < text.size(); i++) {
					if (text.at(i) == '$') {
						i++;
						
						break;
					}
					
					variable_name += text.at(i);
				}
				
				if (variable_name == "PLAYER_NAME")
					actual += activater->getName();
					
				else if (variable_name == "OWN_NAME")
					actual += object->getName();
					
				else if (variable_name == "LINE_BREAK")
					actual += "\n";
					
				continue;
			}
			
			actual += text.at(i);
		}
		
		// Send this text to activater
		if (activater->getObjectType() == OBJECT_TYPE_PLAYER)
			Base::network().sendUnique(((Player*)activater)->getConnectionID(), PacketCreator::text(object, actual, TEXT_DISAPPEAR_MS));
			
		if (response_type_)
			Base::network().sendToAll(PacketCreator::text(object, actual, TEXT_DISAPPEAR_MS));
	}
}

void Action::addText(const string& text) {
	texts_.push_back(text);
}

void Action::setTextResponse(bool response_type) {
	response_type_ = response_type;
}