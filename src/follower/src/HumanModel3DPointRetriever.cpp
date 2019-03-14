/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/
/**
 * @file Person3DPPointRetriver.cpp
 * @authors: Valentina Gaggero <valentina.gaggero@iit.it>
 */

#include "HumanModel3DPointRetriever.h"
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>

using namespace yarp::os;
using namespace FollowerTarget;

bool HumanModel3DPointRetriever::init(yarp::os::ResourceFinder &rf)
{
    m_refFrame=ReferenceFrameOfTarget_t::mobile_base_body_link;

    std::string portname="/follower/humanModelRetriver/rpc";
    if(!m_worldInterfacePort.open(portname))
    {
        yError() << "HumanModel3DPointRetriever: error in opening" << portname << "rpc port";
        return false;
    }
    yError() << "HumanModel3DPointRetriever";


    return true;
}

Target_t HumanModel3DPointRetriever::getTarget(void)
{
    Target_t t(m_refFrame); //it is initialized to false

    // Prapare bottle containg command to send in order to get the current position
    Bottle cmdGet, ansGet;
    cmdGet.clear();
    ansGet.clear();

    cmdGet.addString("getPose");
    cmdGet.addString("Luca");
    cmdGet.addString("SIM_CER_ROBOT::mobile_base_body_link");
    m_worldInterfacePort.write(cmdGet, ansGet);

    //yError() << "HumanModel3DPointRetriever: cmd-GET= " << cmdGet.toString() << "  Ans=" << ansGet.toString();
    //read the answer

    //NOTE: if Luca doesn't exist in Gazebo than the answer is "0.0 0.0 0.0 0.0 0.0 0.0".
    //So is difficult to discriminate  the real pose containing all zeros form the case in which Luca does't exist.
    //For this, the function return always a valid target
    if(ansGet.size() != 0)
    {
        t.point3D[0] = ansGet.get(0).asDouble();
        t.point3D[1] = ansGet.get(1).asDouble();
        t.point3D[2] = ansGet.get(2).asDouble();
        t.isValid=true;
    }


    return t;
}

bool HumanModel3DPointRetriever::deinit(void)
{
    return(TargetRetriever::deinitInputPort());
}

