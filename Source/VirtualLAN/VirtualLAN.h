/*
    Initial author: Convery
    Started: 2017-5-2
    License: Apache 2.0
*/

#pragma once
#include "../StdInclude.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace VirtualLAN
{
    struct Networkaddress { uint32_t Address; uint16_t Port; };
    struct Networkpacket { uint32_t Magic; uint32_t Type; uint32_t Length; };
    struct Networkpacketwrapper : Networkpacket { Networkaddress Origin; std::string Payload; };

    // Helpers.
    Networkaddress Clientaddress;
    void Send(std::string Data);

    // Callback on incoming packets.
    void onPacket(Networkpacketwrapper &Packet)
    {
        if (Packet.Type != Compiletimehash::FNV1a_32(MODULENAME)) return;
        if (Packet.Origin.Address == Clientaddress.Address) return;

        /* Implement your own handler here using Packet.Payload */
    }

    namespace Internal
    {
    #ifdef _WIN32
    #include <winsock2.h>

        SOCKET Socket{ INVALID_SOCKET };
        bool Initialized{ false };
        uint16_t Port{ 28073 };

        void Send(std::string Data)
        {
            if (!Initialized) return;

            sockaddr_in Address;
            Address.sin_family = AF_INET;
            Address.sin_port = htons(Port);
            Address.sin_addr.s_addr = inet_addr("255.255.255.255");
            sendto(Socket, Data.data(), Data.size(), 0, (sockaddr *)&Address, sizeof(Address));
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
                if (Recvlength == -1) continue;

                if (Recvlength >= sizeof(Networkpacket))
                {
                    Networkpacketwrapper Localpacket;
                    std::memcpy(&Localpacket.Magic, Buffer.get() + 0, sizeof(uint32_t));
                    std::memcpy(&Localpacket.Type, Buffer.get() + sizeof(uint32_t), sizeof(uint32_t));
                    std::memcpy(&Localpacket.Length, Buffer.get() + sizeof(uint32_t) + sizeof(uint32_t), sizeof(uint32_t));

                    Localpacket.Origin.Port = ntohs(From.sin_port);
                    Localpacket.Origin.Address = ntohl(From.sin_addr.s_addr);

                    if (sizeof(Networkpacket) + Localpacket.Length <= uint32_t(Recvlength))
                        std::copy_n(Buffer.get() + sizeof(Networkpacket), Localpacket.Length, std::back_inserter(Localpacket.Payload));

                    onPacket(Localpacket);
                }
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
                auto Buffer = std::make_unique<char[]>(sizeof(Networkpacket));

                VirtualLAN::Send("");
                int Recvlength = recvfrom(Socket, Buffer.get(), sizeof(Networkpacket), 0, (sockaddr *)&Client, &Clientlength);
                if (Recvlength == sizeof(Networkpacket))
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

    void Send(std::string Data)
    {
        Networkpacket Packet;
        Packet.Length = Data.size();
        Packet.Magic = Compiletimehash::FNV1a_32("VLAN");
        Packet.Type = Compiletimehash::FNV1a_32(MODULENAME);

        // Formatted data.
        auto Buffer = std::make_unique<char[]>(sizeof(Networkpacket) + Packet.Length);
        std::memcpy(Buffer.get() + sizeof(Networkpacket), Data.data(), Data.size());
        std::memcpy(Buffer.get(), &Packet, sizeof(Networkpacket));

        Internal::Send({ Buffer.get(), sizeof(Networkpacket) + Packet.Length });
    }

    namespace { struct VLANLoad { VLANLoad() { Internal::Initialize(); std::thread(Internal::Listen).detach(); } }; VLANLoad VLANLoader{}; }
}
