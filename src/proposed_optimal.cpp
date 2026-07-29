/*
 * This code implements the square fiducial marker generation method described in:
 * Maximizing Inter-Marker Distance in Square Fiducial Marker Dictionary Generation.
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

static Marker markerFromNumber(unsigned long v, int n)
{
    Marker m(n, BitString(n));
    for(int r=0; r<n; r++)
        for(int c=0; c<n; c++) { m[r][c] = v & 1UL; v >>= 1; }
    return m;
}

static unsigned long markerToNumber(const Marker& m)
{
    unsigned long v = 0;
    int bit = 0;
    for(size_t r=0; r<m.size(); r++)
        for(size_t c=0; c<m[r].size(); c++) { if(m[r][c]) v |= (1UL << bit); bit++; }
    return v;
}

static Dictionary generateCompleteDictionary(int n)
{
    unsigned long total = 1UL << (n*n);
    std::vector<char> seen(total, 0);
    Dictionary D;
    for(unsigned long i=0; i<total; i++)
    {
        if(seen[i]) continue;
        Marker m = markerFromNumber(i, n);
        MarkerSet A = getAnalogousSet(m);
        for(size_t k=0; k<A.size(); k++) seen[markerToNumber(A[k])] = 1;
        D.push_back(m);
    }
    return D;
}

static Dictionary optimizeDictionary(const Dictionary& D_in, int target_dictionary_size, int timelimit, const std::string& outPrefix, std::ofstream& timesFile, std::chrono::steady_clock::time_point tStart)
{
    if(D_in.size()==0) return D_in;

    Dictionary D_out;

    std::vector<int> selfDist(D_in.size());
    int u = 0;
    for(size_t i=0; i<D_in.size(); i++) { selfDist[i] = getMarkerSelfDist(D_in[i]); u = std::max(u, selfDist[i]); }

    while(u>=0)
    {
        std::cout << "Inter-marker distance " << u << "\n";

        std::vector<int> nodeID_to_markerID;
        for(size_t i=0; i<D_in.size(); i++)
            if( selfDist[i] >= u ) nodeID_to_markerID.push_back(i);

        int nNodes = (int)nodeID_to_markerID.size();
        std::vector<MarkerSet> analog(nNodes);
        for(int a=0; a<nNodes; a++) analog[a] = getAnalogousSet(D_in[nodeID_to_markerID[a]]);

        Graph g = Graph(nNodes);
        for(int a=0; a<nNodes; a++)
        {
            const Marker& mA = D_in[nodeID_to_markerID[a]];
            for(int b=a+1; b<nNodes; b++)
            {
                const MarkerSet& AB = analog[b];
                bool edge = true;
                for(size_t k=0; k<AB.size(); k++)
                    if( hamm(mA, AB[k]) < u ) { edge = false; break; }
                if(edge) g.connect(a, b);
            }
        }

        Maxclique solver(g.getData(), nNodes, 0.025, timelimit);
        solver.onImprove = [&](const std::vector<int>& best, double elapsed)
        {
            Dictionary D_best;
            for(size_t k=0; k<best.size(); k++) D_best.push_back( D_in[ nodeID_to_markerID[ best[k] ] ] );
            saveDictionary(D_best, outPrefix + "_d" + toStr(u) + "_incumbent.set");
            timesFile << D_best.size() << " " << std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count() << std::endl;
            std::cout << "  d=" << u << " incumbent " << best.size() << " markers (" << elapsed << "s)" << std::endl;
        };
        int *qmax;
        int qsize=0;
        solver.mcqdyn(qmax, qsize);
        Dictionary D_u;
        for (int i = 0; i < qsize; i++) D_u.push_back( D_in[ nodeID_to_markerID[ qmax[i] ] ] );

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
      "Usage: proposed_optimal [options]\n"
      "  --n N            marker size (N x N); only small N is feasible (<=5)\n"
      "  --target T       dictionary size after the clique optimization\n"
      "  --out-prefix P   prefix for the output dictionaries (one .set per clique)\n"
      "  --times FILE     output timings file (one line \"size seconds\" per clique)\n"
      "  --timelimit S    max seconds per clique search (default 150)\n";
}

int main(int argc, char *argv[])
{
    int n = 0, target = 0, timelimit = 150;
    std::string outPrefix = "", times = "";

    for(int i=1; i<argc; i++)
    {
        std::string a = argv[i];
        if     (a == "--n"          && i+1 < argc) n         = atoi(argv[++i]);
        else if(a == "--target"     && i+1 < argc) target    = atoi(argv[++i]);
        else if(a == "--out-prefix" && i+1 < argc) outPrefix = argv[++i];
        else if(a == "--times"      && i+1 < argc) times     = argv[++i];
        else if(a == "--timelimit"  && i+1 < argc) timelimit = atoi(argv[++i]);
        else { std::cerr << "Unknown or incomplete argument: " << a << "\n\n"; usage(); return 1; }
    }

    if(n <= 0 || target <= 0 || outPrefix.empty() || times.empty())
    { std::cerr << "Required: --n --target --out-prefix --times\n\n"; usage(); return 1; }

    if(n*n > 25)
    { std::cerr << "Error: N=" << n << " enumerates 2^" << (n*n) << " markers, infeasible.\n"; return 2; }

    std::ofstream timesFile(times);
    auto tStart = std::chrono::steady_clock::now();

    Dictionary D_complete = generateCompleteDictionary(n);
    optimizeDictionary(D_complete, target, timelimit, outPrefix, timesFile, tStart);

    return 0;
}
