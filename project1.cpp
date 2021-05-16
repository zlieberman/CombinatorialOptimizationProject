#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <fstream>

using namespace std;

vector<int> generate_random_instance(size_t num_samples, int max_item_size) {
    vector<int> vv(num_samples);
    //https://stackoverflow.com/questions/21516575/fill-a-vector-with-random-numbers-c
    auto gen = [&max_item_size](){return rand()%max_item_size;};
    generate(vv.begin(), vv.end(), gen);
    return vv;
}

vector<int> generate_constant_instance(size_t num_samples, int item_size) {
    vector<int> vv(num_samples);
    fill(vv.begin(), vv.end(), item_size);
    return vv;
}

// get the bin that the object at sample_idx is going to be place in
int get_idx(int value, int num_bins, int sample_idx) {
    return (value / (int)pow(num_bins,sample_idx)) % num_bins;
}

// solve an instance of the bin packing problem given the sizes
// of the objects, the number of bins, and the capacity of each bin
bool solve_instance(vector<int> sizes, int num_bins, int cap) {
    int num_samples = sizes.size();
    double num_combinations = pow(num_bins, num_samples);
    // certificate to store the bin of each sample
    vector<int> bin_idxs;
    cout << "Number of possible combinations: " << num_combinations << endl;
    // iterate over all possible combinations
    for (int ii=0; ii<num_combinations; ++ii) {
        // always reinitialize the capacity of the bins
        // before trying a new combination
        int bins[num_bins];
        fill_n(bins, num_bins, cap);
        // put the balls in the appropriate bin for this combination
        for (int jj=0; jj<num_samples; ++jj) {
            int bin_idx = get_idx(ii, num_bins, jj);
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
    }
    return false;
}

int exhaustive_optimal(const vector<int> sizes, int bin_cap) {
    int opt_bins = 0;
    bool solved = true;
    // start with as many bins as there are objects and decrease
    // that total until no solution for kk bins is found
    for (int kk=sizes.size(); kk>0; --kk) {
        solved = solve_instance(sizes, kk, bin_cap);
        if (!solved) {
            break;
        }
        opt_bins = kk;
    }
    return opt_bins;
}

// make sure that the solution returned from the algorithm is valid
bool verify_solution(const vector<int> sizes, const vector<int> certificate, const int num_bins, const int cap) {
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

typedef struct instance_info {
    vector<int> sizes;
    int bin_capacity;
} instance_info;

void user_prompt(instance_info& instance) {
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

void read_instance(instance_info& instance, string filename) {
    (void)instance; (void)filename;

}

int main(int argc, char* argv[]) {
    srand(time(0));
    instance_info instance;
    if (argc == 1) {
        user_prompt(instance);
    } else if (argc == 2) {
        read_instance(instance, argv[1]);
    } else {
        cout << "Invalid Usage" << endl;
    }
    /*
    cout << "Number of samples: " << num_samples << endl;
    cout << "Maximum item size: " << max_item_size << endl;
    cout << "Number of bins: " << num_bins << endl;
    cout << "Capacity of each bin: " << bin_capacity << endl;
    */
    /*
    // print out the items
    for (auto item : instance.sizes) {
        cout << item;
    }
    */
    cout << endl;
    int opt_bins = exhaustive_optimal(instance.sizes, instance.bin_capacity);
    cout << "Optimal Solution: " << opt_bins << " bins" << endl;

    return 0;
}