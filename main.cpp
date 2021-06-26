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
    for (size_t ii=0; ii<bins.size(); ++ii) {
        // for each completion in the completion tree, if next_item can be added
        // to it without violating the capacity constraint, create a child node
        // with next_item added to that completion
        auto completion = bins[ii];
        double cur_size = accumulate(completion.begin(),completion.end(),0);
        if (cur_size + next_item <= cap) {
            // run the exhaustive search using the new set of subsets
            bin_list child;
            for (size_t jj=0; jj<bins.size(); ++jj) {
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

bin_list
best_fit(size_list sizes, bin_list bins, const double cap)
{
    int num_bins = 1;
    // add the first item to the first bin
    bins.push_back({sizes[0]});
    // current size of each bin
    vector<Bin> bin_sizes;
    bin_sizes.push_back({sizes[0],0});
    // add items to the first bin they fit in
    for (size_t ii=1;ii<=sizes.size();++ii) {
        for (int jj=num_bins-1;jj>=0;--jj) {
            if (bin_sizes[jj].size + sizes[ii] <= cap) { // check if the item fits
                bins[bin_sizes[jj].id].push_back(sizes[ii]);
                bin_sizes[jj].size+=sizes[ii];
                break;
            } else if (jj == 0) { // item didn't fit in any bin
                bins.push_back({sizes[ii]});
                bin_sizes.push_back({sizes[ii],num_bins});
                ++num_bins;
                break;
            }
        }
        // check the most full bins first
        sort(bin_sizes.begin(),bin_sizes.end());
    }
    return bins;
}

double closest_tabu(size_list &vec, double value, size_list tabu_list) {
    sort(vec.begin(),vec.end());
    // find an item greater than the capacity violation that is not
    // in the tabu list
    for (auto it=vec.begin();it!=vec.end();++it) {
        if (*it > value && find(tabu_list.begin(),tabu_list.end(),*it)==tabu_list.end()) {
            auto out = *it;
            vec.erase(it);
            return out;
        }
    }
    // one could not be found so now search for one that is just not in the tabu list
    for (auto it=vec.begin();it!=vec.end();++it) {
        if (find(tabu_list.begin(),tabu_list.end(),*it)==tabu_list.end()) {
            auto out = *it;
            vec.erase(it);
            return out;
        }
    }
    // if we got to this point there are no items that can be swapped out from this bin
    return -1;
}

double closest(size_list &vec, double value) {
    sort(vec.begin(),vec.end());
    auto it = lower_bound(vec.begin(),vec.end(),value);
    if (it == vec.end()) {
        double out = vec.back();
        vec.pop_back();
        return out;
    }
    double out = *it;
    vec.erase(it);
    return out;
}

int 
steepest_descent(size_list sizes, const double cap)
{
    // invoke function to generate initial solution
    bin_list bins = best_fit(sizes, {}, cap);
    int num_bins = bins.size();
    int num_sizes = sizes.size();
    cout << "Greedy Solution: " << num_bins << " bins" << endl;
    if (num_bins <= 1) {
        return num_bins;
    }
    --num_bins;
    // ordered list of bin sizes
    vector<Bin> bin_sizes;
    bins[num_bins-1].insert(bins[num_bins-1].end(),bins[num_bins].begin(),bins[num_bins].end());
    bins.erase(bins.begin()+num_bins);
    for (int ii=0;ii<num_bins;++ii) {
        Bin temp = {(double)accumulate(bins[ii].begin(),bins[ii].end(),0), ii};
        bin_sizes.push_back(temp);
    }
    sort(bin_sizes.begin(),bin_sizes.end());
    // iteratively unpack 1 bin of a feasible solution, redistribute
    // the unpacked items and then search for a feasible
    // solution by iteratively decreasing the capacity violations on bins
    // by swapping the items in different bins
    int max_iters = num_sizes*20;
    while (num_bins > 0) {
        int iters = 0;
        if (accumulate(sizes.begin(),sizes.end(),0) > cap*(num_bins)) {
            return num_bins+1;
        }
        while (bin_sizes[num_bins-1].size > cap) {
            //print_bin_list(bins);
            // first check if a solution with num_bins is even possible
            if (iters > max_iters) {
                return num_bins+1;
            } else {
                // do 1-0 swaps from max to min violation bins
                // keep track of items removed from each bin so as not to add them back
                double violation = bin_sizes[num_bins-1].size - cap;
                auto item = closest(bins[bin_sizes[num_bins-1].id],violation);
                bins[bin_sizes[0].id].push_back({item});
                bin_sizes[0].size+=item;
                bin_sizes[num_bins-1].size-=item;
                // double violation_max = (violation - item)*(item < violation);
                // double violation_min = bin_sizes[0].size - cap*(cap < bin_sizes[0].size);
                // double violation_change = violation_max + violation_min - violation;
                // cout << "violation change: " << violation_change << endl;
                sort(bin_sizes.begin(),bin_sizes.end());
                ++iters;
            }
        }
        int idx = bin_sizes[num_bins-1].id;
        int idx2 = bin_sizes[num_bins-2].id;
        bins[idx2].insert(bins[idx2].end(),bins[idx].begin(),bins[idx].end());
        bins.erase(bins.begin()+idx);
        bin_sizes[num_bins-2].size = (double)accumulate(bins[num_bins-2].begin(),bins[num_bins-2].end(),0);
        bin_sizes.erase(bin_sizes.begin()+num_bins-1);
        sort(bin_sizes.begin(),bin_sizes.end());
        num_bins--;
    }
    return 1;
}

int 
tabu_search(size_list sizes, const double cap)
{
    // invoke function to generate initial solution
    bin_list bins = first_fit_decreasing(sizes, {}, cap);
    int num_bins = bins.size();
    cout << "Greedy Solution: " << num_bins << " bins" << endl;
    if (num_bins <= 1) {
        return num_bins;
    }
    --num_bins;
    // ordered list of bin sizes
    vector<Bin> bin_sizes;
    // unpack the last bin and put all those items in the second to last bin
    bins[num_bins-1].insert(bins[num_bins-1].end(),bins[num_bins].begin(),bins[num_bins].end());
    bins.erase(bins.begin()+num_bins);
    // keep track of items removed from each bin so as not to add them back
    bin_list tabu_list;
    for (int ii=0;ii<num_bins;++ii) {
        Bin temp = {(double)accumulate(bins[ii].begin(),bins[ii].end(),0), ii};
        bin_sizes.push_back(temp);
        tabu_list.push_back({});
    }
    sort(bin_sizes.begin(),bin_sizes.end());
    // iteratively unpack 1 bin of a feasible solution, redistribute
    // the unpacked items and then search for a feasible
    // solution by iteratively decreasing the capacity violations on bins
    // by swapping items from the max to minimum bin
    int max_iters = num_bins*num_bins*num_bins;
    while (num_bins > 0) {
        int iters = 0;
        if (accumulate(sizes.begin(),sizes.end(),0) > cap*(num_bins)) {
            return num_bins+1;
        }
        while (bin_sizes[num_bins-1].size > cap) {
            // first check if a solution with num_bins is even possible
            if (iters > max_iters) {
                return num_bins+1;
            } else {
                // do 1-0 swaps from max to min violation bins
                double violation = bin_sizes[num_bins-1].size - cap;
                auto item = closest_tabu(bins[bin_sizes[num_bins-1].id],violation,tabu_list[bin_sizes[0].id]);
                if (item == -1 && num_bins > 2) {
                    // 1-0 swap the biggest item from this bin into the the second biggest bin
                    item = bins[bin_sizes[num_bins-1].id].back();
                    bin_sizes[num_bins-2].size+=item;
                    bins[bin_sizes[num_bins-2].id].push_back(item);
                    bin_sizes[num_bins-1].size-=item;
                    bins[bin_sizes[num_bins-1].id].pop_back();
                    tabu_list[bin_sizes[num_bins-1].id].push_back({item});
                } else {
                    tabu_list[bin_sizes[num_bins-1].id].push_back({item});
                    bins[bin_sizes[0].id].push_back({item});
                    bin_sizes[0].size+=item;
                    bin_sizes[num_bins-1].size-=item;
                }
                // double violation_max = (violation - item)*(item < violation);
                // double violation_min = bin_sizes[0].size - cap*(cap < bin_sizes[0].size);
                // double violation_change = violation_max + violation_min - violation;
                // cout << "violation change: " << violation_change << endl;
                sort(bin_sizes.begin(),bin_sizes.end());
                ++iters;
            }
        }
        int idx = bin_sizes[num_bins-1].id;
        int idx2 = bin_sizes[num_bins-2].id;
        bins[idx2].insert(bins[idx2].end(),bins[idx].begin(),bins[idx].end());
        bins.erase(bins.begin()+idx);
        bin_sizes[num_bins-2].size = (double)accumulate(bins[num_bins-2].begin(),bins[num_bins-2].end(),0);
        bin_sizes.erase(bin_sizes.begin()+num_bins-1);
        sort(bin_sizes.begin(),bin_sizes.end());
        --num_bins;
        //print_bin_list(bins);
    }
    return 1;
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
    auto start = chrono::high_resolution_clock::now();
    //int opt_bins = item_oriented_branch_and_bound(instance.sizes,bins,instance.bin_capacity,instance.sizes.size());
    //int greedy_bins = first_fit_decreasing(instance.sizes,bin:xs,instance.bin_capacity);
    int steepest_descent_bins = steepest_descent(instance.sizes,instance.bin_capacity);
    int tabu_search_bins = tabu_search(instance.sizes,instance.bin_capacity);
    auto stop = chrono::high_resolution_clock::now();
    //cout << "Optimal Solution: " << opt_bins << " bins" << endl;
    //cout << "Greedy Solution: " << greedy_bins << " bins" << endl;
    cout << "Steepest Descent Solution: " << steepest_descent_bins << " bins" << endl;
    cout << "Tabu Search: " << tabu_search_bins << " bins" << endl;
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "Converged in " << duration.count() << " microseconds" << endl;

    return 0;
}