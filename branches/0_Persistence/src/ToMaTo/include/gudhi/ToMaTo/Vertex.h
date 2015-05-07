//----------------------------------------------------------------------
//----------------------------------------------------------------------
// File:		Vertex.h
// Programmer:		Primoz Skraba
// Description:		Main point data structure example
// Last modified:	August 10, 2009 (Version 0.1)
//----------------------------------------------------------------------
//  Copyright (c) 2009 Primoz Skraba.  All Rights Reserved.
//-----------------------------------------------------------------------
//
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//
//-----------------------------------------------------------------------

#ifndef SRC_TOMATO_INCLUDE_GUDHI_TOMATO_VERTEX__H_
#define SRC_TOMATO_INCLUDE_GUDHI_TOMATO_VERTEX__H_


#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>

//
//basic templated class of
//vertex class

template<class Point_Class,
template <typename S, typename D = std::allocator<S> >
class Container = std::vector>
class Vertex {
 public:
  // this is so that you can template via the container
  // although if you want to use set you must
  // change the above template definition to take
  // the set template (i.e. include a partial ordering)
  // as template template parameters cannot inherit defaults
  // though multiple levels 
  typedef typename Container<Vertex>::iterator Iterator;

 protected:
  Iterator sink;
  double f;

 public:
  typedef struct {

    bool operator()(const Vertex a, const Vertex b) const {
      if (a.func() > b.func()) return true;
      else if (a.func() < b.func()) return false;
      else return typename Point_Class::Less_Than()(a.geometry, b.geometry);
    }
  } Less_Than;

  //store geometry
  Point_Class geometry;

  Vertex() { }

  Vertex(Point_Class p) {
    geometry = p;
  }

  //set function value
  // when initializing
  Vertex(double f_value) {
    f = f_value;
  }

  void set_sink(Iterator x) {
    sink = x;
  }

  //return iterator
  Iterator get_sink() const {
    return sink;
  }

  //set function value
  void set_func(double x) {
    f = x;
  }
  
  //return function value
  double func() const {
    return f;
  }

  //difference operator
  double operator-(const Vertex &B) const {
    return this->func() - B->func();
  }

};

#endif  // SRC_TOMATO_INCLUDE_GUDHI_TOMATO_VERTEX__H_
