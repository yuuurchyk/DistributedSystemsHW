#include "proto2/outcomingrequestcontext/abstractoutcomingrequestcontext.h"

namespace Proto2::OutcomingRequestContext
{

void AbstractOutcomingRequestContext::promiseMarkFilled()
{
    promiseFilled_ = true;
}

bool AbstractOutcomingRequestContext::promiseFilled() const
{
    return promiseFilled_;
}

}    // namespace Proto2::OutcomingRequestContext
