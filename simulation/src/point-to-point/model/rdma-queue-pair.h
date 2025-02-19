#ifndef RDMA_QUEUE_PAIR_H
#define RDMA_QUEUE_PAIR_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ipv4-address.h>
#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/custom-header.h>
#include <ns3/int-header.h>
#include <vector>

namespace ns3 {

	// 管理一个 RDMA（Remote Direct Memory Access）队列对（Queue Pair，QP）


//rdmaRxQueuePair 类是 RDMA 接收端队列对的核心组成部分
// ，负责处理接收端的拥塞控制、数据包管理以及网络连接状态。
//     拥塞控制：
    // 通过 ECNAccount 和 QcnTimerEvent，该类处理接收端的拥塞反馈和管理。ECN 是一种拥塞通知机制，用于在拥塞发生时通知接收端或发送端调整其传输速率。

    // 数据包管理：
    // 通过 ReceiverNextExpectedSeq 和 m_nackTimer，该类可以管理接收的数据包顺序以及处理数据包丢失的情况（通过 NACK 通知发送方重新发送丢失的数据包）。

    // 网络状态管理：
    // 该类还通过 IP 地址、端口号和 IP 标识符管理网络中的连接状态。

// 总结：

class RdmaQueuePair : public Object {
public:
	Time startTime;
	Ipv4Address sip, dip;
	uint16_t sport, dport;
	// 队列对传输的数据大小    好像是传输数据的总大小
	uint64_t m_size;
	// 下一次要发送的序列号   未被确认的最高???序列号
	uint64_t snd_nxt, snd_una; // next seq to send, the highest unacked seq
	//m_pg：    不同优先级队列
	uint16_t m_pg; 
	// uint16_t m_ipid;  IP数据包ID
	uint32_t m_win; // bound of on-the-fly packets     当前窗口的大小，表示可以飞行的数据包数量。
	uint64_t m_baseRtt; // base RTT of this qp
	DataRate m_max_rate; // max rate
	bool m_var_win; // variable window size
	Time m_nextAvail;	//< Soonest time of next send
	uint32_t wp; // current window of packets 当前窗口中的数据包数量
	uint32_t lastPktSize;   // 上一个数据包的大小
	Callback<void> m_notifyAppFinish;  //应用程序完成通知回调函数

	/******************************
	 * runtime states
	 *****************************/
	DataRate m_rate;	//< Current rate


	/// @brief /结构体的内容需要后面再看？？
	struct {
		DataRate m_targetRate;	//< Target rate
		EventId m_eventUpdateAlpha; // 更新 alpha 参数的事件
		double m_alpha; //拥塞控制算法的调整参数 alpha
		bool m_alpha_cnp_arrived; // 指示 CNP 是否到达最后一个时隙
		bool m_first_cnp; // indicate if the current CNP is the first CNP 当前 CNP 是否为第一个 CNP
		EventId m_eventDecreaseRate;// 减速事件
		bool m_decrease_cnp_arrived; // indicate if CNP arrived in the last slot 最近时间段内是否收到用于减速的 CNP
		uint32_t m_rpTimeStage; // 当前处于的速率阶段
		EventId m_rpTimer;
	} mlx; //用于描述MLX拥塞控制算法的状态     可能是dcqcn专用的
	struct {
		uint32_t m_lastUpdateSeq;
		DataRate m_curRate;
		IntHop hop[IntHeader::maxHop]; //// 每跳
		uint32_t keep[IntHeader::maxHop]; 
		uint32_t m_incStage;
		double m_lastGap;
		double u;       // // 初始化时为1   在rdma-queue-pair 
		struct {
			double u;     // 初始化时为1
			DataRate Rc;
			uint32_t incStage;
		}hopState[IntHeader::maxHop];
	} hp; //用于描述HPCC拥塞控制算法的状态
	struct{
		uint32_t m_lastUpdateSeq; //// 上次更新的序列号
		DataRate m_curRate;
		uint32_t m_incStage;
		uint64_t lastRtt;
		double rttDiff;
	} tmly;
	struct{
		uint32_t m_lastUpdateSeq;
		uint32_t m_caState;
		uint32_t m_highSeq; // when to exit cwr
		double m_alpha;
		uint32_t m_ecnCnt;
		uint32_t m_batchSizeOfAlpha;
	} dctcp;
	struct{
		uint32_t m_lastUpdateSeq;
		DataRate m_curRate;
		uint32_t m_incStage;
	}hpccPint;

	/***********
	 * methods
	 **********/
	static TypeId GetTypeId (void);
	RdmaQueuePair(uint16_t pg, Ipv4Address _sip, Ipv4Address _dip, uint16_t _sport, uint16_t _dport);
	void SetSize(uint64_t size);
	void SetWin(uint32_t win);
	void SetBaseRtt(uint64_t baseRtt);
	void SetVarWin(bool v);
	void SetAppNotifyCallback(Callback<void> notifyAppFinish);//用于设置应用程序通知回调。

	uint64_t GetBytesLeft(); //用于获取队列对中剩余的字节数。
	uint32_t GetHash(void); //用于获取队列对的哈希值。这可能用于在哈希表中存储和查找队列对。
	void Acknowledge(uint64_t ack); //，用于确认接收到的数据包。它接受一个参数ack，表示接收到的数据包的序列号
	uint64_t GetOnTheFly(); //用于获取队列对中正在传输的数据包的数量。
	bool IsWinBound(); //用于检查队列对是否受到窗口大小的限制。
	uint64_t GetWin(); // window size calculated from m_rate
	bool IsFinished();
	uint64_t HpGetCurWin(); // window size calculated from hp.m_curRate, used by HPCC
};

class RdmaRxQueuePair : public Object { // Rx side queue pair
public:
	// 用于跟踪 ECN（Explicit Congestion Notification，显式拥塞通知）相关的信息
	struct ECNAccount{
		uint16_t qIndex; //qIndex: 队列的索引
		uint8_t ecnbits; //: ECN 标记位，用于表示拥塞信息
		uint16_t qfb;  //收到udp时，携带ecn的udp数量
		uint16_t total;  //收到udp总数

		ECNAccount() { memset(this, 0, sizeof(ECNAccount));}
	};
	ECNAccount m_ecn_source; //用于存储 ECN 的源信息（拥塞反馈等数据）
	uint32_t sip, dip;  //源和目的 IP
	uint16_t sport, dport;//端口号
	uint16_t m_ipid; //用于标识ip数据包的 id   改：记录该qp对的发送数据包数量   rdma-hw.cc文件内中的getnxtpacket 代码
	uint32_t ReceiverNextExpectedSeq; //即期待接收到的数据包序列号
	Time m_nackTimer;  //负确认计时器（NACK Timer）用于跟踪丢包或未按期望接收到数据包
	
	//   rxQp->m_milestone_rx = m_ack_interval; ??名字和作用不清楚
	int32_t m_milestone_rx; // 接收端的里程碑标记，可能用于标识接收到的某个数据包的状态。
	uint32_t m_lastNACK; //最近一次发送的 NACK（负确认）的序列号
	// 可能与 QCN（Quantized Congestion Notification）相关的机制有关
	EventId QcnTimerEvent; // if destroy this rxQp, remember to cancel this timer 用于定时器事件

	static TypeId GetTypeId (void);
	RdmaRxQueuePair();
	uint32_t GetHash(void);
};





// RDMA 队列对（Queue Pair，QP）管理类  组织和管理多个 RdmaQueuePair 对象

class RdmaQueuePairGroup : public Object {
public:
	// 用于存储指向 RdmaQueuePair 对象的智能指针 Ptr<RdmaQueuePair>。
	std::vector<Ptr<RdmaQueuePair> > m_qps; 
	
	//std::vector<Ptr<RdmaRxQueuePair> > m_rxQps;

	static TypeId GetTypeId (void);
	RdmaQueuePairGroup(void);
	uint32_t GetN(void);  //返回当前队列对组中队列对的数量
	Ptr<RdmaQueuePair> Get(uint32_t idx);
	Ptr<RdmaQueuePair> operator[](uint32_t idx);  //重载 [] 运算符，与 Get 方法类似，通过索引 idx 返回队列对的智能指针
	void AddQp(Ptr<RdmaQueuePair> qp);
	//void AddRxQp(Ptr<RdmaRxQueuePair> rxQp);
	void Clear(void);
};

}

#endif /* RDMA_QUEUE_PAIR_H */
