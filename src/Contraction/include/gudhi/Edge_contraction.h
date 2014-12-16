/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       David Salinas
 *
 *    Copyright (C) 2014  INRIA Sophia Antipolis-Mediterranee (France)
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

#ifndef GUDHI_EDGE_CONTRACTION_H_
#define GUDHI_EDGE_CONTRACTION_H_


#include "gudhi/Skeleton_blocker_contractor.h"
#include "gudhi/Contraction/policies/Edge_length_cost.h"
#include "gudhi/Contraction/policies/First_vertex_placement.h"
#include "gudhi/Contraction/policies/Valid_contraction_policy.h"
#include "gudhi/Contraction/policies/Dummy_valid_contraction.h"
#include "gudhi/Contraction/policies/Link_condition_valid_contraction.h"
#include "gudhi/Utils.h"



/** \defgroup contr Contraction

\author David Salinas

\section Introduction

The purpose of this package is to offer a user-friendly interface for edge contraction simplification of huge simplicial complexes.
It uses the Skeleton-Blocker data-structure whose size remains small  during simplification 
of most used geometrical complexes in topological data analysis such as the Rips or the Delaunay complexes (much lower than the total number of simplices in practice).

The edge contraction operation consists in identifying two vertices of a simplicial complex. 
Several algorithms have been developed in computer graphics that allows to reduce efficiently the size of a
simplicial complex while preserving its geometry 
\cite Garland \cite Lindstrom.
These approaches can be extended to higher-dimensional simplicial complexes. 
The main advantage of using the Skeleton-Blocker data structure for edge contraction is that when the number of blockers is small, 
most operations needed (link computation, edge contraction and so on) have polynomial complexity regarding the size the graph. 
The simplification can be done without enumerating the set of simplices that is often non tracktable in high-dimension and is then very efficient
(sub-linear with regards to the number of simplices in practice).

A typical application of this package for homology group computation and is illustrated in the next three figure where a Rips is built uppon a set of high-dimensional points.
It has initially a huge number of simplices (todo) but simplifying it to a much reduced form with todo vertices takes only few seconds on a desktop machine.
One can then compute homology group with a simplicial complex of less than one hundred simplices instead of running the homology algorithm on the much bigger initial set of 
simplices.



\section Design

This class design is policy based and heavily inspired from the similar edge collapse package of CGAL http://doc.cgal.org/latest/Surface_mesh_simplification/index.html (which is restricted to 2D triangulations).


\subsection Policies

Four policies can be customized in this package.

\li Cost_policy: specify how much cost an edge contraction of a given edge. The edge with lowest cost is iteratively picked and contracted if valid.
\li Valid_policy: specify if a given edge contraction is valid. For instance, this policy can check the link condition which ensures that the homotopy type is preserved afer the edge contraction.
\li Placement: every time an edge is contracted, its points are merge to one point specified by this policy. This may be the middle of the edge of some more sofisticated point such as the minimum of a cost as in 
\cite Garland.


\subsection Visitor

A visitor which implements the class Contraction_visitor gets called when several 





\section Example

 
This example loads points in an off file and build the Rips complex with an user provided parameter. It then simplifies the build Rips complex
while ensuring that homotopy type is preserved during the contraction (edge are contracted only when the link condition is valid).

  \code{.cpp}
#include <boost/timer/timer.hpp>
#include <iostream>
#include "gudhi/Edge_contraction.h"
#include "gudhi/Skeleton_blocker.h"
#include "gudhi/Off_reader.h"


using namespace std;
using namespace Gudhi;
using namespace skbl;
using namespace contraction;

struct Geometry_trait{
	typedef std::vector<double> Point;
};

typedef Geometry_trait::Point Point;
typedef Skeleton_blocker_simple_geometric_traits<Geometry_trait> Complex_geometric_traits;
typedef Skeleton_blocker_geometric_complex< Complex_geometric_traits > Complex;
typedef Edge_profile<Complex> Profile;
typedef Skeleton_blocker_contractor<Complex> Complex_contractor;

template<typename Point>
double eucl_distance(const Point& a,const Point& b){
	double res = 0;
	auto a_coord = a.begin();
	auto b_coord = b.begin();
	for(; a_coord != a.end(); a_coord++, b_coord++){
		res += (*a_coord - *b_coord) * (*a_coord - *b_coord);
	}
	return sqrt(res);
}

template<typename ComplexType>
void build_rips(ComplexType& complex, double offset){
	if (offset<=0) return;
	auto vertices = complex.vertex_range();
	for (auto p = vertices.begin(); p != vertices.end(); ++p)
		for (auto q = p; ++q != vertices.end(); )
			if (eucl_distance(complex.point(*p), complex.point(*q)) < 2 * offset)
				complex.add_edge(*p,*q);
}

int main (int argc, char *argv[])
{
	if (argc!=3){
		std::cerr << "Usage "<<argv[0]<<" GUDHIPATH/src/data/sphere3D.off 0.1 to load the file GUDHIPATH/src/data/sphere3D.off and contract the Rips complex built with paremeter 0.2.\n";
		return -1;
	}

	boost::timer::auto_cpu_timer t;
	Complex complex;

	// load the points
	Skeleton_blocker_off_reader<Complex> off_reader(argv[1],complex,true);
	if(!off_reader.is_valid()){
		std::cerr << "Unable to read file:"<<argv[1]<<std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "build the Rips complex"<<std::endl;

	build_rips(complex,atof(argv[2]));


	std::cout << "Initial complex has "<<
			complex.num_vertices()<<" vertices, and "<<
			complex.num_edges()<<" edges."<<std::endl;

	Complex_contractor contractor(complex,
			new Edge_length_cost<Profile>, 
			contraction::make_first_vertex_placement<Profile>(),
			contraction::make_link_valid_contraction<Profile>(),
			contraction::make_remove_popable_blockers_visitor<Profile>());
	contractor.contract_edges();

	std::cout << "Resulting complex has "<<
			complex.num_vertices()<<" vertices, "<<
			complex.num_edges()<<"edges and "<<
			complex.num_blockers()<<" blockers"<<std::endl;

	return EXIT_SUCCESS;
}
  \endcode




\copyright GNU General Public License v3.                         
\verbatim  Contact: David Salinas,     david.salinas@inria.fr \endverbatim
*/
/** @} */  // end defgroup 



#endif /* GUDHI_EDGE_CONTRACTION_H_ */
