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
    double next_item = sizes.back();
    sizes.pop_back();
    // now we are being passed in the parent, we should use the parent to generate
    // child nodes
    for (int ii=0; ii<bins.size(); ++ii) {
        // for each completion in the completion tree, if next_item can be added
        // to it without violating the capacity constraint, create a child node
        // with next_item added to that completion
        auto completion = bins[ii];
        double cur_size = accumulate(completion.begin(),completion.end(),0);
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

bin_list
first_fit_decreasing(size_list sizes, bin_list bins, const double cap) 
{
    // sort the list of sizes
    sort(sizes.begin(),sizes.end());
    int num_bins = 1;
    int ii=sizes.size()-1;
    // add the largest item to the first bin
    bins.push_back({sizes[ii]});
    size_list bin_sizes({sizes[ii]});
    --ii;
    // add items to the first bin they fit in
    for (;ii>=0;--ii) {
        for (int jj=0;jj<num_bins;++jj) {
            if (bin_sizes[jj] + sizes[ii] <= cap) { // check if the item fits
                bins[jj].push_back(sizes[ii]);
                bin_sizes[jj]+=sizes[ii];
                break;
            } else if (jj == num_bins-1) { // item didn't fit in any bin
                bins.push_back({sizes[ii]});
                bin_sizes.push_back({sizes[ii]});
                ++num_bins;
                break;
            }
        }
    }
    return bins;
}

int
best_fit(size_list sizes, bin_list bins, const double cap)
{
    int num_bins = 1;
    // add the first item to the first bin
    bins.push_back({sizes[0]});
    // heap of current size of each bin
    size_list bin_sizes({sizes[0]});
    // add items to the first bin they fit in
    for (int ii=0;ii<=sizes.size();--ii) {
        for (int jj=num_bins;jj>=0;--jj) {
            if (bin_sizes[jj] + sizes[ii] <= cap) { // check if the item fits
                bins[jj].push_back(sizes[ii]);
                bin_sizes[jj]+=sizes[ii];
                break;
            } else if (jj == 0) { // item didn't fit in any bin
                bins.push_back({sizes[ii]});
                bin_sizes.push_back({sizes[ii]});
                ++num_bins;
                break;
            }
        }
        // check the most full bins first
        sort(bin_sizes.begin(),bin_sizes.end());
    }
    return num_bins;
}

int 
steepest_descent(size_list sizes, const double cap)
{
    // invoke function to generate initial solution
    bin_list bins = first_fit_decreasing(sizes, {}, cap);
    int num_bins = bins.size();
    cout << "Greedy Solution: " << num_bins << " bins" << endl;
    // ordered list of waste (positive if over capacity and negative if under)
    size_list bin_sizes;
    for (auto bin : bins) {
        bin_sizes.push_back(accumulate(bin.begin(),bin.end(),0));
    }

    // iteratively unpack 1 bin of a feasible solution, redistribute
    // the unpacked items and then search for a feasible
    // solution by iteratively decreasing the capacity violations on bins
    // by swapping the items in different bins
    while (*max_element(bin_sizes.begin(),bin_sizes.end()) > cap) {
        // first check if a solution with num_bins-1 bins is even possible
        if (accumulate(sizes.begin(),sizes.end(),0) > cap*(num_bins-1)) {
            return num_bins;
        } else { // search for a feasible solutions with num_bins-1 bins
            // do 1-0 swaps from max to min violation bins
            // find two bins where swapping an item to another
            // bin would reduce the total capacity violation of 
            // item distribution
            for (int ii=0;ii<num_bins;++ii) {
                if (bin_sizes[ii] > cap) {
                    double violation = cap - bin_sizes[ii];
                    // see if an item in this bin can be moved to another bin without
                    // increasing the overall capacity violation
                    for (int jj=0;jj<num_bins;++jj) {
                        if (ii != jj && bin_sizes[jj] < cap) {
                            auto item = closest(bins[ii],violation);
                            bins[jj].push_back({item});
                            bin_sizes[jj]+=item;
                            bin_sizes[ii]-=item;
                        }
                    }
                }
                if (!bin_sizes[ii]) {
                    num_bins--;
                }
            }
        }
    }
    
}

double closest(size_list vec, double value) {
    auto const it = std::lower_bound(vec.begin(), vec.end(), value);
    if (it == vec.end()) { 
        return -1; 
    }
    vec.erase(it);
    return *it;
}

struct HeapBin {
    double capacity;
    double size;
    double violation;
    int id;
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

void
manual_instance(instance_info& instance) {
    cout << "Enter a bin capacity: ";
    cin >> instance.bin_capacity; 
    double item;
    cout << "Type each item size followed by a newline" << endl;
    while (cin >> item) {
        instance.sizes.push_back(item);
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
        double ss;
        while (instance_file >> ss) {
            instance.sizes.push_back(ss);
        }
    }
    random_shuffle(instance.sizes.begin(), instance.sizes.end());
 }

int 
main(int argc, char* argv[]) 
{
    srand(time(0));
    instance_info instance;
    if (argc == 1) {
        //user_prompt(instance);
        manual_instance(instance);
    } else if (argc == 2) {
        read_instance(instance, argv[1]);
    } else {
        cout << "Invalid Usage" << endl;
    }

    cout << "######## Instance Info ########" << endl;
    print_instance_info(instance);

    cout << "######## Finding Solution ########" << endl;
    bin_list bins;
    auto start = high_resolution_clock::now();
    //int opt_bins = item_oriented_branch_and_bound(instance.sizes,bins,instance.bin_capacity,instance.sizes.size());
    //int greedy_bins = first_fit_decreasing(instance.sizes,bin:xs,instance.bin_capacity);
    int steepest_descent_bins = steepest_descent(instance.sizes,instance.bin_capacity);
    auto stop = high_resolution_clock::now();
    //cout << "Optimal Solution: " << opt_bins << " bins" << endl;
    //cout << "Greedy Solution: " << greedy_bins << " bins" << endl;
    cout << "Steepest Descent Solution: " << steepest_descent_bins << " bins" << endl;
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Converged in " << duration.count() << " microseconds" << endl;

    return 0;
}