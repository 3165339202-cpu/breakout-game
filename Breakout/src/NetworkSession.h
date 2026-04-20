#pragma once

#include <string>
#include <netinet/in.h>

class NetworkSession {
private:
    int sockfd;
    bool active;
    bool isHost;
    bool hasPeer;
    sockaddr_in peerAddr;

    bool EnsureSocket();

public:
    NetworkSession();
    ~NetworkSession();

    bool StartHost(int port);
    bool StartClient(const std::string& hostIp, int port);
    void Shutdown();

    bool IsActive() const { return active; }
    bool IsHost() const { return isHost; }
    bool HasPeer() const { return hasPeer; }

    bool ReceiveInput(bool& moveLeft, bool& moveRight);
    bool SendInput(bool moveLeft, bool moveRight);

    bool SendState(const std::string& state);
    bool ReceiveState(std::string& state);
};
