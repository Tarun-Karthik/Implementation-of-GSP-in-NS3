#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
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
      .AddAttribute ("QueueLimit",
                     "Queue limit in packets",
                     UintegerValue (100),
                     MakeUintegerAccessor (&GspQueueDisc::SetQueueLimit),
                     MakeUintegerChecker<uint32_t> ()) // confirm
      .AddAttribute ("A",
                     "Value of alpha",
                     DoubleValue (2),
                     MakeDoubleAccessor (&GspQueueDisc::m_a),
                     MakeDoubleChecker<double> ())
      .AddAttribute("Interval",
                    "Value that will be incremented to the time out",
                    TimeValue (),    //twice of RTT
                    MakeTimeAccessor (&GspQueueDisc::interval),
                    MakeTimeChecker ())
      .AddAttribute("Adaptive Variable",
                    "Controlling the adaptation speed",
                    TimeValue (),
                    MakeTimeAccessor (&GspQueueDisc::adapt),
                    MakeTimeChecker ())

    ;
    return tid;
  }

}
