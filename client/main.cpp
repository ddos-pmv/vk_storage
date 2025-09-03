#include <boost/asio.hpp>

using namespace boost::asio;

int main ()
{
    io_service service;
    ip::tcp::endpoint( boost::asio::ip::address::from_string( "127.0.0.1" ), 8080 );


    return 0;
}
