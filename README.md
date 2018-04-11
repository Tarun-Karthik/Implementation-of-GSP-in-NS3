# Implementation of Global Synchronization Protection scheme in NS-3
## Course Code: CO365

### GSP
The Global Synchronization Protection (GSP) Scheme is a regular tail drop packet queueing 
mechanisim that prevents global synchronisation. Consider N Tcp flows who share a common
bottleneck link. All the flows simultaneously probe the link for extra bandwidth by
gradually increasing their cwnd. When the aggregate bit rate of the flows saturates the
link capacity, the link enters congestion. Any further increase has no effect on the link
throughput and only contributes to the queue length and delay accumulation. When the 
tail-drop queue drops the first packet, the queue length immediately drops by only one 
unit but takes an entire RTT before the cwnd reduction induced by the packet loss shows
its full impact.
During the RTT interval that follows the first drop event the TCP senders of all flows
keep probing for bandwidth at the same pace as before. That is, the packets arriving
to the queue exceed those departing by N /2  units. Since the queue is already full,
it drops N /2 packets. If each dropped packet belongs to a different flow, every other 
flow ends up contracting its cwnd at the same time. If the buffer is far smaller than
required by the BDP rule, the queue depletes and the link operates at sub-capacity levels
until the combined cwnd of all flows returns large enough to establish again a continuous
presence of packets in the queue. The queue collapse may be less severe when losses hit
one or more flows multiple times, so that the fraction of the total population affected
by losses is smaller than 50%, but statistically it still presents a problem.

There are three variants of GSP algorithm:  basic, adaptive, and delay-based. 
#### Basic GSP
In this algorithm global synchronization can be averted by removing the extra 
packet drops after the first one. A Threshold is set well below the buffer limit.
When the first packet is dropped a time interval is started during which all
threshold voilations are ignored. At the end of the no-drop interval the queue length
is well below the drop threshold and requires no further action.

### Adaptive GSP
Since the same GSP configuration must work well under most scenarios of 
practical interest, the scheme must adapt the interval value automatically. 
In single-drop operation the queue is most of the time below the threshold 
and drops a packet only once in many expirations of the maximum interval 
duration. No adaptation is necessary. The interval value must be reduced 
as soon as the queue starts spending more time above the threshold than
below. Let presetInt be the initial and maximum setting for the adaptive 
interval variable, tau the time constant for the adaptation loop, and alpha 
the emphasis factor for the time spent above the threshold, such that the 
reaction to load changes is stronger. As a rule of thumb, tau should be
comfortably larger than presetInt (we set the ratio at 5) and alpha should
not be much larger than 1 

at every packet arrival DO: <br />
cumulTime += (alpha * time_above_threshold – time_below_threshold) <br />
cumulTime = min(maxTime, max(0, cumulTime)) <br />
interval = presetInt / (1 + cumulTime / tau) <br />
NEXT proceed with basic GSP algorithm <br />

### Delay-Based GSP
We enable delay-based operation in GSP by generalizing the meaning of the 
condition queue > threshold. Both terms can be expressed in memory-size units, 
time units, or a combination of the two. The queuing delay can be measured
with one of the methods of CoDel and PIE. CoDel uses timestamps that it 
associates with packets when they arrive to the queue and then subtracts from 
the times of departure. PIE estimates the drain rate for translation of the 
actual queue size into an expected queuing delay. In our experiments we used
the timestamp approach.


### References

[1] Wolfram Lautenschlaeger,  Andrea Francini Global Synchronization Protection for Bandwidth
Sharing TCP Flows in High-Speed Links 

[2]  Wolfram Lautenschlaege Active Queue Management and packet scheduling Internet Draft
