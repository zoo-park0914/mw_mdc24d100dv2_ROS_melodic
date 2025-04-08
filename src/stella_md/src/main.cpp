#include "main.hpp"

// 원본에서 포함했던 헤더들
#include "MW_value.hpp"
#include "MW_serial.hpp"
#include "stella.hpp"
#include <math.h>

// ROS에서 사용하는 것들
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/transform_datatypes.h>

#define convertor_d2r (M_PI / 180.0)

// 전역 변수라면, 여기서 정의
bool RUN = false;

// 이 노드는 ROS1에서 NodeHandle을 멤버 변수로 보유
stellaN1_node::stellaN1_node()
  : nh_("~")  // private namespace
{
  // queue size 10 등
  ahrs_yaw_sub_ = nh_.subscribe<std_msgs::Float64>(
      "imu/yaw", 1,
      &stellaN1_node::ahrs_yaw_data_callback, this);

  cmd_vel_sub_ = nh_.subscribe<geometry_msgs::Twist>(
      "cmd_vel", 10,
      &stellaN1_node::command_velocity_callback, this);

  odom_pub_ = nh_.advertise<nav_msgs::Odometry>("odom", 10);

  // TF broadcaster
  odom_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>();

  // 타이머: 1ms (원본: create_wall_timer(1ms, serial_callback))
  Serial_timer = nh_.createTimer(
      ros::Duration(0.001),
      &stellaN1_node::serial_callback,
      this);

  ROS_INFO("stellaN1_node constructor done (ROS1).");
}

stellaN1_node::~stellaN1_node()
{
  MW_Serial_DisConnect();  // 원본과 동일
}

// callback: imu/yaw
void stellaN1_node::ahrs_yaw_data_callback(const std_msgs::Float64::ConstPtr& msg)
{
  ahrs_yaw = msg->data;
}

// callback: cmd_vel
void stellaN1_node::command_velocity_callback(const geometry_msgs::Twist::ConstPtr& cmd_vel_msg)
{
  if (RUN)
  {
    goal_linear_velocity_ = cmd_vel_msg->linear.x;
    goal_angular_velocity_ = cmd_vel_msg->angular.z;

    dual_m_command(dual_m_command_select::m_lav, goal_linear_velocity_, goal_angular_velocity_);
  }
}

// timer callback
void stellaN1_node::serial_callback(const ros::TimerEvent&)
{
  if (RUN)
  {
    Motor_MonitoringCommand(channel_1, _position);
    Motor_MonitoringCommand(channel_2, _position);

    update_odometry();
  }
}

bool stellaN1_node::update_odometry()
{
  auto Limit_i = [&](int v, int lo, int hi){
    if (abs(v) > lo && abs(v) < hi) return v;
    else return 0;
  };

  auto pulse2meter = [](){
    double meter = ((2 * M_PI * Differential_MobileRobot.wheel_radius) /
                    Differential_MobileRobot.gear_ratio /
                    MyMotorConfiguration.encoder_ppr[0]);
    return meter * -0.1; 
  };

  delta_left = Limit_i((MyMotorCommandReadValue.position[channel_1] - left_encoder_prev) * -1, 0, 10000) * pulse2meter();
  delta_right = Limit_i((MyMotorCommandReadValue.position[channel_2] - right_encoder_prev) * -1, 0, 10000) * pulse2meter();

  delta_s  = (delta_right + delta_left) / 2.0;
  delta_th = (ahrs_yaw * convertor_d2r);

  delta_x  = delta_s * cos(delta_th);
  delta_y  = delta_s * sin(delta_th);

  x += delta_x;
  y += delta_y;

  nav_msgs::Odometry odom;
  tf2::Quaternion Quaternion;
  Quaternion.setRPY(0, 0, delta_th);

  odom.pose.pose.orientation.x = Quaternion.x();
  odom.pose.pose.orientation.y = Quaternion.y();
  odom.pose.pose.orientation.z = Quaternion.z();
  odom.pose.pose.orientation.w = Quaternion.w();

  geometry_msgs::TransformStamped t;
  t.header.stamp = ros::Time::now();  
  t.header.frame_id = "odom";
  t.child_frame_id = "base_footprint";

  t.transform.translation.x = x;
  t.transform.translation.y = y;
  t.transform.translation.z = 0.0;
  t.transform.rotation.x = Quaternion.x();
  t.transform.rotation.y = Quaternion.y();
  t.transform.rotation.z = Quaternion.z();
  t.transform.rotation.w = Quaternion.w();

  odom_broadcaster->sendTransform(t);

  odom.header.frame_id = "odom";
  odom.pose.pose.position.x = x;
  odom.pose.pose.position.y = y;
  odom.pose.pose.position.z = 0.0;

  odom.child_frame_id = "base_footprint";
  odom.twist.twist.linear.x = goal_linear_velocity_;
  odom.twist.twist.angular.z = goal_angular_velocity_;

  odom.header.stamp = ros::Time::now();

  odom_pub_.publish(odom);

  left_encoder_prev = MyMotorCommandReadValue.position[channel_1];
  right_encoder_prev = MyMotorCommandReadValue.position[channel_2];
  return true;
}

// main 함수 그대로
int main(int argc, char *argv[])
{
  ros::init(argc, argv, "stella_md_node");

  MW_Serial_Connect("/dev/MW", 115200);

  if (Robot_Setting(Robot_choice::N1))
    RUN = true;
  Robot_Fault_Checking_RESET();

  stellaN1_node node; // constructor (NodeHandle 멤버 유지)
  ros::spin();

  ros::shutdown();
  return 0;
}

