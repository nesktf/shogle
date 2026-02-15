#pragma once

#include <concepts>
#include <iterator>

namespace shogle::meta {

template<typename Cont>
concept any_std_cont = requires(Cont a, const Cont b) {
  requires std::regular<Cont>;
  requires std::swappable<Cont>;
  requires std::destructible<typename Cont::value_type>;
  requires std::same_as<typename Cont::reference, typename Cont::value_type&>;
  requires std::same_as<typename Cont::const_reference, const typename Cont::value_type&>;
  requires std::forward_iterator<typename Cont::iterator>;
  requires std::forward_iterator<typename Cont::const_iterator>;
  requires std::signed_integral<typename Cont::difference_type>;
  requires std::same_as<typename Cont::difference_type,
                        typename std::iterator_traits<typename Cont::iterator>::difference_type>;
  requires std::same_as<
    typename Cont::difference_type,
    typename std::iterator_traits<typename Cont::const_iterator>::difference_type>;
  { a.begin() } -> std::same_as<typename Cont::iterator>;
  { a.end() } -> std::same_as<typename Cont::iterator>;
  { b.begin() } -> std::same_as<typename Cont::const_iterator>;
  { b.end() } -> std::same_as<typename Cont::const_iterator>;
  { a.cbegin() } -> std::same_as<typename Cont::const_iterator>;
  { a.cend() } -> std::same_as<typename Cont::const_iterator>;
  { a.size() } -> std::same_as<typename Cont::size_type>;
  { a.max_size() } -> std::same_as<typename Cont::size_type>;
  { a.empty() } -> std::same_as<bool>;
};

template<typename Cont, typename T>
concept std_cont_of =
  any_std_cont<Cont> && requires() { requires std::same_as<typename Cont::value_type, T>; };

template<typename Cont, typename T, typename... Args>
concept growable_emplace_container_of =
  std_cont_of<Cont, T> && requires(Cont c, T obj, Args&&... args) {
    { c.emplace_back(obj) } -> std::same_as<typename Cont::reference>;
    { c.emplace_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
    { c.emplace_back(std::forward<Args>(args)...) } -> std::same_as<typename Cont::reference>;
  };

template<typename Cont, typename T>
concept growable_push_container_of = std_cont_of<Cont, T> && requires(Cont c, T obj) {
  { c.push_back(obj) } -> std::same_as<typename Cont::reference>;
  { c.push_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
};

template<typename Cont>
concept reservable_std_cont = any_std_cont<Cont> && requires(Cont c, typename Cont::size_type sz) {
  { c.reserve(sz) } -> std::same_as<void>;
};

template<typename TL, typename... TR>
concept same_as_any = (... or std::same_as<TL, TR>);

template<typename TL, typename... TR>
concept convertible_to_any = (... or std::convertible_to<TL, TR>);

}; // namespace shogle::meta
