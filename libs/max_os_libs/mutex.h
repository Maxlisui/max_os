#pragma once

struct mutex {

    bool is_locked;

public:
    mutex()
        : is_locked(false)
    {
    }

    bool is_locked() const
    {
        bool res = false;
        __atomic_load(&is_locked, &res, __ATOMIC_SEQ_CST);
        return res;
    }

    void lock()
    {
        while (!__sync_bool_compare_and_swap(&is_locked, false, true)) {
            asm volatile("pause");
        }

        __sync_synchronize();
    }

    void unlock()
    {
        __sync_synchronize();

        __atomic_store_n(&is_locked, false, __ATOMIC_SEQ_CST);
        is_locked = false;
    }
};