#ifndef SIMPLE_COXETER_SYSTEM_H_
#define SIMPLE_COXETER_SYSTEM_H_

#include <iostream>
#include <vector>
#include <exception>
#include <Eigen/Sparse>

class Simple_coxeter_system {

  typedef double FT;
  typedef Eigen::SparseMatrix<FT> Matrix;
  typedef Eigen::Triplet<FT> Triplet;
  typedef std::vector<int> Alcove_id;
  typedef Alcove_id Vertex_id;
  typedef std::vector<std::pair<int,int>> Change_range;
  typedef std::vector<Change_range> Local_changes;
  
  class wrong_family : public std::exception {  
  } wrong_family_exception_;

public:
  
  Matrix root_t_;
  char family_;
  unsigned short dimension_;
  
  Simple_coxeter_system() {
  }
  
  Simple_coxeter_system(char family, unsigned short dimension)
    : root_t_(dimension, dimension), family_(family), dimension_(dimension){
    switch (family) {
    case 'A': {
      unsigned short d = dimension;
      std::vector<Triplet> cartan_triplets;
      cartan_triplets.reserve(3*d-2);
      for (unsigned i = 0; i < d; i++) {
        cartan_triplets.push_back(Triplet(i,i,2.0));
      }
      for (unsigned i = 1; i < d; i++) {
        cartan_triplets.push_back(Triplet(i-1,i,-1.0));
        cartan_triplets.push_back(Triplet(i,i-1,-1.0));
      }
      Matrix cartan(d,d);
      cartan.setFromTriplets(cartan_triplets.begin(), cartan_triplets.end());
      // std::cout << "cartan =" << std::endl << cartan << std::endl;
      Eigen::SimplicialLLT<Matrix, Eigen::Lower> chol(cartan);
      root_t_ = chol.matrixL();
      // std::cout << "root^t =" << std::endl << root_t_ << std::endl;
      break;
    }
    case 'D': {
      unsigned short d = dimension;
      assert(d >= 4);
      std::vector<Triplet> cartan_triplets;
      cartan_triplets.reserve(3*d-2);
      for (unsigned i = 0; i < d; i++) {
        cartan_triplets.push_back(Triplet(i,i,2.0));
      }
      for (int i = 0; i < d-1; i++) {
        cartan_triplets.push_back(Triplet(i-1,i,-1.0));
        cartan_triplets.push_back(Triplet(i,i-1,-1.0));
      }
      cartan_triplets.push_back(Triplet(d-1,d-3,-1.0));
      cartan_triplets.push_back(Triplet(d-3,d-1,-1.0));
      Matrix cartan(d,d);
      cartan.setFromTriplets(cartan_triplets.begin(), cartan_triplets.end());
      // std::cout << "cartan =" << std::endl << cartan << std::endl;
      Eigen::SimplicialLLT<Matrix, Eigen::Lower> chol(cartan);
      root_t_ = chol.matrixL();
      // std::cout << "root^t =" << std::endl << root_t_ << std::endl;
      break;
    }  
    default :
      std::cerr << "Simple_coxeter_system : The family " << family << " is not supported. "
                << "Please use A or D family for the constructor (in capital).\n";
      throw wrong_family_exception_;
    }
  }

  unsigned short dimension() const {
    return dimension_;
  }
  
  /** A conversion from Cartesian coordinates to the coordinates of the alcove containing the point.
   *  The matrix' rows are simple root vectors.
   */
  template <class Point,
            class OutputIterator>
  void alcove_coordinates(const Point& p, int level, OutputIterator output_it) const
  {
    switch (family_) {
    case 'A': {
      unsigned short d = p.size();
      assert(d == dimension_);
      Eigen::VectorXd p_vect(d);
      for (short i = 0; i < d; i++)
        p_vect(i) = p[i];
      Eigen::VectorXd scalprod_vect = root_t_ * p_vect;
      for (short i = 0; i < d; i++) {
        FT root_scalprod = 0;
        for (short j = i; j >= 0; j--) {
          root_scalprod += scalprod_vect(j);
          *output_it++ = std::floor(level * root_scalprod);
        }
      }
      break;
    }
    case 'D': {
      unsigned short d = p.size();
      assert(d == dimension_);
      Eigen::VectorXd p_vect(d);
      for (short i = 0; i < d; i++)
        p_vect(i) = p[i];
      Eigen::VectorXd scalprod_vect = root_t_ * p_vect;
      // e_i - e_j
      for (short i = 0; i < d-1; i++) {
        FT root_scalprod = 0;
        for (short j = i; j >= 0; j--) {
          root_scalprod += scalprod_vect(j);
          *output_it++ = std::floor(level * root_scalprod);
        }
      }
      // e_i + e_j
      FT global_scalprod = -scalprod_vect(d-1) - scalprod_vect(d-2);
      for (short i = d-1; i >= 0; i--) {
        global_scalprod += 2*scalprod_vect(i);
        FT root_scalprod = global_scalprod;
        for (short j = i-1; j >= 0; j--) {
          root_scalprod += scalprod_vect(j);
          *output_it++ = std::floor(level * root_scalprod);
        }
      }
      break;
    }
    default :
      std::cerr << "Simple_coxeter_system::alcove_coordinates : The family " << family_ << " is not supported. "
                << "Please use A or D family for the constructor (in capital).\n";
      throw wrong_family_exception_;
    }
  }

  unsigned pos_root_count() {
    switch (family_) {
    case 'A': { return dimension_*(dimension_ + 1)/2; break; } 
    case 'D': { return dimension_*(dimension_ - 1); break; }
    default :
      std::cerr << "Simple_coxeter_system::alcove_coordinates : The family " << family_ << " is not supported. "
                << "Please use A or D family for the constructor (in capital).\n";
      throw wrong_family_exception_;
    }
  }
  
private:  

  int gcd(int a, int b) const {
    return b == 0 ? a : gcd(b, a % b);
  }

  /** Common gcd simplification */
  template <class Id>
  Id reduced_id(Id& id) const {
    int common_gcd = 0;
    for (auto i: id) {
      common_gcd = gcd(i, common_gcd);
      if (common_gcd == 1)
        return id;
    }
    Id id_red(id);
    for (auto i_it = id_red.begin(); i_it != id_red.end(); ++i_it) {
      *i_it = *i_it / common_gcd;
    }
    return id_red;
  }

  template <class S_id_iterator>
  bool valid_coordinate(const Vertex_id& v_id, S_id_iterator& s_it, unsigned d) {
    unsigned k = v_id.size();
    switch (family_) {
    case 'A': {
      int sum = 0;
      for (unsigned i = k; i >= 1; i--) {
        sum += v_id[i];
        if (sum < *s_it || sum > *s_it + 1)
          return false;
        s_it++;
      }
      return true;
    }
    case 'D': {
      if (k == d) {
        int glob_sum = -v_id[d]-v_id[d-1];
        for (unsigned i = d; i >= 1; i--) {
          glob_sum += 2*v_id[i];
          int sum = glob_sum;
          for (short j = i-1; j >= 1; j--) {
            sum += v_id[j];
            if (sum < *s_it || sum > *s_it + 1)
              return false;
            s_it++;
          }
        }
        return true;
      }
      int sum = 0;
      for (unsigned i = k; i >= 1; i--) {
        sum += v_id[i];
        if (sum < *s_it || sum > *s_it + 1)
          return false;
        s_it++;
      }
      return true;
    }
    default :
      std::cerr << "Simple_coxeter_system::alcove_coordinates : The family " << family_ << " is not supported. "
                << "Please use A or D family for the constructor (in capital).\n";
      throw wrong_family_exception_;
    }
    
  }
  
  /** Add the vertices of the given simplex to a vertex-simplex map.
   */
  template <class S_id_iterator>
  void rec_vertices_of_simplex(Vertex_id& v_id, S_id_iterator s_it, unsigned d, std::vector<Vertex_id>& vertices)
  {
    unsigned k = v_id.size();
    if (k == d+1) {
      vertices.emplace_back(v_id);
      return;
    }
    v_id.push_back(*s_it);
    S_id_iterator s_it_copy(s_it);
    if (valid_coordinate(v_id, s_it_copy, d))
      rec_vertices_of_simplex(v_id, s_it_copy, d, vertices);
    v_id.pop_back();

    v_id.push_back(*s_it + 1);
    s_it_copy = s_it;
    if (valid_coordinate(v_id, s_it_copy, d))
      rec_vertices_of_simplex(v_id, s_it_copy, d, vertices);
    v_id.pop_back();
  }

public:  
  
  /** Add the vertices of the given simplex to a vertex-simplex map.
   * The size of si_it->first is d*(d+1)/2.
   */
  std::vector<Vertex_id> vertices_of_simplex(Alcove_id ai_id)
  {
    unsigned d = dimension_;
    Vertex_id v_id(1,*ai_id.begin());
    v_id.reserve(d+1);
    std::vector<Vertex_id> vertices;
    vertices.reserve(d+1);
    rec_vertices_of_simplex(v_id, ai_id.begin()+1, d, vertices);
    return vertices;
  }

  /** Check if the given simplex and vertex are adjacent.
   */
  bool is_adjacent(const Vertex_id& v_id, const Alcove_id& a_id) const {
    auto alcove_it = a_id.begin()+1;
    for (unsigned i = 1; i < v_id.size(); ++i) {
      int sum = 0; 
      for (unsigned j = i; j >= 1; --j) {
        sum += v_id[j];
        double
          v_plane = ((double)sum)/v_id[0],
          a_plane = ((double)*alcove_it++)/a_id[0];
        if (v_plane < a_plane || v_plane > a_plane + 1)
          return false;
      }
    }
    return true;
  }

  
};

#endif
