#ifndef DRONE_DDS_TRANSPORT_DOMAIN_PARTICIPANT_OWNER_H
#define DRONE_DDS_TRANSPORT_DOMAIN_PARTICIPANT_OWNER_H

#include <fastdds/dds/core/Types.hpp>

#include <string_view>

namespace eprosima::fastdds::dds
{

class DomainParticipant;
class DomainParticipantFactory;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class DomainParticipantOwner final
{
  public:
    DomainParticipantOwner(eprosima::fastdds::dds::DomainId_t domainId, std::string_view name);
    ~DomainParticipantOwner() noexcept;

    DomainParticipantOwner(const DomainParticipantOwner &) = delete;
    DomainParticipantOwner &operator=(const DomainParticipantOwner &) = delete;
    DomainParticipantOwner(DomainParticipantOwner &&) = delete;
    DomainParticipantOwner &operator=(DomainParticipantOwner &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::DomainParticipant &participant() noexcept;
    [[nodiscard]] const eprosima::fastdds::dds::DomainParticipant &participant() const noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipantFactory *factory_{nullptr};
    eprosima::fastdds::dds::DomainParticipant *participant_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_DOMAIN_PARTICIPANT_OWNER_H
