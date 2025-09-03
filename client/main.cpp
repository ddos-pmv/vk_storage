#include <iostream>
#include "client.h"

int main() {
    try {
        Client client;
        client.run_cli();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
