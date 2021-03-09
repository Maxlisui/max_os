#pragma once

struct mutex {

    bool locked;

public:
    mutex()
        : locked(false)
    {
    }

    bool is_locked() const
    {
        bool res = false;
        __atomic_load(&locked, &res, __ATOMIC_SEQ_CST);
        return res;
    }

    void lock()
    {
        while (!__sync_bool_compare_and_swap(&locked, false, true)) {
            asm volatile("pause");
        }

        __sync_synchronize();
    }

    void unlock()
    {
        __sync_synchronize();

        __atomic_store_n(&locked, false, __ATOMIC_SEQ_CST);
        locked = false;
    }
};