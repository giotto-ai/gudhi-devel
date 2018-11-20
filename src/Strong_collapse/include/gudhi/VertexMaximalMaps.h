/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Siddharth Pritam 
 *
 *    Copyright (C) 2017 INRIA Sophia Antipolis (France)
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
#pragma once

#include "gudhi/Fake_simplex_tree.h"
#include "gudhi/Simplex_tree.h"
// #include <boost/graph/adjacency_list.hpp>
// #include <boost/graph/bron_kerbosch_all_cliques.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_map>
#include <tuple>
#include <list>
#include <algorithm>
#include <chrono>

#include <ctime>
#include <fstream>

#include <Eigen/Sparse>

using Fake_simplex_tree 	= Gudhi::Fake_simplex_tree ;
using Simplex_tree   		= Gudhi::Simplex_tree<>;
using Vertex 	            = Fake_simplex_tree::Vertex;
using Simplex             	= Fake_simplex_tree::Simplex;

using MapVertexToIndex 	  	= std::unordered_map<Vertex,int>;
using Map 				  	= std::unordered_map<Vertex,Vertex>;

using sparseMatrix 		 	= Eigen::SparseMatrix<double> ;
using sparseRowMatrix  	 	= Eigen::SparseMatrix<double, Eigen::RowMajor> ;

using rowInnerIterator 		= sparseRowMatrix::InnerIterator;
using columnInnerIterator 	= sparseMatrix::InnerIterator;

using intVector 	   = std::vector<int>;
using doubleVector 	   = std::vector<double>;
using vertexVector     = std::vector<Vertex>;
using simplexVector    = std::vector<Simplex>;
using boolVector       = std::vector<bool>;

using doubleQueue 	   = std::queue<double>;
using matixTuple	   = std::tuple<sparseMatrix, sparseRowMatrix>;


//!  Class VertexMaximalMaps 
/*!
  The class for storing the Vertices v/s MaxSimplices Sparse Matrix and performing collapses operations using the N^2() Algorithm.
*/
class VertexMaximalMaps
{
	private:
	typedef typename std::size_t VertexIdx;
	typedef typename std::size_t SimplexIdx;

	typedef typename std::unordered_set<VertexIdx>  vertex_set_of_a_simplex;
	typedef typename std::unordered_set<SimplexIdx> simplex_set_of_a_vertex;

	typedef typename std::unordered_map<VertexIdx, simplex_set_of_a_vertex> vertices_dynamic;
	typedef typename std::unordered_map<VertexIdx, simplex_set_of_a_vertex> simplices_dynamic;



  	std::unordered_map<int,Vertex> rowToVertex;

  	// Vertices strored as an unordered_set
  	std::unordered_set<Vertex> vertices; 

  
  //! Stores the maximal simplices of the original Simplicial Complex.
    /*!
      \code
      simplexVector = std::vector< Simplex >
      \endcode
      This is a vector that stores all the maximal simplices of the Original Simplicial Complex. <br>
      \endverbatim
    */
 	simplexVector maximal_simplices;
 
	//! Stores the Map between vertices<B>rowToVertex  and row indices <B>rowToVertex -> row-index</B>.
    /*!
      \code
      MapVertexToIndex = std::unordered_map<Vertex,int>
      \endcode
      So, if the original simplex tree had vertices 0,1,4,5 <br>
      <B>rowToVertex</B> would store : <br>
      \verbatim
      Values =  | 0 | 1 | 4 | 5 | 
      Indices =   0   1   2   3
      \endverbatim
      And <B>vertexToRow</B> would be a map like the following : <br>
      \verbatim
      0 -> 0
      1 -> 1
      4 -> 2
      5 -> 3
      \endverbatim
    */
	MapVertexToIndex vertexToRow;

	//! Stores the number of vertices in the original Simplicial Complex.
    /*!
      This stores the count of vertices (which is also the number of rows in the Matrix).
    */
	std::size_t rows;
	//int numColpsdRows; // Total number of collapsed rows.
	//! Stores the number of Maximal Simplices in the original Simplicial Complex.
    /*!
      This stores the count of Maximal Simplices (which is also the number of columns in the Matrix).
    */
	std::size_t cols;
	//int numColpsdMxml; // Total number of final maximal simplices.

	std::size_t numMaxSimplices;

	//! Stores the Sparse matrix of double values representing the Original Simplicial Complex.
    /*!
      \code
      sparseMatrix   = Eigen::SparseMatrix<double> ;
      \endcode
      So after counting the number of rows and num of Maximal simplices, this is initialised as : <br>
      \code
      sparseMxSimplices =  sparseMatrix(rows,numMaxSimplices);
      \endcode
      And filled with columns by the Constructor with a Fake Simplex tree as an argument.
    ;
	sparseMatrix* Sparse*/

	sparseMatrix sparseMxSimplices;
	sparseMatrix sparseColpsdMxSimplices;       // Stores the collapsed sparse matrix representaion.
	sparseRowMatrix sparseRowMxSimplices; 		// This is row-major version of the same sparse-matrix, to facilitate easy access to elements when traversing the matrix row-wise.

	Fake_simplex_tree collapsed_fake_simplex_tree;
	Simplex_tree collapsed_simplex_tree;

	//! Stores <I>true</I> for dominated rows and  <I>false</I> for undominated rows. 
    /*!
      Initialised to a vector of length equal to the value of the variable <B>rows</B> with all <I>false</I> values.
      Subsequent removal of dominated vertices is reflected by concerned entries changing to <I>true</I> in this vector.
    */
  	boolVector vertDomnIndicator;  //(domination indicator)
	//! Stores <I>true</I> for maximal simplex(dominated) columns and  <I>false</I> for a non-maximal(non-dominated) columns. 
    /*!
      Initialised to an vector of length equal to the value of the variable <B>cols</B> with all <I>false</I> values.
      Subsequent removal of Maximal Simplices (caused by removal of vertices) is reflected by concerned entries changing to <I>false</I> in this array.
    */
  	boolVector simpDomnIndicator; //(domination indicator)

	//! Stores the indices of the rows to-be checked for domination in the current iteration. 
    /*!
      Initialised to a queue with all row-indices inserted.
      Subsequently once the row is checked for dominated the row-index is poped out from the queue. A row-index is inserted once again if it is a non-zero element of a dominated column.
    */
  	doubleQueue rowIterator;
  	//! Stores the indices of the colums to-be checked for domination in the current iteration. 
    /*!
      Initialised to an empty queue.
      Subsequently once a dominated row is found, its non-zero column indices are inserted.
    */
	doubleQueue columnIterator;

	//! Stores <I>true</I> if the current row is inserted in the queue <B>rowIterator<B> otherwise its value is <I>false<I>. 
    /*!
      Initialised to a boolean vector of length equal to the value of the variable <B>rows</B> with all <I>true</I> values.
      Subsequent removal/addition of a row from <B>rowIterator<B> is reflected by concerned entries changing to <I>false</I>/<I>true</I> in this vector.
    */
	boolVector rowInsertIndicator;  //(current iteration row insertion indicator)

  //! Stores <I>true</I> if the current column is inserted in the queue <B>columnIterator<B> otherwise its value is <I>false<I>. 
    /*!
      Initialised to a boolean vector of length equal to the value of the variable <B>cols</B> with all <I>false</I> values.
      Subsequent addition/removal of a column in  <B>columnIterator<B> is reflected by concerned entries changing to <I>true</I>/<I>false</I> in this vector.
    */
	boolVector colInsertIndicator; //(current iteration column insertion indicator)
	
  //! Map that stores the Reduction / Collapse of vertices.
    /*!
      \code
      Map = std::unordered_map<Vertex,Vertex>
      \endcode
      This is empty to begin with. As and when collapses are done (let's say from dominated vertex <I>v</I> to dominating vertex <I>v'</I>) : <br>
      <B>ReductionMap</B>[<I>v</I>] = <I>v'</I> is entered into the map. <br>
      <I>This does not store uncollapsed vertices. What it means is that say vertex <I>x</I> was never collapsed onto any other vertex. Then, this map <B>WILL NOT</B> have any entry like <I>x</I> -> <I>x</I>.
      Basically, it will have no entry corresponding to vertex <I>x</I> at all. </I> 
    */
	Map ReductionMap;

	std::size_t maxNumCollSiplices;
	std::size_t maxNumInitSimplices;
	
	int  initComplexDimension;
	int  collComplexDimension;

	bool already_collapsed;
	int expansion_limit;


	void init()
	{
		rowToVertex.clear();
    	vertexToRow.clear();
    	maximal_simplices.clear();
    	ReductionMap.clear();
    	
		vertDomnIndicator.clear();
		rowInsertIndicator.clear();
		rowIterator.push(0);
		rowIterator.pop();

		simpDomnIndicator.clear();
		colInsertIndicator.clear();
    	columnIterator.push(0);  				//  A naive way to initialize, might be reduntant.
 		columnIterator.pop(); 

		rows = 0;
		cols = 0;
		
		numMaxSimplices = 0;
		expansion_limit = 3;
	
	  	maxNumInitSimplices  = 0;
	  	maxNumCollSiplices   = 0;
  		initComplexDimension = 0;
  		collComplexDimension = 0;

  		already_collapsed = false;
	}
	
	//!	Function for computing the Fake Simplex_tree corresponding to the core of the complex.
    /*!
      First calls strong_collapse(), and then computes the Fake Simplex_tree of the core using the Sparse matrix that we have.
      How does it compute the Fake simplex tree ? <br>
      Goes over all the columns (remaining MaximalSimplices) and for each of them, inserts that simplex <br>
      ['that simplex' means the maximal simplex with all the (remaining) vertices] with all subfaces using the <br>
      <I>insert_maximal_simplex_and_subfaces()</I> function from Gudhi's Fake_simplex_tree.
    */
 	void after_collapse()
	{
		int simpDim;
		int j = 0;
		maximal_simplices.clear();
     	sparseColpsdMxSimplices   =  sparseMatrix(rows,cols); // Just for debugging purpose.
    	
		for(int co = 0 ; co < cols ; ++co)
		{
			if(not simpDomnIndicator[co]) 				//If the current column is not dominated
			{
				vertexVector mx_simplex_to_insert = readColumn(co); 
				collapsed_fake_simplex_tree.insert_simplex_and_subfaces(mx_simplex_to_insert); 
				collapsed_simplex_tree.insert_simplex_and_subfaces(mx_simplex_to_insert);
				
				maximal_simplices.push_back(Simplex(mx_simplex_to_insert.begin(), mx_simplex_to_insert.end()));
       		
        		for(auto & v:mx_simplex_to_insert)
          			sparseColpsdMxSimplices.insert(vertexToRow[v],j) = 1;
			  	
			  	j++;
			  	simpDim = mx_simplex_to_insert.size() -1;
			  	if(collComplexDimension < simpDim)
			  		collComplexDimension = simpDim;
			  	maxNumCollSiplices += (pow(2, (simpDim+1)) -1); 
			  	// std::cout << "Dimension of the current maximal simplex and current simplex count: " << mx_simplex_to_insert.size() << ", "<< maxNumCollSiplices << std::endl; 
      		}			
		}
    	// std::cout << sparseColpsdMxSimplices << std::endl;
		return ;
	}
	//! Function to fully compact a particular vertex of the ReductionMap.
    /*!
      It takes as argument the iterator corresponding to a particular vertex pair (key-value) stored in the ReductionMap. <br>
	  It then checks if the second element of this particular vertex pair is present as a first element of some other key-value pair in the map.
	  If no, then the first element of the vertex pair in consideration is fully compact. 
	  If yes, then recursively call fully_compact_this_vertex() on the second element of the original pair in consideration and assign its resultant image as the image of the first element of the original pair in consideration as well. 
    */
	void fully_compact_this_vertex(Map::iterator iter)
	{
		Map::iterator found = ReductionMap.find(iter->second);
		if ( found == ReductionMap.end() )
			return;

		fully_compact_this_vertex(found);
		iter->second = ReductionMap[iter->second];
	}

	//! Function to fully compact the Reduction Map.
    /*!
      While doing strong collapses, we store only the immediate collapse of a vertex. Which means that in one round, vertex <I>x</I> may collapse to vertex <I>y</I>.
      And in some later round it may be possible that vertex <I>y</I> collapses to <I>z</I>. In which case our map stores : <br>
      <I>x</I> -> <I>y</I> and also <I>y</I> -> <I>z</I>. But it really should store :
      <I>x</I> -> <I>z</I> and <I>y</I> -> <I>z</I>. This function achieves the same. <br>
      It basically calls fully_compact_this_vertex() for each entry in the map.
    */
	void fully_compact()
	{
		Map::iterator it = ReductionMap.begin();
		while(it != ReductionMap.end())
		{
			fully_compact_this_vertex(it);
			it++;
		}
	}

	template <typename Input_vertex_range>
	std::vector<Simplex> all_faces(const Input_vertex_range &vertex_range){
	    int set_size = vertex_range.size();
	    unsigned int pow_set_size = pow(2, set_size);
	    unsigned int counter, j;
	    std::vector<Simplex> facets;
	    std::vector<Vertex> maxSimplex(vertex_range.begin(), vertex_range.end());
	    // maxSimplex.sort();

	    /*Run from counter 000..0 to 111..1*/
	    for(counter = 1; counter < pow_set_size; counter++)
	    {
	      Simplex f;
	      for(j = 0; j < set_size; j++)
	       {          
	          if(counter & (1<<j))                    /* Check if jth bit in the counter is set/true If set then inserts jth element from vertex_range */
	            f.insert(maxSimplex[j]);
	       }
	       facets.emplace_back(f);
	       f.clear();
	    }
	    return facets;
	}

	void sparse_strong_collapse()
	{
 		complete_domination_check(rowIterator, rowInsertIndicator, vertDomnIndicator, true); 		// Complete check for rows, by setting last argument "true" 
  		complete_domination_check(columnIterator, colInsertIndicator, simpDomnIndicator, false); 	// Complete check for columns, by setting last argument "false"
		if( not rowIterator.empty())
			sparse_strong_collapse();
		else
			return ;
  	}

	void complete_domination_check (doubleQueue& iterator, boolVector& insertIndicator, boolVector& domnIndicator, bool which)
	{
	  	double k;
	  	doubleVector nonZeroInnerIdcs;
	    doubleVector nonZeroOuterIdcs;
	    while(not iterator.empty())       // "iterator" contains list(FIFO) of rows/columns to be considered for domination check 
	    { 
	      	k = iterator.front();
	      	iterator.pop();
	      	insertIndicator[k] = false;
	    	if( not domnIndicator[k]) 				// Check only if not already dominated
	    	{ 
		        nonZeroInnerIdcs  = read(k, true, which); 					          // If which == "true", returns the non-zero columns of row k, otherwise if which == "false" returns the non-zero rows of column k.
		        nonZeroOuterIdcs  = read(nonZeroInnerIdcs[0], false, !which); 	// If which == 'true', returns the non-zero rows of the first non-zero column from previous columns and vice versa... 
		        for (doubleVector::iterator it = nonZeroOuterIdcs.begin(); it!=nonZeroOuterIdcs.end(); it++) 
		        {
		       		int checkDom = pair_domination_check(k, *it, which);   	// "true" for row domination comparison
		        	if( checkDom == 1)                                  	// row k is dominated by *it, k <= *it;
			        {
			            setZero(k, *it, which);
			            break ;
			        }
		          	else if(checkDom == -1)                 				// row *it is dominated by k, *it <= k;
		            	setZero(*it, k, which);
			    }         
	    	}
	    }
	}

	int pair_domination_check( double i, double j, bool which) // True for row comparison, false for column comparison
	{
		if(i != j)
		{
			doubleVector Listi = read(i, false, which);
			doubleVector Listj = read(j, false, which);
			if(Listj.size() <= Listi.size())
			{
      			if(std::includes(Listi.begin(), Listi.end(), Listj.begin(), Listj.end())) // Listj is a subset of Listi
					return -1;
			}
			
			else 
				if(std::includes(Listj.begin(), Listj.end(), Listi.begin(), Listi.end())) // Listi is a subset of Listj
					return 1;
		}
		return 0;	
	}

	doubleVector read(double indx, bool firstEntrOnly, bool which) // Returns list of non-zero rows(which = true)/columns(which = false) of the particular indx. 
	{											// Caution : Check for domination before calling the method.
	  	doubleVector nonZeroIndices;     
	  	if(which)
	  		nonZeroIndices = read<rowInnerIterator, sparseRowMatrix>(sparseRowMxSimplices, simpDomnIndicator, firstEntrOnly, indx);
	  	else
	  		nonZeroIndices = read<columnInnerIterator, sparseMatrix>(sparseMxSimplices, vertDomnIndicator,firstEntrOnly, indx);
	  	return nonZeroIndices;
	}

	void setZero(double dominated, double dominating, bool which)
	{
	  	if (which)
	  	{	
	  		vertDomnIndicator[dominated] = true;
	  		ReductionMap[rowToVertex[dominated]]    = rowToVertex[dominating];
	  		
	  		vertexToRow.erase(rowToVertex[dominated]);
	  		vertices.erase(rowToVertex[dominated]);
	  		rowToVertex.erase(dominated);

	  		setZero<rowInnerIterator, sparseRowMatrix>(sparseRowMxSimplices, simpDomnIndicator, colInsertIndicator, columnIterator, dominated);
	    }
	  	else
	  	{
	   		simpDomnIndicator[dominated] = true;
	   		numMaxSimplices--;
	   		setZero<columnInnerIterator, sparseMatrix>(sparseMxSimplices, vertDomnIndicator, rowInsertIndicator, rowIterator, dominated);
	  	}
	}
 	
 	vertexVector readColumn(double colIndx) // Returns list of non-zero "vertices" of the particular colIndx. the difference is in the return type
	{
		vertexVector rows ; 
  		for (sparseMatrix::InnerIterator itRow(sparseMxSimplices,colIndx); itRow; ++itRow)  // Iterate over the non-zero columns
     		if(not vertDomnIndicator[itRow.index()])  					// Check if the row corresponds to a dominated vertex
      			rows.push_back(rowToVertex[itRow.index()]); 			// inner index, here it is equal to it.row()

  		return rows;
	}

	template<typename type, typename matrix>
  	void setZero(const matrix& m, boolVector& domnIndicator, boolVector& insertIndicator, doubleQueue& iterator, double indx)
	{
	    for (type it(m,indx); it; ++it)  // Iterate over the non-zero rows/columns
	      if(not domnIndicator[it.index()] && not insertIndicator[it.index()]) // Checking if the row/column is already dominated(set zero) or inserted	
	      {  
	        iterator.push(it.index());
	        insertIndicator[it.index()] = true;
	      }
	} 
	template <typename type, typename matrix>
	doubleVector read(const matrix& m, const boolVector& domnIndicator, bool firstEntrOnly, double indx)
	{
		doubleVector nonZeroIndices; 
      	for (type it(m, indx); it; ++it)             // Iterate over the non-zero rows/columns
        	if(not domnIndicator[it.index()])
        	{
            	nonZeroIndices.push_back(it.index());  // inner index, here it is equal to it.row()/it.columns()
        		if(firstEntrOnly)
        			break;
        	}
    
      return nonZeroIndices;
	}

public:

	//! Default Constructor
    /*!
      Only initialises all Data Members of the class to empty/Null values as appropriate.
      One <I>WILL</I> have to create the matrix using the Constructor that has an object of the Simplex_tree class as argument.
    */
	
	VertexMaximalMaps()
	{
		init();
	}

	VertexMaximalMaps(std::size_t expRows, std::size_t expMaxSimp)
	{
		init();
		sparseMxSimplices     = sparseMatrix(expansion_limit*expRows, expansion_limit*expMaxSimp); 		// Initializing sparseMxSimplices, This is a column-major sparse matrix.  
		sparseRowMxSimplices  = sparseRowMatrix(expansion_limit*expRows, expansion_limit*expMaxSimp);    	// Initializing sparseRowMxSimplices, This is a row-major sparse matrix.  
	}

	//! Main Constructor
    /*!
      Argument is an instance of Fake_simplex_tree. <br>
      This is THE function that initialises all data members to appropriate values. <br>
      <B>rowToVertex</B>, <B>vertexToRow</B>, <B>rows</B>, <B>cols</B>, <B>sparseMxSimplices</B> are initialised here.
      <B>vertDomnIndicator</B>, <B>rowInsertIndicator</B> ,<B>rowIterator<B>,<B>simpDomnIndicator<B>,<B>colInsertIndicator<B> and <B>columnIterator<B> are initialised by init_lists() function which is called at the end of this. <br>
      What this does:
      	1. Populate <B>rowToVertex</B> and <B>vertexToRow</B> by going over through the vertices of the Fake_simplex_tree and assign the variable <B>rows</B> = no. of vertices
      	2. Initialise the variable <B>cols</B> to zero and allocate memory from the heap to <B>MxSimplices</B> by doing <br>
      	        <I>MxSimplices = new boolVector[rows];</I>
      	3. Iterate over all maximal simplices of the Fake_simplex_tree and populates the column of the sparseMatrix.
      	4. Initialise <B>active_rows</B> to an array of length equal to the value of the variable <B>rows</B> and all values assigned true. [All vertices are there in the simplex to begin with]
      	5. Initialise <B>active_cols</B> to an array of length equal to the value of the variable <B>cols</B> and all values assigned true. [All maximal simplices are maximal to begin with]
      	6. Calls the private function init_lists().
    */
	VertexMaximalMaps(const Fake_simplex_tree &st)
	{
		init();

		auto begin = std::chrono::high_resolution_clock::now();
	  	maximal_simplices = st.max_simplices();

	    //Extracting the unique vertices using unordered set.
	    for(const Simplex& s : maximal_simplices)
	      for (Vertex v : s)
	        vertices.emplace(v);
 	  	numMaxSimplices   	  = maximal_simplices.size();
		
		sparseMxSimplices     = sparseMatrix(expansion_limit*vertices.size(),expansion_limit*numMaxSimplices); 			// Initializing sparseMxSimplices, This is a column-major sparse matrix.  
		sparseRowMxSimplices  = sparseRowMatrix(expansion_limit*vertices.size(),expansion_limit*numMaxSimplices);    	// Initializing sparseRowMxSimplices, This is a row-major sparse matrix.  
		// There are two sparse matrices, it gives the flexibility of iteratating through non-zero rows and columns efficiently.
		numMaxSimplices = 0; // reset to zero, and then increment or decremented accordingly.
   	
		for(auto &simplex: maximal_simplices) 						// Adding each maximal simplices iteratively.
			insert_maximal_simplex_and_subfaces(simplex);

        auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Sparse Matrix Created  " << std::endl;
		std::cout << "Formation time : " <<  std::chrono::duration<double, std::milli>(end- begin).count()
	              << " ms\n" << std::endl;
		std::cout << "Total number of Initial maximal simplices are: " << cols << std::endl;		
		
		sparseMxSimplices.makeCompressed(); 	             //Optional for memory saving
	  	sparseRowMxSimplices.makeCompressed(); 
       	
 	  	// std::cout << sparseMxSimplices << std::endl;
	}

	//!	Destructor.
    /*!
      Frees up memory locations on the heap.
    */
	~VertexMaximalMaps()
	{
	}

	//!	Function for performing strong collapse.
    /*!
      calls sparse_strong_collapse(), and
      Then, it compacts the ReductionMap by calling the function fully_compact().
    */
	void strong_collapse()
	{
			auto begin_collapse  = std::chrono::high_resolution_clock::now();
			sparse_strong_collapse();
			already_collapsed = true;
			auto end_collapse  = std::chrono::high_resolution_clock::now();
		
			auto collapseTime = std::chrono::duration<double, std::milli>(end_collapse- begin_collapse).count();
			std::cout << "Time of Collapse : " << collapseTime << " ms\n" << std::endl;
			
			// Now we complete the Reduction Map
			fully_compact();
			//Post processing...
			after_collapse();
	}

	bool membership(const Vertex & v)
	{
		auto rw = vertexToRow.find(v);
		if(rw != vertexToRow.end())	
			return true;
		else
			return false;
	}

	template <typename Input_vertex_range>
	bool membership(const Input_vertex_range & vertex_range) 
	{  
		if(vertex_range.size() > 0)
	    {
		    std::vector<size_t>  simp(vertex_range.begin(), vertex_range.end());
	    	doubleVector  current, next;
	    	doubleVector::iterator it;
		    if(membership(simp[0]))	
		    {
		    	current = read(vertexToRow[simp[0]], false, true);   			//Reads row for non-zero column indices
				for(int counter = 1; counter < simp.size(); counter++)
			    {
		    		if(membership(simp[counter]))	
			    	{
			    		next   = read(vertexToRow[simp[counter]], false, true);		// Reads row for non-zero column indices
			    		it 	   = std::set_intersection(current.begin(), current.end(), next.begin(), next.end(), current.begin());
			    		current.resize(it-current.begin());  
			    		if(current.size() == 0)
			    			return false;
			    	}
			    	else
			    		return false;	
		       	}
		       	return true;
		    }
			else
		    	return false; 
		}
		else
		{
			std::cerr << "Please enter a valid simplex for membership check" <<std::endl;	
			exit(-1); 	
		}	
	}

	void contraction(const Vertex & del, const Vertex & keep)
	{
		bool del_mem  = membership (del);
		bool keep_mem = membership(keep);
		if( del_mem && keep_mem)
		{
			doubleVector  del_indcs, keep_indcs, diff;
			auto row_del = vertexToRow.find(del);
			auto row_keep = vertexToRow.find(keep);
			del_indcs = read(row_del->second, false, true);
			keep_indcs = read(row_keep->second, false, true);
			std::set_difference(del_indcs.begin(), del_indcs.end(), keep_indcs.begin(), keep_indcs.end(), std::inserter(diff, diff.begin()));
			for (auto columns : diff) 
			{
				sparseMxSimplices.insert(row_keep->second,columns) 	  = 1;
        		sparseRowMxSimplices.insert(row_keep->second,columns) = 1;	
			}
			setZero(row_del->second, row_keep->second, true);
			complete_domination_check(columnIterator, colInsertIndicator, simpDomnIndicator, false); 
		}
		else if(del_mem && not keep_mem)
		{
			vertexToRow.insert(std::make_pair(keep, vertexToRow.find(del)->second));
			rowToVertex[vertexToRow.find(del)->second] = keep;
			vertices.emplace(keep);
			vertices.erase(del);
			vertexToRow.erase(vertexToRow.find(del));

		}
		else
		{
			std::cerr << "The first vertex entered in the method contraction() doesn't exist in the matrix:" <<std::endl;	
			exit(-1); 	
		}
	}
	 
	template <typename Input_vertex_range>
	void insert_maximal_simplex_and_subfaces(const Input_vertex_range & vertex_range)
	{
		if( not membership(vertex_range))
		{
			vertex_set_of_a_simplex  simp(vertex_range.begin(), vertex_range.end());
			for(auto vertex : simp) 
			{
	    		auto rw = vertices_dynamic.find(vertex);
	    		if(rw != vertexToRow.end())
	    		{
	    			rw->second.emplace_back(cols);
	    			sparseMxSimplices.insert(rw->second,cols) 	  = 1;
	    			sparseRowMxSimplices.insert(rw->second,cols)  = 1;
	    		}
	    		else
	    		{
	    			sparseMxSimplices.insert(rows,cols) 	  = 1;
	    			sparseRowMxSimplices.insert(rows,cols) 	  = 1;
	    			
	    			vertDomnIndicator.push_back(false);
					rowInsertIndicator.push_back(true);
					rowIterator.push(rows);
	    			vertexToRow.insert(std::make_pair(vertex, rows));
	    			rowToVertex.insert(std::make_pair(rows, vertex)); 
	    			vertices.emplace(vertex);
	    			rows++;
	    		}
	    			
			}

			simpDomnIndicator.push_back(false);
			colInsertIndicator.push_back(false);
			cols++;
			numMaxSimplices++;

			int simpDimension = (simp.size()) -1;
			if(initComplexDimension < simpDimension)
				initComplexDimension = simpDimension;
			maxNumInitSimplices += (pow(2, (simpDimension+1))) - 1;	
		}
		// else
		// 	std::cout << "Already a member simplex,  skipping..." << std::endl;
       		
	}
	//!	Function for returning the ReductionMap.
    /*!
      This is the (stl's unordered) map that stores all the collapses of vertices. <br>
      It is simply returned.
    */
	
	Map reduction_map() const
	{
		return ReductionMap;
	}

    long max_num_collapsed_simplices() const
    {
    	return maxNumCollSiplices;
    }
    long max_num_inital_simplices() const
    {
    	return maxNumInitSimplices;
    }
    int initial_dimension() const
    {
    	return initComplexDimension;
    }
    int collapsed_dimension() const
    {
    	return collComplexDimension;
    }
    std::unordered_set<Vertex> vertex_set() const
    {
    	return vertices;
    }
    Fake_simplex_tree fake_simplex_tree_collapsed() const
    {
    	return collapsed_fake_simplex_tree;
    }
    Simplex_tree simplex_tree_collapsed() const
    {
    	return collapsed_simplex_tree;
    }
    sparseMatrix collapsed_matrix() const
    {
    	return sparseColpsdMxSimplices;
    }
    std::size_t number_max_simplices()
    {
    	return numMaxSimplices;
    }
    simplexVector max_simplices() const
    {
    	return maximal_simplices;
    }


};