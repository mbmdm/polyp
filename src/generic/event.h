#pragma once

#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <cstring>

namespace polyp {

template <typename FuncType>
class Event;

template <class RetType, class... Args>
class Event<RetType(Args ...)> final
{
private:
    struct ComparableClosure;
    struct ClosureList;

    using Closure        = std::function<RetType(Args ...)>;
    using ClosureListPtr = std::shared_ptr<ClosureList>;

    struct ComparableClosure
    {
        Closure  mExecutable;

        struct
        {
            void*      object = nullptr;
            uint8_t* function = nullptr;
            int          size = 0;
        } mBound;

        ComparableClosure(const ComparableClosure &) = delete;

        ComparableClosure() { }
         
        ComparableClosure(Closure &&closure) : mExecutable(std::move(closure)) { }

        ~ComparableClosure()
        {
            if (mBound.function != nullptr)
                delete[] mBound.function;
        }

        ComparableClosure & operator=(const ComparableClosure &closure)
        {
            mExecutable   = closure.mExecutable;
            mBound.object = closure.mBound.object;
            mBound.size   = closure.mBound.size;
            if (closure.mBound.size == 0)
            {
                mBound.function = nullptr;
            }
            else
            {
                mBound.function = new uint8_t[closure.mBound.size];
                std::memcpy(mBound.function, closure.mBound.function, closure.mBound.size);
            }

            return *this;
        }

        bool operator==(const ComparableClosure &closure)
        {
            if (mBound.object == nullptr && closure.mBound.object == nullptr)
            {
                return mExecutable.target_type() == closure.mExecutable.target_type();
            }
            else
            {
                return mBound.object == closure.mBound.object && mBound.size == closure.mBound.size
                    && std::memcmp(mBound.function, closure.mBound.function, mBound.size) == 0;
            }
        }
    };

    struct ClosureList
    {
        ComparableClosure *Closures;
        int Count;

        ClosureList(ComparableClosure *closures, int count)
        {
            Closures = closures;
            Count = count;
        }

        ~ClosureList()
        {
            delete[] Closures;
        }
    };

private:
    ClosureListPtr m_events;

private:
    bool add(const ComparableClosure &closure)
    {
        auto events = std::atomic_load(&m_events);
        int count;
        ComparableClosure *closures;
        if (events == nullptr)
        {
            count = 0;
            closures = nullptr;
        }
        else
        {
            count = events->Count;
            closures = events->Closures;
        }

        auto newCount = count + 1;
        auto newClosures = new ComparableClosure[newCount];
        if (count != 0)
        {
            for (int i = 0; i < count; i++)
                newClosures[i] = closures[i];
        }

        newClosures[count] = closure;
        auto newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
        if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
            return true;

        return false;
    }

    bool remove(const ComparableClosure &closure)
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr)
            return true;

        int index = -1;
        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++)
        {
            if (closures[i] == closure)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
            return true;

        auto newCount = count - 1;
        ClosureListPtr newEvents;
        if (newCount == 0) 
        {
            newEvents = nullptr;
        }
        else
        {
            auto newClosures = new ComparableClosure[newCount];
            for (int i = 0; i < index; i++)
                newClosures[i] = closures[i];

            for (int i = index + 1; i < count; i++)
                newClosures[i - 1] = closures[i];

            newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
        }

        if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
            return true;

        return false;
    }

public:
    Event() = default;

    Event(const Event &event)
    {
        *this = event;
    }

    ~Event()
    {
        *this = nullptr;
    }

    void operator =(const Event &event)
    {
        std::atomic_store(&m_events, std::atomic_load(&event.m_events));
    }

    void operator=(nullptr_t nullpointer)
    {
        while (true)
        {
            auto events = std::atomic_load(&m_events);
            if (!std::atomic_compare_exchange_weak(&m_events, &events, ClosureListPtr()))
                continue;

            break;
        }
    }

    bool operator==(nullptr_t nullpointer)
    {
        auto events = std::atomic_load(&m_events);
        return events == nullptr;
    }

    bool operator!=(nullptr_t nullpointer)
    {
        auto events = std::atomic_load(&m_events);
        return events != nullptr;
    }

    void operator+=(Closure f)
    {
        ComparableClosure closure(std::move(f));
        while (true)
        {
            if (add(closure))
                break;
        }
    }

    void operator-=(Closure f)
    {
        ComparableClosure closure(std::move(f));
        while (true)
        {
            if (remove(closure))
                break;
        }
    }

    template <typename TObject>
    void bind(RetType(TObject::*function)(Args...), TObject *object)
    {
        ComparableClosure closure;
        closure.mExecutable = [object, function](Args&&...args)
        {
            return (object->*function)(std::forward<Args>(args)...);
        };
        closure.mBound.size = sizeof(function);
        closure.mBound.function = new uint8_t[closure.mBound.size];
        std::memcpy(closure.mBound.function, (void*)&function, sizeof(function));
        closure.mBound.object = object;

        while (true)
        {
            if (add(closure))
                break;
        }
    }

    template <typename TObject>
    void unbind(RetType(TObject::*function)(Args...), TObject *object)
    {
        ComparableClosure closure;
        closure.mBound.size       = sizeof(function);
        closure.mBound.function = new uint8_t[closure.mBound.size];
        std::memcpy(closure.mBound.function, (void*)&function, sizeof(function));
        closure.mBound.object = object;

        while (true) {
            if (remove(closure)) {
                break;
            }
        }
    }

    void operator()()
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr) {
            return;
        }

        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++) {
            closures[i].mExecutable();
        }
    }

    template <typename TArg0, typename ...Args2>
    void operator()(TArg0 a1, Args2... tail)
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr) {
            return;
        }

        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++) {
            closures[i].mExecutable(a1, tail...);
        }
    }
};

}
