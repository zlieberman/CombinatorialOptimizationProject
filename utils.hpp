#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

typedef std::vector<double> size_list;
typedef std::vector<size_list> bin_list;

typedef struct instance_info {
    size_list sizes;
    long bin_capacity;
} instance_info;

void 
print_instance_info(instance_info instance) 
{    
    std::cout << "Number of samples: " << instance.sizes.size() << std::endl;
    std::cout << "Capacity of each bin: " << instance.bin_capacity << std::endl;
    
    // print out the items
    std::cout << "Item sizes: ";
    for (auto item : instance.sizes) {
        std::cout << item << ",";
    }
    std::cout << std::endl;
}

void 
print_bin_list(bin_list bins) 
{
    for (auto objects : bins) {
        std::cout << "{ ";
        for (auto object : objects) {
            std::cout << object << " ";
        }
        std::cout << "}" << std::endl;
    }
}

#endif