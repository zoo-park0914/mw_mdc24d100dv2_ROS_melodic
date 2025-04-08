#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
import serial
import struct
import math
from sensor_msgs.msg import Imu
import tf

def parse_sensor_data(data):
    if len(data) < 20:
        return None

    if data[0] == 0x55 and data[1] == 0x61:
        Ax = struct.unpack('<h', data[2:4])[0] / 32768.0 * 16 * 9.80665
        Ay = struct.unpack('<h', data[4:6])[0] / 32768.0 * 16 * 9.80665
        Az = struct.unpack('<h', data[6:8])[0] / 32768.0 * 16 * 9.80665

        Gx = struct.unpack('<h', data[8:10])[0] / 32768.0 * 2000 * math.pi / 180.0
        Gy = struct.unpack('<h', data[10:12])[0] / 32768.0 * 2000 * math.pi / 180.0
        Gz = struct.unpack('<h', data[12:14])[0] / 32768.0 * 2000 * math.pi / 180.0

        AngX = struct.unpack('<h', data[14:16])[0] / 32768.0 * 180
        AngY = struct.unpack('<h', data[16:18])[0] / 32768.0 * 180
        AngZ = struct.unpack('<h', data[18:20])[0] / 32768.0 * 180

        # Euler → Quaternion 변환
        roll = AngX * math.pi / 180.0
        pitch = AngY * math.pi / 180.0
        yaw = AngZ * math.pi / 180.0
        q = tf.transformations.quaternion_from_euler(roll, pitch, yaw)

        return Ax, Ay, Az, Gx, Gy, Gz, q
    return None

def imu_publisher():
    rospy.init_node('wt910_imu_publisher')
    pub = rospy.Publisher('/imu/data', Imu, queue_size=50)
    ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=1)

    buffer = bytearray()
    rate = rospy.Rate(50)  # 50 Hz

    try:
        while not rospy.is_shutdown():
            buffer += ser.read(20)
            if len(buffer) >= 20:
                parsed = parse_sensor_data(buffer[:20])
                buffer = buffer[20:]

                if parsed:
                    Ax, Ay, Az, Gx, Gy, Gz, q = parsed
                    imu_msg = Imu()
                    imu_msg.header.stamp = rospy.Time.now()
                    imu_msg.header.frame_id = "imu_link"

                    imu_msg.linear_acceleration.x = Ax
                    imu_msg.linear_acceleration.y = Ay
                    imu_msg.linear_acceleration.z = Az

                    imu_msg.angular_velocity.x = Gx
                    imu_msg.angular_velocity.y = Gy
                    imu_msg.angular_velocity.z = Gz

                    imu_msg.orientation.x = q[0]
                    imu_msg.orientation.y = q[1]
                    imu_msg.orientation.z = q[2]
                    imu_msg.orientation.w = q[3]

                    pub.publish(imu_msg)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass
    finally:
        ser.close()

if __name__ == '__main__':
    imu_publisher()

