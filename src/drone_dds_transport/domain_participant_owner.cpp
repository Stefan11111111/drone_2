#include "drone/dds_transport/domain_participant_owner.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace drone::dds_transport
{
namespace
{

constexpr std::size_t maximumParticipantNameLength{255};
constexpr eprosima::fastdds::dds::DomainId_t maximumDefaultPortDomainId{232};

void validateParticipantConfiguration(const eprosima::fastdds::dds::DomainId_t domainId,
                                      const std::string_view name)
{
    if (domainId > maximumDefaultPortDomainId)
    {
        throw std::invalid_argument{"A DDS domain identifier must not exceed 232 when using "
                                    "standard RTPS ports"};
    }
    if (name.empty())
    {
        throw std::invalid_argument{"A DDS participant name must not be empty"};
    }
    if (name.size() > maximumParticipantNameLength)
    {
        throw std::invalid_argument{"A DDS participant name must not exceed 255 characters"};
    }
}

} // namespace

DomainParticipantOwner::DomainParticipantOwner(const eprosima::fastdds::dds::DomainId_t domainId,
                                               const std::string_view name)
{
    validateParticipantConfiguration(domainId, name);
    factory_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    if (factory_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS did not provide its participant factory"};
    }

    eprosima::fastdds::dds::DomainParticipantQos participantQos;
    participantQos.name(std::string{name});
    participant_ = factory_->create_participant(domainId, participantQos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS could not create participant '" + std::string{name} +
                                 "' in domain " + std::to_string(domainId)};
    }
}

DomainParticipantOwner::~DomainParticipantOwner() noexcept
{
    if (participant_ == nullptr)
    {
        return;
    }

    static_cast<void>(participant_->delete_contained_entities());
    static_cast<void>(factory_->delete_participant(participant_));
}

eprosima::fastdds::dds::DomainParticipant &DomainParticipantOwner::participant() noexcept
{
    return *participant_;
}

const eprosima::fastdds::dds::DomainParticipant &
DomainParticipantOwner::participant() const noexcept
{
    return *participant_;
}

} // namespace drone::dds_transport
