#pragma once

#define DISABLE_COPY(ClassName)                       \
    ClassName(const ClassName &)            = delete; \
    ClassName &operator=(const ClassName &) = delete;
#define DISABLE_MOVE(ClassName)                  \
    ClassName(ClassName &&)            = delete; \
    ClassName &operator=(ClassName &&) = delete;

#define DEFAULT_MOVE(ClassName)                   \
    ClassName(ClassName &&)            = default; \
    ClassName &operator=(ClassName &&) = default;
#define DEFAULT_COPY(ClassName)                        \
    ClassName(const ClassName &)            = default; \
    ClassName &operator=(const ClassName &) = default;

#define DISABLE_COPY_MOVE(ClassName) \
    DISABLE_COPY(ClassName)          \
    DISABLE_MOVE(ClassName)

#define DISABLE_COPY_DEFAULT_MOVE(ClassName) \
    DISABLE_COPY(ClassName)                  \
    DEFAULT_MOVE(ClassName)

#define DISABLE_ALL(ClassName)   \
    DISABLE_COPY_MOVE(ClassName) \
    ClassName()  = delete;       \
    ~ClassName() = delete;
