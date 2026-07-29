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
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"

#include <cstdint>

using namespace operations_research;
using namespace operations_research::sat;

static int getIdAfterRotation(int idx, int r, int msize) {
  int y0 = idx / msize, x0 = idx % msize, x1, y1;
  if (r == 1)      { x1 = msize - y0 - 1; y1 = x0; }
  else if (r == 2) { x1 = msize - x0 - 1; y1 = msize - y0 - 1; }
  else if (r == 3) { x1 = y0;             y1 = msize - x0 - 1; }
  else             { x1 = x0;             y1 = y0; }
  return msize * y1 + x1;
}

static bool isMiddlePoint(int idx, int msize) {
  if (msize % 2 == 0) return false;
  int mid = (msize - 1) / 2;
  return (idx / msize == mid) && (idx % msize == mid);
}

static int getIdAfterReflection(int idx, int r, int N) {
  Marker m(N, BitString(N));
  m[idx / N][idx % N] = 1;
  Marker out = reflectMarker(m);
  for (int k = 0; k < r; k++) out = rotateMarker(out);
  for (int y = 0; y < N; y++)
    for (int x = 0; x < N; x++)
      if (out[y][x]) return N * y + x;
  return -1;
}

// Solve one "compact" step: given the current dictionary and minimum distance d,
// return a new marker minimizing the sum of its distances while keeping every
// distance >= d. Returns 0 = no marker (infeasible/timeout without solution);
// 1 = OPTIMAL (minimal-sum); 2 = FEASIBLE only (time limit hit).
static int packOneMarker(const Dictionary& MS, int N, int d, double timeLimit,
                         int seed, Marker& outMarker) {
  const int totalBits = N * N;
  CpModelBuilder model;

  std::vector<std::vector<int>> reflMap;
  if (REFLECTED) {
    reflMap.assign(4, std::vector<int>(totalBits));
    for (int r = 0; r < 4; r++)
      for (int k = 0; k < totalBits; k++)
        reflMap[r][k] = getIdAfterReflection(k, r, N);
  }

  std::vector<BoolVar> b(totalBits);
  for (int k = 0; k < totalBits; k++) b[k] = model.NewBoolVar();

  LinearExpr objective;

  for (size_t m = 0; m < MS.size(); m++) {
    for (int r = 0; r < 4; r++) {
      LinearExpr ham;
      for (int k = 0; k < totalBits; k++) {
        int conv = getIdAfterRotation(k, r, N);
        if (MS[m][conv / N][conv % N]) ham += (1 - b[k]);
        else                           ham += b[k];
      }
      model.AddGreaterOrEqual(ham, d);
      objective += ham;
    }
    if (REFLECTED) {
      for (int r = 0; r < 4; r++) {
        LinearExpr ham;
        for (int k = 0; k < totalBits; k++) {
          int conv = reflMap[r][k];
          if (MS[m][conv / N][conv % N]) ham += (1 - b[k]);
          else                           ham += b[k];
        }
        model.AddGreaterOrEqual(ham, d);
        objective += ham;
      }
    }
  }

  for (int r = 1; r < 4; r++) {
    LinearExpr selfHam;
    for (int k = 0; k < totalBits; k++) {
      if (isMiddlePoint(k, N)) continue;
      int rot = getIdAfterRotation(k, r, N);
      BoolVar h = model.NewBoolVar();
      model.AddLessOrEqual(h, b[k]);
      model.AddLessOrEqual(h, b[rot]);
      model.AddGreaterOrEqual(LinearExpr(h), b[k] + b[rot] - 1);
      selfHam += b[k];
      selfHam += b[rot];
      selfHam -= 2 * h;
    }
    model.AddGreaterOrEqual(selfHam, d);
    objective += selfHam;
  }
  if (REFLECTED) {
    for (int r = 0; r < 4; r++) {
      LinearExpr selfHam;
      for (int k = 0; k < totalBits; k++) {
        int refl = reflMap[r][k];
        if (refl == k) continue;
        BoolVar h = model.NewBoolVar();
        model.AddLessOrEqual(h, b[k]);
        model.AddLessOrEqual(h, b[refl]);
        model.AddGreaterOrEqual(LinearExpr(h), b[k] + b[refl] - 1);
        selfHam += b[k];
        selfHam += b[refl];
        selfHam -= 2 * h;
      }
      model.AddGreaterOrEqual(selfHam, d);
      objective += selfHam;
    }
  }

  if (!std::getenv("NOSYMBREAK")) {
    for (int r = 1; r < 4; r++) {
      LinearExpr sym;
      for (int k = 0; k < totalBits; k++) {
        if (isMiddlePoint(k, N)) continue;
        int rot = getIdAfterRotation(k, r, N);
        int64_t c = int64_t{1} << k;
        sym += c * b[k];
        sym += (-c) * b[rot];
      }
      model.AddGreaterOrEqual(sym, 0);
    }
    if (REFLECTED) {
      for (int r = 0; r < 4; r++) {
        LinearExpr sym;
        for (int k = 0; k < totalBits; k++) {
          int refl = reflMap[r][k];
          if (refl == k) continue;
          int64_t c = int64_t{1} << k;
          sym += c * b[k];
          sym += (-c) * b[refl];
        }
        model.AddGreaterOrEqual(sym, 0);
      }
    }
  }

  model.Minimize(objective);

  SatParameters params;
  params.set_max_time_in_seconds(timeLimit);
  params.set_random_seed(seed);
  CpSolverResponse resp = SolveWithParameters(model.Build(), params);

  if (resp.status() != CpSolverStatus::OPTIMAL &&
      resp.status() != CpSolverStatus::FEASIBLE) {
    return 0;
  }

  outMarker.assign(N, BitString(N));
  for (int k = 0; k < totalBits; k++) outMarker[k / N][k % N] = SolutionBooleanValue(resp, b[k]);
  return (resp.status() == CpSolverStatus::OPTIMAL) ? 1 : 2;
}

static void usage() {
  std::cout <<
    "Usage: proposed_suboptimal [options]\n"
    "  --n N            marker size (N x N)\n"
    "  --target T       target dictionary size (stop when reached)\n"
    "  --out-prefix P   prefix for the output dictionaries (one .set per distance)\n"
    "  --times FILE     output timings file (one line \"size seconds\" per size)\n"
    "  --seed S         CP-SAT random seed (default 0)\n"
    "  --timelimit S    max seconds per CP-SAT solve (default 150)\n";
}

int main(int argc, char** argv) {
  int N = 0, D = 0, seed = 0;
  double timeLimit = 150.0;
  std::string outPrefix = "", times = "";

  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    if      (a == "--n"          && i + 1 < argc) N         = atoi(argv[++i]);
    else if (a == "--target"     && i + 1 < argc) D         = atoi(argv[++i]);
    else if (a == "--out-prefix" && i + 1 < argc) outPrefix = argv[++i];
    else if (a == "--times"      && i + 1 < argc) times     = argv[++i];
    else if (a == "--seed"       && i + 1 < argc) seed      = atoi(argv[++i]);
    else if (a == "--timelimit"  && i + 1 < argc) timeLimit = atof(argv[++i]);
    else { std::cerr << "Unknown or incomplete argument: " << a << "\n\n"; usage(); return 1; }
  }

  if (N <= 0 || D <= 0 || outPrefix.empty() || times.empty())
  { std::cerr << "Required: --n --target --out-prefix --times\n\n"; usage(); return 1; }

  const int totalBits = N * N;
  const int tauMax = 2 * int(4 * int(totalBits / 4) / 3);

  std::cout << "Packing (CP-SAT) | N=" << N << " D=" << D << " Ref=" << REFLECTED
            << " TimeLimit=" << timeLimit << "s seed=" << seed
            << " | tauMax=" << tauMax << " | restart per distance" << std::endl;

  std::vector<int> maxDistForSize(D + 1, 0);
  std::ofstream timeOut(times);
  auto tStart = std::chrono::steady_clock::now();

  Dictionary MS;
  int prevSavedSize = -1;
  for (int d = tauMax; d >= 1 && (int)MS.size() < D; d--) {
    MS.clear();  // restart: rebuild this distance level from an empty dictionary
    while ((int)MS.size() < D) {
      Marker marker;
      int st = packOneMarker(MS, N, d, timeLimit, seed, marker);
      if (st == 0) break;  // no marker fits at d -> drop to d-1
      MS.push_back(marker);
      const int s = (int)MS.size();
      if (s % 20 == 0) std::cout << "  d=" << d << " size " << s << std::endl;
      if (d > maxDistForSize[s]) {  // keep the highest distance reaching size s
        maxDistForSize[s] = d;
        double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count();
        timeOut << s << " " << elapsed << std::endl;
      }
    }

    const int size = (int)MS.size();
    if (size > 0 && size != prevSavedSize) {
      saveDictionary(MS, outPrefix + "_d" + toStr(d) + "_n" + toStr(size) + ".set");
      std::cout << "  d=" << d << " -> dict size " << size << std::endl;
      prevSavedSize = size;
    } else {
      std::cout << "  d=" << d << " -> dict size " << size << " (no growth)" << std::endl;
    }
  }

  return 0;
}
