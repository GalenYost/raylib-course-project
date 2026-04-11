#pragma once

#include <utility>
#include <new>

template <typename T> class Vector {
    private:
        T *vec;
        unsigned capacity;
        unsigned count;

    public:
        ~Vector() {
            clear();
            ::operator delete(vec);
        }

        Vector() : vec(nullptr), capacity(0), count(0) {};
        Vector(const Vector &other) {
            count = other.count;
            capacity = other.capacity;

            if (capacity > 0) {
                vec = (T *)::operator new(capacity * sizeof(T));
                for (unsigned i = 0; i < count; ++i) {
                    new (&vec[i]) T(other.vec[i]);
                }
            } else {
                vec = nullptr;
            }
        }

        Vector(Vector &&other) noexcept : vec(other.vec), capacity(other.capacity), count(other.count) {
            other.vec = nullptr;
            other.capacity = 0;
            other.count = 0;
        }

        Vector &operator=(const Vector &other) {
            if (this != &other) {
                Vector<T> temp(other);

                std::swap(vec, temp.vec);
                std::swap(capacity, temp.capacity);
                std::swap(count, temp.count);
            }
            return *this;
        }

        Vector<T> &operator=(Vector &&other) noexcept {
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

        T &operator[](unsigned i) { return vec[i]; }

        const T &operator[](unsigned i) const {
            return vec[i];
        }

        T *raw() const { return vec; }

        void push(T el) {
            push(std::move(el), len());
        }

        void push(T el, unsigned idx) {
            if (idx > count) idx = count;

            if (count == capacity) {
                unsigned newCap = capacity ? capacity * 2 : 4;
                T *newVec = (T *)::operator new(newCap * sizeof(T));

                for (unsigned i = 0; i < idx; i++) {
                    new (&newVec[i]) T(std::move(vec[i]));
                    vec[i].~T();
                }

                new (&newVec[idx]) T(std::move(el));

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

                    vec[idx] = std::move(el);
                } else {
                    new (&vec[count]) T(std::move(el));
                }
            }

            count++;
        }

        T pop() {
            T el = std::move(vec[count - 1]);

            vec[count - 1].~T();

            count--;
            return el;
        }

        void remove(unsigned index) {
            if (index >= count) return;

            for (unsigned i = index; i < count - 1; i++) {
                vec[i] = std::move(vec[i + 1]);
            }

            vec[count - 1].~T();
            count--;
        }

        unsigned len() const { return count; }
        unsigned cap() const { return capacity; }
        bool empty() const { return count == 0; }

        void clear() {
            for (unsigned i = 0; i < count; ++i) { vec[i].~T(); }
            count = 0;
        }
};
