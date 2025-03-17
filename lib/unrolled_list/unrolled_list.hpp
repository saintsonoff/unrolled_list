#ifndef _UNROLLED_LIST_HPP_
#define _UNROLLED_LIST_HPP_

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <initializer_list>
#include <concepts>
#include <memory>
#include <cstddef>
#include <xutility>

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
    using value_type = UnrolledListType::value_type;
    using reference = UnrolledListType::reference;
    using const_reference = UnrolledListType::const_reference;
    using pointer = UnrolledListType::pointer;
    using const_pointer = UnrolledListType::const_pointer;
    using size_type = UnrolledListType::size_type;
    using difference_type = UnrolledListType::difference_type;

  public:
    Iterator() noexcept = default;
    Iterator(UnrolledListType::node_t* node_ptr, size_t offest) noexcept
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

    operator typename UnrolledListType::node_t*() noexcept { return chunck_ptr_m; };
    operator const typename UnrolledListType::node_t*() const noexcept { return chunck_ptr_m; };

  private:
    size_t chunck_offset_m = 0;
    UnrolledListType::node_t* chunck_ptr_m = nullptr;
};

} // namespace details



template<std::copy_constructible DataType, typename AllocatorType = std::allocator<DataType>,
    bool is_large = details::is_large_obj_v<DataType>>
class unrolled_list : public details::unrolled_list_storage<DataType, AllocatorType, AllocatorType> {
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


  private: 
    using node_t = unrolled_list::node_t;
    using chunck_allocator_trait_t = unrolled_list::chunck_allocator_trait_t;

  public:

    ~unrolled_list() {
        for (auto elem : *this) {
            chunck_allocator_trait_t::destroy(this->chunck_alloc_m, &elem);
        }
    };

  public:
    template<std::convertible_to<value_type> AddDataType>
    void insert(iterator current_pos, AddDataType&& data) {
        node_t* current_node{current_pos};

        if (current_node->size_m == 10) {

        } else {

        }

    }

  public:
    iterator begin() const noexcept { return {this->begin_node_m, 0}; };
    iterator end() const noexcept { return {this->end_node_m, this->end_node_m ? this->end_node_m->size_m : 0}; };

    const_iterator cbegin() const noexcept { return {iterator{this->begin_node_m, 0}}; };
    const_iterator cend() const noexcept { return {iterator{this->end_node_m, this->end_node_m ? this->end_node_m->size_m : 0}}; };

  private:
    void IncreaseNode(const node_t* current_node) {
        node_t* another_node = CreateNode();

        AddChunckBack(current_node);

        // try {

        // } catch () {
        //     next_node
        // }
    };

    void ReduceNode(const node_t* current_node) {

    };

  private:
    size_t size_m = 0;
};


}  // labwork7

#endif // _UNROLLED_LIST_HPP_