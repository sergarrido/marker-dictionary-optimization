/*
 * This code implements the square fiducial marker generation method described in:
 * AprilTag: A robust and flexible visual fiducial system.
 * E. Olson.
 * 2011 IEEE International Conference on Robotics and Automation (ICRA), 3400-3407.
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

#include <vector>
#include <algorithm> 
#include <iostream>
#include "common.h"

#include <gmpxx.h>
//#define mpz_class long long int
#include <omp.h>

#define NTHREADS 12

using namespace std;



Marker mpz2marker(mpz_class v, int d) {
  Marker a;
  a.resize(d);
  for (int y = 0; y < d; y++) {
      a[y].resize(d);
      for (int x = 0; x < d; x++) {
	  a[y][x] = (v & 1) > 0 ? 1 : 0;
	  v = v>>1;
      }
  }

  return a;
}



    int computeComplexity(Marker d)
    {
        bool verbose = false;

        int width = d[0].size(), height = d.size();

        std::vector< std::vector<int> > out;
	out.resize(height);
	for(int i=0; i<out.size(); i++) out[i].resize(width);
        int numrectangles = 0;

        // initialize output to invalid color.
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
                out[y][x] = -1;

        while (true) {

            // What is our error now?
            int error = 0;

            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    if (d[y][x] != out[y][x])
                        error++;

            // are we done?
            if (error == 0)
                break;

            int bestimprovement = 0;
            int besty0 = -1, bestx0 = -1, besty1 = -1, bestx1 = -1, bestv = -1;

            // search over all rectangles: which one will reduce the
            // error the most?
            for (int y0 = 0; y0 < height; y0++) {
                for (int y1 = y0; y1 < height; y1++) {
                    for (int x0 = 0; x0 < width; x0++) {
                        for (int x1 = x0; x1 < width; x1++) {

                            for (int v = 0; v < 2; v++) {

                                int improvement = 0;

                                for (int y = y0; y <= y1; y++) {
                                    for (int x = x0; x <= x1; x++) {
                                        if (d[y][x] == out[y][x]) {
                                            if (d[y][x] == v) {
                                                // no change, still right.
                                            } else {
                                                improvement--;
                                            }
                                        } else {
                                            if (d[y][x] == v) {
                                                improvement++;
                                            } else {
                                                // no change, still wrong.
                                            }
                                        }
                                    }
                                }

                                if (improvement > bestimprovement) {
                                    besty0 = y0;
                                    bestx0 = x0;
                                    besty1 = y1;
                                    bestx1 = x1;
                                    bestv = v;
                                    bestimprovement = improvement;
                                }
                            }
                        }
                    }
                }
            }

            // implement change
            for (int y = besty0; y <= besty1; y++) {
                for (int x = bestx0; x <= bestx1; x++) {
                    out[y][x] = bestv;
                }
            }

            numrectangles++;

         //   if (verbose)
               //; System.out.printf("(%d %d) (%d %d): color %d improvement %d\n",
               //;                   bestx0, besty0, bestx1, besty1, bestv, bestimprovement);
        }

        return numrectangles;
    }    


    mpz_class rotate90(mpz_class w, int d)
    {
        mpz_class wr = 0;
	mpz_class one = 1;

        for (int r = d-1; r >=0; r--) {
            for (int c = 0; c < d; c++) {
                int b = r + d*c;

                wr = wr << 1;

                if ((w & (one << b))!=0)
                    wr |= 1;

            }
        }
        return wr;
    }   
    
    
    int hammingDistance(mpz_class a, mpz_class b) {
      mpz_class _xor;
      _xor = a^b;
      return mpz_popcount(_xor.get_mpz_t());
    }
    
   
    
    


bool isParam(std::string param, int argc, char **argv ) {
    for (int i=0; i<argc; i++)
        if (std::string(argv[i]) == param ) 
            return true;
    return false;

}
std::string getParam(std::string param, int argc, char **argv, std::string defvalue = "") {
    int idx=-1;
    for (int i=0; i<argc && idx==-1; i++)
        if (std::string(argv[i]) == param) 
            idx = i;
    if (idx == -1 || (idx + 1) >= argc)
        return defvalue;
    else
        return argv[idx+1] ;
} 




/**
 */
int main(int argc, char** argv) {
      
   if(!isParam("--out", argc, argv) || !isParam("--times", argc, argv) || !isParam("--n", argc, argv) || !isParam("--target", argc, argv) ) {
     std::cerr << "Parameters: --n markersize --target dictionarysize --out out.dic --times out.times [--seed seed] [--timelimit secs]" << std::endl;
     std::cerr << "--n <int>        # Marker size (Total bits = n x n)" << std::endl;
     std::cerr << "--target <int>   # Dictionary size (number of markers to generate)" << std::endl;
     std::cerr << "--out <file>     # Output dictionary file" << std::endl;
     std::cerr << "--times <file>   # Output timings file (one line \"size seconds\" per marker)" << std::endl;
     std::cerr << "[--seed <int>]   # Seed for random numbers. Optional. If not specified, use current time" << std::endl;
     std::cerr << "[--timelimit <int>] # Unproductive seconds before relaxing minDist. Optional, default 150" << std::endl;
     exit(-1);
   }  
   
   
    unsigned int markersize = atoi(getParam("--n", argc, argv).c_str());
    unsigned int nmarkers = atoi(getParam("--target", argc, argv).c_str());
    
    unsigned int seed = time(NULL);
    if(isParam("--seed", argc, argv)) seed = atoi(getParam("--seed", argc, argv).c_str());   
    
    uint markerbits = markersize*markersize;
    int c = markerbits/4;
    unsigned int minDist = 2*int((4*c)/3) ;
    
    

  Dictionary D;

  std::ofstream timesFile(getParam("--times", argc, argv));
  auto tStart = std::chrono::steady_clock::now();
  std::vector<mpz_class> rots[4];
  //long starttime;
  //long rotcodes[] = new long[16384];
  //int nrotcodes = 0;
  
  const long PRIME = 982451653; 
  
  int mincomplexity= std::min(10,(int)markerbits/3);
  
  //starttime = System.currentTimeMillis();
  
  // begin our search at a random position to avoid any bias
  // towards small numbers (which tend to have larger regions of
  // solid black).
  gmp_randclass r1 (gmp_randinit_default);
  r1.seed(markerbits*10000 + minDist*100 + mincomplexity + seed);
  
  mpz_class V0;
  V0 = r1.get_z_bits(64);
//   V0 = markerbits*10000 + minhamming*100 + mincomplexity;
  //long V0 = new Random(markerbits*10000 + minhamming*100 + mincomplexity).nextLong();  
  
  
        //long lastreporttime = starttime;
        //long lastprogresstime = starttime;
        //long lastprogressiters = 0;  
  
  mpz_class iter,one,max;
  iter = 0;
  one = 1;
  max = one << markerbits;
  
  mpz_class chunksize = 1000;
  
  
    omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(NTHREADS);     
  
  
    auto tick = std::chrono::steady_clock::now();
    auto tick2 = tick;
      // Unproductive seconds before relaxing minDist. Configurable via -t (default 150).
      int baseMaxTime = isParam("--timelimit", argc, argv) ? atoi(getParam("--timelimit", argc, argv).c_str()) : 150;
      int maxTime = baseMaxTime;
  
  
  //while (iter < (1L << markerbits)) {
    while (D.size()<nmarkers) {
      
        
        mpz_class iter0[NTHREADS], iter1[NTHREADS];
        bool found[NTHREADS];
        mpz_class rot90[NTHREADS], rot180[NTHREADS], rot270[NTHREADS];
            mpz_class goodIter[NTHREADS], goodCode[NTHREADS];
            Marker goodBmat[NTHREADS];        
            
      #pragma omp parallel for
      for(int th=0; th<NTHREADS; th++) {
          iter0[th] = iter + th*chunksize;
          iter1[th] = iter0[th] + chunksize;
          if(max < iter0[th]) iter0[th] = max;
          if(max < iter1[th]) iter1[th] = max;
          
            mpz_class v = V0;
                    
            mpz_class acc = PRIME;
            mpz_class M = iter0[th];  
            
            while (M > 0) {
                if ((M & 1) > 0) {
                    v += acc;
                    v &= (max - 1);
                }

                acc *= 2;
                acc &= (max - 1);
                M >>= 1;
            }    
            
            
            mpz_class i;
            found[th]=false;
            for (i = iter0[th]; i < iter1[th]; i++) {
                
                v += PRIME; // big prime.
                v &= (max - 1);

                rot90[th] = rotate90(v,markersize);
                rot180[th] = rotate90(rot90[th],markersize);
                rot270[th] = rotate90(rot180[th],markersize);
                
                if(hammingDistance(v,rot90[th])<minDist || hammingDistance(v,rot180[th])<minDist || hammingDistance(v,rot270[th])<minDist ) {
                    continue;
                }
                
                bool valid=true;
                for(int j=0; j<rots[0].size(); j++) {
                    if(hammingDistance(v,rots[0][j])<minDist || hammingDistance(v,rots[1][j])<minDist || hammingDistance(v,rots[2][j])<minDist || hammingDistance(v,rots[3][j])<minDist  ) {
                    valid=false;
                    break;
                    }
                }
                if(!valid) {
                    continue;
                }
                
                Marker currBMat = mpz2marker(v,markersize);
                if (computeComplexity(currBMat)>=mincomplexity )  {
                    goodCode[th] = v;
                    goodIter[th] = i;
                    goodBmat[th] = currBMat;
                    found[th] = true;
                    //std::cout << v.get_str() << std::endl;
                    break;
                }
            }                
          
      }
      
    
      iter = iter + NTHREADS*chunksize;
      if(max < iter) iter = max;    
      
      int foundIdx = -1;
      for(int i=0; i<NTHREADS; i++) {
          if(found[i]) {
              foundIdx = i;
              break;
          }
      }
      
      if(foundIdx != -1) {
        D.push_back(goodBmat[foundIdx]);
        rots[0].push_back(goodCode[foundIdx]);
        rots[1].push_back(rot90[foundIdx]);
        rots[2].push_back(rot180[foundIdx]);
        rots[3].push_back(rot270[foundIdx]);      
        iter = goodIter[foundIdx] + 1;
        
        timesFile << D.size() << " " << std::chrono::duration<double>(std::chrono::steady_clock::now()-tStart).count() << std::endl;
        if(D.size() % 20 == 0) std::cout << "  size " << D.size() << std::endl;
        tick = std::chrono::steady_clock::now();
        tick2 = tick;
        
        
      } 
      else {
          
                if( std::chrono::duration<double>(std::chrono::steady_clock::now()-tick2).count() > maxTime) {
                   minDist--;
                   std::cerr << "Reduce distance to " << minDist << std::endl;
                   tick2 = std::chrono::steady_clock::now();  
                   if(D.size() >= 2) maxTime = baseMaxTime;
                   else maxTime = std::min(10, baseMaxTime);

                }             
          
      }
      
      
     
      if(iter >= max) iter = 0;         
      
      
    }
      
        
  

  saveDictionary(D, getParam("--out", argc, argv));

  
  return 0;
}
