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
 * Authors: A Tarun Karthik <tarun.karthik.97@gmail.com>
            Amshula U S <>
            Manoj Kumar <mnkumar493@gmail.com>
 */

 /** Network topology
 *                     10Gb/s
 *                /-------------n2
 *                /
 *      10Gb/s    /
 * n0------------n1
 *                /
 *                /    10Gb/s
 *                /-------------n3
 */

 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/flow-monitor-helper.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/traffic-control-module.h"

 using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("SfqExample");

 NodeContainer n0n1;
 NodeContainer n1n2;
 NodeContainer n1n3;

 Ipv4InterfaceContainer i0i1;
 Ipv4InterfaceContainer i1i2;
 Ipv4InterfaceContainer i1i3;

 uint16_t port = 50000;
