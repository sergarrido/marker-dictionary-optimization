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

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        std::cerr << "Usage: mindistances input.set output.dist" << std::endl;
        std::cerr << "Writes one line \"size distance\" per dictionary prefix." << std::endl;
        return 1;
    }

    Dictionary D = loadDictionary(argv[1]);
    std::ofstream out(argv[2]);

    Dictionary acc;
    int dist = 0;
    for(size_t i=0; i<D.size(); i++)
    {
        int nd = getMarkerSelfDist(D[i]);
        for(size_t j=0; j<acc.size(); j++) nd = std::min(nd, getMarkerDist(D[i], acc[j]));
        dist = (i==0) ? nd : std::min(dist, nd);
        acc.push_back(D[i]);
        out << (i+1) << " " << dist << std::endl;
    }

    return 0;
}
