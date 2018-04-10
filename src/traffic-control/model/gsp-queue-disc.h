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

 #ifndef GSP_QUEUE_DISC_H
 #define GSP_QUEUE_DISC_H

 #include "ns3/queue-disc.h"
 #include "ns3/data-rate.h"

namespace ns3 {
/**
 * \ingroup traffic-control
 *
 * \brief Implements Global Synchronization Protection Queue Management discipline
 */
class GspQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId 
   */
  static TypeId GetTypeId (void);

  /**
   * \brief GspQueueDisc Constructor
   */
  GspQueueDisc ();

  /**
   * \brief GspQueueDisc Destructor
   */
  virtual ~GspQueueDisc ();

  /**
   * \brief Enumeration of the queue disc modes supported in the class.
   */
  enum QueueDiscMode
  {
    QUEUE_DISC_MODE_PACKETS,     
    QUEUE_DISC_MODE_BYTES,       
  };

  /**
   * \brief Set the operating mode to this queue disc. 
   * 
   * \param mode The operating mode of this queue disc.
   */
  void SetMode (QueueDiscMode mode);

  /**
   * \brief Get the operating mode to this queue disc. 
   * 
   * \returns The operating mode of this queue disc.
   */
  QueueDiscMode GetMode (void) const;

  /**
   * \brief Enumeration of the GSP types for this queue disc.
   */
  enum GspMode
  {
    BASIC_GSP,
    ADAPTIVE_GSP,
    DELAY_GSP,
  };

  /**
   * \brief Set the operating GSP type of this queue disc.
   * 
   * \param mode The operating GSP variant of this queue disc.
   */
  void SetGspMode(GspMode mode);

  /**
   * \brief get the operating GSP type of this queue disc.
   * 
   * \returns The operating GSP variant of this queue disc.
   */
  GspMode GetGspMode(void) const;

  /**
   * \brief Enumeration of queue states as defined by GSP for this queue disc.
   */
  enum QueueState
  {
    QUEUE_CLEAR,
    QUEUE_OVERFLOW,
    QUEUE_DRAIN,
  };

  // Reasons for dropping a packet
  static constexpr const char* FORCED_DROP = "Forced drop";      //!< Drops due to queue limit: reactive


private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  // ** Variables supplied by the user
  double m_a;
  double m_threshold;
  Time m_adapt;
  Time m_interval;
  Time m_secThreshold;
  Time m_timeout;
  GspMode m_mode;
  DataRate m_linkBandwidth;
  
  // ** Variables maintained by GSP
  Time m_tiq;
  Time m_timeAboveThreshold;
  Time m_timeBelowThreshold;
  QueueState m_state;
  Time m_maxTime;
  Time m_cumTime;

};
};

 #endif
