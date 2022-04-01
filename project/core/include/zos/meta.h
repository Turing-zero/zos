#ifndef __ZOS_META_H__
#define __ZOS_META_H__
#include <functional>
#include "zos/data.h"
namespace zos{
namespace concepts{
template<typename T,typename... Args>
concept are_convertiable = std::conjunction_v<std::is_convertible<Args,T>...>;
}
namespace meta{
    using socket_callback_type = std::function<void(const void*,size_t)>;
    using callback_type = std::function<void(const zos::Data&)>;
} // namespace zos::meta
} // namespace zos

#endif // __ZOS_META_H__