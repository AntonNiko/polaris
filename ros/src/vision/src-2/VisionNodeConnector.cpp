#include <ros/ros.h>


class VisionNodeConnector
{
private:
    ros::NodeHandle nh_;
public:
    VisionNodeConnector(ros::NodeHandle& nh) :
        nh_(nh)
    {
        
    }

    int operator()() {
        int status = 0;

        while(ros::ok() && !status) {

        }
    }
};

int main(int argc, char **argv) {
    ros::init(argc, argv, "vision");
    ros::NodeHandle nh("~");

    VisionNodeConnector visionNodeConnector(nh);

    int status = visionNodeConnector();

    ros::shutdown();
    return status;
}