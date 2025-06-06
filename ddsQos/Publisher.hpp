#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include "DataDefine.h"
#include "DataDefinePubSubTypes.h"
#include "TestType.hpp"
#include "common.h"
#include <thread>
#include "Subscriber.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;

class MainPublisherListenner :public DataWriterListener
{
public:
  std::atomic_int m_matched;
public:
  MainPublisherListenner();
  ~MainPublisherListenner();
public:
  void on_publication_matched(DataWriter* writer,const PublicationMatchedStatus& info)override;
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
  std::map<std::string,TopicDataType*> pair_topics;
public:
  MainPublisher();
  ~MainPublisher();
public:
  bool init(testTypes type = testTypes::Default);
  bool init(
    const std::string& server_address,
    unsigned short server_port,
    unsigned short server_id,
    TransportKind transport,
    testTypes type = testTypes::Default);
  bool initPubType(const std::string & topicName, const std::string & typeName, TopicDataType * dataType, DataWriterListener * listener);
  bool TargetMatched();
  bool PublishTarget(Target &target);
  bool PublishReplay(Replay &replay);
  void SendMsg();
  void SendMsg(Target& target);
  void SendMsg(Replay& replay);
  void SendMsg(DataChunk& dataChunk);
  void SendMsg(GrpcInfo& grpc);
  void SendMsg(GrpcReplay& replay);
  DomainParticipant* getParticipant();
    class PubListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        PubListener()
            : matched_(0)
        {
        }

        ~PubListener() override
        {
        }

        //! Callback executed when a DataReader is matched or unmatched
        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        //! Callback executed when a DomainParticipant is discovered, dropped or removed
        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    private:

        using eprosima::fastdds::dds::DomainParticipantListener::on_participant_discovery;

        //! Number of DataReaders matched to the associated DataWriter
        std::atomic<std::uint32_t> matched_;
    }
    listener_;
};



#endif