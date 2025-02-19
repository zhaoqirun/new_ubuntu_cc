#ifndef INT_HEADER_H
#define INT_HEADER_H

#include "ns3/buffer.h"
#include <stdint.h>
#include <cstdio>
// IntHop 类被用来表示每个网络跳的遥测数据，
// IntHeader 则是一个包含多个网络跳数据的结构体


namespace ns3 {
// IntHop 和 IntHeader。它们涉及处理 INT（In-band Network Telemetry，即带内网络遥测）信息。
// 被设计用于在网络数据包中嵌入遥测数据，例如每个网络跳的速率、字节数、队列长度和时间戳等信息
class IntHop{   //IntHop 类用于表示每个网络跳的遥测数据。遥测数据包括链路速率、传输的字节数、队列长度以及时间
public:
	static const uint32_t timeWidth = 24;
	static const uint32_t bytesWidth = 20;
	static const uint32_t qlenWidth = 17;
	static const uint64_t lineRateValues[8]; 
	//lineRateValues[8]：存储了可能的链路速率值，如 25Gbps、50Gbps、100Gbps、200Gbps、400Gbps。
	union{
		struct {
			uint64_t lineRate: 64-timeWidth-bytesWidth-qlenWidth,
					 time: timeWidth,
					 bytes: bytesWidth,
					 qlen: qlenWidth;
		};
		uint32_t buf[2];
	};

	static const uint32_t byteUnit = 128;
	static const uint32_t qlenUnit = 80;
	static uint32_t multi;//multi：是一个静态倍数，用于扩展 byteUnit 和 qlenUnit

	uint64_t GetLineRate(){
		return lineRateValues[lineRate];  // ??
	}
	uint64_t GetBytes(){
		//??
		return (uint64_t)bytes * byteUnit * multi;
	}
	uint32_t GetQlen(){
		// ??
		return (uint32_t)qlen * qlenUnit * multi;
	}
	uint64_t GetTime(){
		return time;
	}
	void Set(uint64_t _time, uint64_t _bytes, uint32_t _qlen, uint64_t _rate){
		time = _time;
		bytes = _bytes / (byteUnit * multi);
		qlen = _qlen / (qlenUnit * multi);
		switch (_rate){
			case 25000000000lu:
				lineRate=0;break;
			case 50000000000lu:
				lineRate=1;break;
			case 100000000000lu:
				lineRate=2;break;
			case 200000000000lu:
				lineRate=3;break;
			case 400000000000lu:
				lineRate=4;break;
			default:
				printf("Error: IntHeader unknown rate: %lu\n", _rate);
				break;
		}
	}
	uint64_t GetBytesDelta(IntHop &b){ //计算当前对象与另一个 IntHop 对象之间的字节差异（用于比较两次遥测数据的变化）
		if (bytes >= b.bytes)
			return (bytes - b.bytes) * byteUnit * multi;
		else
			// 如果 bytes 为 30，而 b.bytes 为 250（且字节计数器最大值为 256），则采用计数器环绕的逻辑：(30 + 256 - 250) * byteUnit * multi。         字节循环（环绕）计算：
			return (bytes + (1<<bytesWidth) - b.bytes) * byteUnit * multi;
	}
	uint64_t GetTimeDelta(IntHop &b){
		if (time >= b.time)
			return time - b.time;
		else
			return time + (1<<timeWidth) - b.time;
	}
};

//tHeader 类表示一组遥测数据头
//在数据包的头部用于存储多个网络跳的遥测信息。
class IntHeader{  
public:
	static const uint32_t maxHop = 5;
	enum Mode{
		NORMAL = 0,
		TS = 1,
		PINT = 2,
		NONE
	};
	static Mode mode;
	static int pint_bytes; //是一个静态整数，表示 PINT 模式下的字节数。

	// Note: the structure of IntHeader must have no internal padding, because we will directly transform the part of packet buffer to IntHeader*
	union{
		struct {
			IntHop hop[maxHop];
			uint16_t nhop; //nhop：存储跳数。
		};
		uint64_t ts;     //    ts：存储时间戳。
		union {
			uint16_t power;  //：获取或设置 PINT 模式下的功率值。			struct{
				uint8_t power_lo8, power_hi8;
			};
		}pint;
	};

	IntHeader();
	static uint32_t GetStaticSize();
	void PushHop(uint64_t time, uint64_t bytes, uint32_t qlen, uint64_t rate);
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	uint64_t GetTs(void);
	uint16_t GetPower(void);
	void SetPower(uint16_t);
};

}

#endif /* INT_HEADER_H */
