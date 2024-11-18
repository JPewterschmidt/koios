// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>
#include <memory>
#include <utility>
#include <exception>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

#include "toolpex/move_only.h"

KOIOS_NAMESPACE_BEG

/*! \brief store the return value. Wake this caller coroutine.
 *
 *  \tparam T the return value type just like the one of a regular function.
 *  \tparam Promise the promise type of inherited class.
 */
template<typename T, typename Promise>
class return_value_or_void_base : public toolpex::move_only // See also https://devblogs.microsoft.com/oldnewthing/20210504-00/?p=105176
{
public:
    /*! Take the ownership of the future object */
    auto get_future() { return m_promise.get_future(); }

protected:
    koios::promise<T> m_promise;

    // Called by `unhandled_exception`
    void deal_exception(::std::exception_ptr ep)
    {
        m_promise.set_exception(ep);
    }
};

/*! \brief The major `return_value_or_void` class template.
 *  \tparam T the return type of the task.
 *  \tparam Promise the promise type of the `task`.
 */
template<typename T, typename Promise>
class return_value_or_void 
    : public return_value_or_void_base<T, Promise>
{
public: 
    /*! \brief set the return value.
     *  The compiler generates code which call this function when user `co_return` something.
     *  After store the return value, this function will wake up the caller, 
     *  if there's a caller task call this task by `co_await`.
     */
    template<::std::convertible_to<T> TT = T>
    void return_value(TT&& val)
    {
        this->m_promise.set_value(::std::forward<TT>(val));
    }
};

/*! \brief the specializtion of return_value_or_void, which deal with void return type. */
template<typename Promise>
class return_value_or_void<void, Promise> 
    : public return_value_or_void_base<void, Promise>
{
public:
    /*! \brief Just wake the caller. */
    void return_void() 
    { 
        this->m_promise.set_value();
    }
};

KOIOS_NAMESPACE_END

#endif
