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

#include "Person3DPointRetriever.h"
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>

using namespace yarp::os;
using namespace assistive_rehab;
using namespace FollowerTarget;

Target_t Person3DPointRetriever::getTarget(void)
{
    Target_t t; //it is initialized to false


    Bottle *b = m_inputPort.read(false); //use false in order to make the reading not blocking
    if(nullptr == b)
    {
        t.point3D[0]=100;
        return t;
    }

    Bottle *b1=b->get(0).asList();
    if(nullptr == b1)
    {
        return t;
    }

    if (b1->check("tag"))
    {
        Property prop(b1->toString().c_str());
        m_sk_target.update(prop);
        //if(m_debugOn)
            //yDebug() << "Person3DPPointRetriver: skeleton is updated!";
    }
    else
    {
        if(m_debugOn)
            yDebug() << "Person3DPPointRetriver: tag not exist!";
        return t;
    }

//     string tag=prop.find("tag").asString();
//     if(m_sk_target.getTag() != tag)
//     {
//         yError() << "Person3DPPointRetriver: tag diversi: ho" <<m_sk_target.getTag() <<  " !";
//         return std::make_pair(std::move (point3d), true);
//     }



    const KeyPoint* targetPoint_ptr = m_sk_target[KeyPointTag::shoulder_center];
    if(targetPoint_ptr != nullptr)//is necessary this check??? maybe yes if I can't see the shoulders
    {
        if(targetPoint_ptr->isUpdated())
        {
            t.point3D=targetPoint_ptr->getPoint();
            t.pixel = targetPoint_ptr->getPixel();
            t.isValid=true;
            if(m_debugOn)
                yDebug() << "Person3DPPointRetriver: get the point!! OK!!";
        }
        else
        {
            if(m_debugOn)
                yError() << "Person3DPPointRetriver: the skeleton is not updated!";
        }
    }
    else
    {
        if(m_debugOn)
            yError() << "Person3DPPointRetriver: shoulder_center point is null!";
    }

    return t;
}
