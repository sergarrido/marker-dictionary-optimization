/*
 * This code implements the square fiducial marker generation method described in:
 * Reflection-Aware Generation and Identification of Square Marker Dictionaries.
 * S. Garrido-Jurado, J. Garrido, D. Jurado-Rodriguez, F. Vazquez, R. Munoz-Salinas.
 * Sensors 2022, 22(21), 8548.
 *
 * Copyright (C) 2026  Sergio Garrido Jurado
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "common.h"
#include <cstdlib>
#include <ctime>

static Marker getRandomMarker(int n) {
    Marker res(n, BitString(n));
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) {
            if(rand()%2==0) res[i][j] = 0;
            else res[i][j] = 1;
        }
    }
    return res;
}

static Dictionary generateInitialDictionary(int n, int p)
{
    Dictionary D;
    for(int t = 0; t<p; t++)
    {
        if(t%100==0) std::cout << "Iteration " << t << "/" << p << "\n";
        Marker m_t = getRandomMarker(n);

        bool improvement_found = false;
        do
        {
            int best_sum_hamming = 0;
            std::pair<int,int> best_b = make_pair(0,0);
            improvement_found = false;
            int dH_t = getMinimumHammingDistance(m_t, D);
            int fH_t = getMinimumHammingDistanceMultiplicity(m_t, D);
            for(int b_r=0; b_r<n; b_r++)
            {
                for(int b_c=0; b_c<n; b_c++)
                {
                    std::pair<int,int> b = std::make_pair(b_r, b_c);
                    Marker m_t_b = m_t;
                    m_t_b[b.first][b.second] = !m_t_b[b.first][b.second];

                    int dH_t_b = getMinimumHammingDistance(m_t_b, D);
                    if(getMarkerSelfDist(m_t_b) < dH_t_b) continue;

                    if(dH_t_b < dH_t) continue;

                    if(dH_t_b == dH_t)
                    {
                        int fH_t_b = getMinimumHammingDistanceMultiplicity(m_t_b, D);
                        if(fH_t_b >= fH_t) continue;
                    }

                    std::vector<int> H = getHammingDistancesSet(m_t_b, D);
                    int sum_hamming = 0;
                    for(size_t h=0; h<H.size(); h++) sum_hamming += H[h];

                    if(sum_hamming > best_sum_hamming)
                    {
                        best_sum_hamming = sum_hamming;
                        best_b = b;
                        improvement_found = true;
                    }
                }
            }
            if(improvement_found)
            {
                m_t[best_b.first][best_b.second] = !m_t[best_b.first][best_b.second];
            }
        } while (improvement_found);
        D.push_back(m_t);
    }
    return D;
}

static Dictionary optimizeDictionary(const Dictionary& D_in, int target_dictionary_size, int timelimit, const std::string& outPrefix, std::ofstream& timesFile, std::chrono::steady_clock::time_point tStart)
{
    if(D_in.size()==0) return D_in;
    if(int(D_in.size())<target_dictionary_size) return D_in;

    Dictionary D_out;

    int u = 0;
    for(size_t i=0; i<D_in.size(); i++) u = std::max(u, getMarkerSelfDist(D_in[i]));

    while(u>=0)
    {
        std::cout << "Inter-marker distance " << u << "\n";

        std::vector<int> nodeID_to_markerID;
        std::unordered_map<int, int> markerID_to_nodeID;
        for(size_t i=0; i<D_in.size(); i++)
        {
            if( getMarkerSelfDist(D_in[i]) >= u )
            {
                nodeID_to_markerID.push_back(i);
                markerID_to_nodeID[i] = nodeID_to_markerID.size()-1;
            }
        }
        Graph g = Graph(nodeID_to_markerID.size());
        for(size_t i=0; i<D_in.size(); i++)
        {
            if(markerID_to_nodeID.find(i) == markerID_to_nodeID.end()) continue;
            for(size_t j=i+1; j<D_in.size(); j++)
            {
                if(markerID_to_nodeID.find(j) == markerID_to_nodeID.end()) continue;
                if( getMarkerDist(D_in[i], D_in[j]) >= u ) g.connect(markerID_to_nodeID[i], markerID_to_nodeID[j]);
            }
        }

        Maxclique solver(g.getData(), nodeID_to_markerID.size(), 0.025, timelimit);
        int *qmax;
        int qsize=0;
        solver.mcqdyn(qmax, qsize);
        Dictionary D_u;
        for (int i = 0; i < qsize; i++) D_u.push_back( D_in[ nodeID_to_markerID[ qmax[i] ] ] );

        timesFile << D_u.size() << " " << std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count() << std::endl;

        saveDictionary(D_u, outPrefix + "_d" + toStr(u) + "_n" + toStr((int)D_u.size()) + ".set");

        if(int(D_u.size()) >= target_dictionary_size)
        {
            D_out = D_u;
            return D_out;
        }
        u = u-1;
    }

    return D_out;
}

static void usage()
{
    std::cout <<
      "Usage: generate_dictionary [options]\n"
      "  --n N            marker size (N x N)\n"
      "  --pool P         initial pool size (number of markers to generate)\n"
      "  --target T       dictionary size after the clique optimization\n"
      "  --out-prefix P   prefix for the output dictionaries (one .set per clique)\n"
      "  --times FILE     output timings file (one line \"size seconds\" per clique)\n"
      "  --seed S         RNG seed (default: current time)\n"
      "  --timelimit S    max seconds per clique search (default 150)\n";
}

int main(int argc, char *argv[])
{
    int n = 0, pool = 0, target = 0, timelimit = 150;
    long seed = -1;
    std::string outPrefix = "", times = "";

    for(int i=1; i<argc; i++)
    {
        std::string a = argv[i];
        if     (a == "--n"          && i+1 < argc) n         = atoi(argv[++i]);
        else if(a == "--pool"       && i+1 < argc) pool      = atoi(argv[++i]);
        else if(a == "--target"     && i+1 < argc) target    = atoi(argv[++i]);
        else if(a == "--out-prefix" && i+1 < argc) outPrefix = argv[++i];
        else if(a == "--times"      && i+1 < argc) times     = argv[++i];
        else if(a == "--seed"       && i+1 < argc) seed      = atol(argv[++i]);
        else if(a == "--timelimit"  && i+1 < argc) timelimit = atoi(argv[++i]);
        else { std::cerr << "Unknown or incomplete argument: " << a << "\n\n"; usage(); return 1; }
    }

    if(n <= 0 || pool <= 0 || target <= 0 || outPrefix.empty() || times.empty())
    { std::cerr << "Required: --n --pool --target --out-prefix --times\n\n"; usage(); return 1; }

    srand( seed >= 0 ? (unsigned int)seed : (unsigned int)time(NULL) );

    std::ofstream timesFile(times);
    auto tStart = std::chrono::steady_clock::now();

    Dictionary D_init = generateInitialDictionary(n, pool);
    optimizeDictionary(D_init, target, timelimit, outPrefix, timesFile, tStart);

    return 0;
}
