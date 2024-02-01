#ifndef __TBK_META_H__
#define __TBK_META_H__
#include <atomic>

namespace tbk{
namespace concepts{
template<typename T,typename... Args>
concept are_convertiable = std::conjunction_v<std::is_convertible<Args,T>...>;
template<typename T>
concept Serializable = requires(T t,void* p,size_t size){
    { t.ByteSize() } -> std::convertible_to<std::size_t>;
    { t.SerializeToArray(p,size) } -> std::convertible_to<bool>;
};
} // namespace tbk::concepts


// atomic check
// By default, types are not atomic,
template<typename T> auto constexpr is_atomic = false;
// but std::atomic<T> types are,
template<typename T> auto constexpr is_atomic<std::atomic<T>> = true;
// as well as std::atomic_flag.
template<> auto constexpr is_atomic<std::atomic_flag> = true;

} // namespace tbk

#endif // __TBK_META_H__
