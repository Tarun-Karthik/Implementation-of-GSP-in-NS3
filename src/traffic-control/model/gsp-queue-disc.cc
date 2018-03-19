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
    static TypeId tid = TypeId (ns3::GspQueueDisc)
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
                    TimeValue (),    //twice of RTT
                    MakeTimeAccessor (&GspQueueDisc::SetInterval,
                                      &GspQueueDisc::GetInterval),
                    MakeTimeChecker ())
      .AddAttribute("Adaptive Variable",
                    "Controlling the adaptation speed",
                    TimeValue (),
                    MakeTimeAccessor (&GspQueueDisc::adapt),
                    MakeTimeChecker ())
      .AddAttribute("Threshold",
                    "Limit above which the packet should be dropped",
                    DoubleValue (500),
                    MakeDoubleAccessor(&GspQueueDisc::SetThreshold,
                                       &GspQueueDisc::GetThreshold),
                    MakeDoubleAccessor ())
      .AddAttribute("Timeout",
                    "amount of time for which the packets will not be dropped",
                    TimeValue (Seconds(0)),
                    MakeTimeAccessor (&GspQueueDisc::SetTimeout,
                                      &GspQueueDisc::GetTimeout),
                    MakeTimeChecker ())

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


//******************************************************************************
//                        GSP BASIC
//******************************************************************************
  bool
  GspQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
  {
    NS_LOG_FUNCTION (this << item);
    //Enqueue function goes here
    if(GetCurrentSize () + item > GetThreshold () && Simulator::Now () > GetTimeout ())
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
//******************************************************************************
//******************************************************************************


//******************************************************************************
//                            Adaptive Gsp
//******************************************************************************
bool
GspQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

}




//******************************************************************************
//******************************************************************************

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
