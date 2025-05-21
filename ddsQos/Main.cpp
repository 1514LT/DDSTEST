#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "Discover.hpp"

class Main
{
template <typename Function, typename... Args>
void start(Function&& f, Args&&... args)
{
  std::thread(std::forward<Function>(f), std::forward<Args>(args)...).join();
}
private:
  Discover* discover;
  MainPublisher* pub;
  MainSubscriber* sub;
  std::vector<std::shared_ptr<MainSubscriber>> vt_sub;
public:
  testTypes m_type;

public:
  Main(){};
  ~Main(){};
public:
  void Run()
  {
    RunDiscover();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    RunPublisher();
  }
  void RunDiscover()
  {
    discover = new Discover;
    discover->init("192.168.5.165",9090,1,TransportKind::UDPv4,false,"127.0.0.1",9090,1);
  }
  void RunPublisher()
  {
    pub = new MainPublisher;
    pub->pair_topics["Target"] = new TargetPubSubType;
    pub->init("192.168.5.165",9090,1,TransportKind::UDPv4,m_type);
    Target target;
    target.index(0);
    target.message("hello world");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if(m_type == testTypes::Test)
    {
      target.replayFlag(1);
    }
    while(pub->TargetMatched())
    {
      std::cout << "----->TargetMatched" << std::endl;
      pub->SendMsg(target);
      break;
    }
    if(m_type == testTypes::Durability || m_type == testTypes::ResourceLimits)
    {
      while (1)
      {
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    if(m_type == testTypes::Test)
    {
      MainSubscriber test_sub;
      test_sub.pair_topics["Replay"] = new ReplayPubSubType;
      
      test_sub.init("192.168.5.165",9090,1,TransportKind::UDPv4,testTypes::Test);
      while (1)
      {
        std::this_thread::sleep_for(std::chrono::seconds(2));
      }
    }
    std::cout << "pub exit\n";
    delete pub;
  }
  void RunSubscriber()
  {
    sub = new MainSubscriber;
    sub->pair_topics["Target"] = new TargetPubSubType;
    sub->init("192.168.5.165",9090,1,TransportKind::UDPv4,m_type);
    while (1)
    {
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    delete sub;
  }
};

int main(int argc, char const *argv[])
{
  if(argc < 3)
  {
    std::cerr << "input node + qosType" << std::endl;
    return -1;
  }
  Main main;
  std::string node = argv[1];
  std::string qosType = argv[2];
  switch (qosType[0]) {
    case 'D':
      if (qosType == "Durability") {
        std::cout << "Durability" << std::endl;
        main.m_type = testTypes::Durability;
      } else if (qosType == "Deadline") {
        std::cout << "Deadline" << std::endl;
        main.m_type = testTypes::Deadline;
      } else if (qosType == "Default") {
        std::cout << "Default" << std::endl;
        main.m_type = testTypes::Default;
      }
      break;
    case 'R':
      if (qosType == "ReliablityBestEffort") {
        std::cout << "ReliablityBestEffort" << std::endl;
        main.m_type = testTypes::ReliablityBestEffort;
      } else if (qosType == "ReliablityReliable") {
        std::cout << "ReliablityReliable" << std::endl;
        main.m_type = testTypes::ReliablityReliable;
      } else if (qosType == "ResourceLimits") {
        std::cout << "ResourceLimits" << std::endl;
        main.m_type = testTypes::ResourceLimits;
      }
      break;
    case 'L':
      if (qosType == "Lifespan") {
        std::cout << "Lifespan" << std::endl;
        main.m_type = testTypes::Lifespan;
      }
      break;
    case 'H':
      if (qosType == "History") {
        std::cout << "History" << std::endl;
        main.m_type = testTypes::History;
      }
      break;
    case 'O':
      if (qosType == "Ownership") {
        std::cout << "Ownership" << std::endl;
        main.m_type = testTypes::Ownership;
      }
      break;
    case 'P':
      if (qosType == "PartitionA") {
        std::cout << "PartitionA" << std::endl;
        main.m_type = testTypes::PartitionA;
      } else if (qosType == "PartitionB") {
        std::cout << "PartitionB" << std::endl;
        main.m_type = testTypes::PartitionB;
      }
      break;
    case 'T':
      if (qosType == "Test") {
        std::cout << "Test" << std::endl;
        main.m_type = testTypes::Test;
      }
      break;
  }
  if(node == "discover")
  {
    main.RunDiscover();
  }
  else if(node == "pub")
  {
    main.RunPublisher();
  }
  else if(node == "sub")
  {
    main.RunSubscriber();
  }
  return 0;
}
