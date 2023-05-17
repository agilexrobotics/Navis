#include <iostream>
#include <thread>
#include <mutex>

#include <functional>
#include <condition_variable>
#include <atomic>
#include <vector>

#include "curl_http_client.hpp"
#include "ws_client.hpp"
#include "subscribe_json.h"

mutex mut;
condition_variable cond;
bool is_open_heart_data = true;
std::vector<MPosePosition> m_pose_list;
Mapinfo map_info;

// 位姿点数组转换成json 数据，用于发送实时的点/线导航任务
void MPositionToJson(std::vector<MPosePosition> _list, json &obj)
{
    if (_list.empty()){
        return ;
    }

    for (size_t i = 0; i < _list.size(); ++i) {
        json point = {
            {
                "position",{
                    {"x", _list[i].pose_x},
                    {"y", _list[i].pose_y},
                    {"theta", _list[i].pose_theta}
                }
            },
            {"isNew", _list[i].is_new},
            {"cpx", _list[i].pose_cpx},
            {"cpy",_list[i].pose_cpy}

        };
        obj.push_back(point);
    }
}

// 将获取到图片json数据解析还原成 map_info 结构体
void jsonToMapinfo( const std::string &res, Mapinfo & map_info)
{

    json data = json::parse(res);
    json  j_map_info = data.at("data");
    map_info.gridWidth = j_map_info["mapInfo"].at("gridWidth");
    map_info.gridHeight = j_map_info["mapInfo"].at("gridHeight");
    map_info.originX = j_map_info["mapInfo"].at("originX");
    map_info.originY = j_map_info["mapInfo"].at("originY");
    map_info.resolution = j_map_info["mapInfo"].at("resolution");
//    map_info.task_list = j_map_info["mapInfo"].at("tasks");

}


/**
 * @brief Set the real time task object 设置 实时的点/线导航任务
 * 
 * @param mode 设置模式"point", "path"
 * @param points_array 
 * @param loop_time >=0, 循环次数，loop_time = 0 是无线循环，
 * @return ** std::string 
 */
std::string  set_real_time_task(const::string & mode, const json * points_array,  int loop_time){

    json real_time_task = {
        {"loopTime",loop_time},
        { "mode", mode },
        { "points",*points_array}
    };

    return real_time_task.dump();

}

/**
 * @brief Set the initial pos object
 * 
 * @param pos_ 初始点的位置
 * @param angle ，初始点的角度
 * @return ** std::string 
 */
std::string  set_initial_pos(const MPosition & pos_, double angle)
{
    MQuaternion q;

    quaternion_from_euler(0, 0, angle,q);
    
    json initialpose = {
        {"op", "publish"},
        {"topic", "/initialpose"},
        {"msg", {
            {"header",{
                { "frame_id","map"}
            }},
            {"pose",{
                
                  {"pose",{

                    { "position",{

                       { "x",pos_.x},
                       { "y",pos_.y},
                       { "z",0}}} ,
                    { "orientation",{

                       { "x",q.x},
                       { "y",q.y},
                       { "z",q.z},
                       { "w",q.w} }}
                    }},
              
                  { "covariance",
                      {                  
                          0,0,0,0,0,0,0,0,0,0,0,0,
                          0,0,0,0,0,0,0,0,0,0,0,0,
                          0,0,0,0,0,0,0,0,0,0,0,0
                      }
                  }
                }
             } 
           }
        }
    };

    return initialpose.dump();

}


void ws_client()
{
    //1.实例化 websocket 客户端
    WebSocketClient ws_client; 
    // url 根据实际ip和端口 填写
    std::string url = "ws://192.168.1.101:9090";
    //std::string url = "ws://127.0.0.1:9090";

    ws_client.connect(url);
  
    // 等待2秒 等待连接成功  
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    //2. 心跳数据定时器thread 一秒发送一次心跳信息,与服务端保存长连接的必要条件,收到的数据是二进制乱码
    std::thread([&ws_client ]() {
       string  heart_data = "{\"op\":\"ping\",\"timeStamp\":\"1\"}";
       while (1) {
            ws_client.send_message(heart_data);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
       }
     }).detach();
   

    //3. 需要订阅的话题数据，两种方式的数据，
    //方式1 宏定义
    std::string odom_msg = SUBSCRIBE_ODOM;
    std::string slam_status = SUBSCRIBE_SLAM_STATUS;
    //ws_client.send_message(odom_msg);
    ws_client.send_message(slam_status);

    
    //方式2 json 格式
    json points_raw = {
        {"op","subscribe"},
        {"topic","/points_raw" }//3D点云话题
       // {"compression","cbor" } //开放这条字段，收到的数据会是二进制数据，目前没有解析的接口，因此不建议开放该字段
    };
    //ws_client.send_message(points_raw.dump());

    
    /*4. 建图顺序: 开启录包->结束录包 (对应的id_type: record_data)
                  ->3D建图->结束3D建图 (对应的id_type: map_3d)
                  ->2D建图->结束2D建图  (对应的id_type: map_2d)
      //对应的op_type: start:开启,stop:结束,cancel:取消
                  
    */
    
    //5. 开启导航 (对应的id_type: follow_line)
    std::string file_name, op_type, id_type;
    file_name   = "3221";
    id_type     = "follow_line";//其他字段 map_3d, map_2d, follow_line
    op_type     = "start";//其他字段 stop
    json input_op = {
        {"op", "call_service"},
        {"service", "/input/op"},
        {"args", {
            {"file_name", file_name},//地图名称
            {"op_type",op_type},//start:开启,stop:结束,cancel:取消
            {"id_type",id_type}//录包并启动3D雷达建图,
        }}
    };
    //ws_client.send_message(input_op.dump());
    
    while(1){} //堵塞作用，用于循环监听数据

}

void http_client()
{
    //1.实例化 HttpClient 客户端
    HttpClient http_client;
    // url 根据实际ip和端口 填写
    http_client.set_url("http://192.168.1.101/apiUrl/");

    //2. 链接http server，获取token
    if (false == http_client.login_()){
        return ;
    }

    //3. 获取地图列表与每张地图绑定的任务列表
    std::string  map_list = http_client.get("/map_list?page=1&limit=-1&sortType=&reverse=");
    std::cout<< "map_list =  "<< map_list << std::endl;

    // mapName是单个地图名称，不用包含.png,
    std::string map_name = "3221";
   
    std::string file_name = "2-8.png";//存储到本地的文件名称，file_name 是一个文件的完整的路径
    //4. 获取某张地图,下载到本地,路径需修改
    int rest = http_client.download_file("/downloadpng?mapName"+map_name, file_name.c_str());
    printf("rest = %d", rest);

    //5. 获取地图对应的信息
    std::string res = http_client.get("/map_info?mapName="+map_name);

    //6. 将得到的信息转换成结构体
    jsonToMapinfo(res, map_info);

    //7. 获取地图png任意点
    // 使用windows 系统自带的画图软件打开下载好的地图png,使用鼠标点击图片白色区域任务点,左下角显示该点对应的坐标
    // 或者使用其他图片软件，点击某点显示该点的坐标即可，如下坐标[x,y] 985，165
    MPosition init_pos(178,143);
    
    //8. 再使用该坐标转换成地图真实坐标，
    png_coordinate_to_map(map_info,init_pos);

    //9. 设置初始位置，必须要使用websocket client 发送数据
    std::string str_initial =  set_initial_pos(init_pos, 21);
    
    /*********************
    这里需要特别注意 websocket 客户端发送的数据，不是http_client，
    目前为了测试通整个流程，将其放在了这里，实际使用时，视情况而定。
    ****
    */
    //ws_client.send_message(str_initial);

  
    //以下是假设的设置三个点，，用于导航
    MPosition png_pos_1(176,141), png_pos_2(201,143),png_pos_3(225,136) ;
    png_coordinate_to_map(map_info,png_pos_1);
    png_coordinate_to_map(map_info,png_pos_2);
    png_coordinate_to_map(map_info,png_pos_3);

    // 角度值不是固定的，假设这个三个点连接成一条线，第一个点的is_new = true，
    // 如果有第二条线，则第二线上的第一个点的is_new = true
    MPosePosition a(png_pos_1.x, png_pos_1.y, 50, true,0,0);
    MPosePosition b(png_pos_2.x, png_pos_2.y, 20, false,0,0);
    MPosePosition c(png_pos_3.x, png_pos_3.y, 90, false,0,0);

    // 存储在导航点列表中
    m_pose_list = {a, b, c}; 

    //存储导航点的json 数据
    json  array_pose;
    //导航点列表 转换成 json 数据
    MPositionToJson(m_pose_list, array_pose);
    // 多点导航"point", 多路径导航"path"
    std::string rs_task =  set_real_time_task("point", &array_pose, 2);

    // 10. 设置实时导航任务点
    http_client.post("/realtime_task", rs_task);
    
    // 11. 对当前运行的任务进行处理
    //以服务的形式进行调用，必须使用websocket 客户端发送数据,这里演示，放在了一起，请实际情况而定
    // (1)暂停任务，
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::string pause_task = CALL_SERVICE_PAUSE_TASK;
    //ws_client.send_message(pause_task);
    

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    // (2)恢复任务，
    std::string pause_restore = CALL_SERVICE_RESTORE_TASK;
    //ws_client.send_message(pause_restore);
   
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // (3)结束任务
    std::string pause_cancel = CALL_SERVICE_CANCEL_TASK;
    //ws_client.send_message(pause_cancel);
   
    
    //12.设置列表任务,这里的数据，较为麻烦，请参考api 文档中的 "设置某个地图的某个任务信息"章节进行设置
    //std::string _task;
    //http_client.post("/set_task",_task);

    //13. 获取列表任务信息，两个参数，mapName， taskName
    std::string resp = http_client.get("/get_task?mapName=office&taskName=t11");
   
    //14. 启动列表任务，注意某张地图的任务列表，在获取地图列表接口中可以获取到。
    json _run_task={{"mapName", map_name},{"taskName", "t11"}};
    //两个参数，mapName， taskName
    http_client.post("/run_list_task",_run_task);
    
    while(1){

    }

}


int main() {

    //测试阶段，分成两个函数独立调用，可根据实际情况合并。
     ws_client();
    // http_client();
    return 0;
}
