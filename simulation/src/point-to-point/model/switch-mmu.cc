#include <iostream>
#include <fstream>
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include "switch-mmu.h"

NS_LOG_COMPONENT_DEFINE("SwitchMmu");
namespace ns3 {
	TypeId SwitchMmu::GetTypeId(void){
		static TypeId tid = TypeId("ns3::SwitchMmu")
			.SetParent<Object>()
			.AddConstructor<SwitchMmu>();
		return tid;
	}

	SwitchMmu::SwitchMmu(void){
		// 12 MB（12 * 1024 * 1024）
		buffer_size = 12 * 1024 * 1024;  ///
		// 保留的内存区域设置为 4 KB
		reserve = 4 * 1024;
		//设置为 3 KB  ??? 干什么用的
		resume_offset = 3 * 1024;

		// headroom
		shared_used_bytes = 0;
		memset(hdrm_bytes, 0, sizeof(hdrm_bytes));
		memset(ingress_bytes, 0, sizeof(ingress_bytes));
		memset(paused, 0, sizeof(paused));
		memset(egress_bytes, 0, sizeof(egress_bytes));
	}
	bool SwitchMmu::CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		// 判断新来的数据包 psize大小   加入到port的qindex队列 是否可以

		//当前队列中 headroom 已用字节加上新的数据包是否超出了该端口的 headroom 限制
		//   ??????
		if (psize + hdrm_bytes[port][qIndex] > headroom[port] && psize + GetSharedUsed(port, qIndex) > GetPfcThreshold(port)){
			printf("%lu %u Drop: queue:%u,%u: Headroom full\n", Simulator::Now().GetTimeStep(), node_id, port, qIndex);
			for (uint32_t i = 1; i < 64; i++)
				printf("(%u,%u)", hdrm_bytes[i][3], ingress_bytes[i][3]);
			printf("\n");
			return false;
		}
		return true;
	}
	bool SwitchMmu::CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		return true;
	}
	void SwitchMmu::UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		uint32_t new_bytes = ingress_bytes[port][qIndex] + psize;
		
		//小于reserve    直接放保留区
		// 没有超过预留的缓冲区 reserve，直接更新 ingress_bytes[port][qIndex]。
		if (new_bytes <= reserve){   
			ingress_bytes[port][qIndex] += psize;
		}else { //检查是否超出了 PFC 阈值 thres

			uint32_t thresh = GetPfcThreshold(port);

			// 填满保留区后，剩余的数据包大小    大于阈值，则放入headroom
			if (new_bytes - reserve > thresh){
				hdrm_bytes[port][qIndex] += psize;
			}else {
				// 否则，填满保留区后,放入共享缓冲区
				ingress_bytes[port][qIndex] += psize;
				shared_used_bytes += std::min(psize, new_bytes - reserve);
			}
		}
	}
	void SwitchMmu::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		egress_bytes[port][qIndex] += psize;
	}
	void SwitchMmu::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		// 优先从headroom 缓冲区中 删除
		uint32_t from_hdrm = std::min(hdrm_bytes[port][qIndex], psize);
		// 接着 要从共享缓冲区中扣除的字节数。   
		//  共享缓冲区存入的数据大小为  ingress_bytes[port][qIndex] - reserve
		uint32_t from_shared = std::min(psize - from_hdrm, ingress_bytes[port][qIndex] > reserve ? ingress_bytes[port][qIndex] - reserve : 0);
		hdrm_bytes[port][qIndex] -= from_hdrm;
		ingress_bytes[port][qIndex] -= psize - from_hdrm;
		shared_used_bytes -= from_shared;
	}
	void SwitchMmu::RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize){
		egress_bytes[port][qIndex] -= psize;
	}
	bool SwitchMmu::CheckShouldPause(uint32_t port, uint32_t qIndex){
		//   ??????
		// ????headromm不为空     或者  shared_used_bytes >= PFC阈值   GetPfcThreshold(port)  
		return !paused[port][qIndex] && (hdrm_bytes[port][qIndex] > 0 || GetSharedUsed(port, qIndex) >= GetPfcThreshold(port));
		// uint32_t used = ingress_bytes[port][qIndex];
		// used - reserve >= GetPfcThreshold(port)
		//return (buffer_size - total_hdrm - total_rsrv - shared_used_bytes) >> pfc_a_shift[port];
	}
	//检查是否需要恢复
	bool SwitchMmu::CheckShouldResume(uint32_t port, uint32_t qIndex){ //端口号 队列索引
		//没有暂停 (!paused[port][qIndex])，返回 false，表示不需要恢复。
		if (!paused[port][qIndex]) //
			return false;
		
		uint32_t shared_used = GetSharedUsed(port, qIndex);
		//hdrm_bytes[port][qIndex] 表示每个端口和队列使用的“headroom”字节数,为处理突发流量而保留的额外缓冲空间
		//  
		return hdrm_bytes[port][qIndex] == 0 && (shared_used == 0 || shared_used + resume_offset <= GetPfcThreshold(port));  
	}
	void SwitchMmu::SetPause(uint32_t port, uint32_t qIndex){
		paused[port][qIndex] = true;
	}
	void SwitchMmu::SetResume(uint32_t port, uint32_t qIndex){
		paused[port][qIndex] = false;
	}

	uint32_t SwitchMmu::GetPfcThreshold(uint32_t port){
		//??????
		return (buffer_size - total_hdrm - total_rsrv - shared_used_bytes) >> pfc_a_shift[port];
	}
	uint32_t SwitchMmu::GetSharedUsed(uint32_t port, uint32_t qIndex){
		uint32_t used = ingress_bytes[port][qIndex];
		return used > reserve ? used - reserve : 0;
	}

	// 判断是否接口的某个队列是否应该发送CN拥塞通知
	bool SwitchMmu::ShouldSendCN(uint32_t ifindex, uint32_t qIndex){
		// 最高有限级的控制队列？？
		if (qIndex == 0)
			return false;
		if (egress_bytes[ifindex][qIndex] > kmax[ifindex])
			return true;
		if (egress_bytes[ifindex][qIndex] > kmin[ifindex]){
			double p = pmax[ifindex] * double(egress_bytes[ifindex][qIndex] - kmin[ifindex]) / (kmax[ifindex] - kmin[ifindex]);
			if (UniformVariable(0, 1).GetValue() < p)
				return true;
		}
		return false;
	}
	void SwitchMmu::ConfigEcn(uint32_t port, uint32_t _kmin, uint32_t _kmax, double _pmax){
		kmin[port] = _kmin * 1000;
		kmax[port] = _kmax * 1000;
		pmax[port] = _pmax;
	}
	void SwitchMmu::ConfigHdrm(uint32_t port, uint32_t size){ //每个port  都有headroom
		headroom[port] = size;
	}
	
	void SwitchMmu::ConfigNPort(uint32_t n_port){
		total_hdrm = 0;
		total_rsrv = 0;
		for (uint32_t i = 1; i <= n_port; i++){
			//   ??????
			total_hdrm += headroom[i];  ///   ?????   难道只有一个队列吗？？
			total_rsrv += reserve;
		}
	}
	void SwitchMmu::ConfigBufferSize(uint32_t size){
		buffer_size = size;
	}
}
