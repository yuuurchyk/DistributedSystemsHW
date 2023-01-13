#include "outcomingrequestcontext/abstractoutcomingrequestcontext.h"

namespace Proto::OutcomingRequestContext
{
void AbstractOutcomingRequestContext::promiseMarkFilled()
{
    promiseFilled_ = true;
}

bool AbstractOutcomingRequestContext::promiseFilled() const
{
    return promiseFilled_;
}

}    // namespace Proto::OutcomingRequestContext
