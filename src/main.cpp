#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/URI.h>
#include <regex>
#include "db/Database.h"
#include "handlers/AuthHandler.h"
#include "handlers/UserHandler.h"
#include "handlers/EventHandler.h"

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& resp) override {
        resp.setContentType("application/json");
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        resp.send() << "{\"error\":\"not found\"}";
    }
};

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& req) override {
        std::string path = Poco::URI(req.getURI()).getPath();

        if (path == "/auth/login") return new LoginHandler();
        if (path == "/auth/logout") return new LogoutHandler();
        if (path == "/users" && req.getMethod() == "POST") return new CreateUserHandler();
        if (path == "/users/search") return new SearchUsersByNameHandler();

        std::regex userLoginRe("^/users/([^/]+)$");
        std::smatch m;
        if (std::regex_match(path, m, userLoginRe))
            return new GetUserByLoginHandler(m[1].str());

        if (path == "/events") {
            if (req.getMethod() == "POST") return new CreateEventHandler();
            if (req.getMethod() == "GET") return new GetEventsHandler();
        }

        return new NotFoundHandler();
    }
};

class App : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>&) override {
        Database::instance().init();

        Poco::Net::ServerSocket svs(8080);
        Poco::Net::HTTPServer srv(new RequestHandlerFactory(), svs, new Poco::Net::HTTPServerParams());
        srv.start();
        waitForTerminationRequest();
        srv.stop();
        return Application::EXIT_OK;
    }
};

int main(int argc, char** argv) {
    App app;
    return app.run(argc, argv);
}
