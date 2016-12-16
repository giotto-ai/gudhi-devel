/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Siargey Kachanovich
 *
 *    Copyright (C) 2015  INRIA (France)
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

#ifndef STRONG_WITNESS_COMPLEX_H_
#define STRONG_WITNESS_COMPLEX_H_

#include <utility>
#include <vector>
#include <list>
#include <limits>

#include <gudhi/Active_witness/Active_witness.h>
#include <gudhi/Kd_tree_search.h>

namespace gss = Gudhi::spatial_searching;

namespace Gudhi {
  
namespace witness_complex {

/**
 *  \private
 * \class Strong_witness_complex
 * \brief Constructs strong witness complex for the given sets of witnesses and landmarks.
 * \ingroup witness_complex
 *
 * \tparam Kernel_ requires a <a target="_blank"
 * href="http://doc.cgal.org/latest/Kernel_d/classCGAL_1_1Epick__d.html">CGAL::Epick_d</a> class, which
 * can be static if you know the ambiant dimension at compile-time, or dynamic if you don't.
 * \tparam DimensionTag can be either <a target="_blank"
 * href="http://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Dimension__tag.html">Dimension_tag<d></a>
 * if you know the intrinsic dimension at compile-time,
 * or <a target="_blank"
 * href="http://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Dynamic__dimension__tag.html">CGAL::Dynamic_dimension_tag</a>
 * if you don't.
 */
template< class Kernel_ >
class Strong_witness_complex {
private:
  typedef Kernel_                                                 K;
  typedef typename K::Point_d                                     Point_d;
  typedef typename K::FT                                          FT;
  typedef std::vector<Point_d>                                    Point_range;
  typedef gss::Kd_tree_search<Kernel_, Point_range>               Kd_tree;
  typedef typename Kd_tree::INS_range                             Nearest_landmark_range;
  typedef typename std::vector<Nearest_landmark_range>            Nearest_landmark_table;
  typedef typename Nearest_landmark_range::iterator               Nearest_landmark_row_iterator;
  
  typedef std::vector< double > Point_t;
  typedef std::vector< Point_t > Point_Vector;

  typedef FT Filtration_value;

  
  typedef std::size_t Witness_id;
  typedef typename Nearest_landmark_range::Point_with_transformed_distance Id_distance_pair;
  typedef typename Id_distance_pair::first_type Landmark_id;
  typedef Active_witness<Id_distance_pair, Nearest_landmark_range> ActiveWitness;
  typedef std::list< ActiveWitness > ActiveWitnessList;
  typedef std::vector< Landmark_id > typeVectorVertex;
  typedef std::pair< typeVectorVertex, Filtration_value> typeSimplex;

  typedef Landmark_id Vertex_handle;
  
 private:
  Point_range                         witnesses_, landmarks_;
  Kd_tree                             landmark_tree_;
  
 public:
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* @name Constructor
   */

  //@{

  /**
   *  \brief Initializes member variables before constructing simplicial complex.
   *  \details Records landmarks from the range 'landmarks' into a 
   *           table internally, as well as witnesses from the range 'witnesses'.
   *           Both ranges should have value_type Kernel_::Point_d.
   */
  template< typename LandmarkRange,
            typename WitnessRange >
  Strong_witness_complex(const LandmarkRange & landmarks,
                         const WitnessRange &  witnesses)
    : witnesses_(witnesses), landmarks_(landmarks), landmark_tree_(landmarks_)
  {    
  }

  
  /** \brief Returns the point corresponding to the given vertex.
   */
  template <typename Vertex_handle> 
  Point_d get_point( Vertex_handle vertex ) const
  {
    return landmarks_[vertex];
  }
  
  /** \brief Outputs the strong witness complex of relaxation 'max_alpha_square' 
   *         in a simplicial complex data structure.
   *  \details The function returns true if the construction is successful and false otherwise.
   *  @param[out] complex Simplicial complex data structure, which is a model of
   *              SimplicialComplexForWitness concept.
   *  @param[in] max_alpha_square Maximal squared relaxation parameter.
   *  @param[in] limit_dimension Represents the maximal dimension of the simplicial complex
   *         (default value = no limit).
   */
  template < typename SimplicialComplexForWitness >
  bool create_complex(SimplicialComplexForWitness& complex,
                      FT  max_alpha_square,
                      Landmark_id limit_dimension = std::numeric_limits<Landmark_id>::max()-1)       
  {
    std::size_t nbL = landmarks_.size();
    Landmark_id complex_dim = 0;
    if (complex.num_vertices() > 0) {
      std::cerr << "Strong witness complex cannot create complex - complex is not empty.\n";
      return false;
    }
    if (max_alpha_square < 0) {
      std::cerr << "Strong witness complex cannot create complex - squared relaxation parameter must be non-negative.\n";
      return false;
    }
    if (limit_dimension < 0) {
      std::cerr << "Strong witness complex cannot create complex - limit dimension must be non-negative.\n";
      return false;
    }
    typeVectorVertex vv;
    for (unsigned i = 0; i != nbL; ++i) {
      // initial fill of 0-dimensional simplices
      vv = {i};
      complex.insert_simplex(vv, Filtration_value(0.0));
    }
    for (auto w: witnesses_) {
      ActiveWitness aw(landmark_tree_.query_incremental_nearest_neighbors(w));
      typeVectorVertex simplex;
      typename ActiveWitness::iterator aw_it = aw.begin();
      float lim_dist2 = aw.begin()->second + max_alpha_square;
      while ((Landmark_id)simplex.size() <= limit_dimension + 1 && aw_it != aw.end() && aw_it->second < lim_dist2) {
        simplex.push_back(aw_it->first);
        complex.insert_simplex_and_subfaces(simplex, aw_it->second - aw.begin()->second);
        aw_it++;
      }
      // continue inserting limD-faces of the following simplices
      typeVectorVertex& vertices = simplex; //'simplex' now will be called vertices
      while (aw_it != aw.end() && aw_it->second < lim_dist2) {
        typeVectorVertex facet = {};
        add_all_faces_of_dimension(limit_dimension, vertices, vertices.begin(), aw_it, aw_it->second - aw.begin()->second, facet, complex);
        vertices.push_back(aw_it->first);
        aw_it++;
      }
      if ((Landmark_id)simplex.size() - 1 > complex_dim)
        complex_dim = simplex.size() - 1;
    }
    complex.set_dimension(complex_dim);
    return true;
  }

private:

    /* \brief Adds recursively all the faces of a certain dimension dim-1 witnessed by the same witness.
   * Iterator is needed to know until how far we can take landmarks to form simplexes.
   * simplex is the prefix of the simplexes to insert.
   * The landmark pointed by aw_it is added to all formed simplices.
   */
  template < typename SimplicialComplexForWitness >
  void add_all_faces_of_dimension(Landmark_id dim,
                                  typeVectorVertex& vertices,
                                  typename typeVectorVertex::iterator curr_it,
                                  typename ActiveWitness::iterator aw_it,
                                  FT filtration_value,
                                  typeVectorVertex& simplex,
                                  SimplicialComplexForWitness& sc)
  {
    if (dim > 0)
      while (curr_it != vertices.end()) {
        simplex.push_back(*curr_it);
        typename typeVectorVertex::iterator next_it = ++curr_it;        
        add_all_faces_of_dimension(dim-1,
                                   vertices,
                                   next_it,
                                   aw_it,
                                   filtration_value,
                                   simplex,
                                   sc);
        simplex.pop_back();
        add_all_faces_of_dimension(dim,
                                   vertices,
                                   next_it,
                                   aw_it,
                                   filtration_value,
                                   simplex,
                                   sc);
      } 
    else if (dim == 0) {
      simplex.push_back(aw_it->first);
      sc.insert_simplex_and_subfaces(simplex, filtration_value);
      simplex.pop_back();
    } 
  }      
  
  //@}
};

}  // namespace witness_complex

}  // namespace Gudhi

#endif