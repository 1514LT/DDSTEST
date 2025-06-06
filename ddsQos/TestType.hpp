#ifndef TESTTYPE_HPP
#define TESTTYPE_HPP
#include <iostream>
#include <string>
enum class testTypes {
  Durability,// 持久性
  Deadline, // 截止时间
  ReliablityBestEffort, // 可靠性_最佳努力
  ReliablityReliable, // 可靠性_可靠
  Lifespan, // 生存周期
  History, // 历史
  ResourceLimits, // 资源限制
  Ownership, // 所有权
  PartitionA, // 分区A
  PartitionB, // 分区B
  Test, // UDP跨网络
  BigData, // 大文件传输
  Grpc,
  Default
};

#endif