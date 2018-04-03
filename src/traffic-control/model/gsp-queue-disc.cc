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
                  TimeValue (Seconds(0.2)),
                  MakeTimeAccessor (&GspQueueDisc::m_interval),
                  MakeTimeChecker ())
    .AddAttribute("AdaptiveVariable",
                  "Controlling the adaptation speed",
                  TimeValue (Seconds(1)),
                  MakeTimeAccessor (&GspQueueDisc::m_adapt),
                  MakeTimeChecker ())
    .AddAttribute("Threshold",
                  "Limit above which the packet should be dropped",
                  DoubleValue (),
                  MakeDoubleAccessor(&GspQueueDisc::m_threshold),
                  MakeDoubleChecker<double> ())
    .AddAttribute("Timeout",
                  "amount of time for which the packets will not be dropped",
                  TimeValue (Seconds(0)),
                  MakeTimeAccessor (&GspQueueDisc::m_timeout),
                  MakeTimeChecker ())
    .AddAttribute("State",
                  "Determines the state of the Queue",
                  EnumValue (QUEUE_CLEAR),
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
		              MakeTimeAccessor (&GspQueueDisc::m_startTime),
                  MakeTimeChecker ())
    .AddAttribute("Mode",
                  "Determines which GSP algorithm to use",
                  EnumValue(BASIC_GSP),
                  MakeEnumAccessor(&GspQueueDisc::GetMode,
                                   &GspQueueDisc::SetMode)
                  MakeEnumChecker(BASIC_GSP, "BASIC_GSP",
                                  ADAPTIVE_GSP, "ADAPTIVE_GSP",
                                  DELAY_GSP, "DELAY_GSP"))
    .AddAttribute("TimeAboveThreshold"
                  "Accumulates time difference whenever a packet spends more time than the threshold",
                  TimeValue(Seconds(0)),
                  MakeTimeAccessor (&GspQueueDisc::m_timeAboveThreshold),
                  MakeTimeChecker())
    .AddAttribute("TimeBelowThreshold"
                  "Accumulates time difference whenever a packet spends less time than the threshold",
                  TimeValue(Seconds(0)),
                  MakeTimeAccessor (&GspQueueDisc::m_timeBelowThreshold),
                  MakeTimeChecker())
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
      Time m_cumTime = 0;
      
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
      
      cumTime += m_a*m_timeAboveThreshold;
      
      if(GetState()==QUEUE_CLEAR)
        cumTime = cumTime - m_timeBelowThreshold;
      
      cumTime=min(m_maxTime, max(0, cumTime));
      Time presetInterval = Seconds(0.2);
      SetInterval( presetInterval / ( 1 + cumTime/m_adapt));
      
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
  if(!item)
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
  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();
  if(!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  return item;
}

bool
GspQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  
  if(GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("GspQueueDisc cannot have classes");
      return false;
    }
  
  if(GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("GspQueueDisc needs no packet filter");
      return false;
    }
  
  if(GetNInternalQueues () == 0)
    {
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                            ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }
  
  if(GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("GspQueueDisc needs 1 internal queue");
      return false;
    }
  return true;
}

void 
GspQueueDisc::SetMode(GspMode mode)
{
  if(mode == BASIC_GSP || mode == ADAPTIVE_GSP || mode == DELAY_GSP)
  {
    m_mode = mode;
  }
  else 
  {
    NS_ABORT_MSG("Unknown GSP mode");
    return ;
  }

  if(m_mode == BASIC_GSP || m_mode == DELAY_GSP)
  {
    m_interval = Seconds(0.2);
  }

  return ;
}

GspQueueDisc::GspMode
GspQueueDisc::GetMode()
{
  return m_mode;
}

void
GspQueueDisc::SetState(QueueState state)
{
  if(state == QUEUE_CLEAR || state == QUEUE_OVERFLOW || state == QUEUE_DRAIN)
  {
    m_state = state;
  }
  else
  {
    NS_ABORT_MSG("Unkown Queue State");
    return ;
  }
}

GspQueueDisc::QueueState
GspQueueDisc::GetState()
{
  return m_state;
}

void
GspQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);

  m_maxTime = Seconds(60);
}

} // namespace ns3

