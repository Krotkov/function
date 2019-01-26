//
// Created by krotk on 26.01.2019.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <iostream>
#include <memory>
#include <cstring>
#include <functional>

const int FUNCTION_BUFFER_SIZE = 64;

enum class TYPE {
    EMPTY = 0,
    SMALL,
    BIG
};

template<typename>
class function;

template<typename T, typename... Args>
class function<T(Args...)> {
private:
    class base_holder {
    public:
        virtual ~base_holder() {};

        virtual T invoke(Args &&...) = 0;

        virtual std::unique_ptr<base_holder> copy() const = 0;
    };

    template<typename S>
    class holder : public base_holder {
    public:
        holder(const S &t) : value(t) {}

        holder(S &&t) : value(std::move(t)) {}

        ~holder() = default;

        T invoke(Args &&...args) {
            return value(std::forward<Args>(args)...);
        }

        std::unique_ptr<base_holder> copy() const {
            return std::make_unique<holder<S>>(value);
        }

    private:
        S value;
    };

    union {
        std::unique_ptr<base_holder> func;
        char buf[FUNCTION_BUFFER_SIZE];
    };
    TYPE type;
public:
    function() : func(nullptr), type(TYPE::EMPTY) {}

    explicit function(std::nullptr_t) : func(nullptr), type(TYPE::EMPTY) {}

    function(const function &other) : func(nullptr), type(other.type) {
        switch (type) {
            case TYPE::EMPTY:
                func = nullptr;
                break;
            case TYPE::SMALL:
                memcpy(this->buf, other.buf, FUNCTION_BUFFER_SIZE);
                break;
            case TYPE::BIG:
                func = (std::move(other.func->copy()));
        }
    }

    function(function &&other) noexcept : func(nullptr), type(TYPE::EMPTY) {
        this->swap(other);
    }

    template<typename F>
    function(F f) {
        if (sizeof(holder<F>(f)) <= FUNCTION_BUFFER_SIZE) {
            type = TYPE::SMALL;
            new(this->buf) holder<F>(std::move(f));
        } else {
            type = TYPE::BIG;
            new(this->buf) std::unique_ptr<holder<F>>(std::make_unique<holder<F>>(std::move(f)));
        }
    }

    void swap(function &other) noexcept {
        //std::swap(this->func, other.func);
        std::swap(this->buf, other.buf);
        std::swap(type, other.type);
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(func);
    }

    T operator()(Args&&... args) const {
        switch (type) {
            case TYPE::EMPTY:
                throw std::bad_function_call();
            case TYPE::SMALL:
                return ((base_holder *) (buf))->invoke(std::forward<Args>(args)...);
            case TYPE::BIG:
                return func->invoke(std::forward<Args>(args)...);
        }
    }

    ~function() {
        switch (type) {
            case TYPE::SMALL:
                ((base_holder*)this->buf)->~base_holder();
                break;
            case TYPE::BIG:
                this->func.reset();
        }
    }

    function &operator=(const function &other) {
        auto temp(other);
        this->swap(temp);
        return *this;
    }

    function &operator=(function &&other) noexcept {
        this->swap(other);
        if (other.type == TYPE::BIG) {
            other.func.reset();
        }
        other.type = TYPE::EMPTY;
        return *this;
    }

};

#endif //FUNCTION_FUNCTION_H