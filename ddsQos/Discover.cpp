#include "Discover.hpp"

DiscoverListenler::DiscoverListenler()
{
}

DiscoverListenler::~DiscoverListenler()
{
}

void DiscoverListenler::on_participant_discovery(
  DomainParticipant* participant,
  eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info,
  bool& should_be_ignored)
{
  should_be_ignored = false;
  if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
  {
    std::cout << "New participant discovered:";
    auto names = participant->get_participant_names();
    std::cout << names[names.size()-1] << std::endl;
    std::ostringstream output;
    output << info.info.m_guid;
    std::string guid = output.str();
    mtx_uid.lock();
    m_participant[guid] = names[names.size()-1];
    mtx_uid.unlock();
    bool ignoring_condition = false;
  
    if (ignoring_condition)
    {
      should_be_ignored = true;
    }
  }
  else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
          info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
  {
    std::cout << "New participant lost:";
    std::ostringstream output;
    output << info.info.m_guid;
    std::string guid = output.str();
    std::cout << m_participant[guid] << std::endl;
    mtx_uid.lock();
    m_participant.erase(guid);
    mtx_uid.unlock(); 
  }
}

void DiscoverListenler::on_subscriber_discovery(
  DomainParticipant* participant,
  fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
  if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER)
  {
      std::cout << "New subscriber discovered" << std::endl;
  }
  else if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER)
  {
      std::cout << "New subscriber lost" << std::endl;
  }
}

void DiscoverListenler::on_publisher_discovery(
  DomainParticipant* participant,
  fastrtps::rtps::WriterDiscoveryInfo&& info)
{
  if (info.status == eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER)
  {
      std::cout << "New publisher discovered" << std::endl;
  }
  else if (info.status == eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER)
  {
      std::cout << "New publisher lost" << std::endl;
  }
}



Discover::Discover()
:m_participant(nullptr)
{
}

Discover::~Discover()
{
  DomainParticipantFactory::get_instance()->delete_participant(m_participant);
}

bool Discover::init()
{
  DomainParticipantQos discoverqos;
  discoverqos.name("discover");
  m_participant = DomainParticipantFactory::get_instance()->create_participant(0,discoverqos,&m_domain_listener);
  std::cout << m_participant << std::endl;
  if(!m_participant)
  {
    std::cerr << "Discover m_participant\n";
    return false;
  }
  while (1)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return true;
}

