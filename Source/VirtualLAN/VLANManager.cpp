/*
    Initial author: Convery
    Started: 2017-5-2
    License: Apache 2.0
*/

#include "../StdInclude.h"
#include <unordered_map>
#include "VirtualLAN.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace DHTManager
{
    extern std::mutex Mapguard;
    extern std::unordered_map<std::string /* IP */, bool /* Connected */> DHTConnections;
}

namespace VLANManager
{
    VirtualLAN::Networkaddress Clientaddress;

    namespace Internal
    {
    #ifdef _WIN32

        SOCKET Socket{ INVALID_SOCKET };
        bool Initialized{ false };
        uint16_t Port{ 28073 };

        void Send(std::string Data)
        {
            if (!Initialized) return;

            DHTManager::Mapguard.lock();
            {
                for (auto &Item : DHTManager::DHTConnections)
                {
                    sockaddr_in Address;
                    Address.sin_family = AF_INET;
                    Address.sin_port = htons(Port);
                    Address.sin_addr.s_addr = inet_addr(Item.first.c_str());
                    sendto(Socket, Data.data(), Data.size(), 0, (sockaddr *)&Address, sizeof(Address));
                }
            }
            DHTManager::Mapguard.unlock();
        }
        void Listen()
        {
            int Recvlength = 0;
            auto Buffer = std::make_unique<char[]>(16 * 1024);

            while (Initialized)
            {
                sockaddr_in From;
                int Fromlength = sizeof(From);
                std::memset(Buffer.get(), 0, 16 * 1024);

                Recvlength = recvfrom(Socket, Buffer.get(), 16 * 1024, 0, (sockaddr *)&From, &Fromlength);
                if (ntohl(From.sin_addr.s_addr) != Clientaddress.Address) continue;
                if (Recvlength == -1) continue;

                Send({ Buffer.get(), uint32_t(Recvlength) });
            }
        }
        void Initialize()
        {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);

            Socket = socket(AF_INET, SOCK_DGRAM, 0);
            if (Socket == INVALID_SOCKET) return;

            BOOL Enabled = TRUE;
            if (setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, (char*)&Enabled, sizeof(BOOL)) < 0)
                return;

            sockaddr_in Address;
            Address.sin_family = AF_INET;
            Address.sin_port = htons(Port);
            Address.sin_addr.s_addr = INADDR_ANY;

            if (bind(Socket, (sockaddr*)&Address, sizeof(Address)) < 0)
                return;

            Initialized = true;

            // Initialize Clientaddress with our own address.
            {
                sockaddr_in Client;
                int Clientlength = sizeof(Client);
                auto Buffer = std::make_unique<char[]>(sizeof(VirtualLAN::Networkpacket));

                VirtualLAN::Send("");
                int Recvlength = recvfrom(Socket, Buffer.get(), sizeof(VirtualLAN::Networkpacket), 0, (sockaddr *)&Client, &Clientlength);
                if (Recvlength == sizeof(VirtualLAN::Networkpacket))
                {
                    Clientaddress.Port = ntohs(Client.sin_port);
                    Clientaddress.Address = ntohl(Client.sin_addr.s_addr);
                }
            }
        }
    #else
        /* TODO(Convery): Nix implementation. */
        void Send(std::string Data) {};
    #endif
    }

    namespace { struct VLANManLoad { VLANManLoad() { Internal::Initialize(); std::thread(Internal::Listen).detach(); } }; VLANManLoad VLANLoader{}; }
}
