#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto::IncomingRequestContext
{
AbstractIncomingRequestContext::AbstractIncomingRequestContext(boost::asio::io_context &executionContext)
    : executionContext_{ executionContext }
{
}

}    // namespace Proto::IncomingRequestContext
