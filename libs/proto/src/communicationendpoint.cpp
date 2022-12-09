// #include "proto/communicationendpoint.h"

// #include <algorithm>
// #include <utility>

// #include "reflection/deserialization.h"
// #include "reflection/serialization.h"

// namespace Proto
// {
// struct CommunicationEndpoint::AbstractPendingRequest
// {
//     DISABLE_COPY_MOVE(AbstractPendingRequest)

//     AbstractPendingRequest()          = default;
//     virtual ~AbstractPendingRequest() = default;

//     virtual OpCode opCode()                                                                          = 0;
//     virtual bool   readResponseBody(Reflection::DeserializationContext<boost::asio::const_buffer> &) = 0;

//     virtual void invalidateBadFrame()     = 0;
//     virtual void invalidateDisconnected() = 0;
//     virtual void invalidateTimeout()      = 0;
// };
// template <Concepts::Request Request>
// struct PendingRequest : final CommunicationEndpoint::AbstractPendingRequest
// {
//     OpCode opCode() override { return Concepts::OpCode_v<Request>; }
//     bool   readResponseBody(Reflection::DeserializationContext<boost::asio::const_buffer> &context) override
//     {
//         auto optResponse = context.deserialize<Concepts::Response_t<Request>>();

//         if (optResponse.has_value())
//         {
//             promise.set_value(std::move(optResponse.value()));
//             return true;
//         }
//         else
//         {
//             return false;
//         }
//     }

//     void invalidateBadFrame() override
//     {
//         try
//         {
//             throw BadFrameException{};
//         }
//         catch (...)
//         {
//             promise.set_exception(std::current_exception());
//         }
//     }
//     void invalidateDisconnected() override
//     {
//         try
//         {
//             throw DisconnectedException{};
//         }
//         catch (...)
//         {
//             promise.set_exception(std::current_exception());
//         }
//     }
//     void invalidateTimeout() override
//     {
//         try
//         {
//             throw TimeoutException{};
//         }
//         catch (...)
//         {
//             promise.set_exception(std::current_exception());
//         }
//     }

//     boost::promise<Concepts::Response_t<Request>> promise;
// };

// struct CommunicationEndpoint::AbstractPendingResponse : public std::enable_shared_from_this<AbstractPendingResponse>
// {
//     DISABLE_COPY_MOVE(AbstractPendingResponse)

//     AbstractPendingResponse(std::weak_ptr<CommunicationEndpoint> endpoint, size_t requestId)
//         : endpoint_{ std::move(endpoint) }, requestId_{ requestId }
//     {
//     }
//     virtual ~AbstractPendingResponse() = default;

//     const std::weak_ptr<CommunicationEndpoint> endpoint_;
//     const size_t                               requestId_;

//     bool invalidated_{};
// };
// template <Concepts::Response Response>
// struct PendingResponse : final CommunicationEndpoint::AbstractPendingResponse
// {
//     PendingResponse(std::weak_ptr<CommunicationEndpoint> endpoint, size_t requestId, boost::promise<Response>
//     &response)
//     {
//     }

//     boost::future<Response> future;
// };

// }    // namespace Proto

// namespace Proto
// {
// std::shared_ptr<CommunicationEndpoint>
//     CommunicationEndpoint::create(boost::asio::ip::tcp::socket socket, size_t sendTimeoutMs)
// {
//     return std::shared_ptr<CommunicationEndpoint>(new CommunicationEndpoint{ std::move(socket), sendTimeoutMs });
// }

// CommunicationEndpoint::~CommunicationEndpoint()
// {
//     invalidate();
// }

// void CommunicationEndpoint::run()
// {
//     socket_->run();
// }

// boost::future<Response::AddMessage>
//     CommunicationEndpoint::send_addMessage(std::shared_ptr<const Request::AddMessage> request)
// {
//     return sendRequestImpl(std::move(request));
// }

// boost::future<Response::GetMessages>
//     CommunicationEndpoint::send_getMessages(std::shared_ptr<const Request::GetMessages> request)
// {
//     return sendRequestImpl(std::move(request));
// }

// CommunicationEndpoint::CommunicationEndpoint(boost::asio::ip::tcp::socket socket, size_t sendTimeoutMs)
//     : sendTimeoutMs_{ std::max<size_t>(sendTimeoutMs, 1) },
//       executor_{ socket.get_executor() },
//       socket_{ SocketWrapper::create(std::move(socket)) }
// {
//     socket_->invalidated.connect(
//         [this, weakSelf = weak_from_this()]()
//         {
//             if (auto self = weakSelf.lock(); self != nullptr)
//                 invalidate();
//         });
//     socket_->incomingBuffer.connect(
//         [this, weakSelf = weak_from_this()](boost::asio::const_buffer buffer)
//         {
//             if (auto self = weakSelf.lock(); self != nullptr)
//                 onIncomingBuffer(buffer);
//         });
// }

// template <Concepts::Request Request>
// boost::future<Concepts::Response_t<Request>>
//     CommunicationEndpoint::sendRequestImpl(std::shared_ptr<const Request> request)
// {
// }

// template <Concepts::Response Response>
// void CommunicationEndpoint::sendResponseImpl(size_t requestId, std::shared_ptr<const Response> response)
// {
//     if (response == nullptr)
//         return;

//     auto context = std::make_shared<Reflection::SerializationContext>();

//     context->serializeAndHold(std::make_shared<ResponseHeader>(requestId));
//     context->serializeAndHold(std::move(response));

//     socket_->send(std::move(context));
// }

// void CommunicationEndpoint::onIncomingBuffer(boost::asio::const_buffer buffer)
// {
//     auto context = Reflection::DeserializationContext{ buffer };

//     const auto optIncomingHeader = context.deserialize<IncomingHeader>();
//     if (!optIncomingHeader.has_value())
//     {
//         EN_LOGW << "Could not read the header of the incoming message";
//         socket_->invalidate();
//         return;
//     }

//     const auto &incomingHeader = optIncomingHeader.value();

//     switch (incomingHeader.eventType)
//     {
//     case EventType::REQUEST:
//     {
//         const auto optOpCode = context.deserialize<OpCode>();
//         if (!optOpCode.has_value())
//         {
//             EN_LOGW << "Could not read request op code";
//             socket_->invalidate();
//         }

//         const auto &opCode = optOpCode.value();

//         onIncomingRequest(incomingHeader.requestId, opCode, context);

//         break;
//     }
//     case EventType::RESPONSE:
//     {
//         onIncomingResponse(incomingHeader.requestId, context);
//         break;
//     }
//     default:
//     {
//         EN_LOGW << "Wrong incoming EventType";
//         socket_->invalidate();
//         break;
//     }
//     }
// }

// void CommunicationEndpoint::onIncomingResponse(
//     size_t                                                         requestId,
//     Reflection::DeserializationContext<boost::asio::const_buffer> &body)
// {
//     {
//         const auto it = pendingRequests_.find(requestId);
//         if (it != pendingRequests_.end())
//         {
//             if (!it->second->readResponseBody(body))
//                 it->second->invalidateBadFrame();
//             pendingRequests_.erase(it);
//         }
//     }
//     {
//         const auto it = pendingRequestsTimers_.find(requestId);
//         if (it != pendingRequestsTimers_.end())
//             it->second->cancel();
//         pendingRequestsTimers_.erase(it);
//     }
// }

// void CommunicationEndpoint::onIncomingRequest(
//     size_t                                                         requestId,
//     OpCode                                                         opCode,
//     Reflection::DeserializationContext<boost::asio::const_buffer> &body)
// {
//     switch (opCode)
//     {
//     case Concepts::OpCode_v<Request::AddMessage>:
//         return onIncomingRequestImpl<Request::AddMessage>(requestId, body);
//     case Concepts::OpCode_v<Request::GetMessages>:
//         return onIncomingRequestImpl<Request::GetMessages>(requestId, body);
//     default:
//         EN_LOGW << "Invalid incoming request op code";
//     }
// }

// template <Concepts::Request Request>
// void CommunicationEndpoint::onIncomingRequestImpl()

//     void CommunicationEndpoint::invalidate()
// {
//     for (auto &[_, pendingRequest] : pendingRequests_)
//         pendingRequest->invalidateDisconnected();
//     pendingRequests_.clear();

//     for (auto &[_, timer] : pendingRequestsTimers_)
//         timer->cancel();
//     pendingRequestsTimers_.clear();
// }

// size_t CommunicationEndpoint::getNextRequestId()
// {
//     return requestCounter_.fetch_add(1, std::memory_order_relaxed);
// }

// }    // namespace Proto
