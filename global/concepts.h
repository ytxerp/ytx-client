#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <concepts>

template <typename Container>
concept Iterable = requires(Container c) {
    { c.begin() } -> std::same_as<typename Container::iterator>;
    { c.end() } -> std::same_as<typename Container::iterator>;
};

template <typename T>
concept Resettable = requires(T t) {
    { t.ResetState() } -> std::same_as<void>;
};

#endif // CONCEPTS_H
