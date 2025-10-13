# choreonoid_joy2
ROS2 node publishing sensor_msgs/msg/Joy with the standard joystick mapping of Choreonoid

## Publish joy topic
$ ros2 run choreonoid_joy2 node

## Rename topic (ex. from joy to joy2)
$ ros2 run choreonoid_joy2 node --ros-args -p topic_name:=joy2

## Change input device (ex. from /dev/input/js0 to /dev/input/js1)
$ ros2 run choreonoid_joy2 node --ros-args -p device_name:=/dev/input/js1

## Rename topic & Change input device
$ ros2 run choreonoid_joy2 node --ros-args -p topic_name:=joy2 -p device_name:=/dev/input/js1
