#ifndef SUBSCRIBER_HPP
#define SUBSCRIBER_HPP

#include "DataDefine.h"
#include "DataDefinePubSubTypes.h"
#include "TestType.hpp"
#include "Publisher.hpp"
#include <fstream>
#include <sstream>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <thread>
#include <future>
#include <unordered_map>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>
#include "common.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;

  inline int sum(int a,int b){return a+b;}
  inline int subtract(int a,int b){return a-b;}
class MainSubListener : public DataReaderListener
{
private:
  std::atomic_int m_samples;
  std::atomic_int m_replays;
  std::string startTime;
  std::string endTime;
  std::unordered_map<std::string,int(*)(int,int)> funcMap;
public:
  MainSubListener();
  ~MainSubListener();
  void on_subscription_matched(DataReader * reader, const SubscriptionMatchedStatus & info) override;

  void on_data_available(DataReader * reader) override;

  void on_requested_deadline_missed(DataReader* reader,const RequestedDeadlineMissedStatus& status)override;
public:

};




class MainSubscriber
{
private:
  DomainParticipant* m_participant;
  Subscriber * m_subscriber;
  std::vector<std::pair<Topic*,DataReader*> > m_readers;
  Topic * m_topic;
  std::vector<TypeSupport> m_type;
  MainSubListener m_listener;
  testTypes m_testType;
public:
  std::map<std::string,TopicDataType*> pair_topics;
public:
  MainSubscriber();
  ~MainSubscriber();
  bool init(testTypes type = testTypes::Default);
  bool init(
    const std::string& server_address,
    unsigned short server_port,
    unsigned short server_id,
    TransportKind transport,
    testTypes type = testTypes::Default);
  bool initSubType(const std::string &topicName, const std::string & typeName, TopicDataType *dataType, DataReaderListener * listener);

};



#endif