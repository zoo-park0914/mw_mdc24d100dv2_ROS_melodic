#ifndef MAIN_HPP
#define MAIN_HPP

#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf2_msgs/TFMessage.h>
#include <std_msgs/Float64.h>

// ROS1
#include <ros/ros.h>

class stellaN1_node
{
public:
    stellaN1_node();
    ~stellaN1_node();

private:
    // NodeHandle을 멤버 변수로
    ros::NodeHandle nh_;    

    // ROS timer
    ros::Timer Serial_timer;
    
    // ROS topic publishers
    ros::Publisher odom_pub_;

    // ROS topic subscribers
    ros::Subscriber cmd_vel_sub_;
    ros::Subscriber ahrs_yaw_sub_;
    
    ros::Time time_now;
    
    // TF broadcaster (ROS1 방식)
    std::unique_ptr<tf2_ros::TransformBroadcaster> odom_broadcaster;

    // 원본 변수들 (초기화 값 포함)
    double goal_linear_velocity_;
    double goal_angular_velocity_;
    
    int left_encoder_prev = 0, right_encoder_prev = 0;
    
    double ahrs_yaw = 0.0, delta_th = 0.0, delta_s = 0.0, delta_x = 0.0, delta_y = 0.0;
    double x = 0.0, y = 0.0, th = 0.0, delta_left = 0.0, delta_right = 0.0;

    // 콜백 함수 (ROS1)
    void ahrs_yaw_data_callback(const std_msgs::Float64::ConstPtr& msg);
    void command_velocity_callback(const geometry_msgs::Twist::ConstPtr& cmd_vel_msg);
    void serial_callback(const ros::TimerEvent&);

    bool update_odometry();
};

// 원본 코드에서 RUN이 전역이라면 extern 선언 (필요하면)
extern bool RUN;

#endif // MAIN_HPP

