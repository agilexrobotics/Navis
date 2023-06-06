#ifndef GLOBALJSON_H
#define GLOBALJSON_H
#include <cmath>
//可能会用到的话题。
#define  SUBSCRIBE_SCOUT_STATES               "{\"op\":\"subscribe\", \"topic\":\"/scout_status\"}"
#define  SUBSCRIBE_SENSOR_STATES              "{\"op\":\"subscribe\", \"topic\":\"/sensor_states\"}"
#define  SUBSCRIBE_DISK_STATES                "{\"op\":\"subscribe\", \"topic\":\"/disk_states\"}"
#define  SUBSCRIBE_IMUDATA                    "{\"op\":\"subscribe\", \"topic\":\"/imu/data\"}"
#define  SUBSCRIBE_ODOM                       "{\"op\":\"subscribe\", \"topic\":\"/odom_raw\"}"

#define  SUBSCRIBE_CAMERA_IMAGE               "{\"op\":\"subscribe\", \"topic\":\"/camera_image\"}"

#define  SUBSCRIBE_SCAN                       "{\"op\":\"subscribe\", \"topic\":\"/scan\"}"
#define  SUBSCRIBE_POINTCLOUND                "{\"op\":\"subscribe\", \"topic\":\"/pointclound2\"}"
#define  SUBSCRIBE_GLOBAL_PLAN                "{\"op\":\"subscribe\", \"topic\":\"/move_base/GlobalPlanner/plan\"}"
#define  SUBSCRIBE_LOCAL_PLAN                 "{\"op\":\"subscribe\", \"topic\":\"/local_planner/local_path\"}"

#define  SUBSCRIBE_SLAM_STATUS                "{\"op\":\"subscribe\", \"topic\":\"/slam_status\"}"
#define  CALL_SERVICE_PAUSE_TASK              "{\"op\":\"call_service\",\"service\":\"/run_management/pause\",\"args\":{\"pause\":true,\"reason\":\"\"}}"
#define  CALL_SERVICE_RESTORE_TASK            "{\"op\":\"call_service\",\"service\":\"/run_management/pause\",\"args\":{\"pause\":false,\"reason\":\"\"}}"
#define  CALL_SERVICE_CANCEL_TASK             "{\"op\":\"call_service\",\"service\":\"/run_management/navi_task/cancel\",\"args\":{\"stamp\":{\"secs\":\"\",\"nsecs\":\"\"},\"id\":\"\"}}"

// 地图的具体数据，
struct Mapinfo{
    int gridWidth;//宽度
    int gridHeight;//高度
    double originX;//建图坐标原点x坐标
    double originY;//建图坐标原点y坐标
    double resolution;//分辨率
    std::vector<std::string> task_list;//这个地图绑定的任务
};

struct MPosition {
    double x;
    double y;
    MPosition(double x_, double y_):x(x_),y(y_){    }
};

struct MQuaternion {
    double x;
    double y;
    double z;
    double w;
};

//http-client端使用的导航目标点
struct MPosePosition {
    double pose_x;
    double pose_y;
    double pose_theta;
    bool   is_new;
    double pose_cpx;
    double pose_cpy;
    MPosePosition( double _x,
                   double _y,
                   double _theta,
                   bool   _is_new,
                   double _cpx,
                   double _cpy){
        pose_x     =   _x;
        pose_y     =   _y;
        pose_theta =   _theta;
        is_new     =   _is_new;
        pose_cpx   =   _cpx;
        pose_cpy   =   _cpy;
    }
};


// 从地图真实的坐标转换成 png图片坐标
void map_coordinate_to_png(  const Mapinfo &map_info, MPosition& m_pos)
{
    m_pos.x = (m_pos.x - map_info.originX )/ map_info.resolution;
    m_pos.y = map_info.gridHeight - (m_pos.y - map_info.gridHeight)/ map_info.resolution;
}

// 从 png图片坐标转换成地图真实的坐标
void png_coordinate_to_map( const  Mapinfo &map_info, MPosition& m_pos)
{
    m_pos.x = m_pos.x * map_info.resolution + map_info.originX;
    m_pos.y = (map_info.gridHeight - m_pos.y) * map_info.resolution + map_info.originY;
}

// 欧拉角(角度)先转弧度,转四元数
void quaternion_from_euler(double roll, double pitch,double yaw, MQuaternion & q)
{
    roll = roll * ( M_PI /180);
    pitch = pitch * ( M_PI/180);
    yaw = yaw * ( M_PI/180);
    q.x = sin(roll/2) * cos(pitch/2) * cos(yaw/2) - cos(roll/2) * sin(pitch/2) * sin(yaw/2);
    q.y = cos(roll/2) * sin(pitch/2) * cos(yaw/2) + sin(roll/2) * cos(pitch/2) * sin(yaw/2);
    q.z = cos(roll/2) * cos(pitch/2) * sin(yaw/2) - sin(roll/2) * sin(pitch/2) * cos(yaw/2);
    q.w = cos(roll/2) * cos(pitch/2) * cos(yaw/2) + sin(roll/2) * sin(pitch/2) * sin(yaw/2);
}
// 四元数转成 欧拉角中的yaw 角(弧度)， 再转成角度
float quaternion_to_yaw( MQuaternion & q)
{
    float yaw = atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.z * q.z + q.y * q.y));
    yaw = yaw * (180 / M_PI);
    return yaw;
}




  
#endif // GLOBALJSON_H
