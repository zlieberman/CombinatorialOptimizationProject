#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <numeric>
#include "utils.hpp"
#include <chrono>

using namespace std::chrono;
using namespace std;

size_list 
generate_random_instance(size_t num_samples, int max_item_size) 
{
    size_list vv(num_samples);
    //https://stackoverflow.com/questions/21516575/fill-a-vector-with-random-numbers-c
    auto gen = [&max_item_size](){return rand()%max_item_size+1;};
    generate(vv.begin(), vv.end(), gen);
    return vv;
}

size_list 
generate_constant_instance(size_t num_samples, double item_size) 
{
    size_list vv(num_samples);
    fill(vv.begin(), vv.end(), item_size);
    return vv;
}

int 
item_oriented_branch_and_bound(size_list sizes, bin_list bins, const int cap, int min_bins) 
{
    // if we've inserted every item in the list of sizes we're done
    if (sizes.empty()) {
        //print_bin_list(bins);
        int num_bins = bins.size();
        cout << "Minimum number of bins so far: " << min_bins << endl;
        cout << "Number of bins for this configuration: " << num_bins << endl;
        if (num_bins < min_bins) {
            //cout << "Minimum number of bins so far: " << bins << endl;
            min_bins = num_bins;
        }
        return min_bins;
    }
    int next_item = sizes.back();
    sizes.pop_back();
    // if all of the bins are empty
    if (bins.empty()) {
        //cout << "creating first node" << endl;
        size_list first_completion = { next_item };
        bins.push_back(first_completion);
        return item_oriented_branch_and_bound(sizes, bins, cap, min_bins);
    }
    // now we are being passed in the parent, we should use the parent to generate
    // child nodes
    for (int ii=0; ii<bins.size(); ++ii) {
        // for each completion in the completion tree, if next_item can be added
        // to it without violating the capacity constraint, create a child node
        // with next_item added to that completion
        auto completion = bins[ii];
        int cur_size = accumulate(completion.begin(),completion.end(),0);
        if (cur_size + next_item <= cap) {
            // run the exhaustive search using the new set of subsets
            bin_list child;
            for (int jj=0; jj<bins.size(); ++jj) {
                if (ii == jj) {
                    child.push_back(completion);
                    child[jj].push_back(next_item);
                } else {
                    child.push_back(bins[jj]);
                }
            }
            int bins = item_oriented_branch_and_bound(sizes, child, cap, min_bins);
            if (bins < min_bins) {
                min_bins = bins;
            }
        } 
    }
    // we can also add a child with all of the completions of the parent plus
    // an additional bin with just next_item
    size_list child = { next_item };
    bins.push_back(child);
    int num_bins = item_oriented_branch_and_bound(sizes, bins, cap, min_bins);
    if (num_bins < min_bins) {
        min_bins = num_bins;
    }
    return min_bins;
}

void 
user_prompt(instance_info& instance) 
{
    size_t num_samples;
    int max_item_size;
    cout << "Number of samples: ";
    cin >> num_samples;
    cout << "Maximum item size: ";
    cin >> max_item_size;
    cout << "Capacity of each bin: ";
    cin >> instance.bin_capacity;
    // ask user for either constant or random sizes
    while (1) {
        char instance_type[1];
        cout << "Type 'c' for constant or 'r' for random item sizes" << endl;
        cin >> instance_type;
        if (instance_type[0] == 'c') {
            instance.sizes = generate_constant_instance(num_samples, max_item_size);
            break;
        } else if (instance_type[0] == 'r') {
            instance.sizes = generate_random_instance(num_samples, max_item_size);
            break;
        } else {
            cout << "Invalid input" << endl;
        }
    }
}

// read in the bin capacity and sizes of the objects from
// a text file with appropriate formatting
void 
read_instance(instance_info& instance, string filename) 
{
    ifstream instance_file(filename);
    if (instance_file.good()) {
        string next;
        getline(instance_file, next); // skip the header line
        instance_file >> instance.bin_capacity;
        getline(instance_file, next); // skip the other two numbers
        int ss;
        while (instance_file >> ss) {
            instance.sizes.push_back(ss);
        }
    }
 }

int 
main(int argc, char* argv[]) 
{
    srand(time(0));
    instance_info instance;
    if (argc == 1) {
        user_prompt(instance);
    } else if (argc == 2) {
        read_instance(instance, argv[1]);
    } else {
        cout << "Invalid Usage" << endl;
    }

    cout << "######## Instance Info ########" << endl;
    print_instance_info(instance);

    cout << "######## Finding Solution ########" << endl;
    bin_list bins;
    //sort(instance.sizes.begin(),instance.sizes.end());
    auto start = high_resolution_clock::now();
    int opt_bins = item_oriented_branch_and_bound(instance.sizes,bins,instance.bin_capacity,instance.sizes.size());
    auto stop = high_resolution_clock::now();
    cout << "Optimal Solution: " << opt_bins << " bins" << endl;
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Converged in " << duration.count() << " microseconds" << endl;

    return 0;
}