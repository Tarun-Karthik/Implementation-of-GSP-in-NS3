/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Amshula U.S <usamshula@gmail.com>
 *          A Tarun Karthik <tarunkarthik999@gmail.com>
 *          Manoj Kumar <mnkumar493@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include "ns3/log.h"
#include "fifo-queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"

namespace ns3{

  NS_LOG_COMPONENT_DEFINE ("GspQueueDisc");

  NS_OBJECT_ENSURE_REGISTERED (GspQueueDisc);

  TypeId GspQueueDisc::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::GspQueueDisc")
      .SetParent<DropTailQueue>()   //confirm
      .SetGroupName ("TrafficControl")
      .AddConstructor<GspQueueDisc> ()
      .AddAttribute ("MaxSize",
                     "The max queue size",
                     QueueSizeValue (QueueSize ("1000p")),
                     MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                            &QueueDisc::GetMaxSize),
                     MakeQueueSizeChecker ()) // confirm
      .AddAttribute ("A",
                     "Value of alpha",
                     DoubleValue (2),
                     MakeDoubleAccessor (&GspQueueDisc::m_a),
                     MakeDoubleChecker<double> ())
      .AddAttribute("Interval",
                    "Value that will be incremented to the time out",
                    TimeValue (Seconds(0.2)),    //twice of RTT  | RTT = 100ms so initial interval value should be 200ms
                    MakeTimeAccessor (&GspQueueDisc::SetInterval,
                                      &GspQueueDisc::GetInterval),
                    MakeTimeChecker ())
      .AddAttribute("Adaptive Variable",
                    "Controlling the adaptation speed",
                    TimeValue (Seconds(1)),
                    MakeTimeAccessor (&GspQueueDisc::adapt),
                    MakeTimeChecker ())
      .AddAttribute("Threshold",
                    "Limit above which the packet should be dropped",
                    DoubleValue (),
                    MakeDoubleAccessor(&GspQueueDisc::SetThreshold,
                                       &GspQueueDisc::GetThreshold),
                    MakeDoubleChecker<double> ())
      .AddAttribute("Timeout",
                    "amount of time for which the packets will not be dropped",
                    TimeValue (Seconds(0)),
                    MakeTimeAccessor (&GspQueueDisc::SetTimeout,
                                      &GspQueueDisc::GetTimeout),
                    MakeTimeChecker ())
      .AddAttribute("State",
                    "Determines the state of the Queue",
                    EnumValue (QUEUE_STATE),
                    MakeEnumAccessor (&GspQueueDisc::SetState,
                                      &GspQueueDisc::GetState),
                    MakeEnumChecker (QUEUE_CLEAR,"QUEUE_CLEAR".
                                     QUEUE_OVERFLOW,"QUEUE_OVERFLOW",
                                     QUEUE_DRAIN,"QUEUE_DRAIN"))
      .AddAttribute ("TimeInQueue",
                 "The amount of time taken by the packet in the queue",
                  StringValue (),
                  MakeTimeAccessor (&GspQueueDisc::m_tiq),
                  MakeTimeChecker ())
      .AddAttribute("StartTime",
                 "current time",
                  TimeValue (Seconds(0)),
		              MakeTimeAccessor (&GspQueueDisc::StartTime),
                  MakeTimeChecker ())
      .AddAttribute ("BasicGsp",
                  "Whether to use basic version of GSP",
                  BooleanValue (true),
                  MakeBooleanAccessor (&SfqQueueDisc::m_useBasicGsp),
                  MakeBooleanChecker ())
      .AddAttribute ("AdaptiveGsp",
                     "Whether to use Adaptive version of GSP",
                     BooleanValue (false),
                     MakeBooleanAccessor (&SfqQueueDisc::m_useAdaptiveGsp),
                     MakeBooleanChecker ())
      .AddAttribute ("DelayedGsp",
                     "Whether to use Delayed version of GSP",
                     BooleanValue (false),
                     MakeBooleanAccessor (&SfqQueueDisc::m_useDelayedGsp),
                     MakeBooleanChecker ())
    ;
    return tid;
  }

  GspQueueDisc::GspQueueDisc ()
    : FifoQueueDisc () //check parameters to be SetNetDevice
    {
      NS_LOG_FUNCTION (this);
    }

  GspQueueDisc::~GspQueueDisc ()
  {
    NS_LOG_FUNCTION (this);
  }


  bool
  GspQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
  {
    NS_LOG_FUNCTION (this << item);
    //Enqueue function goes here
    if(m_useBasicGsp)
    {
      if(GetCurrentSize () + item->GetSize () > GetThreshold () && Simulator::Now () > GetTimeout ())
      {
        NS_LOG_LOGIC("Queue Size is greater than Threshold");
        DropBeforeEnqueue (item, FORCED_DROP);
        SetTimeout(Simulator::Now () + GetInterval ());
        return false;
      }
      else
      {
        bool retval = GetInternalQueue (0)->Enqueue (item);


        NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
        NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

        return retval;
      }
    }
    else if(m_useAdaptiveGsp)
    {
      Time cumTime = 0, maxTime;
      if(GetCurrentSize () + item->GetSize () > GetMaxSize ())
      {
        SetState (QUEUE_OVERFLOW);
      }
      else if( GetState () == QUEUE_OVERFLOW && GetCurrentSize () == 0)
      {
        SetState (QUEUE_DRAIN);
      }
      else if ( GetState () == QUEUE_DRAIN && GetCurrentSize () > GetThreshold ())
      {
        SetState (QUEUE_CLEAR);
      }

      // Update Cummulative time

      cumTime += m_a()*time_above_threshold;

      if(GetState()==QUEUE_CLEAR)
        cumTime = cumTime - time_below_threshold;


      cumTime=min(maxTime, max(0, cumTime));

      Time presetInterval = Seconds(0.2);
      SetInterval( presetInterval / ( 1 + cumTime/adapt()));

      // After adapting the interval perform actions as in Basic GSP


      //Basic GSP
      if(GetCurrentSize () + item->GetSize () > GetThreshold () && Simulator::Now () > GetTimeout ())
        {
          NS_LOG_LOGIC("Queue Size is greater than Threshold");
          DropBeforeEnqueue (item, FORCED_DROP);
          SetTimeout(Simulator::Now () + GetInterval ());
          return false;
        }
        else
        {
          bool retval = GetInternalQueue (0)->Enqueue (item);


          NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
          NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

          return retval;
        }
    }
    else if(m_useDelayedGsp)
    {
      StartTime = Simulator::Now();

      //Enqueue function goes here
      //Threshold in miliseconds
      if(m_tiq > GetThreshold () && Simulator::Now () > GetTimeout ())
      {
        NS_LOG_LOGIC("Queue Size is greater than Threshold");
        DropBeforeEnqueue (item, FORCED_DROP);
        SetTimeout(Simulator::Now () + GetInterval ());
        return false;
      }
      else
      {
        bool retval = GetInternalQueue (0)->Enqueue (item);

        NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
        NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

        return retval;
      }
    }
  }

  Ptr<QueueDiscItem>
  GspQueueDisc::DoDequeue (void)
  {
    NS_LOG_FUNCTION (this);
    if(m_useDelayedGsp)
    {
      m_tiq = StartTime - Simulator::Now();
    }
    Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

    if (!item)
      {
        NS_LOG_LOGIC ("Queue empty");
        return 0;
      }

    return item;
  }

  Ptr<const QueueDiscItem>
  GspQueueDisc::DoPeek (void)
  {
    NS_LOG_FUNCTION (this);
  }

  bool
  GspQueueDisc::CheckConfig (void)
  {
    NS_LOG_FUNCTION (this);
  }

  void
  GspQueueDisc::InitializeParams (void)
  {
    NS_LOG_FUNCTION (this);
  }

}
