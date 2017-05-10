#include "micros_flock.cpp"

int main(int argc, char** argv)
{
    ros::init(argc,argv,"turtlebot_roll_node");
    micros_flock::FlockingAlgorithm fa;
    ros::NodeHandle n;
   ros::Publisher pub = n.advertise<geometry_msgs::Twist>("cmd_vel",1000);
    geometry_msgs:: Twist msg;
    ros::Rate loop_rate(20);
    while(ros::ok())
    {
        ros::spinOnce();
        fa.run(msg);
        pub.publish(msg);
        loop_rate.sleep();
    }
}
