#pragma once

#include <iostream>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    // �������������� ArrayPtr ������� ����������
    ArrayPtr() = default;

    // ������ � ���� ������ �� size ��������� ���� Type.
    // ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
    explicit ArrayPtr(size_t size) {
        if (size == 0) return;
        raw_ptr_ = new Type[size]{};
    }

    // ����������� �� ������ ���������, ��������� ����� ������� � ���� ���� nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    // ��������� �����������
    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr(ArrayPtr&& other){
        assert(!raw_ptr_);
        raw_ptr_ = std::move(other.raw_ptr_);
        std::exchange(other.raw_ptr, nullptr);
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        if (this != &rhs) {
            swap(rhs);
        }
        return *this;
    }

    // ��������� ������������
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    // ���������� ��������� �������� � ������, ���������� �������� ������ �������
    // ����� ������ ������ ��������� �� ������ ������ ����������
    [[nodiscard]] Type* Release() noexcept {
        Type* raw_ptr;
        raw_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return raw_ptr;
    }

    // ���������� ������ �� ������� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // ���������� ����������� ������ �� ������� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    // ���������� true, ���� ��������� ���������, � false � ��������� ������
    explicit operator bool() const {
        return raw_ptr_;
    }

    // ���������� �������� ������ ���������, ��������� ����� ������ �������
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // ������������ ��������� ��������� �� ������ � �������� other
    void swap(ArrayPtr& other) noexcept {
        std::swap(this->raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};

