#include <iostream>
#include <random>
#include <vector>

using namespace std;

vector<int> generate_instance(size_t num_samples, int max_item_size) {
    vector<int> vv;
    for (size_t ii = 0; ii < num_samples; ++ii) {
        vv.push_back(rand()%max_item_size);
    }
    return vv;
}

// return a list of numbers representing the base-`base` representation
// of the input `value`
vector<int> to_idx(int value, int base, size_t result_len) {
    vector<int> output;
    for (size_t ii=0; ii<result_len; ++ii) {
        int next_val = value % base;
        output.push_back(next_val);
        value -= next_val;
    }
    return output;
}

bool solve_instance(vector<int> sizes, int K, int B) {
    size_t num_samples = sizes.size();
    for (size_t ii=0; ii<the_number_of_possible_combinations; ++ii) {
        int bins[] = {B, B, B, B}; // the remaining capacities of the bins
        vector<int> bin_idxs = to_idx(ii, K, num_samples);
        for (size_t jj=0; jj<num_samples; ++jj) {
            if (bins[bin_idxs[jj]] >= sizes[jj]) {
                bins[bin_idxs[jj]] -= sizes[jj];
            } else {
                break;
            }
            if (jj == num_samples-1) {
                return true;
            }
        }

    }

}

int main() {
    // something
    size_t num_samples = 10;
    int max_item_size = 10;
    //vector<int> balls = generate_instance(num_samples, max_item_size);
    vector<vector<int>> results = get_combinations(10,2);
    for (vector<int> result : results) {
        for (int item : result) {
            cout << item;
        }
        cout << endl;
    }

    cout << "chum" << endl;

    return 0;
}