#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <time.h>

using namespace std;

vector<int> generate_instance(size_t num_samples, int max_item_size) {
    vector<int> vv;
    for (size_t ii = 0; ii < num_samples; ++ii) {
        vv.push_back(rand()%max_item_size);
    }
    return vv;
}

// get the bin that the object at sample_idx is going to be place in
int get_idx(int value, int num_bins, int sample_idx) {
    return (value / (int)pow(num_bins,sample_idx)) % num_bins;
}

bool solve_instance(vector<int> sizes, int K, int B) {
    int num_samples = sizes.size();
    int num_combinations = (int)pow(K, num_samples);
    cout << "Number of possible combinations: " << num_combinations << endl;
    // iterate over all possible combinations
    for (int ii=0; ii<num_combinations; ++ii) {
        int bins[] = {B, B, B, B}; // the remaining capacities of the bins
        // put the balls in the appropriate bin for this combination
        for (int jj=0; jj<num_samples; ++jj) {
            int bin_idx = get_idx(ii, K, jj);
            if (bins[bin_idx] >= sizes[jj]) {
                bins[bin_idx] -= sizes[jj];
            } else {
                break;
            }
            if (jj == num_samples-1) {
                return true;
            }
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Usage: ./prog num_samples max_item_size num_bins bin_capacity" << endl;
        return -1;
    }
    srand(time(0));
    size_t num_samples = atoi(argv[1]);
    int max_item_size = atoi(argv[2]);
    int num_bins = atoi(argv[3]);
    int bin_capacity = atoi(argv[4]);
    cout << "Number of samples: " << num_samples << endl;
    cout << "Maximum item size: " << max_item_size << endl;
    cout << "Number of bins: " << num_bins << endl;
    cout << "Capacity of each bin: " << bin_capacity << endl;
    vector<int> balls = generate_instance(num_samples, max_item_size);
    /*
    for (auto ball : balls) {
        cout << ball;
    }
    cout << endl;
    */
    bool solved = solve_instance(balls, num_bins, bin_capacity);
    cout << "Instance solved: ";
    cout << boolalpha << solved << endl;

    return 0;
}