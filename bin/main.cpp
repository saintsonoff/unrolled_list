#include <cstddef>
#include <iostream>
#include <vector>
#include <iterator>

#include <unrolled_list.hpp>

int main(int argc, char** argv) {
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

    cont.dbg_print();

    while (cont.size() > 3) {
        auto er_itr = cont.begin();
        // ++er_itr;
        // ++er_itr;
        cont.erase(er_itr);
        cont.dbg_print();
    }

    cont.dbg_print();

    return 0;
}
