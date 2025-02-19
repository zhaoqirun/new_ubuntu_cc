#include "rdma-driver.h"

namespace ns3 {

/***********************
 * RdmaDriver
 **********************/

//  AddTraceSource ("QpComplete", "A qp completes.", ...) 添加一个跟踪源

//将 m_traceQpComplete 这个成员变量与跟踪源关联起来
TypeId RdmaDriver::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::RdmaDriver")
		.SetParent<Object> ()
		.AddTraceSource ("QpComplete", "A qp completes.",
				MakeTraceSourceAccessor (&RdmaDriver::m_traceQpComplete))
		;
	return tid;
}

RdmaDriver::RdmaDriver(){  //RdmaDriver 类是一个与 RDMA（远程直接内存访问）设备相关的驱动程序
}

void RdmaDriver::Init(void){
	//遍历当前节点的网络设备，找到支持 Qbb 的设备，为每个找到的设备创建并管理 RDMA 相关的资源。
	Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4> ();
	#if 0
	m_rdma->m_nic.resize(ipv4->GetNInterfaces());
	for (uint32_t i = 0; i < m_rdma->m_nic.size(); i++){
		m_rdma->m_nic[i] = CreateObject<RdmaQueuePairGroup>();
		// share the queue pair group with NIC
		if (ipv4->GetNetDevice(i)->IsQbb()){
			/// 
			DynamicCast<QbbNetDevice>(ipv4->GetNetDevice(i))->m_rdmaEQ->m_qpGrp = m_rdma->m_nic[i];
		}
	}
	#endif
	///   获取当前节点上的所有网络设备
	for (uint32_t i = 0; i < m_node->GetNDevices(); i++){
		Ptr<QbbNetDevice> dev = NULL;

		if (m_node->GetDevice(i)->IsQbb())
		///类型转换
		// QbbNetDevice 是一种网络设备类，通常用于模拟网络中的 Qbb（Quantized Congestion Notification Backpressure, 量化拥塞通知反馈压力）机制
			dev = DynamicCast<QbbNetDevice>(m_node->GetDevice(i));
		// 将所有网络设备添加到m_rdma->m_nic中
		m_rdma->m_nic.push_back(RdmaInterfaceMgr(dev));
		m_rdma->m_nic.back().qpGrp = CreateObject<RdmaQueuePairGroup>();
	}
	#if 0
	 //返回当前节点上与 IPv4 关联的网络接口数量。
	for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++){
		 //获取每个网络接口设备                                ipv4->IsUp(i)：检查接口是否处于启用状态。
		if (ipv4->GetNetDevice(i)->IsQbb() && ipv4->IsUp(i)){
			Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(ipv4->GetNetDevice(i));
			// add a new RdmaInterfaceMgr for this device
			m_rdma->m_nic.push_back(RdmaInterfaceMgr(dev));
			m_rdma->m_nic.back().qpGrp = CreateObject<RdmaQueuePairGroup>();
		}
	}
	#endif
	// RdmaHw do setup
	m_rdma->SetNode(m_node);
	m_rdma->Setup(MakeCallback(&RdmaDriver::QpComplete, this));
}

void RdmaDriver::SetNode(Ptr<Node> node){
	m_node = node;
}

void RdmaDriver::SetRdmaHw(Ptr<RdmaHw> rdma){
	m_rdma = rdma;
}

void RdmaDriver::AddQueuePair(uint64_t size, uint16_t pg, Ipv4Address sip, Ipv4Address dip, uint16_t sport, uint16_t dport, uint32_t win, uint64_t baseRtt, Callback<void> notifyAppFinish){
	m_rdma->AddQueuePair(size, pg, sip, dip, sport, dport, win, baseRtt, notifyAppFinish);
}

void RdmaDriver::QpComplete(Ptr<RdmaQueuePair> q){
	m_traceQpComplete(q);
}

} // namespace ns3
