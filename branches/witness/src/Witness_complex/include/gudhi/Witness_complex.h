/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Siargey Kachanovich
 *
 *    Copyright (C) 2015  INRIA Sophia Antipolis-Méditerranée (France)
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

#ifndef GUDHI_WITNESS_COMPLEX_H_
#define GUDHI_WITNESS_COMPLEX_H_

#include <boost/container/flat_map.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <algorithm>
#include <utility>
#include "gudhi/reader_utils.h"
#include "gudhi/distance_functions.h"
#include "gudhi/Simplex_tree.h"
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <limits>
#include <math.h>
#include <ctime>
#include <iostream>

// Needed for nearest neighbours
//#include <CGAL/Delaunay_triangulation.h>
//#include <CGAL/Epick_d.h>
//#include <CGAL/K_neighbor_search.h>
//#include <CGAL/Search_traits_d.h>

// Needed for the adjacency graph in bad link search
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

namespace Gudhi {


  /** \addtogroup simplex_tree
   *  Witness complex is a simplicial complex defined on two sets of points in \f$\mathbf{R}^D\f$:
   *  \f$W\f$ set of witnesses and \f$L \subseteq W\f$ set of landmarks. The simplices are based on points in \f$L\f$
   *  and a simplex belongs to the witness complex if and only if it is witnessed (there exists a point \f$w \in W\f$ such that
   *  w is closer to the vertices of this simplex than others) and all of its faces are witnessed as well. 
   */
  template<typename FiltrationValue = double,
           typename SimplexKey      = int,
           typename VertexHandle    = int>
  class Witness_complex: public Simplex_tree<> {

  private:
    
    struct Active_witness {
      int witness_id;
      int landmark_id;
      Simplex_handle simplex_handle;
      
      Active_witness(int witness_id_, int landmark_id_, Simplex_handle simplex_handle_)
        : witness_id(witness_id_),
          landmark_id(landmark_id_),
          simplex_handle(simplex_handle_)
      {}
    };
    
      


  public:
  
    
    /** \brief Type for the vertex handle.
     *
     * Must be a signed integer type. It admits a total order <. */
    typedef VertexHandle Vertex_handle;
    
    /* Type of node in the simplex tree. */ 
    typedef Simplex_tree_node_explicit_storage<Simplex_tree> Node;
    /* Type of dictionary Vertex_handle -> Node for traversing the simplex tree. */
    typedef typename boost::container::flat_map<Vertex_handle, Node> Dictionary;
    typedef typename Dictionary::iterator Simplex_handle;
  
    typedef std::vector< double > Point_t;
    typedef std::vector< Point_t > Point_Vector;
    
    typedef std::vector< Vertex_handle > typeVectorVertex;
    typedef std::pair< typeVectorVertex, Filtration_value> typeSimplex;
    typedef std::pair< Simplex_tree<>::Simplex_handle, bool > typePairSimplexBool;
    
    typedef int Witness_id;
    typedef int Landmark_id;
    typedef std::list< Vertex_handle > ActiveWitnessList;
    
  private:
    /** Number of landmarks
     */
    int nbL;
    /** Desired density
     */
    double density;

  public:

    /** \brief Set number of landmarks to nbL_
     */
    void setNbL(int nbL_)
    {
      nbL = nbL_;
    }

    /** \brief Set density to density_
     */
    void setDensity(double density_)
    {
      density = density_;
    }
    
    /**
     * /brief Iterative construction of the witness complex basing on a matrix of k nearest neighbours of the form {witnesses}x{landmarks}.
     * Landmarks are supposed to be in [0,nbL-1]
     */
    
    template< typename KNearestNeighbours >
    void witness_complex(KNearestNeighbours & knn)
    //void witness_complex(std::vector< std::vector< Vertex_handle > > & knn)
    {
      std::cout << "**Start the procedure witness_complex" << std::endl;
      int k=2; /* current dimension in iterative construction */
      //Construction of the active witness list
      int nbW = knn.size();
      //int nbL = knn.at(0).size();
      typeVectorVertex vv;
      typeSimplex simplex;
      typePairSimplexBool returnValue;
      int counter = 0;
      /* The list of still useful witnesses
       * it will diminuish in the course of iterations
       */
      ActiveWitnessList active_w;// = new ActiveWitnessList();
      for (int i=0; i != nbL; ++i) {
        // initial fill of 0-dimensional simplices
        // by doing it we don't assume that landmarks are necessarily witnesses themselves anymore
        counter++;
        vv = {i};
        /* TODO Filtration */
        returnValue = insert_simplex(vv, Filtration_value(0.0));
        /* TODO Error if not inserted : normally no need here though*/
      }
      //std::cout << "Successfully added landmarks" << std::endl;
      // PRINT2
      //print_sc(root()); std::cout << std::endl;
      int u,v;     // two extremities of an edge
      int count = 0;
      if (nbL > 1) // if the supposed dimension of the complex is >0
        {
          for (int i=0; i != nbW; ++i)
            {
              // initial fill of active witnesses list
              u = knn[i][0];
              v = knn[i][1];
              vv = {u,v};
              returnValue = this->insert_simplex(vv,Filtration_value(0.0));
              if (returnValue.second)
                count++;
              //print_sc(root()); std::cout << std::endl;
              //std::cout << "Added edges" << std::endl;
            }
          std::cout << "The number of edges = " << count << std::endl;
          count = 0;
          //print_sc(root());
          for (int i=0; i != nbW; ++i)
            {
              // initial fill of active witnesses list
              u = knn[i][0];
              v = knn[i][1];
              if ( u > v)
                {
                  u = v;
                  v = knn[i][0];
                  knn[i][0] = knn[i][1];
                  knn[i][1] = v;
                }
              Simplex_handle sh;
              vv = {u,v};
              //if (u==v) std::cout << "Bazzinga!\n";
              sh = (root()->find(u))->second.children()->find(v);
              active_w.push_back(i);
            }
        }
      std::cout << "k=1, active witnesses: " << active_w.size() << std::endl;
      //std::cout << "Successfully added edges" << std::endl;
      count_good = {0,0};
      count_bad = {0,0};
      int D = knn[0].size();
      while (!active_w.empty() && k < D )
        {
	  count_good.push_back(0);
	  count_bad.push_back(0);
          count++;
          //std::cout << "Started the step k=" << k << std::endl;
          typename ActiveWitnessList::iterator it = active_w.begin();
          while (it != active_w.end())
            {
              typeVectorVertex simplex_vector;
              /* THE INSERTION: Checking if all the subfaces are in the simplex tree*/
              // First sort the first k landmarks
              VertexHandle inserted_vertex = knn[*it][k];
              bool ok = all_faces_in(knn, *it, k, inserted_vertex);
              if (ok)
                {
                  for (int i = 0; i != k+1; ++i)
                    simplex_vector.push_back(knn[*it][i]);
                  returnValue = insert_simplex(simplex_vector,0.0);
                  if (returnValue.second)
                    count++;
                  it++;
                }
              else
                active_w.erase(it++); //First increase the iterator and then erase the previous element
            }
	  std::cout << "k=" << k << ", active witnesses: " << active_w.size() << std::endl;
          std::cout << "** k=" << k << ", num_simplices: " <<count << std::endl;
          k++;
        }
      //print_sc(root()); std::cout << std::endl;
    }

    /** \brief Construction of witness complex from points given explicitly
     *  nbL must be set to the right value of landmarks for strategies
     * FURTHEST_POINT_STRATEGY and RANDOM_POINT_STRATEGY and
     * density must be set to the right value for DENSITY_STRATEGY
     */
  // void witness_complex_from_points(Point_Vector point_vector)
  // {
  //   std::vector<std::vector< int > > WL;
  //   landmark_choice_by_random_points(point_vector, point_vector.size(), WL);
  //   witness_complex(WL);
  // }
  
private:

    /** \brief Print functions
    */
    void print_sc(Siblings * sibl)
    {
      if (sibl == NULL)
        std::cout << "&";
      else
        print_children(sibl->members_);
    }
    
    void print_children(Dictionary map)
    {
      std::cout << "(";
      if (!map.empty())
        {
          std::cout << map.begin()->first;
          if (has_children(map.begin()))
            print_sc(map.begin()->second.children());
          typename Dictionary::iterator it;
          for (it = map.begin()+1; it != map.end(); ++it)
            {
              std::cout << "," << it->first;
              if (has_children(it))
                print_sc(it->second.children());
            }
        }
      std::cout << ")";
    }

  public:
    /** \brief Print functions
     */

    void st_to_file(std::ofstream& out_file)
    {
      sc_to_file(out_file, root());
    }

  private:
    void sc_to_file(std::ofstream& out_file, Siblings * sibl)
    {
      assert(sibl);
      children_to_file(out_file, sibl->members_);
    }
    
    void children_to_file(std::ofstream& out_file, Dictionary& map)
    {
      out_file << "(" << std::flush;
      if (!map.empty())
        {
          out_file << map.begin()->first << std::flush;
          if (has_children(map.begin()))
            sc_to_file(out_file, map.begin()->second.children());
          typename Dictionary::iterator it;
          for (it = map.begin()+1; it != map.end(); ++it)
            {
              out_file << "," << it->first << std::flush;
              if (has_children(it))
                sc_to_file(out_file, it->second.children());
            }
        }
      out_file << ")" << std::flush;
    }


    /** \brief Check if the facets of the k-dimensional simplex witnessed 
     *  by witness witness_id are already in the complex.
     *  inserted_vertex is the handle of the (k+1)-th vertex witnessed by witness_id
     */
    template <typename KNearestNeighbours>
    bool all_faces_in(KNearestNeighbours &knn, int witness_id, int k, VertexHandle inserted_vertex)
    {
      //std::cout << "All face in with the landmark " << inserted_vertex << std::endl;
      std::vector< VertexHandle > facet;
      //VertexHandle curr_vh = curr_sh->first;
      // CHECK ALL THE FACETS
      for (int i = 0; i != k+1; ++i)
        {
          if (knn[witness_id][i] != inserted_vertex)
            {
              facet = {};
              for (int j = 0; j != k+1; ++j)
                {
                  if (j != i)
                    {
                      facet.push_back(knn[witness_id][j]);
                    }
                }//endfor
              if (find(facet) == null_simplex())
                return false;
              //std::cout << "++++ finished loop safely\n";
            }//endif
        } //endfor
      return true;
    }
    
    template <typename T>
    void print_vector(std::vector<T> v)
    {
      std::cout << "[";
      if (!v.empty())
        {
          std::cout << *(v.begin());
          for (auto it = v.begin()+1; it != v.end(); ++it)
            {
              std::cout << ",";
              std::cout << *it;
            }
      }
      std::cout << "]";
    }
    
    template <typename T>
    void print_vvector(std::vector< std::vector <T> > vv)
    {
      std::cout << "[";
      if (!vv.empty())
        {
          print_vector(*(vv.begin()));
          for (auto it = vv.begin()+1; it != vv.end(); ++it)
            {
              std::cout << ",";
              print_vector(*it);
            }
        }
      std::cout << "]\n";
    }

  public:
/**
 * \brief Landmark choice strategy by iteratively adding the landmark the furthest from the
 * current landmark set
 * \arg W is the vector of points which will be the witnesses
 * \arg nbP is the number of witnesses
 * \arg nbL is the number of landmarks
 * \arg WL is the matrix of the nearest landmarks with respect to witnesses (output)
 */

  template <typename KNearestNeighbours>
    void landmark_choice_by_furthest_points(Point_Vector &W, int nbP, KNearestNeighbours &WL)
  {
    //std::cout << "Enter landmark_choice_by_furthest_points "<< std::endl;
    //std::cout << "W="; print_vvector(W);
    //double density = 5.;
    Point_Vector wit_land_dist(nbP,std::vector<double>());    // distance matrix witness x landmarks
    typeVectorVertex  chosen_landmarks;                       // landmark list

    WL = KNearestNeighbours(nbP,std::vector<int>());                             
    int current_number_of_landmarks=0;                        // counter for landmarks 
    double curr_max_dist = 0;                                 // used for defining the furhest point from L
    double curr_dist;                                         // used to stock the distance from the current point to L
    double infty = std::numeric_limits<double>::infinity();   // infinity (see next entry)
    std::vector< double > dist_to_L(nbP,infty);               // vector of current distances to L from points
    // double mindist = infty;
    int curr_max_w=0;                                         // the point currently furthest from L 
    int j;
    int temp_swap_int;                                        
    double temp_swap_double;

    //CHOICE OF THE FIRST LANDMARK
    std::cout << "Enter the first landmark stage\n";
    srand(354698);
    int rand_int = rand()% nbP;
    curr_max_w = rand_int; //For testing purposes a pseudo-random number is used here

    for (current_number_of_landmarks = 0; current_number_of_landmarks != nbL; current_number_of_landmarks++)
      {
        //curr_max_w at this point is the next landmark
        chosen_landmarks.push_back(curr_max_w);
        //std::cout << "**********Entered loop with current number of landmarks = " << current_number_of_landmarks << std::endl;
        //std::cout << "WL="; print_vvector(WL);
        //std::cout << "WLD="; print_vvector(wit_land_dist);
        //std::cout << "landmarks="; print_vector(chosen_landmarks); std::cout << std::endl;
        for (auto v: WL)
          v.push_back(current_number_of_landmarks);
        for (int i = 0; i < nbP; ++i)
          {
            // iteration on points in W. update of distance vectors
            
            //std::cout << "In the loop with i=" << i << " and landmark=" << chosen_landmarks[current_number_of_landmarks] << std::endl;
            //std::cout << "W[i]="; print_vector(W[i]); std::cout << " W[landmark]="; print_vector(W[chosen_landmarks[current_number_of_landmarks]]); std::cout << std::endl;
            curr_dist = euclidean_distance(W[i],W[chosen_landmarks[current_number_of_landmarks]]);
            //std::cout << "The problem is not in distance function\n";
            wit_land_dist[i].push_back(curr_dist);
            WL[i].push_back(current_number_of_landmarks);
            //std::cout << "Push't back\n";
            if (curr_dist < dist_to_L[i])
              dist_to_L[i] = curr_dist;
            j = current_number_of_landmarks;
            //std::cout << "First half complete\n";
            while (j > 0 && wit_land_dist[i][j-1] > wit_land_dist[i][j])
              {
                // sort the closest landmark vector for every witness
                temp_swap_int = WL[i][j];
                WL[i][j] = WL[i][j-1];
                WL[i][j-1] = temp_swap_int;
                temp_swap_double = wit_land_dist[i][j];
                wit_land_dist[i][j] = wit_land_dist[i][j-1];
                wit_land_dist[i][j-1] = temp_swap_double;
                --j;
              }
            //std::cout << "result WL="; print_vvector(WL);
            //std::cout << "result WLD="; print_vvector(wit_land_dist);
            //std::cout << "result distL="; print_vector(dist_to_L); std::cout << std::endl;
            //std::cout << "End loop\n";
          }
        //std::cout << "Distance to landmarks="; print_vector(dist_to_L); std::cout << std::endl;
        curr_max_dist = 0;
        for (int i = 0; i < nbP; ++i) {
          if (dist_to_L[i] > curr_max_dist)
            {
              curr_max_dist = dist_to_L[i];
              curr_max_w = i;
            }
        }
        //std::cout << "Chose " << curr_max_w << " as new landmark\n";
      }
    //std::cout << endl;
  }

    /** \brief Landmark choice strategy by taking random vertices for landmarks.
     *
     */

    // template <typename KNearestNeighbours>
    // void landmark_choice_by_random_points(Point_Vector &W, int nbP, KNearestNeighbours &WL)
    // {
    //   std::cout << "Enter landmark_choice_by_random_points "<< std::endl;
    //   //std::cout << "W="; print_vvector(W);
    //   std::unordered_set< int >  chosen_landmarks;              // landmark set

    //   Point_Vector wit_land_dist(nbP,std::vector<double>());    // distance matrix witness x landmarks

    //   WL = KNearestNeighbours(nbP,std::vector<int>());                             
    //   int current_number_of_landmarks=0;                        // counter for landmarks 

    //   srand(24660);
    //   int chosen_landmark = rand()%nbP;
    //   double curr_dist;
      
    //   //int j;
    //   //int temp_swap_int;                                        
    //   //double temp_swap_double;

      
    //   for (current_number_of_landmarks = 0; current_number_of_landmarks != nbL; current_number_of_landmarks++)
    //     {
    //       while (chosen_landmarks.find(chosen_landmark) != chosen_landmarks.end())
    //         {
    //           srand((int)clock());
    //           chosen_landmark = rand()% nbP;
    //           //std::cout << chosen_landmark << "\n";
    //         }
    //       chosen_landmarks.insert(chosen_landmark);
    //       //std::cout << "**********Entered loop with current number of landmarks = " << current_number_of_landmarks << std::endl;
    //       //std::cout << "WL="; print_vvector(WL);
    //       //std::cout << "WLD="; print_vvector(wit_land_dist);
    //       //std::cout << "landmarks="; print_vector(chosen_landmarks); std::cout << std::endl;
    //       for (auto v: WL)
    //         v.push_back(current_number_of_landmarks);
    //       for (int i = 0; i < nbP; ++i)
    //         {
    //           // iteration on points in W. update of distance vectors
              
    //           //std::cout << "In the loop with i=" << i << " and landmark=" << chosen_landmarks[current_number_of_landmarks] << std::endl;
    //           //std::cout << "W[i]="; print_vector(W[i]); std::cout << " W[landmark]="; print_vector(W[chosen_landmarks[current_number_of_landmarks]]); std::cout << std::endl;
    //           curr_dist = euclidean_distance(W[i],W[chosen_landmark]);
    //           //std::cout << "The problem is not in distance function\n";
    //           wit_land_dist[i].push_back(curr_dist);
    //           WL[i].push_back(current_number_of_landmarks);
    //           //std::cout << "Push't back\n";
    //           //j = current_number_of_landmarks;
    //           //std::cout << "First half complete\n";
    //           //std::cout << "result WL="; print_vvector(WL);
    //           //std::cout << "result WLD="; print_vvector(wit_land_dist);
    //           //std::cout << "End loop\n";
    //         }
    //     }
    //   for (int i = 0; i < nbP; i++)
    //     {
    //       sort(WL[i].begin(), WL[i].end(), [&](int j1, int j2){return wit_land_dist[i][j1] < wit_land_dist[i][j2];});
    //     }
    //   //std::cout << endl;
    // }

    /** \brief Landmark choice strategy by taking random vertices for landmarks.
     *
     */

    //    template <typename KNearestNeighbours>
    void landmark_choice_by_random_points(Point_Vector &W, int nbP, std::set<int> &L)
    {
      std::cout << "Enter landmark_choice_by_random_points "<< std::endl;
      //std::cout << "W="; print_vvector(W);
      //std::unordered_set< int >  chosen_landmarks;              // landmark set

      //Point_Vector wit_land_dist(nbP,std::vector<double>());    // distance matrix witness x landmarks

      //WL = KNearestNeighbours(nbP,std::vector<int>());                             
      int current_number_of_landmarks=0;                        // counter for landmarks 

      srand(24660);
      int chosen_landmark = rand()%nbP;
      //double curr_dist;
      //int j;
      //int temp_swap_int;                                        
      //double temp_swap_double; 
      for (current_number_of_landmarks = 0; current_number_of_landmarks != nbL; current_number_of_landmarks++)
        {
          while (L.find(chosen_landmark) != L.end())
            {
              srand((int)clock());
              chosen_landmark = rand()% nbP;
              //std::cout << chosen_landmark << "\n";
            }
          L.insert(chosen_landmark);
          //std::cout << "**********Entered loop with current number of landmarks = " << current_number_of_landmarks << std::endl;
          //std::cout << "WL="; print_vvector(WL);
          //std::cout << "WLD="; print_vvector(wit_land_dist);
          //std::cout << "landmarks="; print_vector(chosen_landmarks); std::cout << std::endl;
          // for (auto v: WL)
          //   v.push_back(current_number_of_landmarks);
          // for (int i = 0; i < nbP; ++i)
          //   {
          //     // iteration on points in W. update of distance vectors
              
          //     //std::cout << "In the loop with i=" << i << " and landmark=" << chosen_landmarks[current_number_of_landmarks] << std::endl;
          //     //std::cout << "W[i]="; print_vector(W[i]); std::cout << " W[landmark]="; print_vector(W[chosen_landmarks[current_number_of_landmarks]]); std::cout << std::endl;
          //     curr_dist = euclidean_distance(W[i],W[chosen_landmark]);
          //     //std::cout << "The problem is not in distance function\n";
          //     wit_land_dist[i].push_back(curr_dist);
          //     WL[i].push_back(current_number_of_landmarks);
          //     //std::cout << "Push't back\n";
          //     //j = current_number_of_landmarks;
          //     //std::cout << "First half complete\n";
          //     //std::cout << "result WL="; print_vvector(WL);
          //     //std::cout << "result WLD="; print_vvector(wit_land_dist);
          //     //std::cout << "End loop\n";
          //   }
        }
      // for (int i = 0; i < nbP; i++)
      //   {
      //     sort(WL[i].begin(), WL[i].end(), [&](int j1, int j2){return wit_land_dist[i][j1] < wit_land_dist[i][j2];});
      //   }
      //std::cout << endl;
    }

    
    /** \brief Construct the matrix |W|x(D+1) of D+1 closest landmarks
     *  where W is the set of witnesses and D is the ambient dimension
     */
    template <typename KNearestNeighbours>
    void nearest_landmarks(Point_Vector &W, std::set<int> &L, KNearestNeighbours &WL)
    {
      int D = W[0].size();
      int nbP = W.size();
      WL = KNearestNeighbours(nbP,std::vector<int>());
      typedef std::pair<double,int> dist_i;
      typedef bool (*comp)(dist_i,dist_i);
      for (int W_i = 0; W_i < nbP; W_i++)
        {
          //std::cout << "<<<<<<<<<<<<<<" << W_i <<"\n"; 
          std::priority_queue<dist_i, std::vector<dist_i>, comp> l_heap([&](dist_i j1, dist_i j2){return j1.first > j2.first;});
          std::set<int>::iterator L_it;
          int L_i;
          for (L_it = L.begin(), L_i=0; L_it != L.end(); L_it++, L_i++)
            {
              dist_i dist = std::make_pair(euclidean_distance(W[W_i],W[*L_it]), L_i);
              l_heap.push(dist);
            }
          for (int i = 0; i < D+1; i++)
            {
              dist_i dist = l_heap.top();
              WL[W_i].push_back(dist.second);
              //WL[W_i].insert(WL[W_i].begin(),dist.second);  
              //std::cout << dist.first << " " << dist.second << std::endl;
              l_heap.pop();
            }
        }
    }

    /** \brief Returns true if the link is good
     */
  bool has_good_link(Vertex_handle v, std::vector< int >& bad_count, std::vector< int >& good_count)
    {
      std::vector< Vertex_handle > star_vertices;
      // Fill star_vertices
      star_vertices.push_back(v);
      for (auto u: complex_vertex_range())
        {
          typeVectorVertex edge = {u,v};
          if (u != v && find(edge) != null_simplex())   
            star_vertices.push_back(u);
        }
      // Find the dimension
      typeVectorVertex init_simplex = {star_vertices[0]};
      bool is_pure = true;
      std::vector<int> dim_coface(star_vertices.size(), 1);
      int d = star_dim(star_vertices, star_vertices.begin()+1, 0, init_simplex, dim_coface.begin()+1) - 1; //link_dim = star_dim - 1
      assert(init_simplex.size() == 1);
      if (!is_pure)
        std::cout << "Found an impure star around " << v << "\n";
      for (int dc: dim_coface)
        is_pure = (dc == dim_coface[0]);
      /*
      if (d == count_good.size())
        {
          std::cout << "Found a star of dimension " << (d+1) << " around " << v << "\nThe star is ";
          print_vector(star_vertices); std::cout << std::endl;
        }
      */
      //if (d == -1) bad_count[0]++;
      bool b= (is_pure && link_is_pseudomanifold(star_vertices,d));
      if (d != -1) {if (b) good_count[d]++; else bad_count[d]++;}
      if (!is_pure) bad_count[0]++;
      return (d != -1 && b && is_pure);
      
    }

    /** \brief Search and output links around vertices that are not pseudomanifolds
     *
     */
    /*
    void write_bad_links(std::ofstream& out_file)
    {
      out_file << "Bad links list\n";
      std::cout << "Entered write_bad_links\n";
      for (auto v: complex_vertex_range())
        {
          std::cout << "Vertex " << v << ": ";
          std::vector< Vertex_handle > link_vertices;
          // Fill link_vertices
          for (auto u: complex_vertex_range())
            {
              typeVectorVertex edge = {u,v};
              if (u != v && find(edge) != null_simplex())   
                link_vertices.push_back(u);
            }
          
          print_vector(link_vertices);
          std::cout << "\n";
          
          // Find the dimension
          typeVectorVertex empty_simplex = {};
          int d = link_dim(link_vertices, link_vertices.begin(),-1, empty_simplex);
          if (link_is_pseudomanifold(link_vertices,d))
            count_good[d]++;
        }
      nc = nbL;
      for (unsigned int i = 0; i != count_good.size(); i++)
	{
	  out_file << "count_good[" << i << "] = " << count_good[i] << std::endl;
	  nc -= count_good[i];
	  if (count_good[i] != 0)
	    std::cout << "count_good[" << i << "] = " << count_good[i] << std::endl;
	}
      for (unsigned int i = 0; i != count_bad.size(); i++)
	{
	  out_file << "count_bad[" << i << "] = " << count_bad[i] << std::endl;
	  nc -= count_bad[i];
	  if (count_bad[i] != 0)
	    std::cout << "count_bad[" << i << "] = " << count_bad[i] << std::endl;
	}
      std::cout << "not_connected = " << nc << std::endl;
    }
    */
  private:

  std::vector<int> count_good;
  std::vector<int> count_bad;
  int nc;

    int star_dim(std::vector< Vertex_handle >& star_vertices,
                 typename std::vector< Vertex_handle >::iterator curr_v,
                 int curr_d,
                 typeVectorVertex& curr_simplex,
                 typename std::vector< int >::iterator curr_dc)
    {
      //std::cout << "Entered star_dim for " << *(curr_v-1) << "\n";
      Simplex_handle sh;
      int final_d = curr_d;
      typename std::vector< Vertex_handle >::iterator it;
      typename std::vector< Vertex_handle >::iterator dc_it;
      //std::cout << "Current vertex is " <<  
      for (it = curr_v, dc_it = curr_dc; it != star_vertices.end(); ++it, ++dc_it)
        {
          curr_simplex.push_back(*it);
          typeVectorVertex curr_simplex_copy(curr_simplex);
          /*
          std::cout << "Searching for ";
          print_vector(curr_simplex);
          std::cout << " curr_dim " << curr_d << " final_dim " << final_d;
          */
          sh = find(curr_simplex_copy); //Need a copy because find sorts the vector and I want star center to be the first
          if (sh != null_simplex())
            {
              //std::cout << " -> " << *it << "\n";
              int d = star_dim(star_vertices, it+1, curr_d+1, curr_simplex, dc_it);
              if (d >= final_d)
                {
                  final_d = d;
                  //std::cout << d << " ";
                  //print_vector(curr_simplex);
		  //std::cout << std::endl;
                }
              if (d >= *dc_it)
                *dc_it = d;
            }
          /*
          else
            std::cout << "\n";
          */
          curr_simplex.pop_back();
        }
      return final_d;
    }

    // color is false is a (d-1)-dim face, true is a d-dim face 
    //typedef bool Color;
    // graph is an adjacency list
    typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Adj_graph;
    // map that gives to a certain simplex its node in graph and its dimension
    //typedef std::pair<boost::vecS,Color> Reference;
    typedef boost::graph_traits<Adj_graph>::vertex_descriptor Vertex_t;
    typedef boost::graph_traits<Adj_graph>::edge_descriptor Edge_t;
    typedef boost::graph_traits<Adj_graph>::adjacency_iterator Adj_it;
    typedef std::pair<Adj_it, Adj_it> Out_edge_it;

    typedef boost::container::flat_map<Simplex_handle, Vertex_t> Graph_map;
    typedef boost::container::flat_map<Vertex_t, Simplex_handle> Inv_graph_map;

    /* \brief Verifies if the simplices formed by vertices given by link_vertices 
     * form a pseudomanifold.
     * The idea is to make a bipartite graph, where vertices are the d- and (d-1)-dimensional
     * faces and edges represent adjacency between them.
     */
    bool link_is_pseudomanifold(std::vector< Vertex_handle >& star_vertices,
                                int dimension)
    {
      Adj_graph adj_graph;
      Graph_map d_map, f_map; // d_map = map for d-dimensional simplices
                              // f_map = map for its facets
      typeVectorVertex init_vector = {};
      add_vertices_to_link_graph(star_vertices,
                                 star_vertices.begin()+1,
                                 adj_graph,
                                 d_map,
                                 f_map,
                                 init_vector,
                                 0, dimension);
      //std::cout << "DMAP_SIZE: " << d_map.size() << "\n";
      //std::cout << "FMAP_SIZE: " << f_map.size() << "\n";
      add_edges_to_link_graph(adj_graph, d_map, f_map);
      for (auto f_map_it : f_map)
        {
          //std::cout << "Degree of " << f_map_it.first->first << " is " << boost::out_degree(f_map_it.second, adj_graph) << "\n";
          if (boost::out_degree(f_map_it.second, adj_graph) != 2)
	    {
              /*
              if (boost::out_degree(f_map_it.second, adj_graph) >= 3)
                {		 
                  std::cout << "This simplex has 3+ cofaces: ";
                  for(auto v : simplex_vertex_range(f_map_it.first))
                    std::cout << v << " ";
                  std::cout << std::endl;
                  Adj_it ai, ai_end; 
                  for (std::tie(ai, ai_end) = boost::adjacent_vertices(f_map_it.second, adj_graph); ai != ai_end; ++ai)
                    {
                      
                    }
                }
              */
	      count_bad[dimension]++;
	      return false;
	    }
        }
      // At this point I know that all (d-1)-simplices are adjacent to exactly 2 d-simplices
      // What is left is to check the connexity
      //std::vector<int> components(boost::num_vertices(adj_graph));
      return true; //Forget the connexity
      //return (boost::connected_components(adj_graph, &components[0]) == 1);
    }

  public:
bool complex_is_pseudomanifold(int dimension)
    {
      Adj_graph adj_graph;
      Graph_map d_map, f_map; // d_map = map for d-dimensional simplices
                              // f_map = map for its facets
      Inv_graph_map inv_d_map;
      typeVectorVertex init_vector = {};
      std::vector<int> star_vertices;
      for (int v: complex_vertex_range())
        star_vertices.push_back(v);
      add_max_simplices_to_graph(star_vertices,
                                 star_vertices.begin(),
                                 adj_graph,
                                 d_map,
                                 f_map,
                                 inv_d_map,
                                 init_vector,
                                 0, dimension);
      std::cout << "DMAP_SIZE: " << d_map.size() << "\n";
      std::cout << "FMAP_SIZE: " << f_map.size() << "\n";
      add_edges_to_link_graph(adj_graph, d_map, f_map);
      for (auto f_map_it : f_map)
        {
          //std::cout << "Degree of " << f_map_it.first->first << " is " << boost::out_degree(f_map_it.second, adj_graph) << "\n";
          if (boost::out_degree(f_map_it.second, adj_graph) != 2)
	    {
              if (boost::out_degree(f_map_it.second, adj_graph) >= 3)
                {		 
                  std::cout << "This simplex has 3+ cofaces: ";
                  for(auto v : simplex_vertex_range(f_map_it.first))
                    std::cout << v << " ";
                  std::cout << std::endl;
                  Adj_it ai, ai_end; 
                  for (std::tie(ai, ai_end) = boost::adjacent_vertices(f_map_it.second, adj_graph); ai != ai_end; ++ai)
                    {
                      auto it = inv_d_map.find(*ai);
                      assert (it != inv_d_map.end());
                      Simplex_handle sh = it->second;
                      for(auto v : simplex_vertex_range(sh))
                        std::cout << v << " ";
                      std::cout << std::endl;
                    }
                }
	      count_bad[dimension]++;
	      return false;
	    }
        }
      // At this point I know that all (d-1)-simplices are adjacent to exactly 2 d-simplices
      // What is left is to check the connexity
      //std::vector<int> components(boost::num_vertices(adj_graph));
      return true; //Forget the connexity
      //return (boost::connected_components(adj_graph, &components[0]) == 1);
    }

  private:
    void add_vertices_to_link_graph(typeVectorVertex& star_vertices,
                                    typename typeVectorVertex::iterator curr_v,
                                    Adj_graph& adj_graph,
                                    Graph_map& d_map,
                                    Graph_map& f_map,
                                    typeVectorVertex& curr_simplex,
                                    int curr_d,
                                    int link_dimension)
    {
      Simplex_handle sh;
      Vertex_t vert;
      typename typeVectorVertex::iterator it;
      //std::pair<typename Graph_map::iterator,bool> resPair;
      //typename Graph_map::iterator resPair;
      //Add vertices
      //std::cout << "Entered add vertices\n";
      for (it = curr_v; it != star_vertices.end(); ++it)
        {
          curr_simplex.push_back(*it);               //push next vertex in question
          curr_simplex.push_back(star_vertices[0]);  //push the center of the star
          /*
          std::cout << "Searching for ";
          print_vector(curr_simplex);
          std::cout << " curr_dim " << curr_d << " d " << dimension << "";
          */
          typeVectorVertex curr_simplex_copy(curr_simplex);
          sh = find(curr_simplex_copy);                   //a simplex of the star
          curr_simplex.pop_back();                   //pop the center of the star
          curr_simplex_copy = typeVectorVertex(curr_simplex);
          if (sh != null_simplex())
            {
              //std::cout << " added\n";
              if (curr_d == link_dimension)
                {
                  sh = find(curr_simplex_copy);               //a simplex of the link
                  assert(sh != null_simplex());          //ASSERT!
                  vert = boost::add_vertex(adj_graph);
                  d_map.emplace(sh,vert);
                }
              else
                {
                  
                  if (curr_d == link_dimension-1)
                    {
                      sh = find(curr_simplex_copy);               //a simplex of the link
                      assert(sh != null_simplex());
                      vert = boost::add_vertex(adj_graph);
                      f_map.emplace(sh,vert);
                    }
                  
                  //delete (&curr_simplex_copy); //Just so it doesn't stack
                  add_vertices_to_link_graph(star_vertices,
                                             it+1,
                                             adj_graph,
                                             d_map,
                                             f_map,
                                             curr_simplex,
                                             curr_d+1, link_dimension);
                }
            }
          /*
          else
            std::cout << "\n";
          */
          curr_simplex.pop_back();                           //pop the vertex in question
        }
    }
    
    void add_edges_to_link_graph(Adj_graph& adj_graph,
                                 Graph_map& d_map,
                                 Graph_map& f_map)
    {
      Simplex_handle sh;
      // Add edges
      //std::cout << "Entered add edges:\n";
      typename Graph_map::iterator map_it;
      for (auto d_map_pair : d_map)
        {
          //std::cout << "*";
          sh = d_map_pair.first;
          Vertex_t d_vert = d_map_pair.second;
          for (auto facet_sh : boundary_simplex_range(sh))
            //for (auto f_map_it : f_map)
            {
              //std::cout << "'"; 
              map_it = f_map.find(facet_sh);
              //We must have all the facets in the graph at this point
              assert(map_it != f_map.end());
              Vertex_t f_vert = map_it->second;
              //std::cout << "Added edge " << sh->first << "-" << map_it->first->first << "\n";
              boost::add_edge(d_vert,f_vert,adj_graph);
            }
        }
    }

    void add_max_simplices_to_graph(typeVectorVertex& star_vertices,
                                    typename typeVectorVertex::iterator curr_v,
                                    Adj_graph& adj_graph,
                                    Graph_map& d_map,
                                    Graph_map& f_map,
                                    Inv_graph_map& inv_d_map,
                                    typeVectorVertex& curr_simplex,
                                    int curr_d,
                                    int link_dimension)
    {
      Simplex_handle sh;
      Vertex_t vert;
      typename typeVectorVertex::iterator it;
      //std::pair<typename Graph_map::iterator,bool> resPair;
      //typename Graph_map::iterator resPair;
      //Add vertices
      //std::cout << "Entered add vertices\n";
      for (it = curr_v; it != star_vertices.end(); ++it)
        {
          curr_simplex.push_back(*it);               //push next vertex in question
          //curr_simplex.push_back(star_vertices[0]);  //push the center of the star
          /*
          std::cout << "Searching for ";
          print_vector(curr_simplex);
          std::cout << " curr_dim " << curr_d << " d " << dimension << "";
          */
          typeVectorVertex curr_simplex_copy(curr_simplex);
          sh = find(curr_simplex_copy);                   //a simplex of the star
          //curr_simplex.pop_back();                   //pop the center of the star
          curr_simplex_copy = typeVectorVertex(curr_simplex);
          if (sh != null_simplex())
            {
              //std::cout << " added\n";
              if (curr_d == link_dimension)
                {
                  sh = find(curr_simplex_copy);               //a simplex of the link
                  assert(sh != null_simplex());          //ASSERT!
                  vert = boost::add_vertex(adj_graph);
                  d_map.emplace(sh,vert);
                  inv_d_map.emplace(vert,sh);
                }
              else
                {
                  
                  if (curr_d == link_dimension-1)
                    {
                      sh = find(curr_simplex_copy);               //a simplex of the link
                      assert(sh != null_simplex());
                      vert = boost::add_vertex(adj_graph);
                      f_map.emplace(sh,vert);
                    }
                  
                  //delete (&curr_simplex_copy); //Just so it doesn't stack
                  add_max_simplices_to_graph(star_vertices,
                                             it+1,
                                             adj_graph,
                                             d_map,
                                             f_map,
                                             inv_d_map,
                                             curr_simplex,
                                             curr_d+1, link_dimension);
                }
            }
          /*
          else
            std::cout << "\n";
          */
          curr_simplex.pop_back();                           //pop the vertex in question
        }
    }

}; //class Witness_complex


  
} // namespace Guhdi

#endif
