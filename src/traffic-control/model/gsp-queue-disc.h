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

 namespace ns3{
   class GspQueueDisc : public QueueDisc{
   public:

    static TypeId GetTypeId (void);

    GspQueueDisc ();

    virtual ~GspQueueDisc ();

    enum GspMode
    {
      BASIC_GSP,
      ADAPTIVE_GSP,
      DELAY_GSP,
    };

    void SetMode(GspMode mode);

    GspMode GetMode(void) const;

    enum QueueState
    {
      QUEUE_CLEAR,
      QUEUE_OVERFLOW,
      QUEUE_DRAIN,
    };

    void SetState(QueueState state);

    QueueState GetState(void) const;

  private:
    virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
    virtual Ptr<QueueDiscItem> DoDequeue (void);
    virtual Ptr<const QueueDiscItem> DoPeek (void);
    virtual bool CheckConfig (void);
    virtual void InitializeParams (void);

    GspMode m_mode;
    QueueState m_state;
    Time m_maxTime;
  };
 }

 #endif
