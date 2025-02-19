/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
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
* Author: Yibo Zhu <yibzh@microsoft.com>
*/
#ifndef QBB_NET_DEVICE_H
#define QBB_NET_DEVICE_H

#include "ns3/point-to-point-net-device.h"
#include "ns3/qbb-channel.h"
//#include "ns3/fivetuple.h"
#include "ns3/event-id.h"
#include "ns3/broadcom-egress-queue.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/rdma-queue-pair.h"
#include <vector>
#include<map>
#include <ns3/rdma.h>

namespace ns3 {
  
// // // set ACK priority on hosts
// 	if (ack_high_prio)
// 		RdmaEgressQueue::ack_q_idx = 0;
class RdmaEgressQueue : public Object{  /// 出队队列
public:
	static const uint32_t qCnt = 8; /// 8个队列
	static uint32_t ack_q_idx;  /// ack队列index
	int m_qlast; //上一个处理队列索引
	uint32_t m_rrlast;  //轮询调度（Round Robin Scheduling）中用于跟踪上一次轮询的队列索引
	
  // 指向一个 DropTail 队列，表示用于存储 ACK 包的队列。DropTail 
  //队列是 ns-3 中的一种常见队列，它按 FIFO 方式排队，并在满时丢弃新来的数据包。
  Ptr<DropTailQueue> m_ackQ; // highest priority queue
  //指向队列对（Queue Pair）组的指针
	Ptr<RdmaQueuePairGroup> m_qpGrp; // queue pairs

	// callback for get next packet
  // Callback<Ptr<Packet>, Ptr<RdmaQueuePair>> 是一个模板类，它表示一个可以被调用的回调函数类型，该类型的回调函数接受一个 Ptr<RdmaQueuePair> 参数，并返回一个 Ptr<Packet>
	typedef Callback<Ptr<Packet>, Ptr<RdmaQueuePair> > RdmaGetNxtPkt;
	RdmaGetNxtPkt m_rdmaGetNxtPkt;

	static TypeId GetTypeId (void);
	RdmaEgressQueue();
  // 指定索引为 qIndex 的队列中出队数据包//
	Ptr<Packet> DequeueQindex(int qIndex);  
  // 根据队列状态（可能是某些队列暂停）选择下一个应该被处理的队列的索引
	int GetNextQindex(bool paused[]);

	int GetLastQueue();  //获取上一次处理的队列索引

  ///获取索引为 qIndex 的队列中数据包的总字节数。
	uint32_t GetNBytes(uint32_t qIndex);  
	uint32_t GetFlowCount(void);
	Ptr<RdmaQueuePair> GetQp(uint32_t i); //获取索引为 i 的队列对

  //恢复指定索引为 i 的队列，使其回到正常工作状态（例如处理被暂停或出错的队列）。
	void RecoverQueue(uint32_t i);

  ///放到高优先级队列中去
	void EnqueueHighPrioQ(Ptr<Packet> p);

	/// 清理高优先级队列，并使用回调 dropCb 追踪被丢弃的数据包
	void CleanHighPrio(TracedCallback<Ptr<const Packet>, uint32_t> dropCb);

	TracedCallback<Ptr<const Packet>, uint32_t> m_traceRdmaEnqueue;
	TracedCallback<Ptr<const Packet>, uint32_t> m_traceRdmaDequeue;
};

/**
 * \class QbbNetDevice
 * \brief A Device for a IEEE 802.1Qbb Network Link.
 */
class QbbNetDevice : public PointToPointNetDevice 
{
public:
  static const uint32_t qCnt = 8;	// Number of queues/priorities used

  static TypeId GetTypeId (void);

  QbbNetDevice ();
  virtual ~QbbNetDevice ();

  /**
   * Receive a packet from a connected PointToPointChannel.
   *
   * This is to intercept the same call from the PointToPointNetDevice
   * so that the pause messages are honoured without letting
   * PointToPointNetDevice::Receive(p) know
   *
   * @see PointToPointNetDevice
   * @param p Ptr to the received packet.
   */
  virtual void Receive (Ptr<Packet> p);

  /**
   * Send a packet to the channel by putting it to the queue
   * of the corresponding priority class
   *
   * @param packet Ptr to the packet to send
   * @param dest Unused
   * @param protocolNumber Protocol used in packet
   */
  virtual bool Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
  virtual bool SwitchSend (uint32_t qIndex, Ptr<Packet> packet, CustomHeader &ch);

  /**
   * Get the size of Tx buffer available in the device
   *
   * @return buffer available in bytes
   */
  //virtual uint32_t GetTxAvailable(unsigned) const;

  /**
   * TracedCallback hooks
   */
  void ConnectWithoutContext(const CallbackBase& callback);
  void DisconnectWithoutContext(const CallbackBase& callback);

  bool Attach (Ptr<QbbChannel> ch);

  virtual Ptr<Channel> GetChannel (void) const;

   void SetQueue (Ptr<BEgressQueue> q);
   Ptr<BEgressQueue> GetQueue ();
   virtual bool IsQbb(void) const;
   void NewQp(Ptr<RdmaQueuePair> qp);
   void ReassignedQp(Ptr<RdmaQueuePair> qp); ///重新分配qp
   void TriggerTransmit(void);

	void SendPfc(uint32_t qIndex, uint32_t type); // type: 0 = pause, 1 = resume

	TracedCallback<Ptr<const Packet>, uint32_t> m_traceEnqueue; // 入队
	TracedCallback<Ptr<const Packet>, uint32_t> m_traceDequeue; //出队
	TracedCallback<Ptr<const Packet>, uint32_t> m_traceDrop;  //丢弃
	TracedCallback<uint32_t> m_tracePfc; // 0: resume, 1: pause
protected:

	//Ptr<Node> m_node;

  bool TransmitStart (Ptr<Packet> p);
  

  // 清理和释放与该类对象相关的资源。
  virtual void DoDispose(void);

  /// Reset the channel into READY state and try transmit again
  virtual void TransmitComplete(void);

  /// Look for an available packet and send it using TransmitStart(p)
  virtual void DequeueAndTransmit(void);

  /// Resume a paused queue and call DequeueAndTransmit()
  virtual void Resume(unsigned qIndex);  //恢复被暂停的队列

  /**
   * The queues for each priority class.
   * @see class Queue
   * @see class InfiniteQueue
   */
  Ptr<BEgressQueue> m_queue;   /// 用于交换机队列

  Ptr<QbbChannel> m_channel;
  
  //pfc
  bool m_qbbEnabled;	//< PFC behaviour enabled
  bool m_qcnEnabled;
  bool m_dynamicth; //是否启用动态阈值
  uint32_t m_pausetime;	//< Time for each Pause
  bool m_paused[qCnt];	//< Whether a queue paused

  //qcn

  /* RP parameters */
  EventId  m_nextSend;		//< The next send event
  /* State variable for rate-limited queues */

  //qcn

  struct ECNAccount{
	  Ipv4Address source;
	  uint32_t qIndex;  //队列的索引
	  uint32_t port;
	  uint8_t ecnbits;
	  uint16_t qfb;  //队列反馈
	  uint16_t total;
  };

  std::vector<ECNAccount> *m_ecn_source;

public:
	Ptr<RdmaEgressQueue> m_rdmaEQ;  /// 用于网卡队列
	void RdmaEnqueueHighPrioQ(Ptr<Packet> p);

	// callback for processing packet in RDMA
	typedef Callback<int, Ptr<Packet>, CustomHeader&> RdmaReceiveCb;
	RdmaReceiveCb m_rdmaReceiveCb;
	// callback for link down
	typedef Callback<void, Ptr<QbbNetDevice> > RdmaLinkDownCb;
	RdmaLinkDownCb m_rdmaLinkDownCb;
	// callback for sent a packet
	typedef Callback<void, Ptr<RdmaQueuePair>, Ptr<Packet>, Time> RdmaPktSent;
	RdmaPktSent m_rdmaPktSent;

	Ptr<RdmaEgressQueue> GetRdmaQueue();
	void TakeDown(); // take down this device
	void UpdateNextAvail(Time t);

	TracedCallback<Ptr<const Packet>, Ptr<RdmaQueuePair> > m_traceQpDequeue; // the trace for printing dequeue
};

} // namespace ns3

#endif // QBB_NET_DEVICE_H
