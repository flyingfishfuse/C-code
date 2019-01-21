#include <cmath>
#include <iostream>
#include </usr/include/stdio.h>
#include <complex>
#include </usr/include/boost/program_options.hpp>
//g++ -L/usr/include/boost/program_options.hpp ./main.cpp -o ./main.o -lboost_program_options
namespace po = boost::program_options;
using namespace std;

const float pi = 3.14159;
const float Vbe= 0.7;
typedef complex<double> dcomp;
const float IMAGINARYNUMBER = sqrt(-1);
const float milli = 0.001;
const float micro = 0.00001;
const float nano = 0.00000001;
const float pico = 0.000000000001;

class LC_circuit{
    public:
        float impedance;
        float *impp = &impedance;
        float resistance;
        float *resp = &resistance;
        float inductance;
        float *indp = &inductance;
        float capacitance;
        float *capp = &capacitance;
        float voltage;
        float *voltp = &voltage;

        bool series;
        float current;
        float a;
        float loss;
        float resonant_frequency_hertz;
        float resonant_frequency_w;

        void configuration(){
            current = *voltp / *resp;
            a = *indp * *capp;
            resonant_frequency_hertz = 1/(2 * pi * sqrt(a));
            resonant_frequency_w = sqrt(1/(a));
            cout << "impp: " << *impp << "\n";
            cout << "capp : " << *capp << "\n";
            cout << "indp:  " << *indp << "\n";
            cout << "resp:  " << *resp << "\n";
            cout << "voltp  " << *voltp << "\n";
            cout << "current" << current << "\n";
            cout << "a=indp*capp:" << a <<"\n";
            cout << "loss=sqrt(*voltp) / current: " << loss <<"\n";
            cout << "resonant_frequency_hertz = 1/(2 * pi * sqrt(a))" << resonant_frequency_hertz <<"\n";
            cout << "resonant_frequency_w = sqrt(1/(a)" << resonant_frequency_w <<"\n";

            if (series){
                *impp = ((pow(resonant_frequency_w , 2) * a - 1)*(-1 * IMAGINARYNUMBER) ) / (resonant_frequency_w * *capp);

            } else {
                *impp = ((-1 * IMAGINARYNUMBER) * resonant_frequency_w * *indp)/ (pow(resonant_frequency_w , 2) * a - 1);
                cout << "impedance: " << *impp << "\n";
            };
        };
};


int main(int argc, char* argv[]) {
    LC_circuit lc_circuit;
    po::options_description desc("RLC circuit calculator, gives the resonant frequency and other important stuff \n number first, then unit of measure. Example: \"--inductance 100 milli\" for 100 millihenries");
    desc.add_options()
        ("help,H", "Print Help Message" )
        ("voltage,V", po::value<float>()->default_value(1), "Voltage of the source" )
        ("inductance,I", po::value<float>()->default_value(1), "Inductance of the inductor" )
        ("capacitance,C", po::value<float>()->default_value(1), "Capacitance of the Capacitor" )
        ("resistance,R", po::value<float>()->default_value(1), "Resistance of the resistor" )
        ("series,S", po::value<vector<string>>(), "\"true\" if series, \"false\" if Parallel, DEFAULTS TO FALSE" )
    ;
    po::variables_map arguments;
    po:store(po::parse_command_line(argc , argv, desc), arguments);
    try {
        po::notify(arguments);
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    };
    if (arguments.count("help")) {
        cout << desc << "\n";
        return 1;
    };
    if (arguments.count("series") == 'true' ) {
        lc_circuit.series = true;
    } else {
        lc_circuit.series = false;
    };
    lc_circuit.resistance = arguments["resistance"].as<float>();
    lc_circuit.capacitance = arguments["capacitance"].as<float>();
    lc_circuit.inductance = arguments["inductance"].as<float>();
    lc_circuit.voltage = arguments["voltage"].as<float>();
    lc_circuit.configuration();
    cout << "Resonating Freq: " << lc_circuit.resonant_frequency_hertz << "\n";
    cout << "Impedance: " << lc_circuit.impedance << "\n";
    cout << "current: " << lc_circuit.current << "\n";    //cout <<  << "\n";
};


