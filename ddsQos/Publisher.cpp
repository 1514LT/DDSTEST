#include "Publisher.hpp"


MainPublisherListenner::MainPublisherListenner()
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

int MainPublisherListenner::getMatched()
{
  return m_matched.load();
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
  #if 0
  WireProtocolConfigQos wire_protocol;
  wire_protocol.builtin.discovery_config.discoveryProtocol =
    eprosima::fastrtps::rtps::DiscoveryProtocol_t::CLIENT;
  // 设置多播地址
  eprosima::fastrtps::rtps::Locator_t multicast_locator;
  eprosima::fastrtps::rtps::IPLocator::setIPv4(multicast_locator, 239, 255, 0, 1);
  multicast_locator.port = 7400;
  participantQos.wire_protocol() = wire_protocol;
#endif
m_domain_participant = DomainParticipantFactory::get_instance()->create_participant(0,participantQos);
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
  case testTypes::UDPCrossNetwork:
    // UDP跨网络
    {

    }
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

bool MainPublisher::TargetMatched()
{
  return m_listener.getMatched() > 0;
}

bool MainPublisher::PublishTarget(Target &target)
{
  return !m_writers.empty() && m_writers[0].second->write(&target);
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
  PublishTarget(target);
}