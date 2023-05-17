#pragma once

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <iostream>
#include <functional>
#include <codecvt>
#include <locale>
#include <memory>
#include <pthread.h>

#include "jsonparser.hpp"


typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef client::connection_ptr Connection_ptr;

class WebSocketClient {
public:
    using Client = websocketpp::client<websocketpp::config::asio_client>;

    WebSocketClient() {
        m_parser =  make_shared<JsonWrapper::Parser>();
        client_.init_asio();
        //绑定回调函数
        client_.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        client_.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
        client_.set_fail_handler(std::bind(&WebSocketClient::on_fail,this, std::placeholders::_1));
        client_.set_open_handler(std::bind(&WebSocketClient::on_open,this, std::placeholders::_1));
    }
    ~WebSocketClient(){
	connection_ptr = nullptr;
	if (client_thread_.joinable()) {
	    //client_thread_.join();
	    pthread_cancel(client_thread_.native_handle());// stop client thread
	}
    }

    void connect(const std::string& uri) {
        websocketpp::lib::error_code ec;
        Connection_ptr connection = client_.get_connection(uri, ec);

        if (ec) {
            //连接存在问题，直接返回
            // std::cout << "Could not create connection because: " << ec.message() << std::endl;
            return;
        }
        //开始连接ws server
        client_.connect(connection);
        set_connection_ptr(connection);
        client_thread_ = std::thread(&Client::run, &client_);
	client_thread_.detach();
        m_url = uri;
    }
	

    void send_message(const std::string & message) {
        if (is_connected) {
            client_.send(get_connection(),message, websocketpp::frame::opcode::text);
        } else { 
            return;
	}
    }

    void disconnect(websocketpp::close::status::value code, const std::string& reason) {
        client_.close(get_connection(), code, reason);
    }

private:
    Client client_;
    std::thread client_thread_;
    std::string m_url;
    Connection_ptr connection_ptr;
    std::shared_ptr<JsonWrapper::Parser> m_parser  ;
    bool is_connected = false;

    // 设置通信句柄
    void set_connection_ptr(Connection_ptr _ptr){
        connection_ptr = _ptr;
    }

    // 获取通信句柄
    Connection_ptr get_connection(){
        return connection_ptr;
    }

    //收到消息的回调函数
    void on_message(websocketpp::connection_hdl hdl, Client::message_ptr msg) {
        if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            std::string payload =  msg->get_payload();
            m_parser->ParseData(payload);//当前只对json 数据进行解析处理
        }else if (msg->get_opcode() == websocketpp::frame::opcode::binary) {

            //std::cout << "Received message:other "<< msg->get_payload()  << std::endl;
            //std::cout << "Received message:other "<< websocketpp::utility::to_hex(msg->get_payload())  << std::endl;
        } else {
           // std::cout << "Received message:else "<<msg->get_payload();
        }

    }

    void on_close(websocketpp::connection_hdl hdl) {
        //Connection closed.
        is_connected = false;
    }

    void on_fail(websocketpp::connection_hdl hdl) {
        // Handle failure here
        is_connected = false;
    }
    void on_open(websocketpp::connection_hdl hdl) {
        // Connection succeed.
        is_connected = true;
    }
};


