#include <array>
#include <ostream>
#include "utils/Types.h"

template <class T, AUInt N>
class Ringbuffer {
    public:
        Ringbuffer() = default;

        /**
         * Creates inplace a buffer filled with a default object constructed
         * from `args`.
         * Allows to play with objects non movable/assignable.
         */
        template <typename... Args>
        constexpr Ringbuffer(Args&&... args)
            : _buffer{ T{std::forward<Args>(args)...} } {}
            
        /**
         * Add @el in the ring buffer if there is space left
         * @return return false if there are no place left
         */
        template <class U> // Template is used to allow both copy and move assignment
        bool tryAdd(U&& el) {
            static_assert(std::is_convertible<T, U>::value, "U must be convertible to T");
            if (isFull()) {
                return false;
            }

            _buffer[_pos] = std::forward<U>(el);
            _incr(_pos);
            ++_size;

            return true;
        }

       /**
        * Add @el in the ring buffer. Overriding the element at the next
        * position if full. 
        */
        template <class U>
        void add(U&& el) {
            static_assert(std::is_convertible<T, U>::value, "U must be convertible to T");
            _buffer[_pos] = std::forward<U>(el);
            _incr(_pos);

            if (isFull()) {
                _incr(_start);
            } else {
                ++_size;
            }
        }

        /**
         * Return a reference to the next element. Invalidating the first
         * element if full.
         */
        T& next() {
            T& ret = _buffer[_pos];
            _incr(_pos);

            if (isFull()) {
                _incr(_start);
            } else {
                ++_size;
            }

            return ret;
        }

        /**
         * Return a reference to first element in the ringbuffer and set it
         * as an available element. Meaning that a call to a non const method
         * can invalidate the reference.
         */
        T& pop() {
            assert(_size > 0);
            T& el = _buffer[_start];
            _incr(_start);
            --_size;

            return el;
        }

        void fill(const T& value) { _buffer.fill(value); }

        T& get() const { return _buffer[_start]; }
        AUInt start() const { return _start; }
        AUInt pos() const { return _pos; }
        AUInt size() const { return _size; }
        constexpr AUInt capacity() const { return N; }
        bool isFull() const { return size() == capacity(); }

    private:
        void _incr(AUInt& val) { val = (val + 1) % N; }
        void _decr(AUInt& val) { val = (val + N - 1) % N; }

        T _buffer[N];
        AUInt _start = 0;
        AUInt _pos  = 0;
        AUInt _size = 0;
};
