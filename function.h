//
// Created by krotk on 26.01.2019.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <iostream>
#include <memory>
#include <cstring>

const int FUNCTION_BUFFER_SIZE = 64;

enum TYPE {
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
    function() : func(nullptr), type(EMPTY) {}

    explicit function(std::nullptr_t) : func(nullptr), type(EMPTY) {}

    function(const function &other) : func(nullptr), type(other.type) {
        switch (type) {
            case EMPTY:
                func = nullptr;
                break;
            case SMALL:
                memcpy(this->buf, other.buf, FUNCTION_BUFFER_SIZE);
                break;
            case BIG:
                func = (std::move(other.func->copy()));
        }
    }

    function(function &&other) noexcept {
        this->swap(other);
    }

    template<typename F>
    function(F f) {
        if (sizeof(holder<F>(f)) <= FUNCTION_BUFFER_SIZE) {
            type = SMALL;
            new(this->buf) holder<F>(std::move(f));
        } else {
            type = BIG;
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

    T operator()(Args... args) const {
        switch (type) {
            case EMPTY:
                throw std::bad_function_call();
            case SMALL:
                return ((base_holder *) (buf))->invoke(std::forward<Args>(args)...);
            case BIG:
                return func->invoke(std::forward<Args>(args)...);
        }
    }

    ~function() {
        switch (type) {
            case SMALL:
                ((base_holder*)this->buf)->~base_holder();
                break;
            case BIG:
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
        return *this;
    }

};

#endif //FUNCTION_FUNCTION_H
