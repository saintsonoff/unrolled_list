#ifndef _CHUNCK_ALLOCATOR_HPP_
#define _CHUNCK_ALLOCATOR_HPP_

#include <array>
#include <list>
#include <cstddef>
#include <stdexcept>
#include <set>
#include <type_traits>
#include <memory>
#include <unistd.h>

namespace labwork7 {

namespace chunck_allocator {

namespace details {

    template<typename Type>
    struct size_of_type {
        static constexpr size_t value = sizeof(Type);
    };
    
    template<typename Type>
    constexpr size_t size_of_type_v = size_of_type<Type>::value;    

    template<typename Type, size_t kMinSize>
    struct is_large_obj {
        static constexpr bool value = size_of_type_v<Type> < kMinSize;
    };
        
    template<typename Type, size_t kMinSize>
    static constexpr bool is_large_obj_v = is_large_obj<Type, kMinSize>::value;
    

template<typename DataType>
class RawStorage {
  public:
    operator DataType*() & {
        return reinterpret_cast<DataType*>(raw_data_arr);
    }
  private:
    alignas(DataType) std::byte raw_data_arr[sizeof(DataType)];
};

template<typename DataType, typename ChunckType>
struct ChunckData {
    RawStorage<DataType> data;
    ChunckType* current_chunck = nullptr;
};

template<typename DataType, size_t kMaxChunckSize>
struct ChunckAllocatorNode {
    using data_t = ChunckData<DataType, ChunckAllocatorNode>;
    
    ChunckAllocatorNode() {
        for (auto& elem : data) {
            elem.current_chunck = this;
        }
    };

    std::array<data_t, kMaxChunckSize> data;
    ChunckAllocatorNode* next = nullptr, *prev = nullptr;
    size_t size = 0;
};

template<typename ChunckType>
struct chunck_traits {
using pointer = ChunckType*;

static pointer exclude(pointer current) {
    if (current->next) {
        current->next->prev = current->prev;
    }

    if (current->prev) {
        current->prev->next = current->next;
    }
    
    return current;
}

static pointer include(pointer current, pointer next, pointer prev) {
    if (next) {
        current->next = next;
        next->prev = current;
    }

    if (prev) {
        current->prev = prev;
        prev->next = current;
    }
    
    return current;
}

static pointer link(pointer prev, pointer next) {
    if (prev) {
        prev->next = next;
    }

    if (next) {
        next->prev = prev;
    }

    if (next) {
        return next;
    }

    return prev;    
}

static ChunckType::data_t* get_data_ptr(pointer current) {
    if (current)
        return &(current->data[current->size]);
    return nullptr; 
};

};

template<typename, typename = void>
struct has_pointer_subtype : std::false_type { using type = void; };

template<typename T>
struct has_pointer_subtype<T, std::void_t<typename T::pointer>> : std::true_type { using type = T::pointer; };

template<typename T>
constexpr bool has_pointer_subtype_v = has_pointer_subtype<T>::value;

template<typename T>
using has_pointer_subtype_t = has_pointer_subtype<T>::type;


template<typename, typename = void>
struct has_const_pointer_subtype : std::false_type { using type = void; };

template<typename T>
struct has_const_pointer_subtype<T, std::void_t<typename T::const_pointer>> : std::true_type { using type = T::const_pointer; };

template<typename T>
constexpr bool has_const_pointer_subtype_v = has_pointer_subtype<T>::value;

template<typename T>
using has_const_pointer_subtype_t = has_const_pointer_subtype<T>::type;


template<typename, typename = void>
struct has_difference_type_subtype : std::false_type { using type = void; };

template<typename T>
struct has_difference_type_subtype<T, std::void_t<typename T::difference_type>> : std::true_type { using type = T::difference_type; };

template<typename T>
constexpr bool has_difference_type_subtype_v = has_difference_type_subtype<T>::value;

template<typename T>
using has_difference_type_subtype_t = has_difference_type_subtype<T>::type;

} // namespace details


template<typename DataType, size_t kMaxChunckSize = 10, typename SuballocatorType = std::allocator<DataType>>
class ChunckAllocator : public std::allocator_traits<SuballocatorType>::
                            template rebind_alloc<typename details::ChunckAllocatorNode<DataType, kMaxChunckSize>>,
                        public std::allocator_traits<SuballocatorType>::
                            template rebind_alloc<DataType> {
  private:
    using allocator_t = std::allocator_traits<SuballocatorType>::template rebind_alloc<DataType>;
    using allocator_traits_t = std::allocator_traits<allocator_t>;

    using base_allocator_t = std::allocator_traits<SuballocatorType>::
        template rebind_alloc<typename details::ChunckAllocatorNode<DataType, kMaxChunckSize>>;

    using base_allocator_traits_t = std::allocator_traits<base_allocator_t>;

  public:
    using value_type = typename allocator_t::value_type;
    using pointer = std::conditional_t<
        details::has_pointer_subtype_v<allocator_t>,
        details::has_pointer_subtype_t<allocator_t>,
    value_type*>;

    using const_pointer = std::conditional_t<
        details::has_pointer_subtype_v<allocator_t>,
        details::has_pointer_subtype_t<allocator_t>,
        const value_type*>;

    using size_type = typename allocator_t::size_type;

    using difference_type = std::conditional_t<
        details::has_difference_type_subtype_v<allocator_t>,
        details::has_difference_type_subtype_t<allocator_t>,
    std::ptrdiff_t>;

    template<typename AnotherDataType>
    struct rebind {
        using other = ChunckAllocator<AnotherDataType, kMaxChunckSize, SuballocatorType>;
    };

  private:
    using node_t = details::ChunckAllocatorNode<value_type, kMaxChunckSize>;
    using node_data_t = details::ChunckAllocatorNode<value_type, kMaxChunckSize>::data_t;
    using chunck_traits = details::chunck_traits<node_t>; 

  public:
    ChunckAllocator() noexcept {  };

    ChunckAllocator(const ChunckAllocator& value) noexcept : ChunckAllocator() {  };
    
    template<typename AnotherDataType>
    ChunckAllocator(const ChunckAllocator<AnotherDataType, kMaxChunckSize,
        typename allocator_traits_t::template rebind_alloc<AnotherDataType>>& value) noexcept : ChunckAllocator() {  };

    ChunckAllocator(ChunckAllocator&& value)
        : b_chunck_m(value.b_chunck_m), e_chunck_m(value.e_chunck_m),
            reserve_cont_m(std::move(value.reserve_cont_m)) {
            b_chunck_m = e_chunck_m = nullptr;
            reserve_cont_m.clear();
    };


    template<typename AnotherDataType>
    ChunckAllocator(ChunckAllocator<AnotherDataType, kMaxChunckSize,
        typename allocator_traits_t::template rebind_alloc<AnotherDataType>>&& value) noexcept : ChunckAllocator() {  };


    ChunckAllocator& operator=(const ChunckAllocator& value) noexcept {
        if (this == &value) {
            return *this;
        }

        clear();
        std::swap(*this, {});
        return *this;
    }

    
    ChunckAllocator& operator=(ChunckAllocator&& value) noexcept {
        if (this == &value)
            return *this;

        std::swap(b_chunck_m, value.b_chunck_m);
        std::swap(e_chunck_m, value.b_chunck_m);
        std::swap(reserve_cont_m, value.reserve_cont_m);

        return *this;
    };

    ~ChunckAllocator() {
        clear();
    }

  public:

    pointer allocate(size_type size) {
        if (size != 1) {
            throw std::logic_error{"You can allocate only single object"};
        }

        node_data_t* ptr = nullptr;
        if (reserve_cont_m.empty()) {
            node_t* current_chunck = nullptr;
            if (e_chunck_m == nullptr) {
                current_chunck = b_chunck_m = e_chunck_m = base_allocator_traits_t::allocate(*this, 1);
                base_allocator_traits_t::construct(*this, current_chunck);

            } else if (e_chunck_m->size == kMaxChunckSize) {
                current_chunck = base_allocator_traits_t::allocate(*this, 1);
                base_allocator_traits_t::construct(*this, current_chunck);

                chunck_traits::link(e_chunck_m, current_chunck);
                e_chunck_m = current_chunck;
            } else {
                current_chunck = e_chunck_m;
            }

            ptr = chunck_traits::get_data_ptr(current_chunck);
            ++(current_chunck->size);
        } else {
            pointer data_ptr = (*(reserve_cont_m.begin()))->data;
            reserve_cont_m.erase(reserve_cont_m.begin());
            return data_ptr;
        }

        pointer data_ptr = ptr->data;
        return data_ptr;
    };

    void deallocate(pointer ptr, size_type size) {
        if (size > kMaxChunckSize) {
            throw std::logic_error{"You can deallocate only single object"};
        }

        if (size != 1) {

        }

        node_data_t* current_data_ptr = reinterpret_cast<node_data_t*>(ptr);
        node_t* current_node = current_data_ptr->current_chunck;

        if (current_node->size == 1) {
            if (b_chunck_m == e_chunck_m) {
                b_chunck_m = e_chunck_m = nullptr;
            }

            if (current_node == b_chunck_m) {
                b_chunck_m = current_node->next;
            }        

            if (current_node == e_chunck_m) {
                e_chunck_m = current_node->prev;
            }

            current_node = chunck_traits::exclude(current_node);

            for (size_t ind = 0, end = current_node->size; ind < end; ++ind) {
                node_data_t* current_ptr = &(current_node->data[ind]);
                reserve_cont_m.erase(current_ptr);
            }

            base_allocator_traits_t::destroy(*this, current_node);
            base_allocator_traits_t::deallocate(*this, current_node, 1);

            return;
        }

        reserve_cont_m.insert(current_data_ptr);
    };

    void destroy(pointer ptr) {
        allocator_traits_t::destroy(*this, ptr);
    }

    template<typename... ArgsTs>
    void construct(pointer ptr, ArgsTs&&... args) {
        allocator_traits_t::construct(*this, ptr, std::forward<ArgsTs>(args)...);
    }

  private:
    void clear() noexcept {
        while (b_chunck_m != e_chunck_m) {
            b_chunck_m = b_chunck_m->next;
            base_allocator_traits_t::destroy(*this, b_chunck_m->prev);
            base_allocator_traits_t::deallocate(*this, b_chunck_m->prev, 1);
        }

        if (b_chunck_m) {
            base_allocator_traits_t::destroy(*this, b_chunck_m);
            base_allocator_traits_t::deallocate(*this, b_chunck_m, 1);
        }
        b_chunck_m = e_chunck_m = nullptr;
    }

  public:
    bool operator==(const ChunckAllocator& value) const noexcept { return this == &value; };
    bool operator!=(const ChunckAllocator& value) const noexcept { return !(*this == value); };
    
  private:
    node_t* b_chunck_m = nullptr;
    node_t* e_chunck_m = nullptr;
  
  private:
    std::set<node_data_t*> reserve_cont_m;
};


} // namespace chunck_allocator

template<typename DataType, size_t kSize = 10, typename AllocatorType = std::allocator<DataType>>
using list = std::list<DataType, chunck_allocator::ChunckAllocator<DataType, kSize, AllocatorType>>;

} // namespace labwork7

template<typename DataType, size_t kSize = 10, typename AllocatorType = std::allocator<DataType>>
using unrolled_list = std::list<DataType, labwork7::chunck_allocator::ChunckAllocator<DataType, kSize, AllocatorType>>;

#endif // _CHUNCK_ALLOCATOR_HPP_