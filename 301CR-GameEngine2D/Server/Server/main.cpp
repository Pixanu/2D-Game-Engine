#include <iostream>
#include <enet/enet.h>


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

int main(int argc, char** argv)
{

	int clientCounter = 0;
	bool IsRunning = true;

	if (enet_initialize() != 0) {
		std::cout << "Enet initialisation failed." << std::endl;
	}

	ENetAddress address;
	ENetHost* server;
	ENetEvent enetEvent;
	ENetPacket* dataPacket;
	address.host = ENET_HOST_ANY;
	address.port = 255;
	server = enet_host_create(&address, 32, 2, 0, 0);
	if (server == NULL)
	{
		std::cout << "Server failed to initialise!" << std::endl;
	}
	ClientPacket* clientPacket = new ClientPacket;
	clientPacket->clientIndex = 0;
	clientPacket->position.x = 0.0f;
	clientPacket->position.y = 0.0f;
	ClientData* clientData = new ClientData;

	while (IsRunning) {
		while (enet_host_service(server, &enetEvent, 0) > 0) {
			switch (enetEvent.type) {

			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "A client connected from address " << enetEvent.peer->address.host << ":" << enetEvent.peer->address.port << std::endl;
				clientData->clientIndex = clientCounter;
				dataPacket = enet_packet_create(clientData, sizeof(ClientData), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(enetEvent.peer, 0, dataPacket);
				enetEvent.peer->data = (void*)"This is a client";
				clientCounter++;

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "A client has disconnected at address " << enetEvent.peer->address.host << ":" << enetEvent.peer->address.port << std::endl;
				clientCounter--;
				if (clientCounter == 0) {
					IsRunning = false;
				}

				break;
			}
		}
	}
	enet_host_destroy(server);
	atexit(enet_deinitialize);
}
