#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

typedef struct instance_info {
    std::vector<double> sizes;
    int bin_capacity;
    int max_item_size;
} instance_info;

void 
print_instance_info(instance_info instance) 
{    
    std::cout << "Number of samples: " << instance.sizes.size() << std::endl;
    std::cout << "Maximum item size: " << instance.max_item_size << std::endl;
    std::cout << "Capacity of each bin: " << instance.bin_capacity << std::endl;
    
    // print out the items
    std::cout << "Item sizes: ";
    for (auto item : instance.sizes) {
        std::cout << item;
    }
    std::cout << std::endl;
}

#endif