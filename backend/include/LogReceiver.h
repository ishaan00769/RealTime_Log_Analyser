#ifndef LOG_RECEIVER_H
#define LOG_RECEIVER_H

#include <string>
#include <functional>

class LogReceiver {
private:
    std::string host;
    int port;
    std::function<void(const std::string&)> logCallback;

public:
    // Initialize server with host, port, and a callback function for processing logs
    LogReceiver(const std::string& serverHost, int serverPort, std::function<void(const std::string&)> callback);

    // Start listening for incoming HTTP requests (blocks the thread)
    void start();
};

#endif // LOG_RECEIVER_H