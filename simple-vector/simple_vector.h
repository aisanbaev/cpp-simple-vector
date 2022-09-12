#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity)
        : capacity_(capacity) {
    }

    size_t GetCapacity() {
        return capacity_;
    }

private:
    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size)
        , capacity_(size) 
    {
        ArrayPtr<Type> tmp(size);
        items_.swap(tmp);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : SimpleVector(size)
    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : SimpleVector(init.size())
    {
        std::move(init.begin(), init.end(), begin());
    }

    explicit SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.GetCapacity());
    }

    // Конструктор копирования
    SimpleVector(const SimpleVector& other) 
        : SimpleVector(other.size_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other) {  
        items_.swap(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    // Оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        
        return *this;
    }

    // Оператор перемещения
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
            items_ = std::exchange(rhs.items_, ArrayPtr<Type>());
        }
        return *this;
    }

    void PushBack(const Type& item) {
        if (capacity_ == 0) {
            Resize(1);
            items_[0] = item;
            return;
        } 

        const size_t push_index = size_;
        Reserve(size_ * 2);
        items_[push_index] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ == 0) {
            Resize(1);
            items_[0] = std::move(item);
            return;
        }

        const size_t push_index = size_;
        Reserve(size_ * 2);
        items_[push_index] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t out_pos = std::distance(begin(), const_cast<Type*>(pos));

        if (capacity_ == 0) {
            PushBack(value);
            return begin();
        }
        else if (capacity_ == size_) {
            Reserve(size_ * 2);
        }

        std::move_backward(begin() + out_pos, end(), end() + 1);
        items_[out_pos] = value;
        ++size_;

        return begin() + out_pos;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t out_pos = std::distance(begin(), const_cast<Type*>(pos));

        if (capacity_ == 0) {
            PushBack(std::move(value));
            return begin();
        }
        else if (capacity_ == size_) {
            Reserve(size_ * 2);
        }

        std::move_backward(begin() + out_pos, end(), end() + 1);
        items_[out_pos] = std::move(value);
        ++size_;

        return begin() + out_pos;
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < cend());

        auto* delete_pos_it = const_cast<Type*>(pos);
        std::move(delete_pos_it + 1, end(), delete_pos_it);
        --size_;

        return delete_pos_it;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);   
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else {
            ArrayPtr<Type> tmp(new_size);
            std::move(begin(), end(), tmp.Get());
            items_.swap(tmp);
            size_ = new_size;
        }

        if (capacity_ < size_) {
            capacity_ = size_;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            items_.swap(tmp);
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return cbegin();
    }

    ConstIterator end() const noexcept {
        return cend();
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs) || (lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs) || (rhs == lhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
