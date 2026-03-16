#pragma once

#include <new>
#include <utility>

template <typename T> class Vector {
    private:
        T *vec;
        unsigned capacity;
        unsigned count;

    public:
        ~Vector();

        Vector();
        Vector(const Vector &other);
        Vector(Vector &&other) noexcept;

        Vector &operator=(const Vector &other);
        Vector &operator=(Vector &&other) noexcept;

        T &operator[](unsigned i);
        const T &operator[](unsigned i) const;

        T *raw() const;

        void push(const T &el);
        void push(const T &el, unsigned idx);

        T pop();
        void remove(unsigned index);

        unsigned len() const;
        unsigned cap() const;
        bool empty() const;

        void display() const;
        void clear();
};
