#include <vec.h>
#include <iostream>
#include <group.h>

template <typename T>
Vector<T>::Vector() : vec(nullptr), capacity(0), count(0) {}

template <typename T> Vector<T>::~Vector() {
    clear();
    ::operator delete(vec);
}

template <typename T> Vector<T>::Vector(const Vector &other) {
    count = other.count;
    capacity = other.capacity;

    if (capacity > 0) {
        vec = (T *)::operator new(capacity * sizeof(T));
        for (unsigned i = 0; i < count; ++i) { new (&vec[i]) T(other.vec[i]); }
    } else {
        vec = nullptr;
    }
}

template <typename T>
Vector<T>::Vector(Vector &&other) noexcept
    : vec(other.vec), capacity(other.capacity), count(other.count) {
    other.vec = nullptr;
    other.capacity = 0;
    other.count = 0;
}

template <typename T> Vector<T> &Vector<T>::operator=(const Vector &other) {
    if (this != &other) {
        Vector<T> temp(other);

        std::swap(vec, temp.vec);
        std::swap(capacity, temp.capacity);
        std::swap(count, temp.count);
    }
    return *this;
}

template <typename T> Vector<T> &Vector<T>::operator=(Vector &&other) noexcept {
    if (this != &other) {
        clear();
        ::operator delete(vec);

        vec = other.vec;
        capacity = other.capacity;
        count = other.count;

        other.vec = nullptr;
        other.capacity = 0;
        other.count = 0;
    }
    return *this;
}

template <typename T> void Vector<T>::clear() {
    for (unsigned i = 0; i < count; ++i) { vec[i].~T(); }
    count = 0;
}

template <typename T> void Vector<T>::push(const T &el) {
    if (count == capacity) {
        unsigned newCap = capacity ? capacity * 2 : 4;

        T *newVec = (T *)::operator new(newCap * sizeof(T));

        for (unsigned i = 0; i < count; i++) {
            new (&newVec[i]) T(std::move(vec[i]));
            vec[i].~T();
        }

        ::operator delete(vec);

        vec = newVec;
        capacity = newCap;
    }
    new (&vec[count]) T(el);
    count++;
}

template <typename T> void Vector<T>::push(const T &el, unsigned idx) {
    if (idx > count) idx = count;

    if (count == capacity) {
        unsigned newCap = capacity ? capacity * 2 : 4;
        T *newVec = (T *)::operator new(newCap * sizeof(T));

        for (unsigned i = 0; i < idx; i++) {
            new (&newVec[i]) T(std::move(vec[i]));
            vec[i].~T();
        }

        new (&newVec[idx]) T(el);

        for (unsigned i = idx; i < count; i++) {
            new (&newVec[i + 1]) T(std::move(vec[i]));
            vec[i].~T();
        }

        ::operator delete(vec);
        vec = newVec;
        capacity = newCap;
    } else {
        if (count > idx) {
            new (&vec[count]) T(std::move(vec[count - 1]));

            for (unsigned i = count - 1; i > idx; i--) {
                vec[i] = std::move(vec[i - 1]);
            }

            vec[idx] = el;
        } else {
            new (&vec[count]) T(el);
        }
    }

    count++;
}

template <typename T> T Vector<T>::pop() {
    T el = std::move(vec[count - 1]);

    vec[count - 1].~T();

    count--;
    return el;
}

template <typename T> void Vector<T>::remove(unsigned index) {
    if (index >= count) return;

    for (unsigned i = index; i < count - 1; i++) {
        vec[i] = std::move(vec[i + 1]);
    }

    vec[count - 1].~T();
    count--;
}

template <typename T> T *Vector<T>::raw() const { return vec; }

template <typename T> T &Vector<T>::operator[](unsigned i) { return vec[i]; }
template <typename T> const T &Vector<T>::operator[](unsigned i) const {
    return vec[i];
}

template <typename T> unsigned Vector<T>::len() const { return count; }
template <typename T> unsigned Vector<T>::cap() const { return capacity; }
template <typename T> bool Vector<T>::empty() const { return count == 0; }

template <typename T> void Vector<T>::display() const {
    printf("[");
    for (unsigned i = 0; i < count; i++) {
        if (i < count - 1) {
            std::cout << vec[i] << ((i < count - 1) ? ", " : "");
        } else {
            std::cout << vec[i];
        }
    }
    printf("]\n");
}

template class Vector<Object *>;
