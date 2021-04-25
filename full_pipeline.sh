set -e

pushd src
g++ -std=c++17 -DACMTUYO -O3 -Wshadow -Wall  -Wextra -D_GLIBCXX_DEBUG -o graph graph.cpp && ./graph 
popd

ipython nbconvert --to python graph_plot.ipynb
python3 graph_plot.py