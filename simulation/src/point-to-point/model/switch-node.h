#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include <unordered_map>
#include <ns3/node.h>
#include "qbb-net-device.h"
#include "switch-mmu.h"
#include "pint.h"

namespace ns3 {

class Packet;

class SwitchNode : public Node{
	static const uint32_t pCnt = 257;	// Number of ports used 交换机的端口数量。
	static const uint32_t qCnt = 8;		// Number of queues/priorities used 每个端口的队列数量或优先级数量。
	uint32_t m_ecmpSeed; 				//   ECMP（等成本多路径）路由的随机种子。
	std::unordered_map<uint32_t, std::vector<int> > m_rtTable; // map from ip address (u32) to possible ECMP port (index of dev)  				路由表，存储 IP 地址到可能的端口索引的映射。

	// monitor of PFC
	uint32_t m_bytes[pCnt][pCnt][qCnt]; // m_bytes[inDev][outDev][qidx] is the bytes from inDev enqueued for outDev at qidx                   ：用于监控从输入设备到输出设备在特定队列中的字节数。
	
	uint64_t m_txBytes[pCnt]; // counter of tx bytes  //每个端口的传输字节计数。

	uint32_t m_lastPktSize[pCnt];   //记录每个端口最近包的大小。
	uint64_t m_lastPktTs[pCnt]; 	// ns 记录每个端口最近包的时间戳（以纳秒为单位）。
	double m_u[pCnt];   			//用于监控或流量管理的其他参数

protected:
	bool m_ecnEnabled;    //指示是否启用 ECN（显式拥塞通知）。
	uint32_t m_ccMode;    //拥塞控制模式。
	uint64_t m_maxRtt;    //最大 RTT（往返时间）

	uint32_t m_ackHighPrio; // set high priority for ACK/NACK  设置 ACK/NACK 的高优先级

private:
	int GetOutDev(Ptr<const Packet>, CustomHeader &ch); 		//确定输出设备。
	void SendToDev(Ptr<Packet>p, CustomHeader &ch); 			//将包发送到设备。
	static uint32_t EcmpHash(const uint8_t* key, size_t len, uint32_t seed); //计算 ECMP 哈希值
	void CheckAndSendPfc(uint32_t inDev, uint32_t qIndex);  //检查并发送 PFC（优先级流量控制）信号。
	void CheckAndSendResume(uint32_t inDev, uint32_t qIndex);   //检查并发送恢复信号。
public:
	Ptr<SwitchMmu> m_mmu;

	static TypeId GetTypeId (void);
	SwitchNode();
	void SetEcmpSeed(uint32_t seed);
	void AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx); //添加路由表条目。
	void ClearTable();
	bool SwitchReceiveFromDevice(Ptr<NetDevice> device, Ptr<Packet> packet, CustomHeader &ch); //处理来自设备的接收包。
	void SwitchNotifyDequeue(uint32_t ifIndex, uint32_t qIndex, Ptr<Packet> p);  //通知出队操作

	// for approximate calc in PINT
	int logres_shift(int b, int l);
	int log2apprx(int x, int b, int m, int l); // given x of at most b bits, use most significant m bits of x, calc the result in l bits
};

} /* namespace ns3 */

#endif /* SWITCH_NODE_H */
