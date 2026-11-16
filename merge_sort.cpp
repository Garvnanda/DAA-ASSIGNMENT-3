// Merge Sort - implementation and running-time experiment
// DAA Assignment (CIC-311) - Experimental Analysis of Algorithms
// Compile: g++ -O2 -std=c++17 merge_sort.cpp -o merge_sort

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
using namespace std;

// ---------- simple random number generator (same numbers on every computer) ----------
unsigned long long seed = 12345;
int nextRandom() {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((seed >> 33) % 1000000);
}

// ---------- the algorithm ----------
// merges a[low..mid] and a[mid+1..high] using the helper array temp
void merge(vector<int>& a, vector<int>& temp, int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    while (i <= mid && j <= high) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= mid) temp[k++] = a[i++];
    while (j <= high) temp[k++] = a[j++];
    for (k = low; k <= high; k++) a[k] = temp[k];
}

void mergeSort(vector<int>& a, vector<int>& temp, int low, int high) {
    if (low >= high) return;
    int mid = (low + high) / 2;
    mergeSort(a, temp, low, mid);        // sort left half
    mergeSort(a, temp, mid + 1, high);   // sort right half
    merge(a, temp, low, mid, high);      // combine
}

void mergeSort(vector<int>& a) {
    vector<int> temp(a.size());
    if (a.size() > 0) mergeSort(a, temp, 0, a.size() - 1);
}

// ---------- input generators ----------
vector<int> makeInput(int n, string type) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = nextRandom();
    if (type == "sorted") sort(a.begin(), a.end());
    if (type == "reverse") sort(a.begin(), a.end(), greater<int>());
    return a;
}

// ---------- one timed run ----------
// Very fast runs (less than 50 ms) are repeated and averaged so the clock can measure them.
double timeOneRun(const vector<int>& input) {
    double total = 0;
    int reps = 0;
    while (total < 50.0) {
        vector<int> a = input;                      // fresh copy, copying is not timed
        auto start = chrono::steady_clock::now();
        mergeSort(a);
        auto stop = chrono::steady_clock::now();
        total += chrono::duration<double, milli>(stop - start).count();
        reps++;
    }
    return total / reps;
}

int main() {
    // ----- 1. correctness check against the built-in sort -----
    bool ok = true;
    for (int t = 0; t < 50; t++) {
        vector<int> a = makeInput(1 + t * 37, "random");
        vector<int> b = a;
        mergeSort(a);
        sort(b.begin(), b.end());
        if (a != b) ok = false;
    }
    cout << "Merge Sort correctness check: " << (ok ? "PASSED" : "FAILED") << "\n\n";
    if (!ok) return 1;

    // ----- 2. timing experiment -----
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000};
    string cases[] = {"random", "sorted", "reverse"};
    ofstream csv("results_merge.csv");
    csv << "algorithm,case,n,run1_ms,run2_ms,run3_ms,avg_ms\n";

    for (string c : cases) {
        cout << "Case: " << c << "\n";
        cout << setw(8) << "n" << setw(12) << "Run1(ms)" << setw(12) << "Run2(ms)"
             << setw(12) << "Run3(ms)" << setw(12) << "Avg(ms)" << setw(10) << "T2n/Tn" << "\n";
        double prevAvg = 0;
        for (int n : sizes) {
            vector<int> input = makeInput(n, c);
            double r[3];
            for (int k = 0; k < 3; k++) r[k] = timeOneRun(input);
            double avg = (r[0] + r[1] + r[2]) / 3;
            cout << fixed << setprecision(4) << setw(8) << n << setw(12) << r[0] << setw(12) << r[1]
                 << setw(12) << r[2] << setw(12) << avg;
            if (prevAvg > 0) cout << setw(10) << setprecision(2) << avg / prevAvg;
            else cout << setw(10) << "-";
            cout << "\n";
            csv << fixed << setprecision(6) << "Merge Sort," << c << "," << n << ","
                << r[0] << "," << r[1] << "," << r[2] << "," << avg << "\n";
            prevAvg = avg;
        }
        cout << "\n";
    }
    csv.close();
    cout << "Results saved to results_merge.csv\n";
    return 0;
}
