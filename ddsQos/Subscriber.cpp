#include "Subscriber.hpp"

MainSubListener::MainSubListener()
{
}

MainSubListener::~MainSubListener()
{
}

void MainSubListener::on_subscription_matched(DataReader * reader, const SubscriptionMatchedStatus & info)
{
  if(info.current_count_change == 1)
  {
      std::cout << "Subscriber matched, topic:" + reader->get_topicdescription()->get_name() + ", count:" + std::to_string(info.current_count) << std::endl;
      std::cout << "Last matched publisher handle:" << info.last_publication_handle << std::endl;
  }
  else if(info.current_count_change == -1)
  {
      std::cout << "Subscriber unmatched, topic:" + reader->get_topicdescription()->get_name() + ", count:" + std::to_string(info.current_count) << std::endl;
      std::cout << "Last unmatched publisher handle:" << info.last_publication_handle << std::endl;
  }
  else 
  {
      std::cout << info.current_count_change << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
  }
}

void MainSubListener::on_data_available(DataReader * reader)
{
  SampleInfo info;
  if (reader->get_topicdescription()->get_name() == "TargetTopic")
  {
    Target msg;
    while (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
    // while (reader->read_next_sample(&msg,&info) == ReturnCode_t::RETCODE_OK)
    {
      if (info.valid_data)
      {
        std::cout << "index:" << msg.index() << std::endl;
        std::cout << "message:" << msg.message() << std::endl;
      }
    }
  }
}

void MainSubListener::on_requested_deadline_missed(DataReader* reader,const RequestedDeadlineMissedStatus& status)
{
  std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}

MainSubscriber::MainSubscriber()
:m_participant(nullptr),
m_subscriber(nullptr),
m_topic(nullptr)
{
}

MainSubscriber::~MainSubscriber()
{
  for(auto readerPair : m_readers)
  {
    m_participant->delete_topic(readerPair.first);
    m_subscriber->delete_datareader(readerPair.second);
  }
  if(m_subscriber)
  {
    m_participant->delete_subscriber(m_subscriber);
  }
  DomainParticipantFactory::get_instance()->delete_participant(m_participant);
}

bool MainSubscriber::init(testTypes type)
{
  m_testType = type;
  // create particpant USING UDP
  DomainParticipantQos sub_domain_qos;
  sub_domain_qos.name("sub");
  auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
  udp_transport->sendBufferSize = 65501;
  udp_transport->receiveBufferSize = 65501;
  udp_transport->non_blocking_send = true;
  // 配置UDP传输
  sub_domain_qos.transport().use_builtin_transports = false;
  sub_domain_qos.transport().user_transports.push_back(udp_transport);
  #ifdef XML
  std::string file = "file://subscriber.xml";
  sub_domain_qos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = false;
  sub_domain_qos.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol = true;
  sub_domain_qos.wire_protocol().builtin.discovery_config.static_edp_xml_config("file://subscriber.xml");
  m_participant = DomainParticipantFactory::get_instance()->create_participant(0, sub_domain_qos);
  #else
  m_participant = DomainParticipantFactory::get_instance()->create_participant(0,sub_domain_qos);
  #endif
  if(!m_participant)
  {
    return false;
  }
  SubscriberQos subQos;
  if(m_testType == testTypes::PartitionA)
  {
    subQos.partition().push_back("partitionA");
  }
  else if(m_testType == testTypes::PartitionB)
  {
    subQos.partition().push_back("partitionB");
  }
  m_subscriber = m_participant->create_subscriber(subQos,nullptr);
  if(!m_subscriber)
  {
    return false;
  }
  return
  initSubType("TargetTopic","Target",new TargetPubSubType,&m_listener);
}
bool MainSubscriber::init(
    const std::string& server_address,
    unsigned short server_port,
    unsigned short server_id,
    TransportKind transport,
    testTypes type)
{
  m_testType = type;
  DomainParticipantQos sub_domain_qos;
  sub_domain_qos.name("sub");
  sub_domain_qos.transport().use_builtin_transports = false;
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
            // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
            descriptor_tmp->sendBufferSize = 65501;
            descriptor_tmp->receiveBufferSize = 65501;
            descriptor_tmp->non_blocking_send = true;
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
            eprosima::fastrtps::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
            break;
        }

        default:
            break;
    }

    // Set participant as DS CLIENT
    sub_domain_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::CLIENT;

    // Set SERVER's GUID prefix
    RemoteServerAttributes remote_server_att;
    remote_server_att.guidPrefix = get_discovery_server_guid_from_id(server_id);

    // Set SERVER's listening locator for PDP
    remote_server_att.metatrafficUnicastLocatorList.push_back(server_locator);

    // Add remote SERVER to CLIENT's list of SERVERs
    sub_domain_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

    // Add descriptor
    sub_domain_qos.transport().user_transports.push_back(descriptor);

    // CREATE THE PARTICIPANT
    m_participant = DomainParticipantFactory::get_instance()->create_participant(0, sub_domain_qos,nullptr,
                    StatusMask::all() >> StatusMask::data_on_readers());

    if (m_participant == nullptr)
    {
        return false;
    }

    std::cout <<
        "Subscriber Participant " << sub_domain_qos.name() <<
        " created with GUID " << m_participant->guid() <<
        " connecting to server <" << server_locator  << "> " <<
        " with Guid: <" << remote_server_att.guidPrefix << "> " <<
        std::endl;
  SubscriberQos subQos;
  if(m_testType == testTypes::PartitionA)
  {
    subQos.partition().push_back("partitionA");
  }
  else if(m_testType == testTypes::PartitionB)
  {
    subQos.partition().push_back("partitionB");
  }
  m_subscriber = m_participant->create_subscriber(subQos,nullptr);
  if(!m_subscriber)
  {
    return false;
  }
  return
  initSubType("TargetTopic","Target",new TargetPubSubType,&m_listener);
}
bool MainSubscriber::initSubType(const std::string &topicName, const std::string & typeName, TopicDataType *dataType, DataReaderListener * listener)
{
  m_type.emplace_back(dataType);
  m_type.back().register_type(m_participant);

  Topic* topic = m_participant->create_topic(topicName,typeName,TOPIC_QOS_DEFAULT);

  if(!topic)
  {
      return false;
  }
  DataReaderQos readerQos;
  switch (m_testType)
  {
  case testTypes::Durability:
    // 提供持久性
    readerQos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    // 配合使用Reliability QoS
    readerQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    // 配合使用History QoS
    readerQos.history().kind = KEEP_ALL_HISTORY_QOS;
    readerQos.history().depth = 1000;  // 根据需要设置深度
    // 配合Resource Limits
    readerQos.resource_limits().max_samples = 1000;
    readerQos.resource_limits().max_instances = 1;
    readerQos.resource_limits().max_samples_per_instance = 1000;
    std::cout << "set durability success" << std::endl;
    break;
  case testTypes::Deadline:
    // 截止时间
    readerQos.deadline().period = 1.0;
    break;
  case testTypes::ReliablityBestEffort:
    // 可靠性
    readerQos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    break;
  case testTypes::ReliablityReliable:
    // 可靠性
    readerQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    break;
  case testTypes::Lifespan:
    // 生存周期
    readerQos.lifespan().duration = 1000;
    break;
  case testTypes::History:
    // 历史
    readerQos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    readerQos.history().kind = KEEP_ALL_HISTORY_QOS;
    readerQos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    readerQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    readerQos.history().depth =  10;
    readerQos.resource_limits().max_samples = 100;
    readerQos.resource_limits().max_instances = 1;
    readerQos.resource_limits().max_samples_per_instance = 100;
    break;
  case testTypes::ResourceLimits:
    // 资源限制
    readerQos.history().kind = KEEP_ALL_HISTORY_QOS;
    readerQos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    readerQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    readerQos.resource_limits().max_samples = 100;
    readerQos.resource_limits().max_instances = 1;
    readerQos.resource_limits().max_samples_per_instance = 100;
    break;
  case testTypes::Ownership:
    // 所有权
    readerQos.ownership().kind = SHARED_OWNERSHIP_QOS;
    break;
  case testTypes::UDPCrossNetwork:
    // UDP跨网络
    break;
  default:
    break;
  }
  DataReader *reader = m_subscriber->create_datareader(topic,readerQos,listener); 

  if(!reader)
  {
      return false;
  }

  m_readers.push_back({topic,reader});
  return true;
}