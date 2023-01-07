#pragma once

#include <cstddef>
#include <memory>

#include "utils/copymove.h"

namespace Proto2
{
class Buffer
{
    DISABLE_COPY_MOVE(Buffer)
public:
    Buffer()  = default;
    ~Buffer() = default;

    size_t     size() const noexcept;
    std::byte *get();

    bool resize(size_t bytes);

private:
    size_t                       size_{};
    std::unique_ptr<std::byte[]> memory_{};
};

}    // namespace Proto2
