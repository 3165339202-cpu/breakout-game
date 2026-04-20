#include "NetworkSession.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>

NetworkSession::NetworkSession()
    : sockfd(-1), active(false), isHost(false), hasPeer(false), peerAddr{} {}

NetworkSession::~NetworkSession() {
    Shutdown();
}

bool NetworkSession::EnsureSocket() {
    if (sockfd >= 0) return true;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    return true;
}

bool NetworkSession::StartHost(int port) {
    Shutdown();
    if (!EnsureSocket()) return false;

    sockaddr_in bindAddr{};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port = htons(port);

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) < 0) {
        Shutdown();
        return false;
    }

    active = true;
    isHost = true;
    hasPeer = false;
    return true;
}

bool NetworkSession::StartClient(const std::string& hostIp, int port) {
    Shutdown();
    if (!EnsureSocket()) return false;

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(0);
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) < 0) {
        Shutdown();
        return false;
    }

    peerAddr = {};
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, hostIp.c_str(), &peerAddr.sin_addr) <= 0) {
        Shutdown();
        return false;
    }

    active = true;
    isHost = false;
    hasPeer = true;

    const char* hello = "I|0|0";
    sendto(sockfd, hello, std::strlen(hello), 0, reinterpret_cast<sockaddr*>(&peerAddr), sizeof(peerAddr));

    return true;
}

void NetworkSession::Shutdown() {
    if (sockfd >= 0) {
        close(sockfd);
    }
    sockfd = -1;
    active = false;
    isHost = false;
    hasPeer = false;
    peerAddr = {};
}

bool NetworkSession::ReceiveInput(bool& moveLeft, bool& moveRight) {
    if (!active || sockfd < 0 || !isHost) return false;

    char buffer[2048];
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                reinterpret_cast<sockaddr*>(&from), &fromLen);
    if (received <= 0) return false;

    buffer[received] = '\0';

    if (!hasPeer) {
        peerAddr = from;
        hasPeer = true;
    }

    if (buffer[0] == 'I') {
        int l = 0;
        int r = 0;
        if (std::sscanf(buffer, "I|%d|%d", &l, &r) == 2) {
            moveLeft = (l != 0);
            moveRight = (r != 0);
            return true;
        }
    }
    return false;
}

bool NetworkSession::SendInput(bool moveLeft, bool moveRight) {
    if (!active || sockfd < 0 || isHost || !hasPeer) return false;

    char packet[32];
    std::snprintf(packet, sizeof(packet), "I|%d|%d", moveLeft ? 1 : 0, moveRight ? 1 : 0);
    ssize_t sent = sendto(sockfd, packet, std::strlen(packet), 0,
                          reinterpret_cast<sockaddr*>(&peerAddr), sizeof(peerAddr));
    return sent > 0;
}

bool NetworkSession::SendState(const std::string& state) {
    if (!active || sockfd < 0 || !isHost || !hasPeer) return false;

    std::string packet = "S|" + state;
    ssize_t sent = sendto(sockfd, packet.c_str(), packet.size(), 0,
                          reinterpret_cast<sockaddr*>(&peerAddr), sizeof(peerAddr));
    return sent > 0;
}

bool NetworkSession::ReceiveState(std::string& state) {
    if (!active || sockfd < 0 || isHost) return false;

    char buffer[4096];
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                reinterpret_cast<sockaddr*>(&from), &fromLen);
    if (received <= 0) return false;

    buffer[received] = '\0';
    if (buffer[0] == 'S' && buffer[1] == '|') {
        state = std::string(buffer + 2);
        return true;
    }
    return false;
}
