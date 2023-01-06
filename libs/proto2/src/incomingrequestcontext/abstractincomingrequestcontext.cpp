#include "proto2/incomingrequestcontext/abstractincomingrequestcontext.h"

namespace Proto2::IncomingRequestContext
{

AbstractIncomingRequestContext::AbstractIncomingRequestContext(boost::asio::io_context &ioContext)
    : ioContext_{ ioContext }
{
}

}    // namespace Proto2::IncomingRequestContext
