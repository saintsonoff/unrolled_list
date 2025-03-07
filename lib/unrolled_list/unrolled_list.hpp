#ifndef _UNROLLED_LIST_HPP_
#define _UNROLLED_LIST_HPP_

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <initializer_list>
#include <concepts>
#include <memory>
#include <cstddef>
#include <utility>

#include "details/storage.hpp"

namespace labwork7 {

namespace details {


template<typename Type>
struct size_of_type {
    static constexpr size_t value = sizeof(Type);
};


template<typename Type>
constexpr size_t size_of_type_v = size_of_type<Type>::value;


template<typename Type>
struct is_large_obj {
    static constexpr bool value = size_of_type_v<Type> < (size_of_type_v<uint64_t> * 3);
};


template<typename Type>
static constexpr bool is_large_obj_v = is_large_obj<Type>::value;

template<typename UnrolledListType>
class Iterator : std::bidirectional_iterator_tag {
  public:
    using value_type = typename UnrolledListType::value_type;
    using reference = typename UnrolledListType::reference;
    using const_reference = typename UnrolledListType::const_reference;
    using pointer = typename UnrolledListType::pointer;
    using const_pointer = typename UnrolledListType::const_pointer;
    using size_type = typename UnrolledListType::size_type;
    using difference_type = typename UnrolledListType::difference_type;

  private:
    using node_t = typename UnrolledListType::node_t;

  public:
    Iterator() noexcept = default;
    Iterator(node_t* node_ptr, size_t offest) noexcept
        : chunck_ptr_m(node_ptr), chunck_offset_m(offest) {  };

  public:
    Iterator& operator++() noexcept {
        ++chunck_offset_m;

        if (chunck_offset_m == chunck_ptr_m->size_m) {
            chunck_ptr_m = chunck_ptr_m->next_chunck_ptr_m;
            chunck_offset_m = chunck_ptr_m ? chunck_ptr_m->size_m : 0;
        }
        return *this;
    };

    Iterator operator++(int) noexcept {
        Iterator result_itr = *this;
        return ++result_itr;
    };

    Iterator& operator--() noexcept {        
        if (chunck_offset_m == 0) {
            chunck_ptr_m = chunck_ptr_m->prev_chunck_ptr_m;
            chunck_offset_m = chunck_ptr_m ? chunck_ptr_m->size_m : 1;
        }
        
        --chunck_offset_m;
        return *this;
    };

    Iterator operator--(int) noexcept {
        Iterator result_itr = *this;
        return --result_itr;
    };

    bool operator==(const Iterator& value) const noexcept {
        return (chunck_offset_m == value.chunck_offset_m) && (chunck_ptr_m == value.chunck_ptr_m);
    };

    bool operator!=(const Iterator& value) const noexcept {
        return !(*this == value);
    };

    const_reference operator*() const noexcept {
        return *(chunck_ptr_m->data_m + chunck_offset_m);
    };

    const_pointer operator->() const noexcept {
        return chunck_ptr_m->data_m + chunck_offset_m;
    };

    operator node_t*() noexcept { return chunck_ptr_m; };
    operator const node_t*() const noexcept { return chunck_ptr_m; };

  private:
    size_t chunck_offset_m = 0;
    node_t* chunck_ptr_m = nullptr;
};

} // namespace details


template<std::copy_constructible DataType, size_t ChunckSize = 10, typename AllocatorType = std::allocator<DataType>>
class unrolled_list {
    friend class details::Iterator<unrolled_list>;

  protected:
    using node_t = UnrolledListNodeChunck<DataType, ChunckSize>;

  public:
    using value_type = std::decay_t<DataType>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    
  public:
    using iterator = details::Iterator<unrolled_list>;
    using const_iterator = std::basic_const_iterator<iterator>;
    using reverse_itrator = std::reverse_iterator<iterator>;
    using const_reverse_itrator = std::basic_const_iterator<std::reverse_iterator<const_iterator>>;

  protected:
  using chunck_traits = chunck_traits<node_t, AllocatorType>;

  using allocator_type = typename chunck_traits::allocator_type;
  using allocator_trait_t = std::allocator_traits<allocator_type>;
  
  using data_allocator_type =  typename std::allocator_traits<AllocatorType>::template rebind_alloc<value_type>;
  using data_allocator_trait_t = std::allocator_traits<data_allocator_type>;
  
  
  public:
    unrolled_list() = default;
    
    template<std::convertible_to<allocator_type> AnotherAllocatorType>
    unrolled_list(AnotherAllocatorType&& alloc) : alloc_m(alloc) {  };


    virtual ~unrolled_list() {  };

  public:



    template<typename... ArgsTs>
    requires requires(ArgsTs&&... args) { value_type{std::forward<ArgsTs>(args)...}; }
    void emplace_back(ArgsTs&&... args) {
        if (size_m == 0) {
            begin_chunck_ptr_m = end_chunck_ptr_m = chunck_traits::CreateChunck();
        }

        emplace(cend(), std::forward<ArgsTs>(args)...);
    }

    template<typename... ArgsTs>
    requires requires(ArgsTs&&... args) { value_type{std::forward<ArgsTs>(args)...}; }
    void emplace(const_iterator pos_itr, ArgsTs&&... args) {
        node_t* emplace_node = pos_itr;

        if (emplace_node->size_m == pos_itr->size_value) {
            node_t pos_copy = *pos_itr;
            emplace_node = SplitNode(pos_copy);

            data_allocator_trait_t::construct(data_alloc_m, emplace_node->data_m + emplace_node->size_m, std::forward<ArgsTs>(args)...);

            std::swap(*pos_itr, pos_copy);
            chunck_traits::IncludeChunckBack(pos_itr, emplace_node);
        } else {
            data_allocator_trait_t::construct(data_alloc_m, emplace_node->data_m + pos_itr->size_m);
        }

        ++(emplace_node->size_m);
        ++size_m;
    };


  public:
    iterator begin() { return {begin_chunck_ptr_m, 0}; };
    const_iterator cbegin() const { return {begin_chunck_ptr_m, 0}; };
    const_iterator cend() const {
        if (end_chunck_ptr_m)
            return {end_chunck_ptr_m, end_chunck_ptr_m->size_m};
        return {nullptr, 0};
    };
    iterator end() const {
        if (end_chunck_ptr_m)
            return {end_chunck_ptr_m, end_chunck_ptr_m->size_m};
        return {nullptr, 0};
    };


  private:
    node_t* SplitNode(node_t* current_node)
      const noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_destructible_v<value_type>) {
        node_t* another_node = chunck_traits::CreateChunck(alloc_m);
        
        if constexpr(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_destructible_v<value_type>) {
            for (size_t offset = 0; offset != current_node->size_m / 2; ++offset) {
                data_allocator_trait_t::construct(data_alloc_m, another_node + offset, std::move(*(current_node->data_m + current_node->size_m - offset)));
                data_allocator_trait_t::destroy(data_alloc_m, current_node->data_m + current_node->size_m - offset);
            }
        } else {
            node_t current_copy = *current_node;

            for (size_t offset = 0; offset != current_copy->size_m / 2; ++offset) {
                data_allocator_trait_t::construct(data_alloc_m, another_node + offset, std::move(*(current_copy->data_m + current_copy->size_m - offset)));
                data_allocator_trait_t::destroy(data_alloc_m, current_copy->data_m + current_copy->size_m - offset);
            }

            std::swap(*current_node, current_copy);
        }

        another_node->size_m = current_node->size_m / 2;
        current_node->size_m -= another_node->size_m;

        return another_node;
    };

  protected:
    node_t* begin_chunck_ptr_m = nullptr;
    node_t* end_chunck_ptr_m = nullptr;

protected:
#if defined(_WIN32) || defined(_WIN64) 
    [[msvc::no_unique_address]] allocator_type chunck_alloc_m;
    [[msvc::no_unique_address]] data_allocator_type data_alloc_m;
#else
    [[no_unique_address]] allocator_type alloc_m;
    [[no_unique_address]] data_allocator_type data_alloc_m;
#endif
    size_t size_m = 0; 
};


}  // labwork7

#endif // _UNROLLED_LIST_HPP_