// Quick Sort (last element as pivot) - implementation and running-time experiment
// DAA Assignment (CIC-311) - Experimental Analysis of Algorithms
// Compile: g++ -O2 -std=c++17 quick_sort.cpp -o quick_sort

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
// Lomuto partition: the last element is the pivot
int partition(vector<int>& a, int low, int high) {
    int pivot = a[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (a[j] <= pivot) {
            i++;
            swap(a[i], a[j]);
        }
    }
    swap(a[i + 1], a[high]);
    return i + 1;
}

// Recursive call is made on the smaller part and a loop handles the bigger part.
// This does not change the running time, it only keeps the recursion depth small
// so that the worst case (sorted input) does not overflow the stack.
void quickSort(vector<int>& a, int low, int high) {
    while (low < high) {
        int p = partition(a, low, high);
        if (p - low < high - p) {
            quickSort(a, low, p - 1);
            low = p + 1;
        } else {
            quickSort(a, p + 1, high);
            high = p - 1;
        }
    }
}

void quickSort(vector<int>& a) {
    if (a.size() > 0) quickSort(a, 0, a.size() - 1);
}

// ---------- input generators ----------
vector<int> makeInput(int n, string type) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = nextRandom();
    if (type == "sorted") sort(a.begin(), a.end());   // worst case for last-element pivot
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
        quickSort(a);
        auto stop = chrono::steady_clock::now();
        total += chrono::duration<double, milli>(stop - start).count();
        reps++;
    }
    return total / reps;
}

void runCase(string c, vector<int> sizes, ofstream& csv) {
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
        csv << fixed << setprecision(6) << "Quick Sort," << c << "," << n << ","
            << r[0] << "," << r[1] << "," << r[2] << "," << avg << "\n";
        prevAvg = avg;
    }
    cout << "\n";
}

int main() {
    // ----- 1. correctness check against the built-in sort -----
    bool ok = true;
    for (int t = 0; t < 50; t++) {
        vector<int> a = makeInput(1 + t * 37, t % 2 == 0 ? "random" : "sorted");
        vector<int> b = a;
        quickSort(a);
        sort(b.begin(), b.end());
        if (a != b) ok = false;
    }
    cout << "Quick Sort correctness check: " << (ok ? "PASSED" : "FAILED") << "\n\n";
    if (!ok) return 1;

    // ----- 2. timing experiment -----
    ofstream csv("results_quick.csv");
    csv << "algorithm,case,n,run1_ms,run2_ms,run3_ms,avg_ms\n";
    runCase("random", {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000}, csv); // average case
    runCase("sorted", {1000, 2000, 4000, 8000, 16000}, csv);                      // worst case
    csv.close();
    cout << "Results saved to results_quick.csv\n";
    return 0;
}
