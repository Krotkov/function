//
// Created by krotk on 26.01.2019.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <iostream>
#include <memory>

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
        holder(const S& t) : value(t) {}
        holder(S&& t) : value(std::move(t)) {}
        ~holder() = default;
        T invoke(Args&&...args) {
            return value(std::forward<Args>(args)...);
        }
        std::unique_ptr<base_holder> copy() const {
            return std::make_unique<holder<S>>(value);
        }
    private:
        S value;
    };
    std::unique_ptr<base_holder> func;
public:
    function() : func(nullptr){}
    explicit function(std::nullptr_t) : func(nullptr) {}
    function(const function & other) {
        func = (std::move(other.func->copy()));
    }

    function(function &&other) noexcept {
        this->swap(other);
    }

    template <typename F>
    function (F f) {
        func = (std::make_unique<holder<F>>(std::move(f)));
    }

    void swap(function& other) {
        std::swap(this->func, other.func);
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(func);
    }

    T operator()(Args... args) const {
        return func->invoke(std::forward<Args>(args)...);
    }

    ~function() {
        func.reset();
    }

    function& operator=(const function& other) {
        auto temp(other);
        this->swap(temp);
        return *this;
    }

    function operator=(function&& other) noexcept {
        this->swap(other);
        return *this;
    }

};

#endif //FUNCTION_FUNCTION_H
