#ifndef _UNROLLED_LIST_STORAGE_HPP_
#define _UNROLLED_LIST_STORAGE_HPP_

#include <cstddef>
#include <memory>
#include <type_traits>

namespace labwork7 {

template<typename DataType, size_t kSize>
class RawArrayStorage {
  public:
    operator DataType*() {
        return reinterpret_cast<DataType*>(raw_data_arr);
    }
  private:
    alignas(DataType) std::byte raw_data_arr[sizeof(DataType) * kSize];
};


template<typename DataType, size_t kSize>
struct UnrolledListNodeChunck {
  public:
    using store_t = RawArrayStorage<DataType, kSize>;

  public:
    static constexpr size_t size_value = kSize;

  public:
    size_t size_m = 0;
    UnrolledListNodeChunck* prev_m = nullptr;
    UnrolledListNodeChunck *next_m = nullptr;
    store_t data_m;
};


template<typename node_t, typename AllocatorType = std::allocator<node_t>>
class chunck_traits {
  public:
    using allocator_type = typename std::allocator_traits<AllocatorType>::template rebind_alloc<node_t>;
    using allocator_trait_t = std::allocator_traits<allocator_type>;
    using node_ptr_t = node_t*;

  public:
    static node_ptr_t CreateChunck(allocator_type& alloc) {
        node_t* chunck_ptr = allocator_trait_t::allocate(alloc, 1);
        allocator_trait_t::construct(alloc, chunck_ptr);
        return chunck_ptr;
    };


    static node_ptr_t AddChunckBack(node_ptr_t current_chunck, allocator_type& alloc) {
        return IncludeChunckFront(current_chunck, CreateChunck(alloc));
    };


    static node_ptr_t AddChunkFront(node_ptr_t current_chunck, allocator_type& alloc) {
        return IncludeChunckFront(current_chunck, CreateChunck(alloc));
    };


    static node_ptr_t IncludeChunckFront(node_ptr_t current_chunck, node_ptr_t included_chunck) noexcept {
        node_ptr_t next = current_chunck;
        node_ptr_t prev = nullptr;

        if (current_chunck && current_chunck->prev_chunck_ptr_m) {
            next = current_chunck->prev_chunck_ptr_m;
        }

        included_chunck->next_chunck_ptr_m = next;
        included_chunck->prev_chunck_ptr_m = prev;

        if (current_chunck) {
            current_chunck->prev_chunck_ptr_m = included_chunck;
        }

        return included_chunck;
    };

    static node_ptr_t IncludeChunckBack(node_ptr_t current_chunck, node_ptr_t included_chunck) noexcept {
        node_ptr_t next = nullptr;
        node_ptr_t prev = current_chunck;

        if (current_chunck && current_chunck->next_chunck_ptr_m) {
            next = current_chunck->next_chunck_ptr_m;
        }

        included_chunck->next_chunck_ptr_m = next;
        included_chunck->prev_chunck_ptr_m = prev;

        if (current_chunck) {
            current_chunck->next_chunck_ptr_m = included_chunck;
        }

        return included_chunck;
    };


    static void RemoveChunck(node_ptr_t& current_chunck, allocator_type& alloc)
      noexcept(std::is_nothrow_destructible_v<node_t>) {
        allocator_trait_t::destroy(alloc, current_chunck);
        allocator_trait_t::deallocate(alloc, current_chunck);
        current_chunck = nullptr;
    };


    static node_ptr_t& ExcludeChunck(node_ptr_t& current_chunck) noexcept {
        if (current_chunck->next_chunck_ptr_m)
            current_chunck->next_chunck_ptr_m->prev_chunck_ptr_m = current_chunck->prev_chunck_ptr_m;

        if (current_chunck->prev_chunck_ptr_m)
            current_chunck->prev_chunck_ptr_m->next_chunck_ptr_m = current_chunck->next_chunck_ptr_m;

        return current_chunck;
    };


    static node_ptr_t& ExcludeChunckBack(node_ptr_t& current_chunck) noexcept {
        return ExcludeChunck(current_chunck->next_chunck_ptr_m);
    };


    static node_ptr_t& ExcludeChunckFront(node_ptr_t& current_chunck) noexcept {
        return ExcludeChunck(current_chunck->prev_chunck_ptr_m);
    };


    void RemoveChunckBack(node_ptr_t& current_chunck, allocator_type& alloc) 
      noexcept(noexcept(RemoveChunck(current_chunck, alloc))) {
        RemoveChunck(ExcludeChunckBack(current_chunck), alloc);
    };


    void RemoveChunckFront(node_ptr_t& current_chunck, allocator_type& alloc) 
      noexcept(noexcept(RemoveChunck(current_chunck, alloc))) {
        RemoveChunck(ExcludeChunckFront(current_chunck), alloc);
    };
};


} // namespace labwork7

#endif // _UNROLLED_LIST_STORAGE_HPP_