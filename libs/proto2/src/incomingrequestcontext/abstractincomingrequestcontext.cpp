#include "incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto2::IncomingRequestContext
{
AbstractIncomingRequestContext::AbstractIncomingRequestContext(boost::asio::io_context &executionContext)
    : executionContext_{ executionContext }
{
}

}    // namespace Proto2::IncomingRequestContext
