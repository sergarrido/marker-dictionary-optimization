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

#ifndef GREEDY_COMMON_H
#define GREEDY_COMMON_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <assert.h>
#include <chrono>
#include <functional>

using namespace std;

typedef std::vector<char> BitString;
typedef std::vector<BitString> Marker;
typedef std::vector<Marker> MarkerSet;
typedef std::vector<Marker> Dictionary;

#define REFLECTED false

template <class T>
std::string toStr(T num)
{
    std::stringstream ss;
    try
    {
        ss << num;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error converting num to string: " << e.what() <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    return ss.str();
}

std::string markerToBitString(const Marker& m);

int hamm(const Marker &m_i, const Marker &m_j);
int hamm(const Dictionary &D, const Marker &m_i);
Marker rotateMarker(const Marker &m);              // 90 degrees
Marker reflectMarker(const Marker &m);
MarkerSet getAnalogousSetRot(const Marker &m);
MarkerSet getAnalogousSetRef(const Marker &m);
MarkerSet getAnalogousSet(const Marker &m);
MarkerSet getSelfAnalogousSet(const Marker &m);
int getMarkerDist(const Marker &m_i, const Marker &m_j);
int getMarkerSelfDist(const Marker &m_i);
int getInterMarkerDistance(const Dictionary &D);
std::vector<int> getHammingDistancesSet(const Marker& m_i, const Dictionary &D);
int getMinimumHammingDistance(const Marker& m_i, const Dictionary &D);
int getMinimumHammingDistanceMultiplicity(const Marker& m_i, const Dictionary &D);


void saveDictionary(const Dictionary &D, string filename);
Dictionary loadDictionary(string filename);


class Graph
{
    public:
    Graph(int nNodes) : _nNodes(nNodes)
    {
        _data = new bool*[nNodes];
        for(int i=0; i<nNodes; i++) _data[i] = new bool[nNodes];

        for(int i=0; i<nNodes; i++)
        {
            for(int j=0; j<nNodes; j++)
             {
                 _data[i][j] = false;
             }
        }
    }

    void connect(uint n1, uint n2)
    {
        _data[n1][n2] = _data[n2][n1] = true;
    }

    bool** getData()
    {
        return _data;
    }

    ~Graph()
    {
        for(int i=0; i<_nNodes; i++) delete [] _data[i];
        delete [] _data;
    }

    private:
    int _nNodes;
    bool **_data;
};


class Maxclique {
  std::chrono::steady_clock::time_point begin;
  const bool* const* e;
  int pk, level;
  const float Tlimit;
  const int maxSeconds;
  class Vertices {
    class Vertex {
      int i, d;
    public:
      void set_i(const int ii)  { i = ii; }
      int get_i() const { return i; }
      void set_degree(int dd) { d = dd; }
      int get_degree() const { return d; }
    };
    Vertex *v;
    int sz;
    static bool desc_degree(const Vertex vi, const Vertex vj) { return (vi.get_degree() > vj.get_degree()); }
  public:
#ifdef DBG
    void dbg_v(const string msg="") const {
      std::cerr << msg << " Vertices: [";
      for (int i=0; i < sz; i++)
	std::cerr << "(" << v[i].get_i() << "," << v[i].get_degree() << ") ";
      std::cerr << "]" << std::endl;
    }
#endif
    Vertices(int size) : sz(0) { v = new Vertex[size]; }
    ~Vertices () {}
    void dispose() { if (v) delete [] v; }
    void sort() { std::sort(v, v+sz, desc_degree); }
    void init_colors();
    void set_degrees(Maxclique&);
    int size() const { return sz; }
    void push(const int ii) { v[sz++].set_i(ii); };
    void pop() { sz--; };
    Vertex& at(const int ii) const { return v[ii]; };
    Vertex& end() const { return v[sz - 1]; };
  };
  class ColorClass {
    int *i;
    int sz;
  public:
#ifdef DBG
    void dbg_i(const string msg="") const {
      std::cerr << msg << " Class: [";
      for (int ii=0; ii < sz; ii++)
	std::cerr << i[ii] << " ";
      std::cerr << "]" << std::endl;
    }
#endif
    ColorClass() : i(0), sz(0) {}
    ColorClass(const int _sz) : i(0), sz(_sz) { init(_sz); }
    ~ColorClass() { if (i) delete [] i;
    }
    void init(const int _sz) { i = new int[_sz]; rewind(); }
    void push(const int ii) { i[sz++] = ii; };
    void pop() { sz--; };
    void rewind() { sz = 0; };
    int size() const { return sz; }
    int& at(const int ii) const { return i[ii]; }
    ColorClass& operator=(const ColorClass& dh) {
      for (int j = 0; j < dh.sz; j++) i[j] = dh.i[j];
      sz = dh.sz;
      return *this;
    }
  };
  Vertices V;
  ColorClass *C, QMAX, Q;
  class StepCount {
    int i1, i2;
  public:
    StepCount() : i1(0), i2(0) {}
    void set_i1(const int ii)  { i1 = ii; }
    int get_i1() const { return i1; }
    void set_i2(const int ii)  { i2 = ii; }
    int get_i2() const { return i2; }
    void inc_i1()  { i1++; }
  };
  StepCount *S;
  bool connection(const int i, const int j) const { return e[i][j]; }
  bool cut1(const int, const ColorClass&);
  void cut2(const Vertices&, Vertices&);
  void color_sort(Vertices&);
  void expand(Vertices);
  void expand_dyn(Vertices);
  void _mcq(int*&, int&, bool);
  void degree_sort(Vertices &R) { R.set_degrees(*this); R.sort(); }
public:
#ifdef DBG
  void dbg_C() const {
    for (int i=0; i < V.size(); i++) {
      std::cerr << "C["<< i << "] : ";
      C[i].dbg_i();
    }
  }
  void dbg_conn() const {
    for (int i=0; i < V.size(); i++) {
      for (int j=0; j < V.size(); j++) {
	std::cerr <<e[i][j];
      }
      std::cerr<< std::endl;
    }
  }
#endif
  Maxclique(const bool* const*, const int, const float=0.025, const int=150);
  std::function<void(const std::vector<int>&, double)> onImprove;
  int steps() const { return pk; }
  void mcq(int* &maxclique, int &sz) { _mcq(maxclique, sz, false); }
  void mcqdyn(int* &maxclique, int &sz) { _mcq(maxclique, sz, true); }
  ~Maxclique() {
    if (C) delete [] C;
    if (S) delete [] S;
    V.dispose();
  };
};

#endif // GREEDY_COMMON_H
