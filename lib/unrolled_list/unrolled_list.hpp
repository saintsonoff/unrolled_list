#ifndef _UNROLLED_LIST_HPP_
#define _UNROLLED_LIST_HPP_

#include <algorithm>
#include <functional>
#include <iostream>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <limits>
#include <list>
#include <type_traits>
#include <initializer_list>
#include <concepts>
#include <memory>
#include <cstddef>
#include <utility>
#include <variant>
#include <memory>

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
        : chunck_ptr_m(node_ptr), chunck_offset_m(offest + 1) {  };

  public:
    Iterator& operator++() noexcept {
        ++chunck_offset_m;

        if (chunck_offset_m - 1 == chunck_ptr_m->size_m && chunck_ptr_m->next_chunck_ptr_m) {
            chunck_ptr_m = chunck_ptr_m->next_chunck_ptr_m;
            chunck_offset_m = 1;
        }
        return *this;
    };

    Iterator operator++(int) const noexcept {
        Iterator result_itr = *this;
        return ++result_itr;
    };

    Iterator& operator--() noexcept {        
        if (chunck_offset_m - 1 == 0 && chunck_ptr_m->prev_chunck_ptr_m) {
            chunck_ptr_m = chunck_ptr_m->prev_chunck_ptr_m;
            chunck_offset_m = chunck_ptr_m ? chunck_ptr_m->size_m : 1;
        }
        
        --chunck_offset_m;
        return *this;
    };

    Iterator operator--(int) const noexcept {
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
        return *(chunck_ptr_m->data_m + chunck_offset_m - 1);
    };

    const_pointer operator->() const noexcept {
        return chunck_ptr_m->data_m + chunck_offset_m - 1;
    };

    explicit operator node_t*() const noexcept { return chunck_ptr_m; };

    operator pointer() noexcept { return chunck_ptr_m->data_m + chunck_offset_m - 1; };
    operator const_pointer() const noexcept { return chunck_ptr_m->data_m + chunck_offset_m - 1; };

    size_t get_chunck_offset() const noexcept { return chunck_offset_m - 1; };

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
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::basic_const_iterator<std::reverse_iterator<const_iterator>>;

  protected:
    using chunck_traits = chunck_traits<node_t, AllocatorType>;

  public:
    using allocator_type = typename chunck_traits::allocator_type;

  protected:
    using allocator_trait_t = std::allocator_traits<allocator_type>;
    
    using data_allocator_type =  typename std::allocator_traits<AllocatorType>::template rebind_alloc<value_type>;
    using data_allocator_trait_t = std::allocator_traits<data_allocator_type>;
  
  
  public:
    unrolled_list() = default;

    template<std::convertible_to<allocator_type> AnotherAllocatorType>
    unrolled_list(AnotherAllocatorType&& alloc) : alloc_m(alloc) {  };


    unrolled_list(value_type data, size_t count = 1) {
        while (count--) {
            emplace_back(data);
        }
    };


    unrolled_list(std::initializer_list<value_type> i_list)
        : unrolled_list(i_list.begin(), i_list.end()) {  };


    template<size_t kAnotherSize>
    unrolled_list(const unrolled_list<DataType, kAnotherSize, AllocatorType>& value, const AllocatorType& alloc)
        : unrolled_list(value.begin(), value.end()) {
        alloc_m = alloc;
        data_alloc_m = alloc_m; 
    };


    template<size_t kAnotherSize>
    unrolled_list(const unrolled_list<DataType, kAnotherSize, AllocatorType>& value)
        : unrolled_list(value, value.alloc_m) {  };

    unrolled_list(const unrolled_list<DataType, ChunckSize, AllocatorType>& value, const AllocatorType& alloc)
        : unrolled_list(value.begin(), value.end()) {
        alloc_m = alloc;
        data_alloc_m = value.data_alloc_m;
    };


    unrolled_list(const unrolled_list<DataType, ChunckSize, AllocatorType>& value)
        : unrolled_list(value, value.alloc_m) {  };


    template<size_t kAnotherSize>
    unrolled_list(unrolled_list<DataType, kAnotherSize, AllocatorType>&& value, const AllocatorType& alloc)
        : alloc_m(alloc), data_alloc_m(value.data_alloc_m) {
        auto current_itr = value.begin();
        auto end_itr = value.end();

        while (current_itr != end_itr) {
            emplace_back(std::move(*current_itr));
        }
    };


    template<size_t kAnotherSize>
    unrolled_list(unrolled_list<DataType, kAnotherSize, AllocatorType>&& value)
        : unrolled_list(std::move(value), value.alloc_m) {  };


    template<size_t kAnotherSize>
    unrolled_list(unrolled_list<DataType, ChunckSize, AllocatorType>&& value, const AllocatorType& alloc) noexcept
            : begin_chunck_ptr_m(value.begin_chunck_ptr_m), end_chunck_ptr_m(value.end_chunck_ptr_m), size_m(value.size_m),
            alloc_m(alloc), data_alloc_m(std::move(alloc_m)) {
        value.begin_chunck_ptr_m = value.end_chunck_ptr_m = nullptr;
        value.size_m = 0;
    };


    unrolled_list(unrolled_list<DataType, ChunckSize, AllocatorType>&& value, const AllocatorType& alloc)
        noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
            : begin_chunck_ptr_m(value.begin_chunck_ptr_m), end_chunck_ptr_m(value.end_chunck_ptr_m), size_m(value.size_m),
            alloc_m(alloc_m), data_alloc_m(std::move(alloc_m)) {
        value.begin_chunck_ptr_m = value.end_chunck_ptr_m = nullptr;
        value.size_m = 0;
    };


    unrolled_list(unrolled_list<DataType, ChunckSize, AllocatorType>&& value)
        noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
            : unrolled_list(std::move(value), std::move(value.alloc_m)) {  };


    template<std::input_iterator InItrType>
    unrolled_list(InItrType beg, InItrType end) : unrolled_list(beg, end, {}) { };


    template<std::input_iterator InItrType>
    unrolled_list(InItrType beg, InItrType end, const AllocatorType& alloc) : alloc_m{alloc} {
        auto current_itr = beg;

        try {
            while (current_itr != end) {
                emplace_back(*current_itr);
                ++current_itr;
            }
        } catch(...) {
            clear();
            throw;
        }
    };


    virtual ~unrolled_list() noexcept(noexcept(clear())) { clear(); };


    unrolled_list& operator=(const unrolled_list& value) {
        if (*this == value) {
            return *this;
        }

        unrolled_list current_copy;

        auto current_itr = value.begin();
        auto end_itr = value.end();

        while (current_itr != end_itr) {
            current_copy.emplace_back(*current_itr);
        }

        clear();

        current_copy.alloc_m = value.alloc_m;
        current_copy.data_alloc_m = value.data_alloc_m;
        *this = std::move(current_copy);

        return *this;
    };


    unrolled_list& operator=(unrolled_list&& value) noexcept(noexcept(clear())) {
        if (this == &value) {
            return *this;
        }

        clear();

        std::swap(*this, value);

        return *this;
    };

  private:

    template<std::same_as<allocator_type> SameAllocatorType>
    unrolled_list(node_t* data, SameAllocatorType alloc)
      : alloc_m(std::forward<SameAllocatorType>(alloc)),
        begin_chunck_ptr_m(data), end_chunck_ptr_m(data), size_m(data->size_m) {  };

  public:

    template<typename... ArgsTs>
    requires std::constructible_from<value_type, ArgsTs...>
    void emplace_back(ArgsTs&&... args) {
        if (empty()) {
            begin_chunck_ptr_m = end_chunck_ptr_m = chunck_traits::CreateChunck(alloc_m);
        }

        if (end_chunck_ptr_m->size_m == end_chunck_ptr_m->size_value) {
            end_chunck_ptr_m = chunck_traits::AddChunckBack(end_chunck_ptr_m, alloc_m);
        }

        data_allocator_trait_t::construct(data_alloc_m, end_chunck_ptr_m->data_m + end_chunck_ptr_m->size_m, std::forward<ArgsTs>(args)...);
        ++(end_chunck_ptr_m->size_m);
        ++size_m;
    };


    template<typename... ArgsTs>
    requires std::constructible_from<value_type, ArgsTs...>
    void emplace_front(ArgsTs&&... args) {
        if (empty()) {
            begin_chunck_ptr_m = end_chunck_ptr_m = chunck_traits::CreateChunck(alloc_m);
        }

        if (begin_chunck_ptr_m->size_m == begin_chunck_ptr_m->size_value) {
            begin_chunck_ptr_m = chunck_traits::AddChunckFront(begin_chunck_ptr_m, alloc_m);
        }

        ChunckPlace(begin_chunck_ptr_m, 0, std::forward<ArgsTs>(args)...);
        ++size_m;
    };


    template<typename... ArgsTs>
    requires std::constructible_from<value_type, ArgsTs...>
    iterator emplace(const_iterator pos_itr, ArgsTs&&... args) {
        node_t* emplace_node = static_cast<node_t*>(pos_itr.base());
        size_type position = pos_itr.base().get_chunck_offset();

        iterator return_itr;

        if (emplace_node->size_m == emplace_node->size_value) {
            node_t pos_copy = *emplace_node;

            emplace_node = SplitNode(&pos_copy, emplace_node == end_chunck_ptr_m);

            if (position < pos_copy.size_value - (pos_copy.size_value / 2)) {
                ChunckPlace(&pos_copy, position, std::forward<ArgsTs>(args)...);
                return_itr = {static_cast<node_t*>(pos_itr.base()), position};
            } else {
                // position -= pos_copy.size_value / 2;
                position -= pos_copy.size_value - (pos_copy.size_value / 2);

                ChunckPlace(emplace_node, position, std::forward<ArgsTs>(args)...);
                return_itr = {emplace_node, position};
            }

            std::swap(*static_cast<node_t*>(pos_itr.base()), pos_copy);
            chunck_traits::IncludeChunckBack(static_cast<node_t*>(pos_itr.base()), emplace_node);


            if (static_cast<node_t*>(pos_itr.base()) == end_chunck_ptr_m) {
                end_chunck_ptr_m = end_chunck_ptr_m->next_chunck_ptr_m;
            }
        } else {
            ChunckPlace(emplace_node, position, std::forward<ArgsTs>(args)...);
            return_itr = pos_itr;
        }

        ++size_m;
        return return_itr;
    };


    template<std::convertible_to<value_type> SameType>
    void push_back(SameType&& value) {
        emplace_back(std::forward<SameType>(value));
    };


    template<std::convertible_to<value_type> SameType>
    void push_front(SameType&& value) {
        emplace_front(std::forward<SameType>(value));
    };


    template<std::convertible_to<value_type> SameType>
    iterator insert(const_iterator pos_itr, SameType&& value) {
        return emplace(pos_itr, std::forward<SameType>(value));
    }


    iterator insert(const_iterator pos_itr, size_type count, const value_type& value) {
        unrolled_list values_cont(value, count);
        return insert(values_cont.begin(), values_cont.end());
    }


    template<std::input_iterator InItrType>
    iterator insert(const_iterator pos_itr, InItrType beg_itr, InItrType end_itr) {
        node_t* pos_node{pos_itr.base()};
        size_t offset = pos_itr.base().get_chunck_offset();

        auto addition_node_deleter = [&alloc = this->alloc_m](node_t* node) mutable {  
            allocator_trait_t::destroy(alloc, node);
            chunck_traits::RemoveChunck(node, alloc);
        };

        std::unique_ptr<node_t, decltype(addition_node_deleter)> addition_pos_itr_node(
            chunck_traits::CreateChunck(alloc_m), addition_node_deleter);

        allocator_trait_t::construct(alloc_m, addition_pos_itr_node.get(), *pos_node);

        iterator addition_pos_itr{addition_pos_itr_node.get(), offset};
        addition_pos_itr_node->next_chunck_ptr_m = addition_pos_itr_node->prev_chunck_ptr_m = nullptr;

        unrolled_list addition_list{addition_pos_itr_node.get(), alloc_m};

        size_t addition_size = 0;
        std::for_each(beg_itr, end_itr, [&](auto&& value) {
            addition_pos_itr = ++addition_list.emplace(addition_pos_itr, std::forward<decltype(value)>(value));
            ++addition_size;
        });

        if (addition_list.begin_chunck_ptr_m->next_chunck_ptr_m != nullptr) {
            chunck_traits::IncludeChunckBack(pos_node, 
                addition_list.begin_chunck_ptr_m->next_chunck_ptr_m, addition_list.end_chunck_ptr_m);

            addition_list.begin_chunck_ptr_m->prev_chunck_ptr_m = pos_node->prev_chunck_ptr_m;
            addition_list.begin_chunck_ptr_m->next_chunck_ptr_m = pos_node->next_chunck_ptr_m;

            if (pos_node == end_chunck_ptr_m) {
                end_chunck_ptr_m = addition_list.end_chunck_ptr_m;
            }

            std::swap(*pos_node, *(addition_list.begin_chunck_ptr_m));
        } else {
            std::swap(*pos_node, *(addition_list.begin_chunck_ptr_m));
        }

        size_m += addition_size;

        addition_list.begin_chunck_ptr_m = addition_list.end_chunck_ptr_m = nullptr;
        addition_list.size_m = 0;

        return pos_itr.base();
    }


  public:

    void pop_back() noexcept(std::is_nothrow_destructible_v<value_type>) {
        pointer destroyed_ptr = end_chunck_ptr_m->data_m + end_chunck_ptr_m->size_m;

        if (end_chunck_ptr_m->size_m == 1) {
            if (end_chunck_ptr_m == begin_chunck_ptr_m) {
                chunck_traits::RemoveChunck(end_chunck_ptr_m, alloc_m);
                end_chunck_ptr_m = nullptr;
            } else {
                end_chunck_ptr_m = end_chunck_ptr_m->prev_chunck_ptr_m;
                data_allocator_trait_t::destroy(data_alloc_m, destroyed_ptr);
                chunck_traits::RemoveChunckBack(end_chunck_ptr_m, alloc_m);
            }

        } else {
            --(end_chunck_ptr_m->size_m);
            data_allocator_trait_t::destroy(data_alloc_m, destroyed_ptr);
        }


        if (!end_chunck_ptr_m) {
            begin_chunck_ptr_m = nullptr;
        }
        --size_m;
    }

    void pop_front() {
        pointer destroyed_ptr = begin_chunck_ptr_m->data_m;

        if (begin_chunck_ptr_m->size_m == 1) {
            if (begin_chunck_ptr_m == end_chunck_ptr_m) {
                chunck_traits::RemoveChunck(begin_chunck_ptr_m, alloc_m);
                begin_chunck_ptr_m = nullptr;
            } else {
                begin_chunck_ptr_m = begin_chunck_ptr_m->next_chunck_ptr_m;
                data_allocator_trait_t::destroy(data_alloc_m, destroyed_ptr);
                chunck_traits::RemoveChunckFront(begin_chunck_ptr_m, alloc_m);
            }
        } else {
            data_allocator_trait_t::destroy(data_alloc_m, begin_chunck_ptr_m->data_m + 0);
            shift_left(begin_chunck_ptr_m->data_m + 1, begin_chunck_ptr_m->data_m + begin_chunck_ptr_m->size_m, 1);
            --(begin_chunck_ptr_m->size_m);
            data_allocator_trait_t::destroy(data_alloc_m, destroyed_ptr);
        }


        if (!begin_chunck_ptr_m) {
            end_chunck_ptr_m = nullptr;
        }
        --size_m;
    };

    iterator erase(const_iterator pos_itr) {
        node_t* current_node = static_cast<node_t*>(pos_itr.base());
        size_t current_offset = pos_itr.base().get_chunck_offset();

        auto addition_node_deleter = [&alloc = this->alloc_m](node_t* node) mutable {  
            allocator_trait_t::destroy(alloc, node);
            chunck_traits::RemoveChunck(node, alloc);
        };

        std::unique_ptr<node_t, decltype(addition_node_deleter)> copy_lifetime_manager(nullptr, addition_node_deleter);
        if constexpr (!std::is_nothrow_move_constructible_v<value_type>) {
            copy_lifetime_manager = chunck_traits::CreateChunck(alloc_m);
            current_node = copy_lifetime_manager.get();
            allocator_trait_t::construct(alloc_m, current_node, static_cast<node_t*>(pos_itr.base()));
        }

        data_allocator_trait_t::destroy(data_alloc_m, current_node->data_m + current_offset);
        shift_left(current_node->data_m + current_offset, current_node->data_m + current_node->size_m, 1);

        --current_node->size_m;
        size_t half_size = (current_node->size_value % 2) + (current_node->size_value / 2);
        if (current_node->size_m < half_size) {
            node_t* credit_node = nullptr;
            if (current_node->next_chunck_ptr_m && current_node->next_chunck_ptr_m->size_m > half_size) {
                credit_node = current_node->next_chunck_ptr_m;
            } else if (current_node->prev_chunck_ptr_m && current_node->prev_chunck_ptr_m->size_m > half_size) {
                credit_node = current_node->prev_chunck_ptr_m;
            }

            if (credit_node) {
                allocator_trait_t::construct(alloc_m, current_node + current_node->size_m,
                    std::move(*(credit_node->data_m + credit_node->size_m)));

                allocator_trait_t::destroy(alloc_m, credit_node->data_m + credit_node->size_m);
                --(credit_node->size_m);
                --(credit_node->size_m);
            } else {
                if (current_node->next_chunck_ptr_m && current_node->next_chunck_ptr_m->size_m <= half_size) {
                    MergeNode(current_node->next_chunck_ptr_m, current_node);
                    chunck_traits::RemoveChunckBack(current_node);
                } else if (current_node->prev_chunck_ptr_m && current_node->prev_chunck_ptr_m->size_m <= half_size) {
                    MergeNode(current_node->prev_chunck_ptr_m, current_node);
                    chunck_traits::RemoveChunckFront(current_node);
                }
            }
        }


        if constexpr (!std::is_nothrow_move_constructible_v<value_type>) {
            std::swap(current_node, copy_lifetime_manager.get());

            if (current_node->next_chunck_ptr_m)
                current_node->next_chunck_ptr_m->prev_chunck_ptr_m = current_node;

            if (current_node->prev_chunck_ptr_m)
                current_node->prev_chunck_ptr_m->next_chunck_ptr_m = current_node;
        }

        return {current_node, pos_itr};
    };

    iterator erase(const_iterator beg_pos_itr, const_iterator end_pos_itr) {

    };

  public:
    iterator begin() { 
        if (begin_chunck_ptr_m)
            return iterator{begin_chunck_ptr_m, 0};
        return iterator{nullptr, 0};
    };


    const_iterator begin() const { 
        if (begin_chunck_ptr_m)
            return iterator{begin_chunck_ptr_m, 0};
        return iterator{nullptr, 0};
    };


    const_iterator cbegin() const {
        if (begin_chunck_ptr_m)
            return iterator{begin_chunck_ptr_m, 0};
        return iterator{nullptr, 0};
    };


    iterator end() {
        if (end_chunck_ptr_m)
            return iterator{end_chunck_ptr_m, end_chunck_ptr_m->size_m};
        return iterator{nullptr, 0};
    };


    const_iterator end() const {
        if (end_chunck_ptr_m)
            return iterator{end_chunck_ptr_m, end_chunck_ptr_m->size_m};
        return iterator{nullptr, 0};
    };


    const_iterator cend() const {
        if (end_chunck_ptr_m)
            return iterator{end_chunck_ptr_m, end_chunck_ptr_m->size_m};
        return iterator{nullptr, 0};
    };


    reverse_iterator rbegin() {
        return reverse_iterator{end()};
    };


    const_reverse_iterator rbegin() const { 
        return {const_iterator{end()}};
    };


    const_reverse_iterator crbegin() const {
        return {const_iterator{end()}};
    };


    reverse_iterator rend() {
        return reverse_iterator{begin()};
    };


    const_reverse_iterator rend() const {
        return {const_iterator{begin()}};
    };


    const_reverse_iterator crend() const {
        return {const_iterator{begin()}};
    };


  public:
    void clear()
        noexcept(noexcept(chunck_traits::RemoveChunck(static_cast<node_t*>(nullptr),
            static_cast<node_t*>(nullptr), std::declval<allocator_type&>())) &&
            std::is_nothrow_destructible_v<value_type>) {
        auto current_itr = cbegin();    
        auto end_itr = cend();
        while (current_itr != end_itr) {
            data_allocator_trait_t::destroy(data_alloc_m, static_cast<const_pointer>(current_itr));
            ++current_itr;
        }

        if (!empty())
            chunck_traits::RemoveChunck(begin_chunck_ptr_m, end_chunck_ptr_m, alloc_m);
    }


  public:
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); };
    size_type size() const noexcept { return size_m; };
    bool empty() const noexcept { return size_m == 0; };
    allocator_type get_allocator() const { return alloc_m; };

  public:
    reference front() { return *begin(); };  
    reference back() { return *(--end()); };  

    const_reference front() const { return *begin(); };  
    const_reference back() const { return *(--end()); };  


  public:
    bool operator==(const unrolled_list& value) const noexcept {
        if (size() != value.size()) {
            return false;
        }

        auto current_itr = begin();
        auto value_itr = value.begin();
        auto current_itr_end = end();

        while (current_itr != current_itr_end) {
            if (*current_itr != *value_itr) {
                return false;
            }
            ++current_itr;
            ++value_itr;
        }

        return true;
    };


    bool operator!=(const unrolled_list& value) const noexcept {
        return !(*this == value);
    };


  private:
    node_t* SplitNode(node_t* current_node, bool is_end)
      noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_destructible_v<value_type>) {
        node_t* another_node = chunck_traits::CreateChunck(alloc_m);
        
        node_t* splited_node_ptr = current_node;

        auto addition_node_deleter = [&alloc = this->alloc_m](node_t* node) mutable {  
            allocator_trait_t::destroy(alloc, node);
            chunck_traits::RemoveChunck(node, alloc);
        };
        std::unique_ptr<node_t, decltype(addition_node_deleter)> sprited_node_smart_ptr(nullptr, addition_node_deleter);
        if constexpr(!std::is_nothrow_move_constructible_v<value_type>) {
    
            sprited_node_smart_ptr.reset(chunck_traits::CreateChunck(alloc_m));

            splited_node_ptr = sprited_node_smart_ptr.get();
            allocator_trait_t::construct(alloc_m, splited_node_ptr, *current_node);
        }

        int balance = is_end ? 0 : splited_node_ptr->size_m % 2;
        for (size_t offset = 0; offset != balance + splited_node_ptr->size_m / 2; ++offset) {
            data_allocator_trait_t::construct(data_alloc_m, another_node->data_m + (balance + splited_node_ptr->size_m / 2) - offset - 1,
                std::move(*(splited_node_ptr->data_m + splited_node_ptr->size_m - offset - 1)));
        }

        for (size_t offset = 0; offset != balance + splited_node_ptr->size_m / 2; ++offset) {
            data_allocator_trait_t::destroy(data_alloc_m, splited_node_ptr->data_m + splited_node_ptr->size_m - offset - 1);
        }

        if constexpr(!std::is_nothrow_move_constructible_v<value_type>) {
            std::swap(*current_node, *splited_node_ptr);
            splited_node_ptr = nullptr;
        }

        another_node->size_m = balance + current_node->size_m / 2;
        current_node->size_m -= another_node->size_m;

        return another_node;
    };


    void MergeNode(node_t* from_node, node_t* to_node)
      noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_destructible_v<value_type>) {       
        node_t* from_node_ptr = from_node;
        node_t* to_node_ptr = to_node;

        auto addition_node_deleter = [&alloc = this->alloc_m](node_t* node) mutable {  
            allocator_trait_t::destroy(alloc, node);
            chunck_traits::RemoveChunck(node, alloc);
        };

        std::unique_ptr<node_t*, decltype(addition_node_deleter)> copy_livetime_controlled[2] = {{nullptr, addition_node_deleter}, {nullptr, addition_node_deleter}};

        if constexpr(std::is_nothrow_move_constructible_v<value_type>) {
            copy_livetime_controlled.reset({chunck_traits::CreateChunck(alloc_m), chunck_traits::CreateChunck(alloc_m)});

            from_node_ptr = copy_livetime_controlled[0].get();
            to_node_ptr = copy_livetime_controlled[1].get();

            allocator_trait_t::construct(alloc_m, from_node_ptr, *from_node);
            allocator_trait_t::construct(alloc_m, to_node_ptr, *to_node);
        }

        for (size_t offset = 0; offset != from_node_ptr->size_m; ++offset) {
            data_allocator_trait_t::construct(data_alloc_m, to_node_ptr->data_m + to_node_ptr->size_m + offset,
                std::move(*(from_node_ptr->data_m + offset)));
        }

        for (size_t offset = 0; offset != from_node_ptr->size_m; ++offset) {
            data_allocator_trait_t::destroy(data_alloc_m, from_node_ptr->data_m + offset);
        }

        if constexpr(std::is_nothrow_move_constructible_v<value_type>) {
            std::swap(from_node_ptr, copy_livetime_controlled[0].get());
            std::swap(to_node_ptr, copy_livetime_controlled[1].get());

            if (from_node_ptr->next_chunck_ptr_m)
                from_node_ptr->next_chunck_ptr_m->prev_chunck_ptr_m = from_node_ptr;

            if (from_node_ptr->prev_chunck_ptr_m)
                from_node_ptr->prev_chunck_ptr_m->next_chunck_ptr_m = from_node_ptr;

            if (to_node_ptr->next_chunck_ptr_m)
                to_node_ptr->next_chunck_ptr_m->prev_chunck_ptr_m = to_node_ptr;

            if (to_node_ptr->prev_chunck_ptr_m)
                to_node_ptr->prev_chunck_ptr_m->next_chunck_ptr_m = to_node_ptr;
        }

        to_node_ptr->size_m += from_node_ptr->size_m; 
        to_node_ptr->size_m = 0;

        return to_node_ptr;
    };
    

    template<typename... ArgsTs>
    requires std::constructible_from<value_type, ArgsTs...>
    void ChunckPlace(node_t* current_chunck, size_t position, ArgsTs&&... args) {
        if (current_chunck->size_m) {
            shift_right(current_chunck->data_m + position, current_chunck->data_m + current_chunck->size_m, 1);
        }

        try {
            data_allocator_trait_t::construct(data_alloc_m, current_chunck->data_m + position, std::forward<ArgsTs>(args)...);
        } catch(...) {
            if (current_chunck->size_m) {
                shift_left(current_chunck->data_m + position + 1, current_chunck->data_m + current_chunck->size_m + 1, 1);
            }    

            throw;
        }

        ++(current_chunck->size_m);
    }


    void shift_right(pointer from, pointer to, size_t shift) {
        auto current = to;

        while (current != from) {
            --current;
            data_allocator_trait_t::construct(data_alloc_m, current + shift, std::move(*current));
            data_allocator_trait_t::destroy(data_alloc_m, current);
        }
    }


    void shift_left(pointer from, pointer to, size_t shift) {
        auto current = from;

        while (current != to) {
            data_allocator_trait_t::construct(data_alloc_m, current - shift, std::move(*current));
            data_allocator_trait_t::destroy(data_alloc_m, current);
            ++current;
        }
    }

  public:
    void dbg_print() const {
        std::cout << "== == == == == debug print == == == == ==\n";
        size_t count = 0;
        auto itr = cbegin();
        auto end_itr = cend();

        if (end_itr.base().get_chunck_offset() == 0) {
            std::cout << "end_itr is nullptr" << "\n";
        }
        
        std::cout << "size: " << size() << "\n";
        while (itr != end_itr) {
            ++count;
            // std::cout << "*" << " ";
            if (!itr.base().get_chunck_offset()) {
                std::cout << "| ";
            }
            std::cout << *itr << " ";
            ++itr;
            if (count > size() + 4)
                break;
        }
        std::cout << "\n";
        std::cout << "count: " << count << "\n";
        std::cout << "== == == == == == end == == == == == ==\n\n";
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


template<std::copy_constructible DataType, size_t ChunckSize = 10, typename AllocatorType = std::allocator<DataType>>
using unrolled_list = labwork7::unrolled_list<DataType, ChunckSize, AllocatorType>;


#endif // _UNROLLED_LIST_HPP_