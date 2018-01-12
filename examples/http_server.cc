/* http_server.cc
   Mathieu Stefani, 07 février 2016
   
   Example of an http server
*/
#include <thrift/concurrency/ThreadManager.h>
#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include "DB_Instance.h"
#include <iostream>


using namespace std;
using namespace Pistache;

DB_Instance *_dbInstance;

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


struct PrintException {
    void operator()(std::exception_ptr exc) const {
        try {
            std::rethrow_exception(exc);
        } catch (const std::exception& e) {
            std::cerr << "An exception occured: " << e.what() << std::endl;
        }
    }
};


struct LoadMonitor {
    LoadMonitor(const std::shared_ptr<Http::Endpoint>& endpoint)
        : endpoint_(endpoint)
        , interval(std::chrono::seconds(1))
    { }

    void setInterval(std::chrono::seconds secs) {
        interval = secs;
    }

    void start() {
        shutdown_ = false;
        thread.reset(new std::thread(std::bind(&LoadMonitor::run, this)));
    }

    void shutdown() {
        shutdown_ = true;
    }

    ~LoadMonitor() {
        shutdown_ = true;
        if (thread) thread->join();
    }

private:
    std::shared_ptr<Http::Endpoint> endpoint_;
    std::unique_ptr<std::thread> thread;
    std::chrono::seconds interval;
	
    std::atomic<bool> shutdown_;

    void run() {
        Tcp::Listener::Load old;
        while (!shutdown_) {
            if (!endpoint_->isBound()) continue;

            endpoint_->requestLoad(old).then([&](const Tcp::Listener::Load& load) {
                old = load;

                double global = load.global;
                if (global > 100) global = 100;

                if (global > 1)
                    std::cout << "Global load is " << global << "%" << std::endl;
                else
                    std::cout << "Global load is 0%" << std::endl;
            },
            Async::NoExcept);

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
};


class MyHandler : public Http::Handler {

    HTTP_PROTOTYPE(MyHandler)

    void onRequest(
            const Http::Request& req,
            Http::ResponseWriter response) {

        if (req.resource() == "/ping") {
            if (req.method() == Http::Method::Get) {

                using namespace Http;

                auto query = req.query();
                if (query.has("chunked")) {
                    std::cout << "Using chunked encoding" << std::endl;

                    response.headers()
                        .add<Header::Server>("pistache/0.1")
                        .add<Header::ContentType>(MIME(Text, Plain));

                    response.cookies()
                        .add(Cookie("lang", "en-US"));

                    auto stream = response.stream(Http::Code::Ok);
                    stream << "PO";
                    stream << "NG";
                    stream << ends;
                }
                else {
                    response.send(Http::Code::Ok, "PONG");
                }

            }
        }
        else if (req.resource() == "/get") {
            if (req.method() == Http::Method::Post) {
				string temp;
				_dbInstance->db_get(req.body(), &temp);
                response.send(Http::Code::Ok, temp, MIME(Text, Plain));
            } else {
                response.send(Http::Code::Method_Not_Allowed);
            }
        }
        else if (req.resource() == "/put") {
			if (req.method() == Http::Method::Post) {
				std::string body = req.body();
				std::vector<std::string> temp = split(body, ':');
				_dbInstance->db_put(temp[0], temp[1]);
//				cout << "reached here\n";
				response.send(Http::Code::Ok);
			}
			else {
                response.send(Http::Code::Method_Not_Allowed);
			}
        }
		else if (req.resource() == "/delete") {
			 if (req.method() == Http::Method::Post) {
		//		db.put(Options, req.body());
				response.send(Http::Code::Ok);
			}
			else {
                response.send(Http::Code::Method_Not_Allowed);
			}
		}
/*
        else if (req.resource() == "/timeout") {
            response.timeoutAfter(std::chrono::seconds(2));
        }
        else if (req.resource() == "/static") {
            if (req.method() == Http::Method::Get) {
                Http::serveFile(response, "README.md").then([](ssize_t bytes) {;
                    std::cout << "Sent " << bytes << " bytes" << std::endl;
                }, Async::NoExcept);
            }
        } else {
            response.send(Http::Code::Not_Found);
        }
*/
	
    }

    void onTimeout(const Http::Request& req, Http::ResponseWriter response) {
        response
            .send(Http::Code::Request_Timeout, "Timeout")
            .then([=](ssize_t) { }, PrintException());
    }

};

int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 10;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);
    static constexpr size_t Workers = 4;

		
    
	cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

	_dbInstance = new DB_Instance(true);
	// Backup thread     
//	std::thread t1(dynamic_backup, _dbInstance->db, _dbInstance->timeToWait);

	cout << "Can start operation now\n" << std::flush;	
	
	auto server = std::make_shared<Http::Endpoint>(addr);

    auto opts = Http::Endpoint::options()
        .threads(thr)
        .flags(Tcp::Options::InstallSignalHandler);
    server->init(opts);
    server->setHandler(Http::make_handler<MyHandler>());
    server->serve();

    std::cout << "Shutdowning server" << std::endl;
    server->shutdown();
}
