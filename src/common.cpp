/*
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

std::string markerToBitString(const Marker& m)
{
    stringstream ss;
    for(size_t r=0; r<m.size(); r++)
    {
        for(size_t c=0; c<m[r].size(); c++)
        {
            ss << int(m[r][c]) << " ";
        }
    }
    return ss.str();
}

int hamm(const Marker &m_i, const Marker &m_j)
{
    int dist = 0;
    for(size_t r=0; r<m_i.size(); r++)
    {
        for(size_t c=0; c<m_i[r].size(); c++)
        {
            dist += (m_i[r][c] != m_j[r][c])?1:0;
        }
    }
    return dist;
}

int hamm(const Dictionary &D, const Marker &m_i)
{
    int minDist = int(m_i.size() * m_i.size());
    for(size_t j=0; j<D.size(); j++) minDist = std::min(minDist, hamm(m_i, D[j]));
    return minDist;
}

// 90 degrees
Marker rotateMarker(const Marker &m)
{
    int n = int(m.size());
    Marker out(n, BitString(n));
    for (int r=0; r<n; r++)
    {
        for (int c=0; c<n; c++)
        {
            out[r][c] = m[n-c-1][r];
        }
    }

    return out;
}

Marker reflectMarker(const Marker &m)
{
    int n = int(m.size());
    Marker out(n, BitString(n));
    for(int r=0; r<n; r++)
    {
	    for(int c=0; c<n; c++)
        {
            out[r][c] = m[r][n-1-c];
        }
    }
    return out;
}

MarkerSet getAnalogousSetRot(const Marker &m)
{
    MarkerSet A_rot(4);
    A_rot[0] = m;
    for(size_t l=1; l<4; l++)
    {
        A_rot[l] = rotateMarker(A_rot[l-1]);
    }

    return A_rot;
}

MarkerSet getAnalogousSetRef(const Marker &m)
{
    MarkerSet A_rot = getAnalogousSetRot(m);

    MarkerSet A_ref(4);
    for(size_t l=0; l<4; l++)
    {
        A_ref[l] = reflectMarker(A_rot[l]);
    }

    return A_ref;
}

MarkerSet getAnalogousSet(const Marker &m)
{
    MarkerSet A = getAnalogousSetRot(m);
    if(REFLECTED)
    {
        MarkerSet A_ref = getAnalogousSetRef(m);
        A.insert(A.end(), A_ref.begin(), A_ref.end());
    }
    return A;
}

MarkerSet getSelfAnalogousSet(const Marker &m)
{
    MarkerSet A = getAnalogousSet(m);
    MarkerSet As;
    As.insert(As.end(), A.begin()+1, A.end());

    return As;
}

int getMarkerDist(const Marker &m_i, const Marker &m_j)
{
    int minDist = int(m_i.size() * m_i.size());
    std::vector<Marker> A_j = getAnalogousSet(m_j);
    for(size_t k=0; k<A_j.size(); k++)
    {
        minDist = std::min(minDist, hamm(m_i, A_j[k]));
    }
    return minDist;
}

int getMarkerSelfDist(const Marker &m_i)
{
    int minDist = int(m_i.size() * m_i.size());
    MarkerSet As_i = getSelfAnalogousSet(m_i);
    for(size_t k=0; k<As_i.size(); k++)
    {
        minDist = std::min(minDist, hamm(m_i, As_i[k]));
    }
    return minDist;
}

int getInterMarkerDistance(const Dictionary &D)
{
    if(D.size()==0) return 0;
    int minDist = int(D[0].size() * D[0].size());
    for(size_t i=0; i<D.size(); i++)
    {
        minDist = std::min(minDist, getMarkerSelfDist(D[i]));
    }
    for(size_t i=0; i<D.size(); i++)
    {
        for(size_t j=i+1; j<D.size(); j++)
        {
            minDist = std::min(minDist, getMarkerDist(D[i], D[j]));
        }
    }
    return minDist;
}

std::vector<int> getHammingDistancesSet(const Marker& m_i, const Dictionary &D)
{
    std::vector<int> H;
    for(size_t j=0; j<D.size(); j++)
    {
        MarkerSet A_j = getAnalogousSet(D[j]);
        for(size_t k=0; k<A_j.size(); k++)
        {
            H.push_back( hamm(m_i, A_j[k]) );
        }
    }
    return H;
}

int getMinimumHammingDistance(const Marker& m_i, const Dictionary &D)
{
    std::vector<int> H = getHammingDistancesSet(m_i, D);
    if(H.empty()) return 0;
    return *std::min_element(H.begin(), H.end());
}

int getMinimumHammingDistanceMultiplicity(const Marker& m_i, const Dictionary &D)
{
    std::vector<int> H = getHammingDistancesSet(m_i, D);
    int minValue = getMinimumHammingDistance(m_i, D);
    return std::count(H.begin(), H.end(), minValue);
}

void saveDictionary(const Dictionary &D, string filename)
{
    if(D.empty()) return;

    std::ofstream fs(filename);
    fs << "%YAML:1.0\n---\n";
    fs << "nmarkers: " << int(D.size()) << "\n";
    fs << "markersize: " << int(D[0].size()) << "\n";

    for (unsigned int i=0; i<D.size(); i++) {
        fs << "marker_" << i << ": \"";
        for(size_t r=0; r<D[i].size(); r++)
        {
            for(size_t c=0; c<D[i][r].size(); c++) fs << (D[i][r][c]?1:0);
        }
        fs << "\"\n";
    }
}

Dictionary loadDictionary(string filename)
{
    Dictionary D;
    std::ifstream fs(filename);
    if(!fs.is_open())
    {
        std::cerr << "Error: cannot open dictionary file '" << filename << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    int markersize = 0;
    std::string line;
    while(std::getline(fs, line))
    {
        std::size_t p = line.find(':');
        if(p == std::string::npos) continue;
        std::string key = line.substr(0, p);
        std::string val = line.substr(p + 1);
        if(key == "markersize")
        {
            markersize = std::atoi(val.c_str());
        }
        else if(key.rfind("marker_", 0) == 0)
        {
            std::size_t a = val.find('"');
            std::size_t b = val.rfind('"');
            if(a == std::string::npos || b <= a) continue;
            std::string bits = val.substr(a + 1, b - a - 1);
            Marker m(markersize, BitString(markersize));
            for(int r=0; r<markersize; r++)
                for(int c=0; c<markersize; c++)
                    m[r][c] = (bits[r*markersize + c] == '1') ? 1 : 0;
            D.push_back(m);
        }
    }
    return D;
}


/////////////////////////////////////////////////////////////////////
// The following code estimates the maximum clique in a graph
// It should be replaced for a licence suitable for OpenCV
/////////////////////////////////////////////////////////////////////


/*
    Copyright 2007-2012 Janez Konc

    If you use this program, please cite:
    Janez Konc and Dusanka Janezic. An improved branch and bound algorithm for the
    maximum clique problem. MATCH Commun. Math. Comput. Chem., 2007, 58, 569-590.

    More information at: http://www.sicmm.org/~konc

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

Maxclique::Maxclique (const bool* const* conn, const int sz, const float tt, const int maxSec) : pk(0), level(1), Tlimit(tt), maxSeconds(maxSec), V(sz), QMAX(sz), Q(sz) {
  assert(conn!=0 && sz>0);
  for (int i=0; i < sz; i++) V.push(i);
  e = conn;
  C = new ColorClass[sz + 1];
  for (int i=0; i < sz + 1; i++) C[i].init(sz + 1);
  S = new StepCount[sz + 1];
  begin = std::chrono::steady_clock::now();
}

void Maxclique::_mcq(int* &maxclique, int &sz, bool dyn) {
  V.set_degrees(*this);
  V.sort();
  V.init_colors();
  if (dyn) {
    for (int i=0; i < V.size() + 1; i++) {
      S[i].set_i1(0);
      S[i].set_i2(0);
    }
    expand_dyn(V);
  }
  else
    expand(V);
  maxclique = new int[QMAX.size()];
  for (int i=0; i<QMAX.size(); i++) {
    maxclique[i] = QMAX.at(i);
  }
  sz = QMAX.size();
}

void Maxclique::Vertices::init_colors() {
  const int max_degree = v[0].get_degree();
  for (int i = 0; i < max_degree; i++)
    v[i].set_degree(i + 1);
  for (int i = max_degree; i < sz; i++)
    v[i].set_degree(max_degree + 1);
}

void Maxclique::Vertices::set_degrees(Maxclique &m) {
  for (int i=0; i < sz; i++) {
    int d = 0;
    for (int j=0; j < sz; j++)
      if (m.connection(v[i].get_i(), v[j].get_i())) d++;
    v[i].set_degree(d);
  }
}

bool Maxclique::cut1(const int pi, const ColorClass &A) {
  for (int i = 0; i < A.size(); i++)
    if (connection(pi, A.at(i)))
      return true;
  return false;
}

void Maxclique::cut2(const Vertices &A, Vertices &B) {
  for (int i = 0; i < A.size() - 1; i++) {
    if (connection(A.end().get_i(), A.at(i).get_i()))
      B.push(A.at(i).get_i());
  }
}

void Maxclique::color_sort(Vertices &R) {
  int j = 0;
  int maxno = 1;
  int min_k = QMAX.size() - Q.size() + 1;
  C[1].rewind();
  C[2].rewind();
  int k = 1;
  for (int i=0; i < R.size(); i++) {
    int pi = R.at(i).get_i();
    k = 1;
    while (cut1(pi, C[k]))
      k++;
    if (k > maxno) {
      maxno = k;
      C[maxno + 1].rewind();
    }
    C[k].push(pi);
    if (k < min_k) {
      R.at(j++).set_i(pi);
    }
  }
  if (j > 0) R.at(j-1).set_degree(0);
  if (min_k <= 0) min_k = 1;
  for (k = min_k; k <= maxno; k++)
    for (int i = 0; i < C[k].size(); i++) {
      R.at(j).set_i(C[k].at(i));
      R.at(j++).set_degree(k);
    }
}

void Maxclique::expand(Vertices R) {
  while (R.size()) {
    if (Q.size() + R.end().get_degree() > QMAX.size()) {
      Q.push(R.end().get_i());
      Vertices Rp(R.size());
      cut2(R, Rp);
      if (Rp.size()) {
        color_sort(Rp);
	pk++;
        expand(Rp);
      }
      else if (Q.size() > QMAX.size()) {
        //std::cerr << "step = " << pk << " current max. clique size = " << Q.size() << std::endl;
	QMAX = Q;
      }
      Rp.dispose();
      Q.pop();
    }
    else {
      return;
    }
    R.pop();
  }
}

void Maxclique::expand_dyn(Vertices R) {
  S[level].set_i1(S[level].get_i1() + S[level - 1].get_i1() - S[level].get_i2());
  S[level].set_i2(S[level - 1].get_i1());

  while (R.size()) {

std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
uint timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
//std::cerr << "Time difference = " << timeElapsed << "[s]" << std::endl;
if((int)timeElapsed > maxSeconds)
{
  std::cerr << "Timeout mcqd" << std::endl;
  break;
}

    if (Q.size() + R.end().get_degree() > QMAX.size()) {
      Q.push(R.end().get_i());
      Vertices Rp(R.size());
      cut2(R, Rp);
      if (Rp.size()) {
        if ((float)S[level].get_i1()/++pk < Tlimit) {
          degree_sort(Rp);
        }
        color_sort(Rp);
	S[level].inc_i1();
	level++;
	expand_dyn(Rp);
	level--;
      }
      else if (Q.size() > QMAX.size()) {
        //std::cerr << "step = " << pk << " current max. clique size = " << Q.size() << std::endl;
	QMAX = Q;
	if (onImprove) {
	  std::vector<int> best;
	  for (int i = 0; i < QMAX.size(); i++) best.push_back(QMAX.at(i));
	  double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
	  onImprove(best, elapsed);
	}
      }
      Rp.dispose();
      Q.pop();
    }
    else {
      return;
    }
    R.pop();
  }
}
