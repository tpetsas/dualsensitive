
// udp.h
#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <mutex>

/**
 * Defines the function pointer type used for receiving raw UDP payloads.
 */
using CallbackFunc = void(*)(const std::vector<uint8_t>& payload);



namespace udp {

  enum class Status {
        Success,
        WSAStartupFailed,
        SocketCreationFailed,
        BindFailed,
        SendFailed,
        CallbackNotProvided,
        ServerAlreadyRunning,
        ClientAlreadyRunning,
        NotInitialized
    };


    /**
     * Starts a UDP server on the specified port and sets a callback to
     * handle incoming packets.
     *
     * @param serverPort The port to listen on.
     * @param callback   A function that receives the raw byte payload of each packet.
     * @return Status::Success if the server started successfully.
     *         Status::WSAStartupFailed if Winsock initialization failed.
     *         Status::SocketCreationFailed if the server socket could not be created.
     *         Status::BindFailed if binding the socket to the port failed.
     *         Status::CallbackNotProvided if no callback function was supplied.
     *         Status::ServerAlreadyRunning if the server is already running.
     */
    Status startServer(uint16_t serverPort, CallbackFunc callback);

    /**
     * Starts the UDP client and initializes the destination address for sending packets.
     *
     * @param serverPort      The destination server's port.
     * @return Status::Success if initialized successfully.
     *         Status::SocketCreationFailed if the client socket couldn't be created.
     *         Status::ClientAlreadyRunning if already initialized.
     */
    Status startClient(uint16_t serverPort);

    /**
     * Sends a UDP packet to the pre-initialized server from startClient().
     *
     * @param payload A vector containing the raw data to send.
     * @return Status::Success if the packet was sent successfully.
     *         Status::NotInitialized if the client socket or address is not set.
     *         Status::SendFailed if the send operation failed.
     */
    Status send(const std::vector<uint8_t>& payload);

    /**
     * Stops the currently running UDP server.
     * Has no effect if the server is not running.
     */
    void stopServer();

    /**
     * Stops the UDP client and closes the client socket if open.
     */
    void stopClient();
}
