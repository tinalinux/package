#ifndef CPPVECTOR_H
#define CPPVECTOR_H

#define C_VECTOR

#ifndef C_VECTOR
    #include <vector>
    #define CppVector std::vector
#else
    #include <memory.h>

    //#define

    template <typename T>
    class CppVector
    {
    public:
        CppVector()
            : m_len(0)
        {
            m_capacity = 32;
            m_ptr = new T[m_capacity];
        }

        ~CppVector()
        {
            if (m_capacity > 0)
            {
                delete [] m_ptr;
            }
        }

    public:
        void push_back(const T& item)
        {
            if (m_len >= m_capacity)
            {
                T* ptr = NULL;

                m_capacity += 32;
                ptr = new T[m_capacity];
                memcpy(ptr, m_ptr, sizeof(T) * m_len);
                delete [] m_ptr;
                m_ptr = ptr;
            }

            m_ptr[m_len] = item;
            m_len++;
        }

        void clear()
        {
            m_len = 0;
        }

        size_t size() const
        {
            return m_len;
        }

        T& operator[](size_t index)
        {
            return m_ptr[index];
        }

        const T& operator[](size_t index) const
        {
            return m_ptr[index];
        }

    private:
        size_t m_capacity;
        size_t m_len;
        T* m_ptr;
    };
#endif

#endif // CPPVECTOR_H
