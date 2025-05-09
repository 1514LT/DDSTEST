#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include "DataDefine.h"
#include "DataDefinePubSubTypes.h"
#include "TestType.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;

class MainPublisherListenner :public DataWriterListener
{
private:
  std::atomic_int m_matched;
public:
  MainPublisherListenner();
  ~MainPublisherListenner();
public:
  void on_publication_matched(DataWriter* writer,const PublicationMatchedStatus& info)override;
  int getMatched();
  void on_offered_deadline_missed(DataWriter* writer,const OfferedDeadlineMissedStatus& status) override;
};



class MainPublisher
{
private:
  DomainParticipant* m_domain_participant;
  Publisher* m_publisher;
  std::vector<TypeSupport> m_typeVec;
  std::vector<std::pair<Topic*,DataWriter*> > m_writers;
  MainPublisherListenner m_listener;
  testTypes m_type;
public:
  MainPublisher();
  ~MainPublisher();
public:
  bool init(testTypes type = testTypes::Default);
  bool initPubType(const std::string & topicName, const std::string & typeName, TopicDataType * dataType, DataWriterListener * listener);
  bool TargetMatched();
  bool PublishTarget(Target &target);
  void SendMsg();
  void SendMsg(Target& target);
};



#endif