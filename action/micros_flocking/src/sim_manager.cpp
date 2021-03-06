#include "ros/ros.h"
#include "tf/tf.h"
#include "geometry_msgs/Twist.h"
#include "micros_flocking/Neighbor.h"
#include "micros_flocking/Position.h"
#include "micros_flocking/Gradient.h"
#include "nav_msgs/Odometry.h"
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <utility>
#include <cmath>
#include <ctime>
using namespace std;
#define R 12
#define sendT 1
#define perror 0  //value
#define verror 0 //rate
int robotnum=50;
class OdomHandle
{
    public:
    ros::Subscriber sub;
    ros::Publisher pub;
    ros::Publisher neighbor_pub;

    ros::Subscriber gradient_sub;
    double _px,_py,_vx,_vy;
    pair<double,double> _position,_velocity;
    int _r_id; 
    int rcvcount;
    int gradient;
    OdomHandle(int r_id)
    {
        ros::NodeHandle n;
        stringstream ss;
        ss<<"/robot_"<<r_id<<"/base_pose_ground_truth";
        sub = n.subscribe(ss.str(), 1000, &OdomHandle::cb,this);

        stringstream ss1;
        ss1<<"/robot_"<<r_id<<"/position";
        pub = n.advertise<micros_flocking::Position>(ss1.str(),1000);

        stringstream ss2;
        ss2<<"/robot_"<<r_id<<"/neighbor";
        neighbor_pub = n.advertise<micros_flocking::Neighbor>(ss2.str(),1000);

        stringstream ss3;
        ss3<<"/robot_"<<r_id<<"/gradient";
        gradient_sub = n.subscribe(ss3.str(), 1000, &OdomHandle::cb_gradient,this);
        _px=0;
        _py=0;
        _vx=0;
        _vy=0;
        _position=pair<double,double>(0,0);
        _velocity=pair<double,double>(0,0);
        _r_id = r_id;
        rcvcount = 0;
        gradient = -1;
    }
    
    void cb(const nav_msgs::Odometry::ConstPtr & msg)
    {
        //cout<<this->_px<<" "<<_r_id<<" "<<_vy<<endl;
        //_py=1;_px=1;
        if(rcvcount % sendT !=0)
        {
            rcvcount++;
            return;
         }
        rcvcount++;
        _px=msg->pose.pose.position.x;
        _py=msg->pose.pose.position.y;
        _vx=msg->twist.twist.linear.x;
        _vy=msg->twist.twist.linear.y;
        _position.first=_px;_position.second=_py;
        _velocity.first=_vx;_velocity.second=_vy;

        micros_flocking::Position sendmsg;
        sendmsg.px=_px+(rand()%2001-1000)/1000.0*perror;
        sendmsg.py=_py+(rand()%2001-1000)/1000.0*perror;
        sendmsg.vx=(_vx+3)*(1+(rand()%2001-1000)/1000.0*verror)-3;
        sendmsg.vy=(_vy+3)*(1+(rand()%2001-1000)/1000.0*verror)-3;

        sendmsg.gradient = gradient;
        sendmsg.theta = tf::getYaw(msg->pose.pose.orientation);
        pub.publish(sendmsg);
        //_vx=1;_vy=1;//myx=1;myy=1;
        //cout<<_r_id<<endl;
    }

    void cb_gradient(const micros_flocking::Gradient::ConstPtr & msg)
    {
         gradient = msg -> gradient;
     }
    void close()
    {
        sub.shutdown();
    }
};

static vector<OdomHandle*> odom_list;

vector<vector<int> > adj_list;

double dist(int i,int j)
{
    double re=pow(odom_list[i]->_px-odom_list[j]->_px,2)+pow(odom_list[i]->_py-odom_list[j]->_py,2);
    //if(i==4)
    //cout<<re<<endl;
    return sqrt(re);
    
}

int main(int argc, char** argv)
{
   ros::init(argc,argv,"sim_manager");
   ros::NodeHandle n;
   srand(time(0));
   bool param_ok = ros::param::get ("~robotnum", robotnum);
   for(int i=0;i<robotnum;i++)
   {
      OdomHandle *p=new OdomHandle(i);
      odom_list.push_back(p);
      adj_list.push_back(vector<int>());
   }
   //neighbor_list.push_back(NeighborHandle(1));
   ros::Rate loop_rate(20);
   
   while(ros::ok())
   {
      ros::spinOnce();
      for(int i=0;i<robotnum;i++)
      {
           for(int j=i+1;j<robotnum;j++)
           {
                if(dist(i,j)>0&&dist(i,j)<R)
                {
                    adj_list[i].push_back(j);
                    adj_list[j].push_back(i);
                }
           }
      }
      for(int i=0;i<robotnum;i++)
      {
           micros_flocking::Neighbor sendmsg;
           sendmsg.data = adj_list[i];
           odom_list[i]->neighbor_pub.publish(sendmsg);
           adj_list[i]=vector<int>();
      }
      loop_rate.sleep();
   }
   return 0;
}
