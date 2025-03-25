#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <iterator>

#include <unrolled_list.hpp>
// #include <chunck_allocator.hpp>

int main(int argc, char** argv) {
#if 1
    auto print_list = [](auto& cont) {
        for (auto elem : cont) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    };

    auto itr_print_list = [](auto& cont) {
        auto it = cont.begin();
        auto end_it = cont.end();

        size_t count = 0;
        while (it != end_it) {
            std::cout << *it << " ";
            ++it;
            ++count;
        }
        std::cout << std::endl 
            << "count: " << count << "\n"
            << "size: " << cont.size() <<
            std::endl;
    };


    labwork7::unrolled_list<int>::iterator asffas{};

    labwork7::unrolled_list<int, 5> cont;
    cont.emplace_back(-9); 
    cont.dbg_print();

    cont.emplace_front(-8);
    cont.dbg_print();

    cont.emplace_front(-8);
    cont.dbg_print();

    cont.emplace(cont.begin(), -6);
    cont.dbg_print();

    cont.emplace(cont.begin(), -7);
    cont.dbg_print();

    cont.emplace_front(-8);
    cont.dbg_print();

    cont.emplace(cont.begin(), -6);
    cont.dbg_print();

    cont.emplace(cont.begin(), -7);
    cont.dbg_print();

    cont.emplace_front(-8);
    cont.dbg_print();

    cont.emplace(cont.begin(), -6);
    cont.dbg_print();

    cont.emplace(cont.begin(), -7);
    cont.dbg_print();

    cont.emplace_front(-8);
    cont.dbg_print();

    cont.emplace(cont.begin(), -6);
    cont.dbg_print();

    cont.emplace(cont.begin(), -7);    
    cont.dbg_print();
    
    // int arr[] = {1, 2, 3};
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
    size_t arr_size = sizeof(arr) / sizeof(arr[0]);


    auto itr = cont.end();
    --itr;
    --itr;
    --itr;
    --itr;
    --itr;
    --itr;

    itr = cont.insert(itr, arr, arr + arr_size);

    cont.dbg_print();

    cont.pop_back();
    cont.dbg_print();

    cont.pop_back();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    cont.pop_front();
    cont.dbg_print();

    decltype(cont) cont2 = cont;
    cont2.dbg_print();


    std::cout << "\n\n/////////////////////////////////////////////////////\n\n";

    decltype(cont) er_rang_cont;

    for (int elem = 0; elem < 1000; ++elem) {
        er_rang_cont.push_back(elem);
    }

    er_rang_cont.dbg_print();

    while (cont.size() > 3) {
        auto er_itr = cont.begin();
        // ++er_itr;
        // ++er_itr;
        cont.erase(er_itr);
        cont.dbg_print();
    }

    auto er_beg_itr = er_rang_cont.begin();
    auto er_end_itr = er_rang_cont.end();


    std::cout << "\n\n/////////////////////////////////////////////////////\n\n";
    er_rang_cont.dbg_print();


    er_beg_itr++;
    er_beg_itr++;
    er_beg_itr++;
    er_beg_itr++;
    er_beg_itr++;

    --er_end_itr;
    --er_end_itr;
    --er_end_itr;
    --er_end_itr;
    --er_end_itr;

    er_rang_cont.erase(er_beg_itr, er_end_itr);
    er_rang_cont.dbg_print();

    er_rang_cont.erase(er_rang_cont.begin(), er_rang_cont.end());
    er_rang_cont.dbg_print();
#endif


#if 0
    using alloc_t = labwork7::chunck_allocator::ChunckAllocator<int>;

    alloc_t alloc;

    using node_t = labwork7::chunck_allocator::details::ChunckAllocatorNode<int, 10>;
    using data_t = node_t::data_t;


    int* ptr;

    ptr = alloc.allocate(1);
    alloc.construct(ptr, 30);

    std::cout << *ptr << std::endl;

    alloc.destroy(ptr);
    alloc.deallocate(ptr, 1);

    std::string s;
    std::cin >> s;
    std::cout << s << std::endl;


    std::cout << typeid(alloc_t::rebind<float>::other).name() << std::endl;

    using other_alloc_t = alloc_t::rebind<float>::other;

    other_alloc_t other_alloc;

    float* other_ptr;

    other_ptr = other_alloc.allocate(1);
    other_alloc.construct(other_ptr, 30.89);

    std::cout << *other_ptr << std::endl;

    other_alloc.destroy(other_ptr);
    other_alloc.deallocate(other_ptr, 1);
#endif


#if 0
    labwork7::list<int> lst;

    std::vector<labwork7::list<int>::iterator> itr_cont;
    for (size_t i = 0; i < 100; ++i) {
        lst.push_back(i);
        itr_cont.push_back(--lst.end());
    }

    size_t offens = 3;
    for (size_t ind = 0; ind != itr_cont.size() / offens; ++ind) {
        std::cout << *itr_cont[ind * offens] << " ";
        lst.erase(itr_cont[ind * offens]);
    }
    std::cout << "\ncount: " << itr_cont.size() / 4 << "\n";
    std::cout << std::endl << "--------------------------------------------" << std::endl;

    for (size_t i = 0; i < 200; ++i) {
        lst.push_back(i);
        itr_cont.push_back(--lst.end());
    }

    for (auto& elem : lst) {
        std::cout << elem << " ";
    }

    for (size_t ind = 0; ind != itr_cont.size() / offens; ++ind) {
        lst.erase(itr_cont[ind * offens]);
    }


    std::cout << "\n-----------------------------------------------\n" << std::endl;

    for (auto& elem : lst) {
        std::cout << elem << " ";
    }


    labwork7::list<int> lst_cpy = lst;

    lst_cpy.push_back(999);
    lst_cpy.push_back(999);
    lst_cpy.push_back(999);
    lst_cpy.push_back(999);
    lst_cpy.push_back(999);
    lst_cpy.erase(lst_cpy.begin());
    lst_cpy.erase(lst_cpy.begin());
    lst_cpy.erase(lst_cpy.begin());
    lst_cpy.erase(lst_cpy.begin());
    lst_cpy.push_back(999);
    lst_cpy.push_back(999);

    std::cout << "\n-----------------------------------------------\n" << std::endl;

    for (auto& elem : lst_cpy) {
        std::cout << elem << " ";
    }
    


    std::cout << std::endl;
#endif

    return 0;
}
