// Knuth-Morris-Pratt (KMP) string matching - implementation and running-time experiment
// DAA Assignment (CIC-311) - Experimental Analysis of Algorithms
// Compile: g++ -O2 -std=c++17 kmp.cpp -o kmp

#include <iostream>
#include <vector>
#include <string>
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
// lps[i] = length of the longest proper prefix of pattern[0..i] that is also a suffix
vector<int> buildLPS(const string& p) {
    int m = p.size();
    vector<int> lps(m, 0);
    int len = 0, i = 1;
    while (i < m) {
        if (p[i] == p[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }
    return lps;
}

// returns the starting index of every occurrence of p in t
vector<int> kmpSearch(const string& t, const string& p) {
    vector<int> found;
    int n = t.size(), m = p.size();
    if (m == 0 || m > n) return found;
    vector<int> lps = buildLPS(p);
    int i = 0, j = 0;              // i -> text, j -> pattern
    while (i < n) {
        if (t[i] == p[j]) {
            i++;
            j++;
            if (j == m) {          // full match
                found.push_back(i - m);
                j = lps[j - 1];
            }
        } else if (j > 0) {
            j = lps[j - 1];        // do not move i back, reuse what already matched
        } else {
            i++;
        }
    }
    return found;
}

// ---------- check using the built-in string::find ----------
vector<int> builtinSearch(const string& t, const string& p) {
    vector<int> found;
    size_t pos = t.find(p);
    while (pos != string::npos) {
        found.push_back(pos);
        pos = t.find(p, pos + 1);
    }
    return found;
}

// ---------- inputs ----------
string randomText(int n, int alphabet) {
    string s(n, 'a');
    for (int i = 0; i < n; i++) s[i] = 'a' + nextRandom() % alphabet;
    return s;
}

// ---------- one timed run ----------
// Very fast runs (less than 50 ms) are repeated and averaged so the clock can measure them.
double timeOneRun(const string& t, const string& p, int& matches) {
    double total = 0;
    int reps = 0;
    while (total < 50.0) {
        auto start = chrono::steady_clock::now();
        vector<int> f = kmpSearch(t, p);
        auto stop = chrono::steady_clock::now();
        matches = f.size();
        total += chrono::duration<double, milli>(stop - start).count();
        reps++;
    }
    return total / reps;
}

int main() {
    // ----- 1. correctness check -----
    bool ok = true;
    for (int t = 0; t < 50; t++) {
        string text = randomText(2000 + t * 100, 2 + t % 3);
        string pat = randomText(1 + t % 6, 2 + t % 3);
        if (kmpSearch(text, pat) != builtinSearch(text, pat)) ok = false;
    }
    string aText(5000, 'a');
    if (kmpSearch(aText, "aaab") != builtinSearch(aText, "aaab")) ok = false;
    if (kmpSearch(aText, "aaaa") != builtinSearch(aText, "aaaa")) ok = false;
    cout << "KMP correctness check (vs string::find): " << (ok ? "PASSED" : "FAILED") << "\n\n";
    if (!ok) return 1;

    // ----- 2. timing experiment (n = text length, pattern length m fixed) -----
    int sizes[] = {1000000, 2000000, 4000000, 8000000, 16000000};
    ofstream csv("results_kmp.csv");
    csv << "algorithm,case,n,run1_ms,run2_ms,run3_ms,avg_ms\n";

    for (int c = 0; c < 2; c++) {
        string caseName = (c == 0) ? "random text" : "worst-case text";
        cout << "Case: " << caseName << "\n";
        cout << setw(10) << "n" << setw(12) << "Run1(ms)" << setw(12) << "Run2(ms)"
             << setw(12) << "Run3(ms)" << setw(12) << "Avg(ms)" << setw(10) << "T2n/Tn"
             << setw(10) << "matches" << "\n";
        double prevAvg = 0;
        for (int n : sizes) {
            string text, pat;
            if (c == 0) {                       // random text over {a,b,c,d}, pattern of length 8
                text = randomText(n, 4);
                pat = "abcdabca";
            } else {                            // text = aaaa...a, pattern = aaaa...ab (m = 1000)
                text = string(n, 'a');
                pat = string(999, 'a') + "b";
            }
            double r[3];
            int matches = 0;
            for (int k = 0; k < 3; k++) r[k] = timeOneRun(text, pat, matches);
            double avg = (r[0] + r[1] + r[2]) / 3;
            cout << fixed << setprecision(4) << setw(10) << n << setw(12) << r[0] << setw(12) << r[1]
                 << setw(12) << r[2] << setw(12) << avg;
            if (prevAvg > 0) cout << setw(10) << setprecision(2) << avg / prevAvg;
            else cout << setw(10) << "-";
            cout << setw(10) << matches << "\n";
            csv << fixed << setprecision(6) << "KMP," << caseName << "," << n << ","
                << r[0] << "," << r[1] << "," << r[2] << "," << avg << "\n";
            prevAvg = avg;
        }
        cout << "\n";
    }
    csv.close();
    cout << "Results saved to results_kmp.csv\n";
    return 0;
}
