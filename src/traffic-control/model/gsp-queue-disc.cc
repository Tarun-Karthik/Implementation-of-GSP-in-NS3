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
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/abort.h"
#include "gsp-queue-disc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GspQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (GspQueueDisc);

TypeId GspQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GspQueueDisc")
    .SetParent<QueueDisc> ()                      
    .SetGroupName ("TrafficControl")
    .AddConstructor<GspQueueDisc> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())                     
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_DISC_MODE_PACKETS),
                   MakeEnumAccessor (&GspQueueDisc::SetMode,
                                     &GspQueueDisc::GetMode),
                   MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
                                    QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))
    .AddAttribute ("A",
                   "Value of alpha",
                   DoubleValue (2),
                   MakeDoubleAccessor (&GspQueueDisc::m_a),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Interval",
                   "Value that will be incremented to the time out",
                   TimeValue (Seconds (0.2)),
                   MakeTimeAccessor (&GspQueueDisc::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("AdaptiveVariable",
                   "Controlling the adaptation speed",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&GspQueueDisc::m_adapt),
                   MakeTimeChecker ())
    .AddAttribute ("Threshold",
                   "Limit in queue size above which the packet should be dropped",
                   DoubleValue (),
                   MakeDoubleAccessor (&GspQueueDisc::m_threshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ThresholdSeconds",
                   "Equivalent to Threshold but in Seconds",
                   TimeValue (),
                   MakeTimeAccessor (&GspQueueDisc::m_secThreshold),
                   MakeTimeChecker ())
    .AddAttribute ("Timeout",
                   "amount of time for which the packets will not be dropped",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&GspQueueDisc::m_timeout),
                   MakeTimeChecker ())
    .AddAttribute ("GspMode",
                   "Determines which GSP algorithm to use",
                   EnumValue (),
                   MakeEnumAccessor (&GspQueueDisc::GetGspMode,
                                     &GspQueueDisc::SetGspMode),
                   MakeEnumChecker (BASIC_GSP, "BASIC_GSP",
                                    ADAPTIVE_GSP, "ADAPTIVE_GSP",
                                    DELAY_GSP, "DELAY_GSP"))
    ;
    return tid;
}

GspQueueDisc::GspQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE) 
{
  NS_LOG_FUNCTION (this);
}

GspQueueDisc::~GspQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
GspQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);

  if (mode == QUEUE_DISC_MODE_BYTES)
    {
      SetMaxSize (QueueSize (QueueSizeUnit::BYTES, GetMaxSize ().GetValue ()));
    }
  else if (mode == QUEUE_DISC_MODE_PACKETS)
    {
      SetMaxSize (QueueSize (QueueSizeUnit::PACKETS, GetMaxSize ().GetValue ()));
    }
  else
    {
      NS_ABORT_MSG ("Unknown queue size unit");
    }
}

GspQueueDisc::QueueDiscMode
GspQueueDisc::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return (GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS ? QUEUE_DISC_MODE_PACKETS : QUEUE_DISC_MODE_BYTES);
}

bool GspQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  if (m_mode == BASIC_GSP)
    {
      if (GetCurrentSize ().GetValue () + item->GetSize () > m_threshold && Simulator::Now () > m_timeout)
        {
          NS_LOG_LOGIC ("Queue Size is greater than Threshold");
          DropBeforeEnqueue (item, FORCED_DROP);
          m_timeout = Simulator::Now () + m_interval;
          return false;
        }
    }
  else if (m_mode == ADAPTIVE_GSP)
    {

      if (GetCurrentSize ().GetValue () + item->GetSize () > GetMaxSize ().GetValue ())
        {
          m_state = QUEUE_OVERFLOW;
        }
      else if (m_state == QUEUE_OVERFLOW && GetCurrentSize ().GetValue () == 0)
        {
          m_state = QUEUE_DRAIN;
        }
      else if (m_state == QUEUE_DRAIN && GetCurrentSize ().GetValue () > m_threshold)
        {
          m_state = QUEUE_CLEAR;
        }

      m_cumTime += m_a * m_timeAboveThreshold;

      if (m_state == QUEUE_CLEAR)
        {
          m_cumTime = m_cumTime - m_timeBelowThreshold;
        }

      m_cumTime = std::min (m_maxTime, std::max (Seconds (0), m_cumTime));
      Time presetInterval = Time (Seconds (0.2));
      m_interval = (presetInterval / (1 + m_cumTime / m_adapt));

      if (GetCurrentSize ().GetValue () + item->GetSize () > m_threshold && Simulator::Now () > m_timeout)
        {
          NS_LOG_LOGIC ("Queue Size is greater than Threshold");
          DropBeforeEnqueue (item, FORCED_DROP);
          m_timeout = Simulator::Now () + m_interval;
          return false;
        }
    }
  else if (m_mode == DELAY_GSP)
    {
      if (m_tiq > m_secThreshold && Simulator::Now () > m_timeout)
        {
          NS_LOG_LOGIC ("Time in Queue is greater than Threshold");
          DropBeforeEnqueue (item, FORCED_DROP);
          m_timeout = Simulator::Now () + m_interval;
          return false;
        }
    }
  bool retval = GetInternalQueue (0)->Enqueue (item);
  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());
  return retval;
}

Ptr<QueueDiscItem>
GspQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Time t_queueTime = Time (Seconds (Simulator::Now () - item->GetTimeStamp ()));

  if (m_mode == DELAY_GSP)
    {
      m_tiq = t_queueTime;
    }

  if (m_mode == ADAPTIVE_GSP)
    {
      if (t_queueTime > m_secThreshold)
        {
          m_timeAboveThreshold = Time (Seconds (t_queueTime - m_secThreshold));
        }
      else
        {
          m_timeBelowThreshold = Time (Seconds (m_secThreshold - t_queueTime));
        }
    }
  return item;
}

Ptr<const QueueDiscItem>
GspQueueDisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();
  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  return item;
}

void 
GspQueueDisc::SetGspMode(GspMode mode)
{
  if(mode == BASIC_GSP || mode == ADAPTIVE_GSP || mode == DELAY_GSP)
  {
    m_mode = mode;
  }
  else 
  {
    NS_ABORT_MSG("Unknown GSP mode");
  }
  return ;
}

GspQueueDisc::GspMode
GspQueueDisc::GetGspMode(void) const
{
  return m_mode;
}

bool GspQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("GspQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("GspQueueDisc needs no packet filter");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("GspQueueDisc needs 1 internal queue");
      return false;
    }
  return true;
}

void GspQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);

  //Initialize m_secThreshold using bandwidth-delay product
  m_maxTime = Time (Seconds (60));
  m_cumTime = Time (Seconds (0));
}

} // namespace ns3
