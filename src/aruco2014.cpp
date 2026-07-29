/*
 * This code implements the square fiducial marker generation method described in:
 * Automatic generation and detection of highly reliable fiducial markers under occlusion.
 * S. Garrido-Jurado, R. Munoz-Salinas, F. J. Madrid-Cuevas, M. J. Marin-Jimenez.
 * Pattern Recognition 2014, 47(6), 2280-2292.
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

#include <iostream>
#include <stdlib.h>
#include "common.h"
#include "math.h"
#include <omp.h>

#define NTHREADS 12



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


int main(int argc,char **argv)
{
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

    if(isParam("--seed", argc, argv)) srand( atoi(getParam("--seed", argc, argv).c_str()) );
    else srand(time(NULL));
    
    uint markerbits = markersize*markersize;
    int c = markerbits/4;
    unsigned int minDist = 2*int((4*c)/3) ;
    
    Dictionary D;
    D.reserve(nmarkers);

    std::ofstream timesFile(getParam("--times", argc, argv));
    auto tStart = std::chrono::steady_clock::now();

    auto tick = std::chrono::steady_clock::now();
    auto tick2 = tick;
    
    omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(NTHREADS);   
    
    
    int nTransitions = markersize-1;
    std::vector<int> transitionsWeigth(nTransitions);
    int totalWeigth = 0; 
    for(int i=0; i<nTransitions; i++) {
	transitionsWeigth[i] = i;
	totalWeigth += i;
    } 
	    
    //uint countUnproductive=0;
    // Time (seconds) of unproductive search before relaxing minDist.
    // Configurable via -t (default 150, as in the original paper experiments).
    int baseMaxTime = isParam("--timelimit", argc, argv) ? atoi(getParam("--timelimit", argc, argv).c_str()) : 150;
    int maxTime = baseMaxTime;
    while(D.size() < nmarkers) {
      
      Marker emptyMarker[NTHREADS][4];
      bool valid[NTHREADS];
      

      #pragma omp parallel for
      for(int th=0; th<NTHREADS; th++) {
	//emptyMarker[th][0] = getRandomMarker(markersize);
	  
	emptyMarker[th][0].resize(markersize);
	for(int w=0; w<markersize; w++) {
	    BitString bs;
	    bs.resize(markersize);
	    int randomNum = rand()%totalWeigth;
	    int currentNTransitions=nTransitions-1;
	    for(int k=0; k<nTransitions; k++) {
		if(transitionsWeigth[k] > randomNum) {
		    currentNTransitions = k;
		    break;
		}
	    }
	    std::vector<int> transitionsIndexes(nTransitions);
	    for(int i=0; i<nTransitions; i++) transitionsIndexes[i] = i;
	    for(int s=int(transitionsIndexes.size())-1; s>0; s--) std::swap(transitionsIndexes[s], transitionsIndexes[rand()%(s+1)]);

	    std::vector<int> selectedIndexes;
	    for(int k=0; k<currentNTransitions; k++) 
		selectedIndexes.push_back(transitionsIndexes[k]);
	    std::sort(selectedIndexes.begin(), selectedIndexes.end());
	    int currBit = rand()%2;
	    int currSelectedIndexesIdx = 0;
	    for(int k=0; k<markersize; k++) {
		bs[k] = currBit;
		if(currSelectedIndexesIdx<selectedIndexes.size() &&
		    k==selectedIndexes[currSelectedIndexesIdx]) {
		    currBit = 1 - currBit;
		    currSelectedIndexesIdx++;
		}
	    }
	    emptyMarker[th][0][w] = bs;
	}

	emptyMarker[th][1] = rotateMarker(emptyMarker[th][0]);
	emptyMarker[th][2] = rotateMarker(emptyMarker[th][1]);
	emptyMarker[th][3] = rotateMarker(emptyMarker[th][2]);


      	if(hamm(emptyMarker[th][0], emptyMarker[th][1])>=minDist && 
		 hamm(emptyMarker[th][0], emptyMarker[th][2])>=minDist && 
		 hamm(emptyMarker[th][0], emptyMarker[th][3])>=minDist && 
		 hamm(D,emptyMarker[th][0])>=minDist && 
		 hamm(D,emptyMarker[th][1])>=minDist &&
		 hamm(D,emptyMarker[th][2])>=minDist &&
		 hamm(D,emptyMarker[th][3])>=minDist ) {
			valid[th] = true;
	 }
	else valid[th] = false;

      }

	int validIdx = -1;
	for(int i=0; i<NTHREADS; i++) {
		if(valid[i]) {
			validIdx = i;
			break;
		}
	}

	if(validIdx != -1) {
		D.push_back(emptyMarker[validIdx][0]);
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
                   else maxTime = (baseMaxTime < 10) ? baseMaxTime : 10;

      		}   
       }
	
	

    }
    
    saveDictionary(D, getParam("--out", argc, argv));
    
//  D.saveToFile(argv[1]);


}
