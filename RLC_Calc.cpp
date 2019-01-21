#include <cmath>
#include <cstdlib>
#include <iostream>
#include </usr/include/stdio.h>
#include <complex>
#include </usr/include/boost/program_options.hpp>
#include <string>
#include <algorithm>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <numeric>
//g++ -L/usr/include/boost/program_options.hpp ./main.cpp -o ./main.o -lboost_program_options
namespace po = boost::program_options;
using namespace std;

const float pi = 3.14159;
const float Vbe= 0.7;
typedef complex<double> dcomp;
const float IMAGINARYNUMBER = sqrt(-1);
const float MILLI = 0.001;
const float MICRO = 0.00001;
const float NANO = 0.00000001;
const float PICO = 0.000000000001;

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
        float resonant_frequency;
        float damping_factor;
        float overdamped;
        float underdamped;
        float q_factor;
        float attenuation;
        float frequency_domain;
        float bandwidth;

        void configuration(){
            current = *voltp / *resp;
            a = *indp * *capp;
            resonant_frequency_hertz = 1/(2 * pi * sqrt(a));
            resonant_frequency_w = sqrt(1/(a));
            cout << "resonant_frequency_hertz:  " << resonant_frequency_hertz <<"\n";
            cout << "resonant_frequency_w:  " << resonant_frequency_w <<"\n";

            if (series = true){

                attenuation = *resp / 2 * *indp;
                resonant_frequency = 1/(sqrt(*indp * *capp));
                damping_factor = ( *resp / 2 ) * (sqrt(*capp * *indp));
                q_factor = (1 / *resp * sqrt( *indp / *capp ));
                bandwidth = 2 * attenuation / resonant_frequency;
                cout << "Attenuation: " << attenuation << "\n";
                cout << "Q Factor: " << q_factor << "\n";
                cout << "Bandwidth: " << bandwidth << "\n";

                *impp = ((pow(resonant_frequency_w , 2) * a - 1)*(-1 * IMAGINARYNUMBER) ) / (resonant_frequency_w * *capp);
                float imp = real(*impp);
                cout << "impedance: " << imp << "\n";
                cout << "reactance: " << *impp << "\n";

                cout << "Damping Factor: " << damping_factor << "\n";
                if (damping_factor < 1) {
                    underdamped = 1;
                    cout << "Underdamped" << "\n";
                } else if (damping_factor > 1) {
                    overdamped = 1;
                    cout << "Overdamped " << "\n";
                };

            } else if (series = false) {

                attenuation = 1 / ( 2 * *resp * *capp );
                damping_factor = ( 1 / ( 2 * *resp )) * sqrt( *indp / *capp );
                q_factor = *resp * sqrt( *capp / *indp);
                bandwidth = (1/ *resp) * sqrt(*indp / *capp);
                frequency_domain = 1/( IMAGINARYNUMBER * resonant_frequency * *indp) + IMAGINARYNUMBER * resonant_frequency * *capp + 1 / *resp ;
                cout << "Attenuation: " << attenuation << "\n";
                cout << "Q Factor: " << q_factor << "\n";
                cout << "Bandwidth: " << bandwidth << "\n";
                cout << "Frequency Domain: " << frequency_domain << "\n";

                *impp = ((-1 * IMAGINARYNUMBER) * resonant_frequency_w * *indp)/ (pow(resonant_frequency_w , 2) * a - 1);
                float imp = real(*impp);
                cout << "impedance: " << imp << "\n";
                cout << "reactance: " << *impp << "\n";

                cout << "Damping Factor: " << damping_factor << "\n";
                if (damping_factor < 1){
                    underdamped = 1;
                } else if (damping_factor > 1){
                    overdamped = 1;
                } else {
                    cout << "you managed to make a number that is neither greater than or less than or even equal to 1 ... GOOD JOB!\n";
                };
            };
        };
};


int main(int argc, char* argv[]) {

    LC_circuit lc_circuit;
    po::options_description desc("RLC circuit calculator, gives the resonant frequency and other important stuff \n number first, then unit of measure. Example: \"--inductance 100 milli\" for 100 millihenries");
    desc.add_options()
        ("help,H", "Print Help Message" )
        ("voltage,V", po::value<float>()->default_value(3.0), "Voltage of the source" )
        ("inductance,I", po::value< std::string >()->multitoken(), "Inductance of the inductor (milli, micro, nano, pico)" )
        ("capacitance,C", po::value< std::string >()->multitoken(), "Capacitance of the Capacitor (milli, micro, nano, pico)" )
        ("resistance,R", po::value<float>()->default_value(100), "Resistance of the resistor in ohms ONLY, Defaults to \"100 ohm\"")
        ("series,S", po::value< vector<string> >(), "\"true\" if series, \"false\" if Parallel, DEFAULTS TO FALSE" )
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

  try {
    std::string string_input0 = arguments["capacitance"].as< std::string >();
    //std::string capacitance_args = boost::algorithm::join(split_input0);
    std::vector<string> capacitance_args;
    cout << string_input0 << "Farads \n";
    boost::split(capacitance_args , string_input0 , boost::is_any_of(" "));

    if (capacitance_args[1] == "nano") {
        float cap = std::stoi(capacitance_args[0]) * NANO;
        lc_circuit.capacitance = cap;
    } else if (capacitance_args[1] == "micro") {
        float cap = std::stoi(capacitance_args[0]) * MICRO;
        lc_circuit.capacitance = cap;
    } else if (capacitance_args[1] == "pico") {
        float cap = std::stoi(capacitance_args[0]) * PICO;
        lc_circuit.capacitance = cap;
    };

  } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
  };



  try {

    std::string string_input1 = arguments["inductance"].as< std::string >();
    cout << string_input1 << " Henries \n" ;
    //std::string inductance_args = boost::algorithm::join(split_input1);
    //accumulate(begin(split_input1), end(split_input1), inductance_args);
    std::vector<string> inductance_args;
    boost::split(inductance_args , string_input1 , boost::is_any_of(" "));

    if (inductance_args[1] == "nano") {
        float ind = std::stoi(inductance_args[0]) * NANO;
        lc_circuit.inductance = ind;
        cout << "induct: " << ind  << "\n";
    } else if (inductance_args[1] == "micro") {
        float ind = std::stoi(inductance_args[0]) * MICRO;
        lc_circuit.inductance = std::stoi(inductance_args[0]) * MICRO;
        cout << "induct: " << ind << "\n";
    } else if (inductance_args[1] == "pico") {
        float ind = std::stoi(inductance_args[0]) * PICO;
        lc_circuit.inductance = ind;
        cout <<  "induct: " << ind << "\n";
    };


  } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
  };
    lc_circuit.resistance = arguments["resistance"].as<float>();
    //lc_circuit.capacitance = arguments["capacitance"].as< vector<std:string> >();
    //lc_circuit.inductance = arguments["inductance"].as< vector<std:string> >();
    lc_circuit.voltage = arguments["voltage"].as<float>();
    lc_circuit.configuration();
    //cout << "ARGS RESIST " << arguments["resistance"][1];
    cout << "Resonating Freq: " << lc_circuit.resonant_frequency_hertz << "\n";
    cout << "current: " << lc_circuit.current << "\n";    //cout <<  << "\n";
};
