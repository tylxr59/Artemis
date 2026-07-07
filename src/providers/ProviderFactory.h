#pragma once

#include <memory>

namespace Artemis {

class AgentProvider;

std::unique_ptr<AgentProvider> makeDefaultAgentProvider();

} // namespace Artemis
