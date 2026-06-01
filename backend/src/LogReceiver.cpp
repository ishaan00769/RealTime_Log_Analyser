#include "LogReceiver.h"
//#define CPPHTTPLIB_OPENSSL_SUPPORT // Omit if not using HTTPS
#include "httplib.h"
#include <iostream>

LogReceiver::LogReceiver(const std::string& serverHost, int serverPort, std::function<void(const std::string&)> callback)
    : host(serverHost), port(serverPort), logCallback(callback) {}

void LogReceiver::start() {
    httplib::Server svr;

    // Define the POST endpoint for receiving streaming json logs
    svr.Post("/logs", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.body.empty()) {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Empty body\"}", "application/json");
            return;
        }

        try {
            // Forward the payload to our processing pipeline
            logCallback(req.body);
            
            res.status = 200;
            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        }
    });

    std::cout << "[HTTP Server] Listening on http://" << host << ":" << port << "/logs\n";
    svr.listen(host.c_str(), port);
}