/*
    udp.cpp is part of DualSensitive
    https://github.com/tpetsas/dualsensitive

    Contributors of this file:
    05.2025 Thanasis Petsas

    Licensed under the MIT License
*/


// Handles UDP-based communication between DualSensitive client and server modes.
// This module provides functionality for:
// - Spinning up a UDP server to receive trigger requests
// - Sending UDP packets to a remote server (client mode)

#include <udp.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <iostream>
#include <iomanip>
#pragma comment(lib, "ws2_32.lib")

#define MAX_PAYLOAD_SIZE 1024

// TODO:
// * add logging
// * use WSAGetLastError() to reserverPort the exact errors related to winsock API

static SOCKET serverSocket = INVALID_SOCKET;
static SOCKET clientSocket = INVALID_SOCKET;
static sockaddr_in serverAddress; // cached server address used by the client
static std::thread serverThread;
static std::atomic<bool> serverRunning = false;
static CallbackFunc packetHandler = nullptr;
static std::mutex initMutex;
static WSAEVENT serverEvent = WSA_INVALID_EVENT;

namespace udp {

    // Launches a background thread to run the UDP server
    Status startServer(uint16_t serverPort, CallbackFunc callback) {
        if (!callback) {
            //_LOG("udp::startServer - callback not set!");
            return Status::CallbackNotProvided;
        }

        std::lock_guard<std::mutex> lock(initMutex);

        if (serverRunning) return Status::ServerAlreadyRunning;


        packetHandler = callback;

        WSADATA wsaData;

        // use version 2.2 of the Winsock API
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return Status::WSAStartupFailed;
        }

        serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET) {
            WSACleanup();
            return Status::SocketCreationFailed;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        std::cout << "[UDP Server] Starting on port: " << serverPort << std::endl;

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &serverAddr.sin_addr, ipStr, sizeof(ipStr));
        std::cout << "[UDP Server] Binding to " << ipStr << ":" << ntohs(serverAddr.sin_port) << std::endl;


        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            return Status::BindFailed;
        }

        // add event handle to wake up recvfrom() when either:
        // - data arrives (default behaviof)
        // - the socket is closed
        serverEvent = WSACreateEvent();
        WSAEventSelect(serverSocket, serverEvent, FD_READ | FD_CLOSE);

        serverRunning = true;
        serverThread = std::thread([]() {
            char buffer[MAX_PAYLOAD_SIZE];
            sockaddr_in clientAddr{};
            int clientLen = sizeof(clientAddr);
            while (serverRunning) {
                DWORD waitResult = WSAWaitForMultipleEvents(1, &serverEvent, FALSE, 1000, FALSE);
                if (waitResult == WSA_WAIT_FAILED) break;
                if (waitResult == WSA_WAIT_TIMEOUT) continue;

                WSANETWORKEVENTS networkEvents;
                if (WSAEnumNetworkEvents(serverSocket, serverEvent, &networkEvents) == SOCKET_ERROR) break;

                if (networkEvents.lNetworkEvents & FD_READ) {
                    int recvLen = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                                           (sockaddr*)&clientAddr, &clientLen);
                    if (recvLen == SOCKET_ERROR) {
                        std::cerr << "recvfrom error: " << WSAGetLastError() << std::endl;
                    }
                    std::cout << "[UDP Server] Received packet of size " << recvLen << std::endl;
                    if (recvLen > 0 && packetHandler) {
                        std::vector<uint8_t> payload(buffer, buffer + recvLen);
                        packetHandler(payload);
                    }
                }
                if (networkEvents.lNetworkEvents & FD_CLOSE) {
                    break;
                }
            }
        });

        return Status::Success;
    }

    // Gracefully shuts down the UDP server

    void stopServer() {
        std::lock_guard<std::mutex> lock(initMutex);

        if (!serverRunning) return;
        serverRunning = false;

        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;

        // Close the associated WSA event (important to unblock wait)
        if (serverEvent != WSA_INVALID_EVENT) {
            WSASetEvent(serverEvent); // Wake the thread NOW
            WSACloseEvent(serverEvent);
            serverEvent = WSA_INVALID_EVENT;
        }

        if (serverThread.joinable()) {
            serverThread.join();
        }

        WSACleanup();
    }

    Status startClient(uint16_t serverPort) {
        std::lock_guard<std::mutex> lock(initMutex);
        if (clientSocket != INVALID_SOCKET)
            return Status::ClientAlreadyRunning;

        WSADATA wsaData;

        // use version 2.2 of the Winsock API
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return Status::WSAStartupFailed;
        }

        clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#if 1

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return Status::SocketCreationFailed;
        }
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);
        //inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
        serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
            return Status::Success;
        }

        // Sends a trigger payload to a remote UDP server
    Status send(const std::vector<uint8_t>& payload) {
        if (clientSocket == INVALID_SOCKET)
            return Status::NotInitialized;
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &serverAddress.sin_addr, ipStr, sizeof(ipStr));
        std::cerr << "Sending to " << ipStr << ":" << ntohs(serverAddress.sin_port)
                  << " - payload size: " << payload.size() << std::endl;
        int result = sendto(clientSocket,
                reinterpret_cast<const char*>(payload.data()),
                static_cast<int>(payload.size()),
                0,
                reinterpret_cast<sockaddr*>(&serverAddress),
                sizeof(serverAddress)
        );

        if (result == SOCKET_ERROR) {
            std::cerr << "error: " << WSAGetLastError() << std::endl;
            return Status::SendFailed;
        }

        std::cerr << "payload: ";
        for (uint8_t byte : payload) {
            std::cerr << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
        }
        std::cerr << "payload (size: " << std::dec << payload.size() << " sent successfully" << std::endl;

        return Status::Success;
    }

    void stopClient() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
    }
}
