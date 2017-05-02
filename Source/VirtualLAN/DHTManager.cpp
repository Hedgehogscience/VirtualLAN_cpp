/*
    Initial author: Convery
    Started: 2017-5-2
    License: Apache 2.0
*/

#include "../StdInclude.h"
#include <unordered_map>
#include <queue>

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/add_torrent_params.hpp>

namespace DHTManager
{
    // Magnet queue.
    std::queue<std::string> Magnetqueue;

    // Connected peers.
    std::mutex Mapguard;
    std::unordered_map<std::string /* IP */, bool /* Connected */> DHTConnections;

    // Cleanup thread for the connections map.
    void Cleanup()
    {
        while (true)
        {
            // We delay the cleanup in case someone drops for a minute.
            std::this_thread::sleep_for(std::chrono::seconds(1));
            Mapguard.lock();
            {
                std::vector<std::string> Purgelist;
                printf("Connected to: %i peers\n", DHTConnections.size());

                for (auto &Item : DHTConnections)
                    if (!Item.second)
                        Purgelist.push_back(Item.first);

                for (auto &Item : Purgelist)
                    DHTConnections.erase(Item);
            }
            Mapguard.unlock();
        }
    }

    // Find peers.
    void Listen()
    {
        libtorrent::session Session;
        libtorrent::settings_pack Pack;
        Pack.set_int(libtorrent::settings_pack::alert_mask, libtorrent::alert::peer_notification);

        while (true)
        {
            if (!Magnetqueue.empty())
            {
                libtorrent::add_torrent_params Params;
                Params.url = Magnetqueue.front();
                Params.max_connections = -1;
                Params.download_limit = 1;
                Params.upload_limit = 1;
                Params.save_path = ".";
                Magnetqueue.pop();

                Session.async_add_torrent(Params);
            }

            std::vector<libtorrent::alert *> Alerts;
            Session.pop_alerts(&Alerts);

            for (auto Item : Alerts)
            {
                if (auto Connected = libtorrent::alert_cast<libtorrent::peer_connect_alert>(Item))
                {
                    Mapguard.lock();
                    {
                        DHTConnections[Connected->ip.address().to_string()] = true;
                    }
                    Mapguard.unlock();

                    DebugPrint(va("IP %s connected!\n", Connected->ip.address().to_string().c_str()).c_str());
                }
                if (auto Disconnected = libtorrent::alert_cast<libtorrent::peer_disconnected_alert>(Item))
                {
                    Mapguard.lock();
                    {
                        DHTConnections[Disconnected->ip.address().to_string()] = false;
                    }
                    Mapguard.unlock();

                    DebugPrint(va("IP %s disconnected!\n", Disconnected->ip.address().to_string().c_str()).c_str());
                }
            }
        }
    }

    namespace { struct DHTLoader { DHTLoader() { std::thread(Cleanup).detach(); std::thread(Listen).detach(); } }; DHTLoader Loader{}; }
}
