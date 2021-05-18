/*
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
*/