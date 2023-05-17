#ifndef PARSE_H
#define PARSE_H

#include <unordered_map>
#include <string>
#include "json.hpp"
#include "base64.hpp"

using namespace  nlohmann;

namespace  JsonWrapper{


class Parser
{
public:
    Parser(){
        InitOpcodeHandle();
    };
    virtual ~Parser(){

    };
    //解析websocket的发来的json数据
    void ParseData(const std::string & data){
        if (data.empty()) {
          return ;
        }

        json doc = json::parse(data);
        string str_opcode, str_topic, str_service;

        str_opcode =  doc.at("op");
        if (str_opcode.empty())
            return;

        if ("publish" == str_opcode) {
          str_topic = doc.at("topic");
          json msg_val = doc.at("msg");

          (this->*m_opcode_mapdict[str_topic])(msg_val);//对不同话题的处理函数
        } else if("service_response" == str_opcode){
           str_service = doc.at("service");
          (this->*m_opcode_mapdict[str_service])(doc);//对不同服务的处理函数
        }

    };


private:
  void InitOpcodeHandle(){
      //map三种插入数据的方式 ["topic",func]
      m_opcode_mapdict.insert(pair<std::string, OpcodeFunc>("/odom_raw",&Parser::OdomHandle));
      m_opcode_mapdict["/slam_status"]  = &Parser::SlamStatusHandle;
      m_opcode_mapdict["/input/op"]  = &Parser::InputopHandle;
      m_opcode_mapdict["/points_raw"]  = &Parser::PointsRawHandle;
  };
  //odom_raw的回调函数
  bool OdomHandle(const json &data_val)
  {
      return  true;
  };

  //slam_status的回调函数
  bool SlamStatusHandle(const json &data_val)
  {
      return  true;
  };

  //input/op的回调函数
  bool InputopHandle(const json &data_val)
  {
      return  true;
  }

  
  bool PointsRawHandle(const json &data_val)
  {
      
      std::string base64_str = data_val.at("data"); 

      size_t decoded_length = 0;
      // base64_ 解码成uint8_t 数组
      uint8_t *data = base64_decode(base64_str.c_str(), decoded_length);
      
      //得到原始的data 数组
      /*
      for (size_t i = 0; i < decoded_length; ++i)
      {
        std::cout<<static_cast<int>(data[i])<< " ";
      }
      */
      delete data;
      return  true;
  }
private:
  typedef bool(Parser::*OpcodeFunc)(const json & val);

  //存储话题及其对应的处理函数。
  std::unordered_map<std::string, OpcodeFunc> m_opcode_mapdict;


};
}
#endif // PARSE_H
