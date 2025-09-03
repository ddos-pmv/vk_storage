#include <boost/program_options.hpp>
#include <iostream>

#include "node.h"

int main(int argc, char *argv[]) {
    try {
        uint16_t port = 2222;

        boost::program_options::options_description desc("Node options");
        desc.add_options()("help,h", "Show help message")(
                "port,p", boost::program_options::value<uint16_t>(&port)->default_value(2222), "Port to listen on");

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        Node<> node(port);
        node.run();

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
