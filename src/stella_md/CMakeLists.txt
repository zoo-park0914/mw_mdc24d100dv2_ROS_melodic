cmake_minimum_required(VERSION 3.0.2)
project(stella_md)

## 1) 필요한 ROS 패키지
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  geometry_msgs
  nav_msgs
  tf2
  tf2_ros
  tf2_msgs
  # serial (만약 ROS1의 serial 패키지를 사용하려면 추가)
)

## 2) catkin_package 설정
catkin_package(
  INCLUDE_DIRS src    # main.hpp가 src 폴더에 있다면
  CATKIN_DEPENDS roscpp std_msgs geometry_msgs nav_msgs tf2 tf2_ros tf2_msgs
)

## 3) include_directories
# - main.hpp가 src 폴더에 있고,
# - mw/serial 폴더에 *.h가 있으므로 각각 추가
include_directories(
  src
  ${CMAKE_CURRENT_SOURCE_DIR}/include/mw
  ${CMAKE_CURRENT_SOURCE_DIR}/include/serial
  ${catkin_INCLUDE_DIRS}
)

## 4) 빌드 대상
# 'type="stella_md_node"'에 맞춰 실행 파일 이름은 stella_md_node 로 함
add_executable(stella_md_node
  src/main.cpp           # main.cpp에 main() 함수와 클래스 구현
  serial/list_ports_linux.cc
  serial/serial.cc
  serial/unix.cc
  mw/stella.cpp
)

## 5) 링크 라이브러리
target_link_libraries(stella_md_node
  ${catkin_LIBRARIES}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/MW_MotorDriver_aarch64.a
  # 필요하다면 -pthread, -lMWMotorDriver 등 추가
)

############# (선택) install 설정 #############
install(TARGETS stella_md_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)

