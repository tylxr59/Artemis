#include "providers/ProviderFactory.h"

#include "providers/codex/CodexClient.h"

#include <memory>

namespace Artemis {

std::unique_ptr<AgentProvider> makeDefaultAgentProvider()
{
    return std::make_unique<CodexClient>();
}

} // namespace Artemis
