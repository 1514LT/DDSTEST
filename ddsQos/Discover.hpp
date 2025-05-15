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

#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <atomic>
#include <thread>
#include <map>
#include <sstream>
#include <mutex>
#include "common.h"
#include "TestType.hpp"
using namespace eprosima::fastdds::dds;
using namespace eprosima;



class DiscoverListenler : public DomainParticipantListener
{ 
private:
  // guid:name
  std::map<std::string,std::string> m_participant;
  std::mutex mtx_uid;
  std::atomic_int m_participanter;
  std::atomic_int m_subscriber;
  std::atomic_int m_publisher;
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
  bool init(
        std::string server_address,
        unsigned short server_port,
        unsigned short server_id,
        TransportKind transport,
        bool has_connection_server,
        std::string connection_server_address,
        unsigned short connection_server_port,
        unsigned short connection_server_id);
};



#endif