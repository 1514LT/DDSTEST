#ifndef DISCOVER_HPP
#define DISCOVER_HPP

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <atomic>
#include <thread>
#include <map>
#include <sstream>
#include <mutex>
using namespace eprosima::fastdds::dds;
using namespace eprosima;

class DiscoverListenler : public DomainParticipantListener
{ 
private:
  // guid:name
  std::map<std::string,std::string> m_participant;
  std::mutex mtx_uid;
  // std::atomic<std::map<u_int32_t,std::string>> m_participant;
public:
  DiscoverListenler();
  ~DiscoverListenler() override;

  void on_participant_discovery (
    DomainParticipant* participant,
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info,
    bool& should_be_ignored)override;

  void on_subscriber_discovery(
    DomainParticipant* participant,
    fastrtps::rtps::ReaderDiscoveryInfo&& info)override;
  
  void on_publisher_discovery(
    DomainParticipant* participant,
    fastrtps::rtps::WriterDiscoveryInfo&& info)override;
};




class Discover
{
private:
  DomainParticipant* m_participant;
  DiscoverListenler m_domain_listener;
public:
  Discover();
  ~Discover();
public:
  bool init();
};



#endif