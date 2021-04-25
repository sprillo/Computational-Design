#include <stdio.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>
#include <string.h>
#include <fstream>
#include <assert.h>

#define forn(i, n) for(int i = 0; i < int(n); i++)
#define mp make_pair

using namespace std;

bool verbose = false;
const int maxN = 10000;
int N;
int M;
int left_value[maxN];
int right_value[maxN];
vector<int> g[2 * maxN];
vector<vector<int> > connected_components;

void sample_random_function(){
    forn(i, N){
        // left_value[i] = 2 * i;
        // right_value[i] = 2 * i + 1;
        left_value[i] = i;
        right_value[i] = i + N;
    }
}

void add_edge(int left, int right){
    if(verbose) cerr << "Adding edge " << left << " " << right + N << endl;
    g[left].push_back(right + N);
    g[right + N].push_back(left);
}

void sample_random_graph(){
    forn(i, 2 * N){
        g[i].clear();
    }
    forn(i, M){
        int left = rand() % N;
        int right = rand() % N;
        add_edge(left, right);
    }
}

bool visited[2 * maxN];
vector<int> current_component;

void dfs(int v){
    assert(!visited[v]);
    visited[v] = true;
    current_component.push_back(v);
    for(int u: g[v]){
        if(!visited[u]){
            dfs(u);
        }
    }
}

void compute_connected_components(){
    connected_components.clear();
    forn(i, 2 * N) visited[i] = false;
    forn(i, 2 * N){
        if(!visited[i]){
            current_component.clear();
            dfs(i);
            connected_components.push_back(current_component);
        }
    }
    if(verbose){
        for(vector<int> curr_connected_component: connected_components){
            cerr << "Connected component: " << endl;
            for(int v: curr_connected_component){
                cerr << v << " ";
            }
            cerr << endl;
        }
    }
}

pair<int, int> global_optima = mp(-1, -1);

void compute_global_optima(){
    global_optima = mp(-1, -1);
    int max_left = -1000;
    int max_right = -1000;
    forn(i, N) max_left = max(max_left, left_value[i]);
    forn(i, N) max_right = max(max_right, right_value[i]);
    global_optima = mp(max(max_left, max_right), min(max_left, max_right));
    assert(global_optima >= mp(0, 0)); // Since all values are positive
}

pair<int, int> dataset_maxima = mp(-1, -1);

void compute_dataset_maxima(){
    dataset_maxima = mp(-1, -1);
    forn(v, N) for(int u: g[v]){
        assert(u >= N);
        assert(u < 2 * N);
        dataset_maxima = max(dataset_maxima, mp(max(left_value[v], right_value[u - N]), min(left_value[v], right_value[u - N])));
    }
    assert(dataset_maxima >= mp(0, 0)); // Since all values are positive and we sample at least 1 edge.
}

pair<int, int> designed_val = mp(-1, -1);

void show(vector<int> v){
    for(int u: v) cerr << u << " ";
}

void design(){
    designed_val = mp(-1, -1);
    for(vector<int> curr_connected_component: connected_components){
        int best_left = -1000;
        int best_right = -1000;
        for(int v: curr_connected_component){
            // cerr << "v = " << v << endl;
            if(v < N){
                assert(0 <= v);
                best_left = max(best_left, left_value[v]);
            } else {
                assert(v < 2 * N);
                best_right = max(best_right, right_value[v - N]);
            }
        }
        // cerr << best_left << " " << best_right << endl;
        // cerr << "Connected component ";
        // show(curr_connected_component);
        // cerr << " value = " << best_left + best_right << endl;
        if(best_left >= 0 && best_right >= 0)
            designed_val = max(designed_val, mp(max(best_left, best_right), min(best_left, best_right)));
    }
    assert(designed_val >= mp(0, 0)); // Since all values are positive and we sample at least 1 edge.
}

int determine_global_opt_success(){
    assert(!(designed_val > global_optima));
    return 1 * (designed_val == global_optima);
}

int determine_design_success(){
    assert(!(designed_val > global_optima));
    return 1 * ((designed_val == global_optima) || (designed_val > dataset_maxima));
}

vector<int> Ns;
vector<int> Ms;
vector<double> probs_fully_connected;
vector<double> probs_global_opt_success;
vector<double> probs_design_success;

bool in_same_connected_component(int a, int b){
    for(auto curr_connected_component: connected_components){
        if(count(curr_connected_component.begin(), curr_connected_component.end(), a)){
            return count(curr_connected_component.begin(), curr_connected_component.end(), b + N) > 0;
        }
    }
    return false;
}

void doit(){
    int counter_fully_connected = 0;
    int counter_global_opt_success = 0;
    int counter_design_success = 0;
    int repetitions = 1000;
    forn(repetition, repetitions){
        sample_random_graph();
        sample_random_function();
        // forn(i, N) cerr << "left_value[" << i << "] = " << left_value[i] << endl;
        // forn(i, N) cerr << "right_value[" << i + N << "] = " << right_value[i] << endl;
        compute_connected_components();
        compute_global_optima();
        // cerr << "Global optima = " << global_optima << endl;
        compute_dataset_maxima();
        // cerr << "Dataset maxima = " << dataset_maxima << endl;
        design();
        // cerr << "Designed value = " << designed_val << endl;
        counter_global_opt_success += determine_global_opt_success();
        // Global opt should success IFF N-1 and N-1 are in the same connected component.
        if(in_same_connected_component(N - 1, N - 1)){
            assert(determine_global_opt_success() == 1);
        } else {
            assert(determine_global_opt_success() == 0);
        }
        counter_design_success += determine_design_success();
        if(connected_components.size() == 1) counter_fully_connected++;
        // for(auto c: connected_components){
        //     if(c.size() > 1)
        //        cerr << c.size() << " ";
        // } cerr << endl;
    }
    Ns.push_back(N);
    Ms.push_back(M);
    probs_fully_connected.push_back(double(counter_fully_connected) / repetitions);
    probs_global_opt_success.push_back(double(counter_global_opt_success) / repetitions);
    probs_design_success.push_back(double(counter_design_success) / repetitions);
    cout << "Prob. fully connected = " << double(counter_fully_connected) / repetitions << endl;
    cout << "Prob. global opt success = " << double(counter_global_opt_success) / repetitions << endl;
    cout << "Prob. design success = " << double(counter_design_success) / repetitions << endl;
}

void write_out_results(){
    ofstream fout("graph_results.txt");
    string res = "N,M,prob_fully_connected,prob_global_opt_success,prob_design_success\n";
    forn(i, Ns.size()){
        res += to_string(Ns[i]) + "," + to_string(Ms[i]) + "," + to_string(probs_fully_connected[i]) + "," + to_string(probs_global_opt_success[i]) + "," + to_string(probs_design_success[i]) + "\n";
    }
    fout << res;
}

void run_Ms_and_Ns(){
    vector<int> vec_ns{100, 1000, 10000};
    N = -1e8;
    for(auto n: vec_ns){
        N = n;
        M = -1e8;
        // int ms[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
        // vector<int> vec_ms(ms, ms + sizeof(ms) / sizeof(int));
        vector<int> vec_ms;
        int prev_m = -1;
        for(int i = 0; i < 150; i++){
            int m = int(pow(1.1, i));
            if(m == prev_m) continue;
            vec_ms.push_back(m);
            prev_m = m;
        }
        for(auto m: vec_ms){
            M = m;
            cerr << "Running N = " << N << ", M = " << M << endl;
            assert(N <= maxN);
            doit();
        }
    }
    write_out_results();
}

int main(){
    cout << "Running graph.cpp" << endl;
    srand(time(NULL));
    run_Ms_and_Ns();
    return 0;
}