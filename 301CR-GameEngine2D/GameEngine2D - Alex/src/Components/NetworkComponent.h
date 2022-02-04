/*#ifndef NETWORKCOMPONENT_H
#define NETWORKCOMPONENT_H

#include <enet/include/enet/enet.h>

struct Vector2 {
	float x;
	float y;
};

struct ClientData {
	int packetType = 0;
	int clientIndex;
};

struct ClientPacket {
	int clientIndex;
	Vector2 position;
};

struct EnetData {

	ENetAddress address;
	ENetHost* client;
	ENetPeer* peer;
	ENetEvent enetEvent;
};

#endif // !NETWORKCOMPONENT_H*/
