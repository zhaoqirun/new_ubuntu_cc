/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 */

#ifndef POINT_TO_POINT_NET_DEVICE_H
#define POINT_TO_POINT_NET_DEVICE_H

#include <string.h>
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class Queue;
class PointToPointChannel;
class ErrorModel;

/**
 * \defgroup point-to-point PointToPointNetDevice
 * This section documents the API of the ns-3 point-to-point module. For a generic functional description, please refer to the ns-3 manual.
 */

/**
 * \ingroup point-to-point
 * \class PointToPointNetDevice
 * \brief A Device for a Point to Point Network Link.
 *
 * This PointToPointNetDevice class specializes the NetDevice abstract
 * base class.  Together with a PointToPointChannel (and a peer 
 * PointToPointNetDevice), the class models, with some level of 
 * abstraction, a generic point-to-point or serial link.
 * Key parameters or objects that can be specified for this device 
 * include a queue, data rate, and interframe transmission gap (the 
 * propagation delay is set in the PointToPointChannel).
 */
class PointToPointNetDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a PointToPointNetDevice
   *
   * This is the constructor for the PointToPointNetDevice.  It takes as a
   * parameter a pointer to the Node to which this device is connected, 
   * as well as an optional DataRate object.
   */
  PointToPointNetDevice ();

  /**
   * Destroy a PointToPointNetDevice
   *
   * This is the destructor for the PointToPointNetDevice.
   */
  virtual ~PointToPointNetDevice ();

  /**
   * Set the Data Rate used for transmission of packets.  The data rate is
   * set in the Attach () method from the corresponding field in the channel
   * to which the device is attached.  It can be overridden using this method.
   *
   * @see Attach ()
   * @param bps the data rate at which this object operates
   */
  void SetDataRate (DataRate bps); //置数据率。用于指定设备操作时的数据传输速率。

  DataRate GetDataRate();    //获取当前数据率。返回设备的传输速率。

  /**
   * Set the interframe gap used to separate packets.  The interframe gap
   * defines the minimum space required between packets sent by this device.
   *
   * @param t the interframe gap time
   */
  void SetInterframeGap (Time t);  //定义发送数据包之间的最小时间间隔。

  /**
   * Attach the device to a channel.
   *
   * @param ch Ptr to the channel to which this object is being attached.
   */
  bool Attach (Ptr<PointToPointChannel> ch);  //

  /**
   * Attach a queue to the PointToPointNetDevice.
   *
   * The PointToPointNetDevice "owns" a queue that implements a queueing 
    method such as DropTail or RED.
   *
   * @see Queue
   * @see DropTailQueue
   * @param queue Ptr to the new queue.
   */
  void SetQueue (Ptr<Queue> queue); //设置队列。将一个队列附加到设备上。

  /**
   * Get a copy of the attached Queue.
   *
   * @returns Ptr to the queue.
   */
  Ptr<Queue> GetQueue (void) const;

  /**
   * Attach a receive ErrorModel to the PointToPointNetDevice.
   *
   * The PointToPointNetDevice may optionally include an ErrorModel in
   * the packet receive chain.
   *
   * @see ErrorModel
   * @param em Ptr to the ErrorModel.
   */
  void SetReceiveErrorModel (Ptr<ErrorModel> em); //设置接收错误模型。可以在接收链中包含错误模型，模拟数据包丢失。

  /**
   * Receive a packet from a connected PointToPointChannel.
   *
   * The PointToPointNetDevice receives packets from its connected channel
   * and forwards them up the protocol stack.  This is the public method
   * used by the channel to indicate that the last bit of a packet has 
   * arrived at the device.
   *
   * @see PointToPointChannel
   * @param p Ptr to the received packet.
   */
  //  接收来自通道的包并将其转发至协议栈。这是由通道调用的方法，表示一个数据包已到达设备。
  void Receive (Ptr<Packet> p);

  // The remaining methods are documented in ns3::NetDevice*

  virtual void SetIfIndex (const uint32_t index); //设置接口索引。将接口索引设置为指定的值。
  virtual uint32_t GetIfIndex (void) const;      //获取接口索引。返回设备的接口标识符。

  virtual Ptr<Channel> GetChannel (void) const;

  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;

  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;

  virtual bool IsLinkUp (void) const;       //检查链路是否连接。

  virtual void AddLinkChangeCallback (Callback<void> callback);  //添加一个回调函数，用于监测链路状态变化。

  virtual bool IsBroadcast (void) const;  //检查当前网络设备是否支持广播功能
  virtual Address GetBroadcast (void) const;  //：获取设备的广播地址。

  virtual bool IsMulticast (void) const; //检查当前网络设备是否支持多播功能
  virtual Address GetMulticast (Ipv4Address multicastGroup) const; //获取特定多播组的地址。根据提供的多播组地址返回对应的设备多播地址。

  virtual bool IsPointToPoint (void) const; //检查当前设备是否为点对点连接
  virtual bool IsBridge (void) const;  //检查当前设备是否为桥接设备

  // 发送数据包到指定目标。将数据包通过网络设备发送到目标地址。
  virtual bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
  // 从指定源地址发送数据包到目标地址
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  // 返回的指针指向当前设备所在的节点。
  virtual Ptr<Node> GetNode (void) const;
  //将设备与特定节点关联。
  virtual void SetNode (Ptr<Node> node);
  //检查当前设备是否需要使用 ARP 进行地址解析
  virtual bool NeedsArp (void) const;
  //设置数据包接收时的回调函数。
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  //获取与指定 IPv6 地址对应的多播地址。返回设备所支持的多播地址。
  virtual Address GetMulticast (Ipv6Address addr) const;
  // 设置混杂模式下的数据包接收回调函数。
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  //   不确定，看.cc代码文件
  virtual bool SupportsSendFrom (void) const;

protected:
  void DoMpiReceive (Ptr<Packet> p); //处理从 MPI（Message Passing Interface）通道接收到的数据包。

  virtual void DoDispose (void);  //释放对象间的循环引用或未释放的资源。
  
    /**
   * Enumeration of the states of the transmit machine of the net device.
   */
  enum TxMachineState
  {
    READY,   /**< The transmitter is ready to begin transmission of a packet */ 
           //表示网络设备的传输器处于空闲状态，可以开始新的数据包传输。

    BUSY     /**< The transmitter is busy transmitting a packet */
    // 表示网络设备的传输器正在传输数据包，当前不能处理新的数据包传输请求。
  };
  /**
   * The state of the Net Device transmit state machine.
   * @see TxMachineState
   */
  TxMachineState m_txMachineState;
  
private:
  //该函数私有化意味着用户无法直接通过赋值将一个 PointToPointNetDevice 对象赋值给另一个。
  PointToPointNetDevice& operator = (const PointToPointNetDevice &);
  //禁止拷贝构造函数。类似于赋值操作符的私有化，禁止了通过拷贝构造函数创建 PointToPointNetDevice
  //  对象的副本。这种设计防止了网络设备的非预期复制
  PointToPointNetDevice (const PointToPointNetDevice &);


protected:

  /**
   * \returns the address of the remote device connected to this device
   * through the point to point channel.
   */
  Address GetRemote (void) const; //获取与当前设备通过点对点通道连接的远程设备的地址。

  /**
   * Adds the necessary headers and trailers to a packet of data in order to
   * respect the protocol implemented by the agent.
   * \param p packet
   * \param protocolNumber protocol number
   * 为数据包添加必要的头部和尾部信息。具体内容取决于协议号 protocolNumber
   */
  void AddHeader (Ptr<Packet> p, uint16_t protocolNumber);

  /**
   * Removes, from a packet of data, all headers and trailers that
   * relate to the protocol implemented by the agent
   * \param p Packet whose headers need to be processed
   * \param param An integer parameter that can be set by the function   ？？？意思
   * \return Returns true if the packet should be forwarded up the
   * protocol stack.
   * 从数据包中移除与协议相关的头部和尾部信息。处理完成后，返回是否应继续将数据包向上传递到协议栈。
   */
  bool ProcessHeader (Ptr<Packet> p, uint16_t& param);

  /**
   * Start Sending a Packet Down the Wire.
   *
   * The TransmitStart method is the method that is used internally in the
   * PointToPointNetDevice to begin the process of sending a packet out on
   * the channel.  The corresponding method is called on the channel to let
   * it know that the physical device this class represents has virtually
   * started sending signals.  An event is scheduled for the time at which
   * the bits have been completely transmitted.
   *
   * @see PointToPointChannel::TransmitStart ()
   * @see TransmitCompleteEvent ()
   * @param p a reference to the packet to send
   * @returns true if success, false on failure
   * ，用于启动数据包的传输过程。该方法表示网络设备开始通过物理信道发送数据包，并根据设备的传输速率调度传输完成事件。
   */
  bool TransmitStart (Ptr<Packet> p);

  /**
   * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
   *
   * The TransmitComplete method is used internally to finish the process
   * of sending a packet out on the channel.
   * 在数据包发送完成后，调用此方法完成数据包的传输过程，并开始帧间间隔（Interframe Gap）
   * 。帧间间隔确保在两个连续数据包之间有适当的时间间隔。
   */
  void TransmitComplete (void);

  void NotifyLinkUp (void);  //通知设备链路已建立，通常会调用此方法向上层协议栈或监听器通知链路状态的变化。



  /**
   * The data rate that the Net Device uses to simulate packet transmission
    timing.
   * @see class DataRate
   */
  DataRate       m_bps;  //设备的传输速率，单位为比特每秒（bps）。

  /**
   * The interframe gap that the Net Device uses to throttle packet
   * transmission
   * @see class Time
   * 表示设备在发送两个数据包之间的最小间隔时间。该变量用于控制帧间间隔，以模拟物理层的间隔时间要求。
   */
  Time           m_tInterframeGap;

  /**
   * The PointToPointChannel to which this PointToPointNetDevice has been
   * attached.
   * @see class PointToPointChannel
   */
  Ptr<PointToPointChannel> m_channel; //该成员变量代表设备所属的点对点通道。

  /**
   * The Queue which this PointToPointNetDevice uses as a packet source.
   * Management of this Queue has been delegated to the PointToPointNetDevice
   * and it has the responsibility for deletion.
   * @see class Queue
   * @see class DropTailQueue
   * 指向当前设备使用的队列对象，是数据包的来源
   */
  Ptr<Queue> m_queue;

  /**
   * Error model for receive packet events
   * 用于处理接收数据包时的错误模型（ErrorModel）。该成员变量允许在数据包接收过程中引入错误，
   * 用于模拟现实网络中的数据包丢失或损坏。
   */
  Ptr<ErrorModel> m_receiveErrorModel;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *   当数据包进入设备的 MAC 层，准备传输时触发的回调。这是在数据包被排队等待发送之前的时刻，
   * 位于 L3/L2（网络层与链路层）转换点。
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;  //准备马上将要发送的时刻

  /**
   * The trace source fired when packets coming into the "top" of the device
   * at the L3/L2 transition are dropped before being queued for transmission.
   *
   * \see class CallBackTraceSource
   * 当数据包进入设备的 MAC 层时被丢弃的回调。
   * 可能由于队列已满或其他原因而导致丢弃
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a promiscuous trace (which doesn't mean a lot here
   * in the point-to-point device).
   *
   * \see class CallBackTraceSource
   * 在数据包被设备成功接收，并在其被向上层协议栈传递之前触发的回调。
   * 这是混杂模式下的追踪，用于接收与本设备无关的所有包。
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a non-promiscuous trace (which doesn't mean a lot 
   * here in the point-to-point device).
   *
   * \see class CallBackTraceSource
   * 在数据包成功接收并传递到上层协议栈之前触发的回调。不同于混杂模式，
   * 这个回调仅追踪与当前设备有关的包。
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but are dropped before being forwarded up to higher layers (at the L2/L3 
   * transition).
   *
   * \see class CallBackTraceSource
   * 当设备成功接收到数据包，但在传递到上层协议栈之前被丢弃时触发的回调。
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * The trace source fired when a packet begins the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   * 当物理层medium 开始传输数据包时触发的回调。
   */
  TracedCallback<Ptr<const Packet> > m_phyTxBeginTrace;

  /**
   * The trace source fired when a packet ends the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   * 当物理层完成数据包的接收时触发的回调，表明设备已完整接收到整个数据包。
   */
  TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet before it tries
   * to transmit it.
   *
   * \see class CallBackTraceSource
   * 当物理层在接收数据包时发生错误并丢弃数据包时触发的回调。
   */
  TracedCallback<Ptr<const Packet> > m_phyTxDropTrace;

  /**
   * The trace source fired when a packet begins the reception process from
   * the medium -- when the simulated first bit(s) arrive.
   *
   * \see class CallBackTraceSource
   * 数据包发送后，将要到达接受者的 时刻触发的回调。
   */
  TracedCallback<Ptr<const Packet> > m_phyRxBeginTrace;

  /**
   * The trace source fired when a packet ends the reception process from
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet it has received.
   * This happens if the receiver is not enabled or the error model is active
   * and indicates that the packet is corrupt.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected 
   * to the device.  Unlike your average everyday sniffer, this trace source 
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where 
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   * 模拟非混杂模式下的协议嗅探器，监听设备上的网络流量。
   * 发送时数据包从设备队列中出列、准备传输时触发。对应于 Linux 中的 dev_queue_xmit_nit 调用，位于数据包实际发送到网络介质之前。
   * 
   * 接收时：当设备接收到数据包并准备调用接收回调之前触发。对应于 Linux 中 netif_receive_skb 调用之前。
   * 用于记录或分析通过设备传输的实际网络数据包，帮助开发者在仿真中观察网络流量
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where 
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   * 发送时：数据包从设备队列中出列、准备传输时触发，与 m_snifferTrace 相同，位于数据包发送到介质之前。
    接收时：设备接收到数据包并准备调用接收回调之前触发，捕获所有目的地址的数据包，
    甚至那些不属于当前设备的数据包。
   */
  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  Ptr<Node> m_node;
  Mac48Address m_address; //使用 48 位的 MAC 地址格式
  NetDevice::ReceiveCallback m_rxCallback; //当网络设备接收到数据包时，会调用这个回调函数来处理该数据包。
  NetDevice::PromiscReceiveCallback m_promiscCallback; //混杂模式下的数据包接收回调函数
  uint32_t m_ifIndex; //网络设备的接口索引（Interface Index）
  bool m_linkUp; //链路状态是否处于连接
  TracedCallback<> m_linkChangeCallbacks;

  static const uint16_t DEFAULT_MTU = 1500;

  /**
   * The Maximum Transmission Unit.  This corresponds to the maximum 
   * number of bytes that can be transmitted as seen from higher layers.
   * This corresponds to the 1500 byte MTU size often seen on IP over 
   * Ethernet.
   */
  uint32_t m_mtu;

  Ptr<Packet> m_currentPkt;

  /**
   * \brief PPP to Ethernet protocol number mapping
   * \param protocol A PPP protocol number
   * \return The corresponding Ethernet protocol number
   */
  static uint16_t PppToEther (uint16_t protocol);

  /**
   * \brief Ethernet to PPP protocol number mapping
   * \param protocol An Ethernet protocol number
   * \return The corresponding PPP protocol number
   */
  static uint16_t EtherToPpp (uint16_t protocol);
};

} // namespace ns3

#endif /* POINT_TO_POINT_NET_DEVICE_H */
