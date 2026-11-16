// Small-input comparison of Insertion, Merge and Quick sort
// Finds the input size where the fastest algorithm changes.
// DAA Assignment (CIC-311) - Experimental Analysis of Algorithms
// Compile: g++ -O2 -std=c++17 sort_crossover.cpp -o sort_crossover

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
using namespace std;

unsigned long long seed = 12345;
int nextRandom() {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((seed >> 33) % 1000000);
}

// ---------- the three algorithms (same code as in the separate files) ----------
void insertionSort(vector<int>& a) {
    int n = a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

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
    mergeSort(a, temp, low, mid);
    mergeSort(a, temp, mid + 1, high);
    merge(a, temp, low, mid, high);
}
void mergeSort(vector<int>& a) {
    vector<int> temp(a.size());
    if (a.size() > 0) mergeSort(a, temp, 0, a.size() - 1);
}

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
void quickSort(vector<int>& a, int low, int high) {
    while (low < high) {
        int p = partition(a, low, high);
        if (p - low < high - p) { quickSort(a, low, p - 1); low = p + 1; }
        else { quickSort(a, p + 1, high); high = p - 1; }
    }
}
void quickSort(vector<int>& a) {
    if (a.size() > 0) quickSort(a, 0, a.size() - 1);
}

// ---------- timing ----------
// One sort of a tiny array takes less time than the clock can measure, so we
// make many random arrays (a "batch"), time sorting all of them, and divide.
double timeBatch(int algo, int n) {
    int batch = 200000 / n + 1;
    vector<vector<int>> arrays(batch, vector<int>(n));
    for (int b = 0; b < batch; b++)
        for (int i = 0; i < n; i++) arrays[b][i] = nextRandom();

    auto start = chrono::steady_clock::now();
    for (int b = 0; b < batch; b++) {
        if (algo == 0) insertionSort(arrays[b]);
        else if (algo == 1) mergeSort(arrays[b]);
        else quickSort(arrays[b]);
    }
    auto stop = chrono::steady_clock::now();
    return chrono::duration<double, micro>(stop - start).count() / batch;   // microseconds per sort
}

int main() {
    string names[] = {"Insertion Sort", "Merge Sort", "Quick Sort"};
    int sizes[] = {4, 8, 16, 32, 64, 128, 256, 512};
    ofstream csv("results_crossover.csv");
    csv << "algorithm,n,run1_us,run2_us,run3_us,avg_us\n";

    cout << "Average time per sort in microseconds (random input)\n";
    cout << setw(6) << "n" << setw(16) << "Insertion" << setw(16) << "Merge" << setw(16) << "Quick"
         << setw(12) << "Fastest" << "\n";
    for (int n : sizes) {
        double avg[3];
        for (int algo = 0; algo < 3; algo++) {
            double r[3];
            for (int k = 0; k < 3; k++) r[k] = timeBatch(algo, n);
            avg[algo] = (r[0] + r[1] + r[2]) / 3;
            csv << fixed << setprecision(6) << names[algo] << "," << n << ","
                << r[0] << "," << r[1] << "," << r[2] << "," << avg[algo] << "\n";
        }
        int best = 0;
        for (int algo = 1; algo < 3; algo++) if (avg[algo] < avg[best]) best = algo;
        cout << fixed << setprecision(4) << setw(6) << n << setw(16) << avg[0] << setw(16) << avg[1]
             << setw(16) << avg[2] << setw(12) << names[best].substr(0, names[best].find(' ')) << "\n";
    }
    csv.close();
    cout << "\nResults saved to results_crossover.csv\n";
    return 0;
}
