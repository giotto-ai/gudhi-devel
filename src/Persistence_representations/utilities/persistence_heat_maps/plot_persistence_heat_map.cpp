/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Pawel Dlotko
 *
 *    Copyright (C) 2016  INRIA (France)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gudhi/Persistence_heat_maps.h>

#include <iostream>
#include <sstream>

using constant_scaling_function = Gudhi::Persistence_representations::constant_scaling_function;
using Persistence_heat_maps = Gudhi::Persistence_representations::Persistence_heat_maps<constant_scaling_function>;

int main(int argc, char** argv) {
  std::cout << "This program creates a gnuplot script from a persistence heat maps stored in a file (the file needs "
            << "to be created beforehand). Please call the code with the name of a single heat maps file \n";
  if (argc != 2) {
    std::cout << "Wrong parameter list, the program will now terminate \n";
    return 1;
  }
  Persistence_heat_maps l;
  l.load_from_file(argv[1]);
  l.plot(argv[1]);
  return 0;
}