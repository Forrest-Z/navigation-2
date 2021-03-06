/*
* Copyright (C)2015  iCub Facility - Istituto Italiano di Tecnologia
* Author: Marco Randazzo
* email:  marco.randazzo@iit.it
* website: www.robotcub.org
* Permission is granted to copy, distribute, and/or modify this program
* under the terms of the GNU General Public License, version 2 or any
* later version published by the Free Software Foundation.
*
* A copy of the license can be found at
* http://www.robotcub.org/icub/license/gpl.txt
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details
*/
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Port.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/Os.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>

#include <iostream>
#include <iomanip>
#include <string>

#include "baseControl.h"

/**
 * \section baseControl
 * This core module controls the robot joints in order to achieve the desired cartesian velocity command, sent either via a joystick or via YARP port/ROS topic
 * The module requires the definition of the robot kinematics. So far only ikart_V1, ikart_V2, cer types are valid options.as robot_type parameter.
 * Four control modes are available: velocity_no_pid sets the control modes of the motors to velocity and sends individual velocity references, computed by from user cartesian commands, to the joints.
 * velocity_pid also controls the motors in velocity modes, but it also implements and external closed loop on the realized cartesian trajectory.
 * openloop_no_pid/openloop_pid are similar, but the individual motors are controlled by sending pwm references instead of velocity commands.
 * The module also performs the odometry computation and publishes it onto a yarp port/ROS topic.
 * A detailed description of configuration parameters available for the module is provided in the README.md file.
 */

/////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    //Initialize Yarp network
    Network yarp;

    if (!yarp.checkNetwork())
    {
        std::cerr << "Sorry YARP network does not seem to be available, is the yarp server available?\n";
        return -1;
    }

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("navigation");
    rf.setDefaultConfigFile("baseCtrl.ini");
    rf.configure(argc,argv);

    if (rf.check("help"))
    {
        yInfo("Possible options: ");
        yInfo("'rate <r>' sets the threads rate (default 20ms).");
        yInfo("'no_filter' disables command filtering.");
        yInfo("'no_motors' motor interface will not be opened.");
        yInfo("'no_start' do not automatically enables pwm.");
        yInfo("'joystick_connect' tries to automatically connect to the joystickCtrl output.");
        yInfo("'skip_robot_interface_check' does not connect to robotInterface/rpc (useful for simulator)");
        return 0;
    }

    //Starts the control module
    CtrlModule mod;
    return mod.runModule(rf);
}
