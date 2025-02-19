#ifndef SWITCH_MMU_H
#define SWITCH_MMU_H

#include <unordered_map>
#include <ns3/node.h>

namespace ns3 {

class Packet;


//SwitchMmu 类是交换机的内存管理单元（MMU），用于管理交换机的端口、队列以及流量控制的相关操作。
// 它负责处理流量的入口和出口（Ingress 和 Egress）控制，包括检查是否应当暂停（PFC 流量控制）、
// 调整 ECN（显式拥塞通知）、缓冲区配置等操作
class SwitchMmu: public Object{
public:
	static const uint32_t pCnt = 257;	// Number of ports used 交换机支持的端口数量（257个）
	static const uint32_t qCnt = 8;	// Number of queues/priorities used 每个端口使用的队列/优先级数量

	static TypeId GetTypeId (void);

	SwitchMmu(void);
	
	//检查某个端口（port）和队列（qIndex）是否允许数据包（psize）从入口进入
	bool CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);
	
	//检查某个端口（port）和队列（qIndex）是否允许数据包（psize）从出口发送
	bool CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);
	//	更新某个端口（port）和队列（qIndex）的入口（Ingress）流量控制
	void UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);
	// 更新某个端口（port）和队列（qIndex）的出口（Egress）流量控制
	void UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);
	// 入口（Ingress）流量控制中移除数据包（psize）
	void RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);
	// 出口（Egress）流量控制中移除数据包（psize）
	void RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize);

	// 检查指定端口和队列是否应该发送 PFC 暂停帧。
	bool CheckShouldPause(uint32_t port, uint32_t qIndex);
	// 检查指定端口和队列是否应该恢复发送。
	bool CheckShouldResume(uint32_t port, uint32_t qIndex);

	//设置指定端口和队列为暂停状态，标记该队列已经发送了 PFC 暂停帧。
	void SetPause(uint32_t port, uint32_t qIndex);
	//设置恢复状态，标记队列可以继续传输
	void SetResume(uint32_t port, uint32_t qIndex);

	//void GetPauseClasses(uint32_t port, uint32_t qIndex);
	//bool GetResumeClasses(uint32_t port, uint32_t qIndex);

	uint32_t GetPfcThreshold(uint32_t port);  //获取指定端口的 PFC 阈值。
	uint32_t GetSharedUsed(uint32_t port, uint32_t qIndex); //获取共享缓冲区指定端口和队列的已使用字节数。

	bool ShouldSendCN(uint32_t ifindex, uint32_t qIndex);  //：检查是否应该发送拥塞通知	（CN）。

	// 配置指定端口的 ECN 参数（如最小和最大队列长度，以及最大丢包概率）。
	void ConfigEcn(uint32_t port, uint32_t _kmin, uint32_t _kmax, double _pmax);

	// 配置指定端口的队列头房间大小（Headroom），用于流量控制。
	void ConfigHdrm(uint32_t port, uint32_t size);
	// 配置交换机的端口数量。
	void ConfigNPort(uint32_t n_port);
	// 配置交换机的缓冲区总大小。
	void ConfigBufferSize(uint32_t size);

	// config
	uint32_t node_id; //交换机节点的唯一标识符。
	uint32_t buffer_size; //交换机的总缓冲区大小。
	uint32_t pfc_a_shift[pCnt]; //PFC 相关的移位变量，用于流量控制计算。
	uint32_t reserve;  //保留的缓冲区大小

	// reserve buffer to absorb in-flight packets.
	// The reserved buffer is also called ‘headroom’.
	uint32_t headroom[pCnt];  //每个端口的缓冲区 headroom
	uint32_t resume_offset;
	uint32_t kmin[pCnt], kmax[pCnt]; //每个端口的 ECN 配置参数，分别表示队列长度的最小和最大值。
	double pmax[pCnt];
	uint32_t total_hdrm; //总缓冲区 headroom
	uint32_t total_rsrv; //总保留缓冲区。

	// runtime
	uint32_t shared_used_bytes;  //交换机当前使用的共享缓冲区大小
	uint32_t hdrm_bytes[pCnt][qCnt];  //每个端口和队列使用的 headroom 字节数。
	uint32_t ingress_bytes[pCnt][qCnt]; //每个端口和队列在入口方向使用的字节数。
	uint32_t paused[pCnt][qCnt];  //标记端口和队列的暂停状态
	uint32_t egress_bytes[pCnt][qCnt]; //每个端口和队列在出口方向使用的字节数。
};

} /* namespace ns3 */

#endif /* SWITCH_MMU_H */

