// #pragma once

// #include "proto/deserialization.h"

// #include <optional>
// #include <string>
// #include <type_traits>
// #include <vector>

// namespace Proto
// {
// template <typename ConstBufferSequence>
// DeserializationPtr<ConstBufferSequence>::DeserializationPtr(
//     const ConstBufferSequence &seq)
//     : current_{ boost::asio::buffers_begin(seq) }, end_{ boost::asio::buffers_end(seq)
//     }
// {
// }

// template <typename ConstBufferSequence>
// template <typename T>
// bool DeserializationPtr<ConstBufferSequence>::has(size_t count)
// {
//     const auto requiredSize = sizeof(T) * count;
//     return (end_ - current_) >= requiredSize;
// }

// template <typename ConstBufferSequence>
// template <typename T>
// T DeserializationPtr<ConstBufferSequence>::get()
// {
//     static_assert(std::is_same_v<T, std::decay_t<T>>);
//     static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

//     T res;
//     std::copy(current_,
//               current_ + sizeof(T),
//               reinterpret_cast<typename Iterator::value_type *>(&res));

//     current_ += sizeof(T);

//     return res;
// }

// template <typename ConstBufferSequence>
// template <typename T>
// auto DeserializationPtr<ConstBufferSequence>::get(size_t count)
//     -> std::tuple<Iterator, Iterator>
// {
//     static_assert(std::is_same_v<T, std::decay_t<T>>);
//     static_assert(std::is_pod_v<T>);

//     auto res = std::make_tuple(current_, current_ + sizeof(T) * count);
//     current_ += sizeof(T) * count;

//     return res;
// }

// template <typename ConstBufferSequence>
// bool DeserializationPtr<ConstBufferSequence>::atEnd() const noexcept
// {
//     return current_ == end_;
// }

// }    // namespace Proto

// namespace Proto::detail::Deserialization
// {
// template <typename T>
// class Deserializer
// {
// public:
//     template <typename ConstBufferSequence>
//     static std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &);
// };

// template <typename T>
// class Deserializer<std::optional<T>>
// {
// public:
//     template <typename ConstBufferSequence>
//     static std::optional<std::optional<T>>
//         deserialize(DeserializationPtr<ConstBufferSequence> &);
// };

// template <typename T>
// class Deserializer<std::vector<T>>
// {
// public:
//     template <typename ConstBufferSequence>
//     static std::optional<std::vector<T>>
//         deserialize(DeserializationPtr<ConstBufferSequence> &);
// };

// template <>
// class Deserializer<std::string>
// {
// public:
//     template <typename ConstBufferSequence>
//     static std::optional<std::string>
//         deserialize(DeserializationPtr<ConstBufferSequence> &);
// };

// template <typename T>
// template <typename ConstBufferSequence>
// std::optional<T>
//     Deserializer<T>::deserialize(DeserializationPtr<ConstBufferSequence> &ptr)
// {
//     static_assert(std::is_same_v<T, std::decay_t<T>>);

//     if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
//     {
//         if (ptr.template has<T>())
//             return ptr.template get<T>();
//         else
//             return {};
//     }
//     else
//     {
//         auto allGood = true;

//         auto res   = T{};
//         auto tuple = res.tie();

//         auto visit = [&](auto &target)
//         {
//             if (!allGood)
//                 return;

//             auto res = Deserializer<std::decay_t<decltype(target)>>::deserialize(ptr);
//             if (res.has_value())
//                 target = std::move(res.value());
//             else
//                 allGood = false;
//         };

//         std::apply([&](auto &...val) { (..., visit(val)); }, tuple);

//         if (allGood)
//             return res;
//         else
//             return {};
//     }
// }

// template <typename T>
// template <typename ConstBufferSequence>
// std::optional<std::optional<T>> Deserializer<std::optional<T>>::deserialize(
//     DeserializationPtr<ConstBufferSequence> &ptr)
// {
//     if (!ptr.template has<bool>())
//         return {};

//     const auto present = ptr.template get<bool>();

//     if (!present)
//         return std::optional<T>{};

//     auto optRes = Deserializer<T>::deserialize(ptr);
//     if (optRes.has_value())
//         return std::optional<std::optional<T>>{ std::move(optRes.value()) };
//     else
//         return {};
// }

// template <typename T>
// template <typename ConstBufferSequence>
// std::optional<std::vector<T>> Deserializer<std::vector<T>>::deserialize(
//     DeserializationPtr<ConstBufferSequence> &ptr)
// {
//     if (!ptr.template has<size_t>())
//         return {};

//     const auto size = ptr.template get<size_t>();

//     auto res = std::vector<T>{};
//     res.reserve(size);

//     for (auto i = size_t{}; i < size; ++i)
//     {
//         auto optRes = Deserializer<T>::deserialize(ptr);
//         if (!optRes.has_value())
//             return {};
//         else
//             res.push_back(std::move(optRes.value()));
//     }

//     return res;
// }

// template <typename ConstBufferSequence>
// std::optional<std::string>
//     Deserializer<std::string>::deserialize(DeserializationPtr<ConstBufferSequence>
//     &ptr)
// {
//     static_assert(std::is_pod_v<std::string::value_type>);

//     if (!ptr.template has<size_t>())
//         return {};

//     const auto size = ptr.template get<size_t>();

//     auto res = std::string{};
//     res.reserve(size);

//     if (!ptr.template has<std::string::value_type>(size))
//         return {};

//     const auto [l, r] = ptr.template get<std::string::value_type>(size);

//     res.insert(res.begin(), l, r);

//     return res;
// }

// }    // namespace Proto::detail::Deserialization

// namespace Proto
// {
// template <Concepts::Deserializable T, typename ConstBufferSequence>
// std::optional<T> deserialize(DeserializationPtr<ConstBufferSequence> &ptr)
// {
//     auto res = detail::Deserialization::Deserializer<T>::deserialize(ptr);

//     if (!ptr.atEnd())
//         return {};
//     else
//         return res;
// }

// }    // namespace Proto
