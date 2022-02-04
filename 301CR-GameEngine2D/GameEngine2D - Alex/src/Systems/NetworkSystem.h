/*#ifndef NETWORKSYSTEM_H
#define NETWORKSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/NetworkComponent.h"

class NetworkSystem : public System
{
public:
	NetworkSystem() {
		RequireComponent<EnetData>();
		RequireComponent<ClientPacket>();
		RequireComponent<ClientData>();
		RequireComponent<Vector2>();

		void NetworkInit() {
			if (enet_initialize() != 0) {
				std::cout << "	Enet initialisation failed." << std::endl;
				return -104;
			}

			(*enetdata).client = enet_host_create(NULL, 1, 2, 0, 0);
			if ((*enetdata).client == NULL)
			{
				std::cout << "	Client failed to initialise!" << std::endl;
				return -104;
			}
			enet_address_set_host(&(*enetdata).address, "localhost");
			(*enetdata).address.port = 255;
			(*enetdata).peer = enet_host_connect((*enetdata).client, &(*enetdata).address, 2, 0);
			if ((*enetdata).peer == NULL) {
				std::cout << "	No available peers for initializing an ENet connection." << std::endl;
				return -104;
			}
			ClientData* clientData = new ClientData;
			int* packetType = new int;
			*packetType = -1;
			ClientPacket* clientPacket = new ClientPacket;
			ENetPacket* dataPacket;
			int clientIndex = -1;

			return 104;
		}

		void NetworkUpdate() {

			while (enet_host_service((*enetdata).client, &(*enetdata).enetEvent, 0) > 0) {
				switch ((*enetdata).enetEvent.type) {
				case ENET_EVENT_TYPE_RECEIVE:
					std::cout << "	Client connected." << std::endl;
				}
			}
		}
	}
};

#endif // !NETWORKSYSTEM_H*/
