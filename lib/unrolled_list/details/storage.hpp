#ifndef _UNROLLED_LIST_STORAGE_HPP_
#define _UNROLLED_LIST_STORAGE_HPP_

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <memory>
#include <exception>
#include <iterator>

namespace labwork7 {


namespace details {


template<typename DataType, size_t ChunckSize>
struct NodeChunck {
  public:
    NodeChunck* prev_chunck_ptr_m = nullptr;
    NodeChunck* next_chunck_ptr_m = nullptr;
  public:
    size_t size_m = 0;
    DataType* data_m = nullptr;
};

template<std::copy_constructible DataType, typename ChunckAllocatorType, typename ListAllocatorType = ChunckAllocatorType, size_t ChunckSize = 10>
class unrolled_list_storage {
  public:
    using node_t = NodeChunck<DataType, ChunckSize>;
    using chunck_alloc_t = ChunckAllocatorType;
    using list_alloc_t = typename std::allocator_traits<ListAllocatorType>::template rebind_alloc<node_t>;
    using chunck_allocator_trait_t = std::allocator_traits<chunck_alloc_t>;
    using list_allocator_trait_t = std::allocator_traits<list_alloc_t>;

  public:
    using allocator_type = list_alloc_t;

  public:
    unrolled_list_storage() noexcept = default;

    template<size_t AnotherChunkSize>
    unrolled_list_storage(const unrolled_list_storage<DataType, ChunckAllocatorType, ListAllocatorType, AnotherChunkSize>& value)
        : begin_node_m(nullptr), end_node_m(nullptr), size_m(0),
            chunck_alloc_m(value.chunck_alloc_m), list_alloc_m(value.list_alloc_m) {
        node_t* begin_node = nullptr, end_node = nullptr;
        size_t current_size = value.size_m * AnotherChunkSize / ChunckSize;

        if (current_size == 0)
            return;

        size_t chunck_count = 1;
        begin_node = end_node = CreateChunck();

        while (chunck_count != current_size) {
            end_node = AddChunkBack(end_node);
            ++chunck_count;
        }

        size_m = current_size;
        begin_node_m = begin_node;
        end_node_m = end_node;
    };

    unrolled_list_storage(unrolled_list_storage&& value) noexcept
        : begin_node_m(value.begin_node_m), end_node_m(value.end_node_m), size_m(value.size_m),
            chunck_alloc_m(value.chunck_alloc_m), list_alloc_m(value.list_alloc_m) { value = {}; };

    template<size_t AnotherChunkSize>
    unrolled_list_storage& operator=(const unrolled_list_storage<DataType, ChunckAllocatorType, ListAllocatorType, AnotherChunkSize>& value) {
        if (this == &value) {
            return;
        }

        unrolled_list_storage current_chunck_size_storage = value;
        
        std::swap(*this, current_chunck_size_storage);
    };

    unrolled_list_storage& operator=(unrolled_list_storage&& value) {
        if (this == &value) {
            return;
        }
        
        std::swap(*this, value);
    };


    ~unrolled_list_storage() { 
        Clear();
    };

  public:
    node_t* LinkChunckBack(node_t* current_chunck, node_t* another_chunck) noexcept {
        if (!current_chunck) {
            if (another_chunck) {
                another_chunck->prev_chunck_ptr_m = current_chunck;
            }
            return another_chunck;
        }

        if (!another_chunck) {
            if (current_chunck) {
                current_chunck->next_chunck_ptr_m = another_chunck;
            }
            return another_chunck;
        }

        if (current_chunck->next_chunck_ptr_m) {
            another_chunck->next_chunck_ptr_m = current_chunck->next_chunck_ptr_m;
            
            current_chunck->next_chunck_ptr_m->prev_chunck_ptr_m = another_chunck;
        }

        current_chunck->next_chunck_ptr_m = another_chunck;
        another_chunck->prev_chunck_ptr_m = current_chunck;

        return another_chunck;
    }


    node_t* LinkChunckFront(node_t* current_chunck, node_t* another_chunck) noexcept {
        if (!current_chunck) {
            if (another_chunck) {
                another_chunck->next_chunck_ptr_m = current_chunck;
            }
            return another_chunck;
        }

        if (!another_chunck) {
            if (current_chunck) {
                current_chunck->prev_chunck_ptr_m = another_chunck;
            }
            return another_chunck;
        }


        if (current_chunck->prev_chunck_ptr_m) {
            another_chunck->prev_chunck_ptr_m = current_chunck->prev_chunck_ptr_m;
            
            current_chunck->prev_chunck_ptr_m->next_chunck_ptr_m = another_chunck;
        }

        current_chunck->prev_chunck_ptr_m = another_chunck;
        another_chunck->next_chunck_ptr_m = current_chunck;

        return another_chunck;
    }


    node_t* AddChunkBack(node_t* current_chunck) const {
        node_t* next = nullptr;
        node_t* prev = current_chunck;

        if (current_chunck && current_chunck->next_chunck_ptr_m) {
            next = current_chunck->next_chunck_ptr_m;
        }
        
        node_t* new_node = CreateChunck();

        new_node->next_chunck_ptr_m = next;
        new_node->prev_chunck_ptr_m = prev;

        if (current_chunck) {
            current_chunck->next_chunck_ptr_m = new_node;
        }

        return new_node;
    };


    node_t* AddChunkFront(node_t* current_chunck) const {
        node_t* next = current_chunck;
        node_t* prev = nullptr;

        if (current_chunck && current_chunck->prev_chunck_ptr_m) {
            next = current_chunck->prev_chunck_ptr_m;
        }
        
        node_t* new_node = CreateChunck();

        new_node->next_chunck_ptr_m = next;
        new_node->prev_chunck_ptr_m = prev;

        if (current_chunck) {
            current_chunck->prev_chunck_ptr_m = new_node;
        }

        return new_node;
    };


    node_t* CreateChunck() const {
        node_t* new_node = list_allocator_trait_t::allocate(list_alloc_m, 1);

        list_allocator_trait_t::construct(list_alloc_m, new_node);

        try {
            new_node->data_m = chunck_allocator_trait_t::allocate(list_alloc_m, ChunckSize);
        } catch (const std::bad_alloc&) {
            list_allocator_trait_t::destroy(list_alloc_m, new_node);
            list_allocator_trait_t::deallocate(list_alloc_m, new_node);
            throw;
        }

        return new_node;
    };


    void RemoveChunkBack(node_t* current_chunck) noexcept {        
        node_t* next = nullptr;
        if (current_chunck->next_chunck_ptr_m) {
            next = current_chunck->next_chunck_ptr_m->next_chunck_ptr_m;
            if (next) {
                next->prev_chunck_ptr_m = current_chunck;
            }
        }

        RemoveChunck(current_chunck->next_chunck_ptr_m);

        current_chunck->next_chunck_ptr_m = next;
    };

    void RemoveChunkFront(node_t* current_chunck) noexcept {
        
        node_t* prev = nullptr;
        if (current_chunck->prev_chunck_ptr_m) {
            prev = current_chunck->prev_chunck_ptr_m->prev_chunck_ptr_m;

            if (prev) {
                prev->next_chunck_ptr_m = current_chunck;
            }
        }

        RemoveChunck(current_chunck->prev_chunck_ptr_m);

        current_chunck->prev_chunck_ptr_m = prev;
    };

    void RemoveChunck(node_t* current_chunck) noexcept {
        // for (size_t offset = 0; offset < current_chunck->size_m; ++offset) {
        //     chunck_allocator_trait_t::destroy(chunck_alloc_m, current_chunck->data_m + offset);
        // }

        chunck_allocator_trait_t::deallocate(chunck_alloc_m, current_chunck->data_m, ChunckSize);

        list_allocator_trait_t::destroy(list_alloc_m, current_chunck);
        list_allocator_trait_t::deallocate(list_alloc_m, current_chunck, 1);
    };

    void Clear() noexcept {
        while (begin_node_m->next_chunck_ptr_m != end_node_m) {
            RemoveChunkBack(begin_node_m);
        }

        RemoveChunck(begin_node_m);
        RemoveChunck(end_node_m);

        begin_node_m = end_node_m = nullptr;
        size_m = 0;
    };

  public:
    bool empty() const noexcept { return size_m == 0; };

  protected:
    node_t* begin_node_m = nullptr;
    node_t* end_node_m = nullptr;

  private:
    size_t size_m = 0;

  protected:
#if defined(_WIN32) || defined(_WIN64) 
    [[msvc::no_unique_address]] chunck_alloc_t chunck_alloc_m;
    [[msvc::no_unique_address]] list_alloc_t list_alloc_m;
#else
    [[no_unique_address]] chunck_alloc_t chunck_alloc_m;
    [[no_unique_address]] list_alloc_t list_alloc_m;
#endif
};


} // namespace details


} // namespace labwork7

#endif // _UNROLLED_LIST_STORAGE_HPP_