#include <stdio.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>
#include <string.h>
#include <fstream>

#define forn(i, n) for(int i = 0; i < int(n); i++)

using namespace std;

bool verbose = false;
const int maxN = 10000;
int N;
int M;
float left_value[maxN];
float right_value[maxN];
vector<int> g[2 * maxN];
vector<vector<int> > connected_components;

default_random_engine generator;
uniform_real_distribution<double> distribution(0.0,1.0);

void sample_random_function(){
    forn(i, N){
        left_value[i] = distribution(generator);
        right_value[i] = distribution(generator);
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

float global_optima = 0.0;

void compute_global_optima(){
    global_optima = -1e8;
    float max_left = -1e8;
    float max_right = -1e8;
    forn(i, N) max_left = max(max_left, left_value[i]);
    forn(i, N) max_right = max(max_right, right_value[i]);
    global_optima = max_left + max_right;
    assert(global_optima >= 0); // Since all values are positive
}

float dataset_maxima = 0.0;

void compute_dataset_maxima(){
    dataset_maxima = -1e8;
    forn(v, N) for(int u: g[v]){
        assert(u >= N);
        assert(u < 2 * N);
        dataset_maxima = max(dataset_maxima, left_value[v] + right_value[u - N]);
    }
    assert(dataset_maxima >= 0.0); // Since all values are positive and we sample at least 1 edge.
}

float designed_val = 0.0;

void show(vector<int> v){
    for(int u: v) cerr << u << " ";
}

void design(){
    designed_val = -1e8;
    for(vector<int> curr_connected_component: connected_components){
        float best_left = -1e8;
        float best_right = -1e8;
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
        designed_val = max(designed_val, best_left + best_right);
    }
    assert(designed_val >= 0.0); // Since all values are positive and we sample at least 1 edge.
}

int determine_global_opt_success(){
    return 1 * (designed_val >= global_optima - 1e-16);
}

int determine_design_success(){
    return 1 * ((designed_val >= global_optima - 1e-16) || (designed_val > dataset_maxima + 1e-16));
}

vector<int> Ns;
vector<int> Ms;
vector<float> probs_fully_connected;
vector<float> probs_global_opt_success;
vector<float> probs_design_success;

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
        counter_design_success += determine_design_success();
        if(connected_components.size() == 1) counter_fully_connected++;
        // for(auto c: connected_components){
        //     if(c.size() > 1)
        //        cerr << c.size() << " ";
        // } cerr << endl;
    }
    Ns.push_back(N);
    Ms.push_back(M);
    probs_fully_connected.push_back(float(counter_fully_connected) / repetitions);
    probs_global_opt_success.push_back(float(counter_global_opt_success) / repetitions);
    probs_design_success.push_back(float(counter_design_success) / repetitions);
    cout << "Prob. fully connected = " << float(counter_fully_connected) / repetitions << endl;
    cout << "Prob. global opt success = " << float(counter_global_opt_success) / repetitions << endl;
    cout << "Prob. design success = " << float(counter_design_success) / repetitions << endl;
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
        for(int i = 0; i < 120; i++){
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