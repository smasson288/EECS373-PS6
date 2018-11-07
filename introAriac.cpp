#include <ros/ros.h>
#include <iostream>
#include <string>
#include <std_srvs/Trigger.h>
#include <osrf_gear/ConveyorBeltControl.h>
#include <osrf_gear/DroneControl.h>
#include <osrf_gear/LogicalCameraImage.h>
using namespace std;

bool g_take_new_snapshot = false;
osrf_gear::LogicalCameraImage g_cam2_data;

void cam2CB(const osrf_gear::LogicalCameraImage& message_holder){
    if(g_take_new_snapshot){
        ROS_INFO_STREAM("image from cam2: " << message_holder << endl);
        g_cam2_data=message_holder;
        //g_take_new_snapshot=false;
    }
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "introAriac");
    ros::NodeHandle n;

    ros::ServiceClient startupClient = n.serviceClient<std_srvs::Trigger>("/ariac/start_competition");
    std_srvs::Trigger startup_srv;
    

    ros::ServiceClient conveyorClient = n.serviceClient<osrf_gear::ConveyorBeltControl>("/ariac/conveyor/control");
    osrf_gear::ConveyorBeltControl conveyor_srv;

    ros::ServiceClient droneClient = n.serviceClient<osrf_gear::DroneControl>("ariac/drone");
    osrf_gear::DroneControl drone_srv;

    ros::Subscriber camera_subscriber = n.subscribe("ariac/logical_camera_2", 1, cam2CB);


    //check if successful
    //startup_srv.response.success=false;
    startupClient.call(startup_srv);
    while(!startup_srv.response.success){
        ROS_WARN("not successful");
        startupClient.call(startup_srv);
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("got success response from startup service");

    //start the conveyor
    conveyor_srv.request.power = 100;
	conveyorClient.call(conveyor_srv);
    //check if successful
    while(!conveyor_srv.response.success){
        ROS_WARN("not successful");
		conveyorClient.call(conveyor_srv);
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("got success response from conveyor service");

	//finds box with camera
    g_take_new_snapshot = true;
    while(g_cam2_data.models.size()<1) {
        ros::spinOnce();
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("I see a box");

	g_take_new_snapshot = true;
	//get box in middle of camera
	while(g_cam2_data.models.size()==1 && g_cam2_data.models[0].pose.position.z < 0){
		ros::spinOnce();
        ros::Duration(0.05).sleep();
	}
	ROS_INFO("box in middle of camera");

    //stop conveyor
    conveyor_srv.request.power = 0;
	conveyorClient.call(conveyor_srv);
    //check if successful
    while(!conveyor_srv.response.success){
        ROS_WARN("not successful");
		conveyorClient.call(conveyor_srv);
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("got success response from conveyor service");

	//waits 5 seconds under camera
	ROS_INFO("Holding still for 5 seconds");
    ros::Duration(5.0).sleep();

    //start conveyor
    conveyor_srv.request.power = 100;
	conveyorClient.call(conveyor_srv);
    //check if successful
    while(!conveyor_srv.response.success){
        ROS_WARN("not successful");
		conveyorClient.call(conveyor_srv);
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("got success response from conveyor service");

    //drone pick up box
    drone_srv.request.shipment_type = "shipping_box";
	droneClient.call(drone_srv);
    //check if successful
    while(!drone_srv.response.success){
        ROS_WARN("not successful");
        droneClient.call(drone_srv);
        ros::Duration(0.5).sleep();
    }
    ROS_INFO("got success response from drone service");

  return 0;
}
