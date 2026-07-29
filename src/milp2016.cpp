/*
 * This code implements the square fiducial marker generation method described in:
 * Generation of fiducial marker dictionaries using mixed integer linear programming.
 * S. Garrido-Jurado, R. Munoz-Salinas, F. J. Madrid-Cuevas, R. Medina-Carnicer.
 * Pattern Recognition 2016, 51, 481-491.
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

static void usage() {
  std::cout <<
    "Usage: milp2016 [options]\n"
    "  --n N            marker size (N x N)\n"
    "  --target T       dictionary size (number of markers)\n"
    "  --out FILE       output dictionary file\n"
    "  --times FILE     output timings file (one line \"size seconds\" per marker)\n"
    "  --seed S         CP-SAT random seed (default 0)\n"
    "  --timelimit S    max seconds per marker (CP-SAT solve, default 150)\n";
}

int main(int argc, char** argv) {
  int N = 0, D = 0, seed = 0;
  double timeLimit = 150.0;
  std::string out = "", times = "";

  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    if      (a == "--n"         && i + 1 < argc) N         = atoi(argv[++i]);
    else if (a == "--target"    && i + 1 < argc) D         = atoi(argv[++i]);
    else if (a == "--out"       && i + 1 < argc) out       = argv[++i];
    else if (a == "--times"     && i + 1 < argc) times     = argv[++i];
    else if (a == "--seed"      && i + 1 < argc) seed      = atoi(argv[++i]);
    else if (a == "--timelimit" && i + 1 < argc) timeLimit = atof(argv[++i]);
    else { std::cerr << "Unknown or incomplete argument: " << a << "\n\n"; usage(); return 1; }
  }

  if (N <= 0 || D <= 0 || out.empty() || times.empty())
  { std::cerr << "Required: --n --target --out --times\n\n"; usage(); return 1; }

  const int totalBits = N * N;
  const int tauMax = 2 * int(4 * int(totalBits / 4) / 3);

  std::cout << "MIP (CP-SAT) | N=" << N << " D=" << D << " Ref=" << REFLECTED
            << " TimeLimit=" << timeLimit << "s | tauMax=" << tauMax << std::endl;

  std::vector<std::vector<int>> reflMap;
  if (REFLECTED) {
    reflMap.assign(4, std::vector<int>(totalBits));
    for (int r = 0; r < 4; r++)
      for (int k = 0; k < totalBits; k++)
        reflMap[r][k] = getIdAfterReflection(k, r, N);
  }

  Dictionary MS;
  int upperBound = tauMax;

  std::ofstream timeOut(times);
  auto tStart = std::chrono::steady_clock::now();

  for (int t = 0; t < D; t++) {
    CpModelBuilder model;

    std::vector<BoolVar> b(totalBits);
    for (int k = 0; k < totalBits; k++) b[k] = model.NewBoolVar();

    IntVar tau = model.NewIntVar(Domain(0, upperBound));

    for (size_t m = 0; m < MS.size(); m++) {
      for (int r = 0; r < 4; r++) {
        LinearExpr ham;
        for (int k = 0; k < totalBits; k++) {
          int conv = getIdAfterRotation(k, r, N);
          if (MS[m][conv / N][conv % N]) ham += (1 - b[k]);
          else                           ham += b[k];
        }
        model.AddGreaterOrEqual(ham, tau);
      }
      if (REFLECTED) {
        for (int r = 0; r < 4; r++) {
          LinearExpr ham;
          for (int k = 0; k < totalBits; k++) {
            int conv = reflMap[r][k];
            if (MS[m][conv / N][conv % N]) ham += (1 - b[k]);
            else                           ham += b[k];
          }
          model.AddGreaterOrEqual(ham, tau);
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
      model.AddGreaterOrEqual(selfHam, tau);
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
        model.AddGreaterOrEqual(selfHam, tau);
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

    model.Maximize(tau);

    SatParameters params;
    params.set_max_time_in_seconds(timeLimit);
    params.set_random_seed(seed);
    CpSolverResponse resp = SolveWithParameters(model.Build(), params);

    if (resp.status() != CpSolverStatus::OPTIMAL &&
        resp.status() != CpSolverStatus::FEASIBLE) {
      std::cerr << "No feasible marker at t=" << t
                << " (status=" << CpSolverStatus_Name(resp.status())
                << "); stopping with " << MS.size() << " markers.\n";
      break;
    }

    Marker marker(N, BitString(N));
    for (int k = 0; k < totalBits; k++) marker[k / N][k % N] = SolutionBooleanValue(resp, b[k]);
    MS.push_back(marker);

    const int achieved = (int)SolutionIntegerValue(resp, tau);
    upperBound = std::min(upperBound, achieved);

    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count();
    timeOut << (t + 1) << " " << elapsed << std::endl;

    if ((t + 1) % 20 == 0 || t + 1 == D)
      std::cout << "  marker " << (t + 1) << "/" << D
                << "  dist=" << achieved << std::endl;
  }

  saveDictionary(MS, out);
  std::cout << "Wrote " << MS.size() << " markers to " << out << std::endl;
  return 0;
}
