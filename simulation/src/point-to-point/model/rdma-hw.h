#ifndef RDMA_HW_H
#define RDMA_HW_H

#include <ns3/rdma.h>
#include <ns3/rdma-queue-pair.h>
#include <ns3/node.h>
#include <ns3/custom-header.h>
#include "qbb-net-device.h"
#include <unordered_map>
#include "pint.h"

namespace ns3 {

struct RdmaInterfaceMgr{  /// 网卡
	Ptr<QbbNetDevice> dev;
	Ptr<RdmaQueuePairGroup> qpGrp;

	RdmaInterfaceMgr() : dev(NULL), qpGrp(NULL) {}
	RdmaInterfaceMgr(Ptr<QbbNetDevice> _dev){
		dev = _dev;
	}
};

class RdmaHw : public Object {
public:

	static TypeId GetTypeId (void);
	RdmaHw();

	Ptr<Node> m_node;
	DataRate m_minRate;		//< Min sending rate
	uint32_t m_mtu;   		//最大传输单元（
	uint32_t m_cc_mode;     //which mode of DCQCN is running
	double m_nack_interval;    //m_nack_interval定义了发送方在接收到NACK后，重新发送数据包的时间间隔。
	uint32_t m_chunk;   //数据块的大小  L2ChunkSize
	uint32_t m_ack_interval;  // m_ack_interval定义了接收方在接收到数据包后，发送ACK的??间隔。  L2AckInterval
	bool m_backto0;  //   Layer 2 go back to zero transmission.

	// Use variable window size or not   |  Fast React to congestion feedback
	bool m_var_win,           m_fast_react;
	bool m_rateBound;                  //Bound packet sending by rate, for test only
	//用于存储和管理当前运行的所有RDMA网络接口。
	std::vector<RdmaInterfaceMgr> m_nic; // list of running nic controlled by this RdmaHw
	std::unordered_map<uint64_t, Ptr<RdmaQueuePair> > m_qpMap; // mapping from uint64_t to qp
	std::unordered_map<uint64_t, Ptr<RdmaRxQueuePair> > m_rxQpMap; // mapping from uint64_t to rx qp
	//std::vector<int>可能存储可能的ECMP（等价多路径）端口索引。
	std::unordered_map<uint32_t, std::vector<int> > m_rtTable; // map from ip address (u32) to possible ECMP port (index of dev)

	// qp complete callback  
	//它接受一个Ptr<RdmaQueuePair>类型的参数，用于处理QP（队列对）完成时的回调
	typedef Callback<void, Ptr<RdmaQueuePair> > QpCompleteCallback;
	//用于存储QP完成时的回调函数
	QpCompleteCallback m_qpCompleteCallback;

	void SetNode(Ptr<Node> node);
	void Setup(QpCompleteCallback cb); // setup shared data and callbacks with the QbbNetDevice
	
	// 根据源IP地址、目标IP地址、协议号和端口号生成一个唯一的键值，用于在m_qpMap中查找QP。
	static uint64_t GetQpKey(uint32_t dip, uint16_t sport, uint16_t pg); // get the lookup key for m_qpMap
	// 
	Ptr<RdmaQueuePair> GetQp(uint32_t dip, uint16_t sport, uint16_t pg); // get the qp
	

	//用于获取QP所在的数据链路层接口的索引。
	uint32_t GetNicIdxOfQp(Ptr<RdmaQueuePair> qp); // get the NIC index of the qp
	
	// 创建新的 RDMA 传输会话，设置队列对及其相关的网络参数
	void AddQueuePair(uint64_t size, uint16_t pg, Ipv4Address _sip, Ipv4Address _dip, uint16_t _sport, uint16_t _dport, uint32_t win, uint64_t baseRtt, Callback<void> notifyAppFinish); // add a new qp (new send)
	
	//删除一个队列对（QP）
	void DeleteQueuePair(Ptr<RdmaQueuePair> qp);

	Ptr<RdmaRxQueuePair> GetRxQp(uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport, uint16_t pg, bool create); // get a rxQp

	uint32_t GetNicIdxOfRxQp(Ptr<RdmaRxQueuePair> q); // get the NIC index of the rxQp
	void DeleteRxQp(uint32_t dip, uint16_t pg, uint16_t dport);

	/// 设置处理方式
	int ReceiveUdp(Ptr<Packet> p, CustomHeader &ch);
	int ReceiveCnp(Ptr<Packet> p, CustomHeader &ch);
	int ReceiveAck(Ptr<Packet> p, CustomHeader &ch); // handle both ACK and NACK
	int Receive(Ptr<Packet> p, CustomHeader &ch); // callback function that the QbbNetDevice should use when receive packets. Only NIC can call this function. And do not call this upon PFC
	//处理一般的网络数据包，并做出适当的响应。注意，该函数不能处理 PFC（优先级流控）事件

	void CheckandSendQCN(Ptr<RdmaRxQueuePair> q);  //检查队列对（QP）的状态，并决定是否发送 QCN ???
	int ReceiverCheckSeq(uint32_t seq, Ptr<RdmaRxQueuePair> q, uint32_t size); //检查接收的序列号 seq 是否与期望的序列号匹配
	void AddHeader (Ptr<Packet> p, uint16_t protocolNumber);   // 指定需要添加的协议类型
	static uint16_t EtherToPpp (uint16_t protocol);   ///     以太网映射到 PPP 

	void RecoverQueue(Ptr<RdmaQueuePair> qp);
	void QpComplete(Ptr<RdmaQueuePair> qp);
	void SetLinkDown(Ptr<QbbNetDevice> dev);

	// call this function after the NIC is setup
	void AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx);  //为路由表添加一条新的表项
	void ClearTable();
	void RedistributeQp();  //重新分配队列对qp

	Ptr<Packet> GetNxtPacket(Ptr<RdmaQueuePair> qp); // get next packet to send, inc snd_nxt
	void PktSent(Ptr<RdmaQueuePair> qp, Ptr<Packet> pkt, Time interframeGap);
	void UpdateNextAvail(Ptr<RdmaQueuePair> qp, Time interframeGap, uint32_t pkt_size);
	void ChangeRate(Ptr<RdmaQueuePair> qp, DataRate new_rate);
	/******************************
	 * Mellanox's version of DCQCN
	 *****************************/
	double m_g; //EwmaGain    Control gain parameter which determines the level of rate decrease  DoubleValue(1.0 / 16)

	// 当收到第一个 CNP（拥塞通知包）时，设置为链路速率的一个比例。
	double m_rateOnFirstCNP; // the fraction of line rate to set on first CNP

	bool m_EcnClampTgtRate;   //启用 ECN钳制目标速率
	double m_rpgTimeReset;   //Rate Progression    The rate increase timer at RP in microseconds
	double m_rateDecreaseInterval;  //速率减少的间隔时间
	uint32_t m_rpgThreshold;  //The rate increase timer at RP
	double m_alpha_resume_interval;  //恢复 Alpha 参数的时间间隔  DoubleValue(55.0),
	DataRate m_rai;		//< Rate of additive increase   Rate increment unit in AI period"    DataRateValue(DataRate("5Mb/s")),
	DataRate m_rhai;		//< Rate of hyper-additive increase  DataRateValue(DataRate("50Mb/s")),

	// the Mellanox's version of alpha update:
	// every fixed time slot, update alpha.
	void UpdateAlphaMlx(Ptr<RdmaQueuePair> q);
	void ScheduleUpdateAlphaMlx(Ptr<RdmaQueuePair> q);

	// Mellanox's version of CNP receive
	void cnp_received_mlx(Ptr<RdmaQueuePair> q);

	// Mellanox's version of rate decrease
	// It checks every m_rateDecreaseInterval if CNP arrived (m_decrease_cnp_arrived).
	// If so, decrease rate, and reset all rate increase related things
	void CheckRateDecreaseMlx(Ptr<RdmaQueuePair> q);
	void ScheduleDecreaseRateMlx(Ptr<RdmaQueuePair> q, uint32_t delta);

	// Mellanox's version of rate increase
	void RateIncEventTimerMlx(Ptr<RdmaQueuePair> q);
	void RateIncEventMlx(Ptr<RdmaQueuePair> q);
	void FastRecoveryMlx(Ptr<RdmaQueuePair> q);
	void ActiveIncreaseMlx(Ptr<RdmaQueuePair> q);
	void HyperIncreaseMlx(Ptr<RdmaQueuePair> q);

	/***********************
	 * High Precision CC
	 ***********************/
	double m_targetUtil;  //The Target Utilization of the bottleneck bandwidth, by default 95%  
	double m_utilHigh;    //The upper bound of Target Utilization of the bottleneck bandwidth, by default 98%
	uint32_t m_miThresh;  //Threshold of number of consecutive AI before MI  UintegerValue(5),
	bool m_multipleRate;   //Maintain multiple rates in HPCC",BooleanValue(true),
				
	bool m_sampleFeedback; //  Whether sample feedback or not   boolean
	void HandleAckHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
	void UpdateRateHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
	void UpdateRateHpTest(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
	void FastReactHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

	/**********************
	 * TIMELY
	 *********************/
	//Alpha of TIMELY   DoubleValue(0.875),
	//Beta of TIMELY    DoubleValue(0.8),
	double m_tmly_alpha, m_tmly_beta;  
	//alpha 和 beta 是控制传输速率变化的参数，alpha 控制 RTT 增加时的反应，beta 控制 RTT 减小时的反应。
	//TLow of TIMELY (ns)  UintegerValue(50000),
	//inRtt of TIMELY (ns)
	uint64_t m_tmly_TLow, m_tmly_THigh, m_tmly_minRtt;  /// 阈值是多少，最小 RTT 是多少
	void HandleAckTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
	void UpdateRateTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool us);
	void FastReactTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

	/**********************
	 * DCTCP
	 *********************/
	DataRate m_dctcp_rai;   //DCTCP 的加性增加速率。 DCTCP's Rate increment unit in AI period  DataRateValue(DataRate("1000Mb/s")),
	void HandleAckDctcp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

	/*********************
	 * HPCC-PINT
	 ********************/
	uint32_t pint_smpl_thresh;  //   PINT 的采样阈值     PINT's sampling threshold in rand()%65536  
	void SetPintSmplThresh(double p);
	void HandleAckHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
	void UpdateRateHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
};

} /* namespace ns3 */

#endif /* RDMA_HW_H */
