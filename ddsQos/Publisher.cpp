#include "Publisher.hpp"
void MainPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void MainPublisher::PubListener::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Discovered Participant with GUID " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT ||
            info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Dropped Participant with GUID " << info.info.m_guid << std::endl;
    }
}

MainPublisherListenner::MainPublisherListenner()
:m_matched(0)
{
}

MainPublisherListenner::~MainPublisherListenner()
{
}

void MainPublisherListenner::on_publication_matched(DataWriter * dataWriter, const PublicationMatchedStatus & info)
{
  if(info.current_count_change == 1)
  {
    m_matched = info.current_count;
    std::cout << "Publisher matched, topic:" + dataWriter->get_topic()->get_name() +", count: " + std::to_string(info.current_count) << std::endl;
  }
  else if(info.current_count_change == -1)
  {
    m_matched = info.current_count;
    std::cout << "Publisher unmatched, topic:" + dataWriter->get_topic()->get_name() + ", count: " + std::to_string(info.current_count) << std::endl;
  }
  else
  {
    std::cout << info.current_count_change
              << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
  }
}

void MainPublisherListenner::on_offered_deadline_missed(DataWriter* writer,const OfferedDeadlineMissedStatus& status)
{
  std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}



MainPublisher::MainPublisher()
:m_domain_participant(nullptr),
m_publisher(nullptr)
{
}

MainPublisher::~MainPublisher()
{
  for(auto writerPair : m_writers)
  {
      m_domain_participant->delete_topic(writerPair.first);
      m_publisher->delete_datawriter(writerPair.second);
  }
  if(m_publisher)
  {
    m_domain_participant->delete_publisher(m_publisher);
  }
  DomainParticipantFactory::get_shared_instance()->delete_participant(m_domain_participant);
}

bool MainPublisher::init(testTypes type)
{
  m_type = type;
  // creat pariticpant USING UDP
  DomainParticipantQos participantQos;
  participantQos.name("pub");
  auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
  udp_transport->sendBufferSize = 65501;
  udp_transport->receiveBufferSize = 65501;
  udp_transport->non_blocking_send = true;
  
  participantQos.transport().user_transports.push_back(udp_transport);
  participantQos.transport().use_builtin_transports = false;
  #ifdef XML
  std::string file = "file://publisher.xml";
  std::cout << "define XML" << std::endl;
  participantQos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = false;
  participantQos.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol = true;
  participantQos.wire_protocol().builtin.discovery_config.static_edp_xml_config("file://publisher.xml");
  m_domain_participant = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
  #else
  m_domain_participant = DomainParticipantFactory::get_instance()->create_participant(0,participantQos);
  #endif
  if(!m_domain_participant)
  {
    return false;
  }
  // // creat publiser
  PublisherQos publisherQos;
  if(m_type == testTypes::PartitionA)
  {
    publisherQos.partition().push_back("partitionA");
  }
  else if(m_type == testTypes::PartitionB)
  {
    publisherQos.partition().push_back("partitionB");
  }
  m_publisher = m_domain_participant->create_publisher(publisherQos,nullptr);

return initPubType("TargetTopic","Target",new TargetPubSubType,&m_listener);
}

bool MainPublisher::init(
  const std::string& server_address,
  unsigned short server_port,
  unsigned short server_id,
  TransportKind transport,
  testTypes type)
{
  m_type = type;
  DomainParticipantQos participantQos;
  participantQos.name("pub");

  participantQos.transport().use_builtin_transports = false;


  std::string ip_server_address(server_address);
  // Check if DNS is required
  if (!is_ip(server_address))
  {
      ip_server_address = get_ip_from_dns(server_address, transport);
  }

  if (ip_server_address.empty())
  {
      return false;
  }

  // Create DS SERVER locator
  eprosima::fastdds::rtps::Locator server_locator;
  eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(server_locator, server_port);

  std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

  switch (transport)
  {
      case TransportKind::SHM:
          descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
          server_locator.kind = LOCATOR_KIND_SHM;
          break;

      case TransportKind::UDPv4:
      {
          auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
  // auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
          descriptor_tmp->sendBufferSize = 65501;
          descriptor_tmp->receiveBufferSize = 65501;
          descriptor_tmp->non_blocking_send = true;
  
  // participantQos.transport().user_transports.push_back(descriptor_tmp);
          descriptor = descriptor_tmp;

          server_locator.kind = LOCATOR_KIND_UDPv4;
          eprosima::fastrtps::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
          break;
      }

      case TransportKind::UDPv6:
      {
          auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
          // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
          descriptor = descriptor_tmp;

          server_locator.kind = LOCATOR_KIND_UDPv6;
          eprosima::fastrtps::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
          break;
      }

      case TransportKind::TCPv4:
      {
          auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
          // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
          // One listening port must be added either in the pub or the sub
          descriptor_tmp->add_listener_port(0);
          descriptor = descriptor_tmp;

          server_locator.kind = LOCATOR_KIND_TCPv4;
          eprosima::fastrtps::rtps::IPLocator::setLogicalPort(server_locator, server_port);
          eprosima::fastrtps::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
          break;
      }

      case TransportKind::TCPv6:
      {
          auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
          // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
          // One listening port must be added either in the pub or the sub
          descriptor_tmp->add_listener_port(0);
          descriptor = descriptor_tmp;

          server_locator.kind = LOCATOR_KIND_TCPv6;
          eprosima::fastrtps::rtps::IPLocator::setLogicalPort(server_locator, server_port);
          eprosima::fastrtps::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
          break;
      }

      default:
          break;
  }

  // Set participant as DS CLIENT
  participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol =
          eprosima::fastrtps::rtps::DiscoveryProtocol_t::CLIENT;

  // Set SERVER's GUID prefix
  RemoteServerAttributes remote_server_att;
  remote_server_att.guidPrefix = get_discovery_server_guid_from_id(server_id);

  // Set SERVER's listening locator for PDP
  remote_server_att.metatrafficUnicastLocatorList.push_back(server_locator);

  // Add remote SERVER to CLIENT's list of SERVERs
  participantQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

  // Add descriptor
  participantQos.transport().user_transports.push_back(descriptor);

  // CREATE THE PARTICIPANT
  m_domain_participant = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

  if (m_domain_participant == nullptr)
  {
      return false;
  }

  std::cout <<
      "Publisher Participant " << participantQos.name() <<
      "\ncreated with GUID " << m_domain_participant->guid() <<
      "\nconnecting to server <" << server_locator  << "> " <<
      "\nwith Guid: <" << remote_server_att.guidPrefix << "> " <<
      std::endl;
  PublisherQos publisherQos;
  if(m_type == testTypes::PartitionA)
  {
    publisherQos.partition().push_back("partitionA");
  }
  else if(m_type == testTypes::PartitionB)
  {
    publisherQos.partition().push_back("partitionB");
  }
  m_publisher = m_domain_participant->create_publisher(publisherQos,nullptr);
  for(auto pair:pair_topics)
  {
    std::cout << pair.first << std::endl;
    initPubType(pair.first+"Topic",pair.first,pair.second,&m_listener);
  }
  return true;
}
bool MainPublisher::initPubType(const std::string & topicName, const std::string & typeName, TopicDataType * dataType, DataWriterListener * listener)
{
  m_typeVec.emplace_back(dataType);
  m_typeVec.back().register_type(m_domain_participant);
  Topic *topic = m_domain_participant->create_topic(topicName,typeName,TOPIC_QOS_DEFAULT);
  if(!topic)
  {
      return false;
  }
  DataWriterQos writer_qos;
  switch (m_type)
  {
  case testTypes::Durability:
    // 提供持久性
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    // 配合使用Reliability QoS
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    // 配合使用History QoS
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.history().depth = 1000;  // 根据需要设置深度
    // 配合Resource Limits
    writer_qos.resource_limits().max_samples = 1000;
    writer_qos.resource_limits().max_instances = 1;
    writer_qos.resource_limits().max_samples_per_instance = 1000;
    std::cout << "set durability success" << std::endl;
    break;
  case testTypes::Deadline:
    // 截止时间
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.deadline().period = 1.0;
    break;
  case testTypes::ReliablityBestEffort:
    // 可靠性
    writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    break;
  case testTypes::ReliablityReliable:
    // 可靠性
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    break;
  case testTypes::Lifespan:
    // 生存周期
    writer_qos.lifespan().duration = 1000;
    break;
  case testTypes::History:
    // 历史
    writer_qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.history().depth =  50;
    writer_qos.resource_limits().max_samples = 100;
    writer_qos.resource_limits().max_instances = 1;
    writer_qos.resource_limits().max_samples_per_instance = 100;

    // writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    break;
  case testTypes::ResourceLimits:
    // 资源限制
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.resource_limits().max_samples = 100;
    writer_qos.resource_limits().max_instances = 1;
    writer_qos.resource_limits().max_samples_per_instance = 100;
    break;
  case testTypes::Ownership:
    // 所有权
    writer_qos.ownership().kind = SHARED_OWNERSHIP_QOS;
    break;
  case testTypes::Test:
    break;
  case testTypes::BigData:
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.history().depth = 1000;
    writer_qos.resource_limits().max_samples = 1000;
    writer_qos.resource_limits().max_instances = 1;
    writer_qos.resource_limits().max_samples_per_instance = 1000;
    break;
  case testTypes::Default:
    break;
  default:
    break;
  }
  



  // // 生存周期
  // writer_qos.lifespan().duration = 1.0;
  DataWriter *writer = m_publisher->create_datawriter(topic,writer_qos,listener); 
  if(!writer)
  {
    return false;
  }
  m_writers.push_back({topic,writer});
  return true;
}

DomainParticipant* MainPublisher::getParticipant()
{
  return m_domain_participant;
}
bool MainPublisher::TargetMatched()
{
  return m_listener.m_matched > 0;
}

bool MainPublisher::PublishTarget(Target &target)
{
  return !m_writers.empty() && m_writers[0].second->write(&target);
}
bool MainPublisher::PublishReplay(Replay &replay)
{
  std::cout << "---->send Replay msg" << std::endl;
  return !m_writers.empty() && m_writers[0].second->write(&replay);
}
void MainPublisher::SendMsg()
{
  Target target;
  target.index(1);
  target.message("hello world");
  PublishTarget(target);
}
void MainPublisher::SendMsg(Target& target)
{
  int index = 0;
  std::cout << "send msg\n"; 
  if(m_type == testTypes::Test)
  {
    target.index(index);
    target.message("hello world");
    target.replayFlag(1);
    PublishTarget(target);
  }
  else if(m_type == testTypes::Default)
  {
    target.index(index);
    target.message("hello world");
    target.replayFlag(0);
    PublishTarget(target);
  }
  else
  {
    for(int i=0;i<200;i++)
    {
      target.index(index);
      target.message("hello world");
      target.replayFlag(0);
      PublishTarget(target);
      // std::this_thread::sleep_for(std::chrono::milliseconds(10));
      index++;
    }
  }
  
  std::cout << "send over\n";
}
void MainPublisher::SendMsg(Replay& replay)
{
  PublishReplay(replay);
  std::cout << "send over" << std::endl;
}

void MainPublisher::SendMsg(DataChunk& dataChunk)
{
  m_writers[0].second->write(&dataChunk);
  return;
}