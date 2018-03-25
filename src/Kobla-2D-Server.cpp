#include "Log.h"
#include "Base.h"

using namespace std;

void process() {
	const unsigned int port = Base::settings().get<unsigned short>("port");
	Base::network().start(port);
}

int main() {
	Base::settings().parse("config");
	process();
	
	return 0;
}