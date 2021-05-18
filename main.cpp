#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <numeric>
#include "utils.hpp"

using namespace std;

vector<int> 
generate_random_instance(size_t num_samples, int max_item_size) 
{
    vector<int> vv(num_samples);
    //https://stackoverflow.com/questions/21516575/fill-a-vector-with-random-numbers-c
    auto gen = [&max_item_size](){return rand()%max_item_size+1;};
    generate(vv.begin(), vv.end(), gen);
    return vv;
}

vector<int> 
generate_constant_instance(size_t num_samples, double item_size) 
{
    vector<int> vv(num_samples);
    fill(vv.begin(), vv.end(), item_size);
    return vv;
}

// get the bin that the object at sample_idx is going to be place in
long
get_idx(long value, int num_bins, int sample_idx) 
{
    // cout << "value: " << value << endl;
    // cout << "num_bins: " << num_bins << endl;
    // cout << "sample_idx: " << sample_idx << endl;
    // long exp = (long)pow(num_bins, sample_idx);
    // cout << exp << endl;
    // cout << value/exp << endl;
    return (long)(value / (long)pow(num_bins, sample_idx)) % num_bins;
}

long
get_start_combination(const int num_bins) {
    double start_idx = 0;
    for (int coef=1; coef<num_bins; ++coef) {
        start_idx += (num_bins - coef) * pow(num_bins,coef-1);
    }
    return (long)start_idx;
}

int 
item_oriented_branch_and_bound(vector<int> sizes, completion_tree comp_tree, const int cap, int min_bins) 
{
    // base case
    if (sizes.empty()) {
        /*cout << "base case reached" << endl;
        for (auto completion : comp_tree.completions) {
            cout << "{ ";
            for (auto item : completion) {
                cout << item << " ";
            }
            cout << "}" << endl;
        }
        */
        min_bins = comp_tree.completions.size();
        //cout << "Feasible Solution with " << min_bins << " bins" << endl;
        return min_bins;
    }

    int next_item = sizes.back();
    sizes.pop_back();

    // if the tree is empty add the first item as its only completion
    if (comp_tree.completions.empty()) {
        //cout << "creating first node" << endl;
        vector<int> first_completion = { next_item };
        comp_tree.completions.push_back(first_completion);
        return item_oriented_branch_and_bound(sizes, comp_tree, cap, min_bins);
    }

    // now we are being passed in the parent, we should use the parent to generate
    // child nodes
    for (int ii=0; ii<comp_tree.completions.size(); ++ii) {
        // for each completion in the completion tree, if next_item can be added
        // to it without violating the capacity constraint, create a child node
        // with next_item added to that completion
        auto completion = comp_tree.completions[ii];
        int cur_size = accumulate(completion.begin(),completion.end(),0);
        if (cur_size + next_item <= cap) {
            //cout << "creating child" << endl;
            // make a new child node in the tree
            completion_tree child;
            for (int jj=0; jj<comp_tree.completions.size(); ++jj) {
                if (ii == jj) {
                    child.completions.push_back(completion);
                    child.completions[jj].push_back(next_item);
                } else {
                    child.completions.push_back(comp_tree.completions[jj]);
                }
            }
            comp_tree.children.push_back(child);
            int bins = item_oriented_branch_and_bound(sizes, child, cap, min_bins);
            if (bins < min_bins) {
                min_bins = bins;
                cout << "Minimum number of bins so far: " << min_bins << endl;
            }
        } 
    }
    // we can also add a child with all of the completions of the parent plus
    // an additional bin with just next_item
    completion_tree child;
    child.completions = comp_tree.completions;
    vector<int> next_completion = { next_item };
    child.completions.push_back(next_completion);
    comp_tree.children.push_back(child);
    int bins = item_oriented_branch_and_bound(sizes, child, cap, min_bins);
    if (bins < min_bins) {
        min_bins = bins;
        cout << "Minimum number of bins so far: " << min_bins << endl;
    }
    //cout << "reached end" << endl;
    return min_bins;
}

// solve an instance of the bin packing problem given the sizes
// of the objects, the number of bins, and the capacity of each bin
bool 
solve_exhaustive(const vector<double> sizes, const int num_bins, const int cap) 
{
    int num_samples = sizes.size();
    double num_combinations = pow(num_bins, num_samples);
    // certificate to store the bin of each sample
    //vector<int> bin_idxs;
    cout << "Number of possible combinations: " << num_combinations << endl;
    cout << "Number of bins: " << num_bins << endl;
    long start = get_start_combination(num_bins);
    // iterate over all possible combinations
    for (long ii=start; ii<num_combinations; ++ii) {
        // always reinitialize the capacity of the bins
        // before trying a new combination
        int bins[num_bins];
        fill_n(bins, num_bins, cap);
        vector<int> bin_idxs;
        // put the balls in the appropriate bin for this combination
        for (long jj=0; jj<num_samples; ++jj) {
            int bin_idx = get_idx(ii, num_bins, jj);
            cout << bin_idx << ",";
            // add bin of each sample to the certificate
            bin_idxs.push_back(bin_idx);
            if (bins[bin_idx] >= sizes[jj]) {
                bins[bin_idx] -= sizes[jj];
            } else {
                break;
            }
            // if this loop runs through every sample we know we have
            // reached a valid solution
            if (jj == num_samples-1) {
                return true;
            }
        }
        break;
    }
    return false;
}

int 
exhaustive_optimal(const vector<double> sizes, int bin_cap) 
{
    int opt_bins = 0;
    bool solved = true;
    int kk = sizes.size();
    int low = 0;
    int high = kk;
    // start with as many bins as there are objects and decrease
    // that total until no solution for kk bins is found
    while (low != kk && high != low) {
        //cout << low << " " << kk << " " << high << endl;
        solved = solve_exhaustive(sizes, kk, bin_cap);
        if (!solved) {
            low = kk;
            kk = (int)((kk + high) / 2);
        } else {
            opt_bins = kk;
            high = kk;
            kk = (int)((kk + low) / 2);
        }
    }
    return opt_bins;
}

// make sure that the solution returned from the algorithm is valid
bool 
verify_solution(const vector<double> sizes, const vector<int> certificate, const int num_bins, const int cap) 
{
    int bins[num_bins];
    fill_n(bins, num_bins, cap);
    for (size_t ii=0; ii<sizes.size(); ++ii) {
        size_t idx = certificate[ii];
        if (bins[idx] >= sizes[ii]) {
            bins[idx] -= sizes[ii];
        } else {
            return false;
        }
    }
    return true;
}

void 
user_prompt(instance_info& instance) 
{
    size_t num_samples;
    int max_item_size;
    cout << "Number of samples: ";
    cin >> num_samples;
    cout << endl;
    cout << "Maximum item size: ";
    cin >> max_item_size;
    cout << endl;
    cout << "Capacity of each bin: ";
    cin >> instance.bin_capacity;
    cout << endl;
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
        double ss;
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
    completion_tree comp_tree;
    sort(instance.sizes.begin(),instance.sizes.end());
    int opt_bins = item_oriented_branch_and_bound(instance.sizes,comp_tree,instance.bin_capacity,instance.sizes.size());
    //int opt_bins = exhaustive_optimal(instance.sizes, instance.bin_capacity);
    cout << "Optimal Solution: " << opt_bins << " bins" << endl;

    return 0;
}