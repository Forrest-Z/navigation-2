/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file FollowerModule.cpp
 * @authors: Valentina Gaggero <valentina.gaggero@iit.it>
 */

#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/Bottle.h>

#include "FollowerModule.h"

#ifdef ASSISTIVE_REHAB_AVAILABLE
#include "Person3DPointRetriever.h"
#endif

#include "Ball3DPointRetriever.h"

#include <yarp/os/SystemClock.h>

using namespace std;
using namespace yarp::os;
using namespace FollowerTarget;





//------------------------ buffer helper test ---------------------

double FollowerModule::getPeriod()
{
    // module periodicity (seconds), called implicitly by the module.
    return m_period;
}


// This is our main function. Will be called periodically every getPeriod() seconds
bool FollowerModule::updateModule()
{
    double t=yarp::os::SystemClock::nowSystem();
    Target_t targetpoint;


//     switch(m_targetType)
//     {
//         case FollowerTargetType::person:
//         { targetpoint= (dynamic_cast<Person3DPointRetriever*>(m_pointRetriever_ptr))->getTarget(); break;}
//
//         case FollowerTargetType::redball:
//         { targetpoint= (dynamic_cast<Ball3DPointRetriever*>(m_pointRetriever_ptr))->getTarget(); break;}
//
//         default: break;
//     };

    targetpoint = m_pointRetriever_ptr->getTarget();
    m_followerResult= m_follower.followTarget(targetpoint);
    double diff = yarp::os::SystemClock::nowSystem()-t;

    if(diff!= 0)
    {
        m_statInfo.addVal(diff);
        if(m_statInfo.finish())
        {
            //yError() << "***** STAT: avg="<<m_statInfo.calculateAvg() <<"min="<<m_statInfo.getMin()<< "max=" <<m_statInfo.getMax();
            m_statInfo.reset();
        }
    }


    return true;
}




// Message handler. Just echo all received messages.
bool FollowerModule::respond(const Bottle& command, Bottle& reply)
{
    reply.clear();
    if (command.get(0).asString()=="help")
    {
        reply.addVocab(Vocab::encode("many"));
        reply.addString("Available commands are:");
        reply.addString("start");
        reply.addString("stop");
        reply.addString("verbose 0/1");
        reply.addString("help_provided");

        return true;
    }
    else if(command.get(0).asString()=="start")
    {
        m_follower.start();
        reply.addString("OK.started");
    }
    else if(command.get(0).asString()=="stop")
    {
        reply.addString("OK.stopped");
        m_follower.stop();
    }
    else if(command.get(0).asString()=="help_provided")
    {
        reply.addString("OK.help_provided");
        m_follower.helpProvided();
    }
    else
    {
        reply.addString("you");
        reply.addString("said");
        reply.append(command);
    }
    m_rpcPort.reply(reply);
    return true;

}



// Configure function. Receive a previously initialized
// resource finder object. Use it to configure your module.
// If you are migrating from the old module, this is the function
// equivalent to the "open" method.
bool FollowerModule::configure(yarp::os::ResourceFinder &rf)
{

    // 1) configure the follower
    if(!m_follower.configure(rf))
    {
        yError() << "Error reading configuration file";
        return false;
    }

    m_targetType=m_follower.getTargetType();

    // 2) read period and input port of this module from config file
    std::string inputPortName;
    Bottle config_group = rf.findGroup("GENERAL");
    if (config_group.isNull())
    {
        yError() << "Missing GENERAL group! the module uses default value!";
    }
    else
    {
        if (config_group.check("period")){ m_period = config_group.find("period").asFloat64();}
        if (config_group.check("inputPort"))  {inputPortName = config_group.find("inputPort").asString(); }
    }


    bool debugOn = false;
    Bottle debug_group = rf.findGroup("DEBUG");
    if (!debug_group.isNull())
    {
        if (debug_group.check("enable"))  { debugOn = debug_group.find("enable").asBool(); }
    }


    // 3) initialize the target retriever
    if(m_targetType == TargetType_t::redball)
    {
        m_pointRetriever_ptr = std::make_unique<Ball3DPointRetriever>();
    }
#ifdef ASSISTIVE_REHAB_AVAILABLE
    else if (m_targetType == TargetType_t::person)
    {
        m_pointRetriever_ptr = std::make_unique<Person3DPointRetriever>();
    }
#endif 
    else
    {
        yError() << "m_targetType not available!";
        return false;
    }


    if(! m_pointRetriever_ptr->init("/follower/" + inputPortName +":i", debugOn))
    {
        yError() << "Error in initializing the Target Retriever";
        return false;
    }

    m_rpcPort.open("/follower/rpc");
    attach(m_rpcPort);
    #ifdef TICK_SERVER
    TickServer::configure_tick_server("/follower");
    #endif

    return true;
}
// Interrupt function.
bool FollowerModule::interruptModule()
{
    m_rpcPort.interrupt();
    m_rpcPort.close();

    if(m_pointRetriever_ptr!=nullptr)
        m_pointRetriever_ptr->deinit();

    m_follower.close();
    return true;
}
// Close function, to perform cleanup.
bool FollowerModule::close()
{
    if(m_pointRetriever_ptr!=nullptr)
        m_pointRetriever_ptr->deinit();

    m_pointRetriever_ptr.reset();

    m_follower.close();

    return true;
}


FollowerModule::FollowerModule():m_period(DefaultPeriodOfMudule), m_targetType(TargetType_t::person), m_statInfo(100, -1, 1000), m_followerResult(Result_t::notRunning)
{}
FollowerModule::~FollowerModule(){;}


#ifdef TICK_SERVER
ReturnStatus FollowerModule::request_tick(const std::string& params)
{
    m_follower.start();
    return ReturnStatus::BT_RUNNING;
}

ReturnStatus FollowerModule::request_status()
{
    switch(m_followerResult)
    {
        case(Result_t::ok):
        case(Result_t::lostTarget):
        case(Result_t::autoNavigation):
            {return ReturnStatus::BT_RUNNING;}

        case(Result_t::notRunning): {return ReturnStatus::BT_HALTED;}
        case(Result_t::failed): {return ReturnStatus::BT_FAILURE;}
        case(Result_t::error): {return ReturnStatus::BT_ERROR;}
        default:{return ReturnStatus::BT_ERROR;}
    };
}

ReturnStatus FollowerModule::request_halt()
{
    m_follower.stop();
    return ReturnStatus::BT_HALTED;
}

#endif


//------------------------------------------------
// private function
//------------------------------------------------
