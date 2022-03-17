#pragma once

#include <cassert>
#include <stdexcept>
#include <initializer_list>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj (size_t capacity) {
        capacity_ = capacity;
    }

    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // ������ ������ �� size ���������, ������������������ ��������� �� ���������
    explicit SimpleVector(size_t size) :
        size_(size), capacity_(size)
    {
        ArrayPtr<Type> tmp_ptr(size_);
        vec_ptr_.swap(tmp_ptr);
        std::fill(begin(), end(), 0);
    }

    // ������ ������ �� size ���������, ������������������ ��������� value
    SimpleVector(size_t size, const Type& value) :
        size_(size), capacity_(size)
    {
        ArrayPtr<Type> tmp_ptr(size_);
        vec_ptr_.swap(tmp_ptr);
        std::fill(begin(), end(), value);
    }

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
        size_(init.size()), capacity_(init.size())
    {
        ArrayPtr<Type> tmp_ptr(init.size());
        vec_ptr_.swap(tmp_ptr);
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector (ReserveProxyObj Reserve) :
        size_(0), capacity_(Reserve.capacity_)
    {
        ArrayPtr<Type> tmp_ptr(capacity_);
        vec_ptr_.swap(tmp_ptr);
    }

    SimpleVector(const SimpleVector& other) {
        assert(capacity_ == 0);
        SimpleVector tmp_vec(other.size_);
        std::copy(other.begin(), other.end(), tmp_vec.begin());
        swap(tmp_vec);
    }

    SimpleVector(SimpleVector&& other){
        assert(capacity_ == 0);
        SimpleVector tmp_vec(other.size_);
        std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), tmp_vec.begin());
        swap(tmp_vec);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            swap(rhs);
        }
        return *this;
    }

    // ��������� ������� � ����� �������
    // ��� �������� ����� ����������� ����� ����������� �������
    void PushBack(const Type& item) {
        if (capacity_ != 0 && capacity_ == size_) {
            ArrayPtr<Type> tmp_ptr(size_ * 2);
            std::copy(begin(), end(), &tmp_ptr[0]);
            vec_ptr_.swap(tmp_ptr);
            capacity_ *= 2;
        } else if (capacity_ == 0) {
            ArrayPtr<Type> tmp_ptr(1);
            vec_ptr_.swap(tmp_ptr);
            capacity_ = 1;
        }

        vec_ptr_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ != 0 && capacity_ == size_) {
            ArrayPtr<Type> tmp_ptr(size_ * 2);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp_ptr[0]);
            vec_ptr_.swap(tmp_ptr);
            capacity_ *= 2;
        } else if (capacity_ == 0) {
            ArrayPtr<Type> tmp_ptr(1);
            vec_ptr_.swap(tmp_ptr);
            capacity_ = 1;
        }

        vec_ptr_[size_] = std::move(item);
        ++size_;
    }

    // ��������� �������� value � ������� pos.
    // ���������� �������� �� ����������� ��������
    // ���� ����� �������� �������� ������ ��� �������� ���������,
    // ����������� ������� ������ ����������� �����, � ��� ������� ������������ 0 ����� ������ 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (capacity_ != 0 && capacity_ == size_) {
            ArrayPtr<Type> tmp_ptr(size_ * 2);
            size_t out_pos = std::distance(begin(), const_cast<Type*>(pos));

            std::copy(begin(), begin() + out_pos, &tmp_ptr[0]);
            tmp_ptr[out_pos] = value;
            std::copy(begin() + out_pos, end(), &tmp_ptr[out_pos + 1]);

            vec_ptr_.swap(tmp_ptr);
            ++size_;
            capacity_ *= 2;
            return &vec_ptr_[out_pos];

        } else if (capacity_ == 0 && pos == begin()) {
            ArrayPtr<Type> tmp_ptr(1);
            tmp_ptr[0] = value;
            vec_ptr_.swap(tmp_ptr);
            ++size_;
            capacity_ = 1;
            return &vec_ptr_[0];

        } else if (size_ < capacity_){
            auto* out_pos = const_cast<Type*>(pos);
            std::copy_backward(out_pos, end(), end() + 1);
            *out_pos = value;
            ++size_;
            return out_pos;
        }

        return const_cast<Type*>(pos);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        if (capacity_ != 0 && capacity_ == size_) {
            ArrayPtr<Type> tmp_ptr(size_ * 2);
            size_t insert_pos = std::distance(begin(), const_cast<Type*>(pos));

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + insert_pos), &tmp_ptr[0]);
            tmp_ptr[insert_pos] = std::move(value);
            std::copy(std::make_move_iterator(begin() + insert_pos), std::make_move_iterator(end()), &tmp_ptr[insert_pos + 1]);

            vec_ptr_.swap(tmp_ptr);
            ++size_;
            capacity_ *= 2;
            return &vec_ptr_[insert_pos];

        } else if (capacity_ == 0 && pos == begin()) {
            ArrayPtr<Type> tmp_ptr(1);
            tmp_ptr[0] = std::move(value);
            vec_ptr_.swap(tmp_ptr);
            ++size_;
            capacity_ = 1;
            return &vec_ptr_[0];

        } else if (size_ < capacity_){
            auto* out_pos = const_cast<Type*>(pos);
            std::copy_backward(std::make_move_iterator(out_pos), std::make_move_iterator(end()), end() + 1);
            *out_pos = std::move(value);
            ++size_;
            return out_pos;
        }

        return const_cast<Type*>(pos);
    }


    void Reserve(size_t new_capacity) {
        if (new_capacity < capacity_) {
            return;
        }
        ArrayPtr<Type> tmp_ptr(new_capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp_ptr[0]);
        vec_ptr_.swap(tmp_ptr);
        capacity_ = new_capacity;
   }

    // "�������" ��������� ������� �������. ������ �� ������ ���� ������
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // ������� ������� ������� � ��������� �������
    Iterator Erase(ConstIterator pos) {
        auto* i = const_cast<Type*>(pos);
        auto* j = i + 1;
        while (j < end()) {
            *i = std::move(*j);
            ++i;
            ++j;
        }
        --size_;
        return const_cast<Type*>(pos);
    }

    // ���������� �������� � ������ ��������
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        vec_ptr_.swap(other.vec_ptr_);
    }

    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept {
        return size_;
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // ���������� ������ �� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return vec_ptr_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        return vec_ptr_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    Type& At(size_t index) {
        if (index < capacity_) {
            return vec_ptr_[index];
        } else {
            throw std::out_of_range("invalid index");
        }
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    const Type& At(size_t index) const {
        if (index < capacity_) {
            return vec_ptr_[index];
        } else {
            throw std::out_of_range("invalid index");
        }
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept {
        size_ = 0;
    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� �������� �� ��������� ��� ���� Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            ArrayPtr<Type> tmp_ptr(new_size);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp_ptr[0]);
            std::fill(&tmp_ptr[size_], &tmp_ptr[new_size], 0);
            vec_ptr_.swap(tmp_ptr);
            size_ = new_size;
            capacity_ = new_size;

        } else if (new_size < capacity_ && new_size > size_) {
            std::fill(&vec_ptr_[size_], &vec_ptr_[new_size], 0);
            size_ = new_size;

        } else {
            size_ = new_size;
        }
    }

    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator begin() noexcept {
        return &vec_ptr_[0];
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator end() noexcept {
        return &vec_ptr_[size_];
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cbegin() const noexcept {
        return &vec_ptr_[0];
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cend() const noexcept {
        return &vec_ptr_[size_];
    }

private:
    ArrayPtr<Type> vec_ptr_;
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

