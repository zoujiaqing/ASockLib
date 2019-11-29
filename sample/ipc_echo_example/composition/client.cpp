#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <cassert>
#include "ASock.hpp"
#include "../../msg_defines.h"

///////////////////////////////////////////////////////////////////////////////
class EchoClient 
{
    public:
        bool initialize_ipc_client(const char* ipc_sock_path);
        bool send_to_server(const char* data, int len);
        bool is_connected() { return ipc_client_.is_connected();}
        std::string  GetLastErrMsg(){return  ipc_client_.GetLastErrMsg() ; }

    private:
        ASock   ipc_client_ ; //composite usage
        size_t  on_calculate_data_len(asock::Context* context_ptr); 
        bool    on_recved_complete_data(asock::Context* context_ptr, char* data_ptr, int len); 
        void    on_disconnected_from_server() ; 
};

///////////////////////////////////////////////////////////////////////////////
bool EchoClient::initialize_ipc_client(const char* ipc_sock_path)
{
    //register callbacks
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    ipc_client_.set_cb_on_calculate_packet_len(std::bind(
                       &EchoClient::on_calculate_data_len, this, _1));

    ipc_client_.set_cb_on_recved_complete_packet(std::bind(
                       &EchoClient::on_recved_complete_data, this, _1,_2,_3));

    ipc_client_.set_cb_on_disconnected_from_server(std::bind(
                       &EchoClient::on_disconnected_from_server, this));

    //connect timeout is 10 secs, max message length is approximately 1024 bytes...
    if(!ipc_client_.init_ipc_client(ipc_sock_path) )
    {
        std::cerr <<"["<< __func__ <<"-"<<__LINE__ 
                  <<"] error! "<< ipc_client_.GetLastErrMsg() <<"\n"; 
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
size_t EchoClient::on_calculate_data_len(asock::Context* context_ptr)
{
    //user specific : calculate your complete packet length 
    if( context_ptr->recv_buffer.GetCumulatedLen() < (int)CHAT_HEADER_SIZE )
    {
        return asock::MORE_TO_COME ; //more to come 
    }

    ST_MY_HEADER header ;
    context_ptr->recv_buffer.PeekData(CHAT_HEADER_SIZE, (char*)&header);  
    size_t supposed_total_len = std::atoi(header.msg_len) + CHAT_HEADER_SIZE;
    assert(supposed_total_len<=context_ptr->recv_buffer.GetCapacity());
    return supposed_total_len ;
}

///////////////////////////////////////////////////////////////////////////////
bool EchoClient:: on_recved_complete_data(asock::Context* context_ptr, 
                                          char*           data_ptr, 
                                          int             len) 
{
    //user specific : your whole data has arrived.
    char packet[asock::DEFAULT_PACKET_SIZE]; 
    memcpy(&packet, data_ptr+CHAT_HEADER_SIZE, len-CHAT_HEADER_SIZE);
    packet[len-CHAT_HEADER_SIZE] = '\0';
    
    std::cout << "* server response ["<< packet <<"]\n";
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool EchoClient:: send_to_server(const char* data, int len)
{
    return ipc_client_.send_to_server(data, len);
}

///////////////////////////////////////////////////////////////////////////////
void EchoClient::on_disconnected_from_server() 
{
    std::cout << "* server disconnected ! \n";
	exit(1);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    if(argc !=2)
    {
        std::cout << "usage : " << argv[0] << " ipc_socket_full_path \n\n";
        return 1;
    }

    EchoClient client;
    client.initialize_ipc_client(argv[1]);

    std::string user_msg  {""}; 
    while( client.is_connected() )
    {
        std::cin.clear();
        getline(std::cin, user_msg); 
        int msg_len = user_msg.length();

        if(msg_len>0)
        {
            ST_MY_HEADER header;
            snprintf(header.msg_len, sizeof(header.msg_len), "%d", msg_len );

            //you don't need to send twice like this..
            if(! client.send_to_server( reinterpret_cast<char*>(&header), 
                                      sizeof(ST_MY_HEADER)) )
            {
                std::cerr <<"["<< __func__ <<"-"<<__LINE__ <<"] error! "
                          << client.GetLastErrMsg() <<"\n"; 
                return 1;
            }
            if(! client.send_to_server(user_msg.c_str(), msg_len) )
            {
                std::cerr <<"["<< __func__ <<"-"<<__LINE__ <<"] error! "
                          << client.GetLastErrMsg() <<"\n"; 
                return 1;
            }
        }
    }
    std::cout << "client exit...\n";
    return 0;
}

