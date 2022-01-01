//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC// Copyright (C) 2006-2022 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
#ifndef OOMPH_TRIANGLE_SCAFFOLD_MESH_HEADER
#define OOMPH_TRIANGLE_SCAFFOLD_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
//mpi headers
#include "mpi.h"
#endif


#include "triangle_mesh.h"
#include "Telements.h"


namespace oomph
{



//=====================================================================
/// \short Triangle Mesh that is based on input files generated by the
/// triangle mesh generator Triangle.
//=====================================================================
class TriangleScaffoldMesh : public virtual Mesh
{

public:
 
 /// Empty constructor
 TriangleScaffoldMesh() {} 

 /// \short Constructor: Pass the filenames of the triangle files
 TriangleScaffoldMesh(const std::string& node_file_name,
                      const std::string& element_file_name,
                      const std::string& poly_file_name);
 
#ifdef OOMPH_HAS_TRIANGLE_LIB  

 /// \short Constructor: Pass the TriangulateIO object
 TriangleScaffoldMesh(TriangulateIO& triangle_data); 

#endif

 /// Broken copy constructor
 TriangleScaffoldMesh(const TriangleScaffoldMesh&) 
  { 
   BrokenCopy::broken_copy("TriangleScaffoldMesh");
  } 
 
 /// Broken assignment operator
 void operator=(const TriangleScaffoldMesh&) 
  {
   BrokenCopy::broken_assign("TriangleScaffoldMesh");
  }


 /// Empty destructor 
 ~TriangleScaffoldMesh() {}

 /// \short Return the global node of each local node 
 /// listed element-by-element e*n_local_node + n_local
 /// Note that the node numbers are indexed from 1
 unsigned global_node_number(const unsigned &i)
 {return Global_node[i];}
 
 /// \short Return the boundary id of the i-th edge in the e-th element:
 /// This is zero-based as in triangle. Zero means the edge is not
 /// on a boundary. Postive numbers identify the boundary.
 /// Will be reduced by one to identify the oomph-lib boundary. 
 unsigned edge_boundary(const unsigned& e, const unsigned& i) const
  {return Edge_boundary[e][i];}

 /// \short Return the number of internal edges
 unsigned nglobal_edge()
 {return Nglobal_edge;}

 /// \short Return the global index of the i-th edge in the e-th element:
 /// The global index starts from zero
 unsigned edge_index(const unsigned& e, const unsigned& i) const
  {return Edge_index[e][i];}

 /// \short Return the attribute of the element e
 double element_attribute(const unsigned &e) const
  {return Element_attribute[e];}

 /// Vectors of hole centre coordinates
 Vector<Vector<double> >& internal_point(){return Hole_centre;}
 
 protected: 

 /// Check mesh integrity -- performs some internal consistency checks
 /// and throws error if violated.
 void check_mesh_integrity();

 /// \short Number of internal edges
 unsigned Nglobal_edge;

 /// \short Storage for global node numbers listed element-by-element
 Vector<unsigned> Global_node;

 /// \short Vector of vectors containing the boundary ids of the
 /// elements' edges
 Vector<Vector<unsigned> > Edge_boundary;

 /// \short Vector of vectors containing the global edge index of
 // the elements' edges
 Vector<Vector<unsigned> > Edge_index;

 /// \short Vector of double attributes for each element
 Vector<double> Element_attribute;

 /// Vectors of hole centre coordinates
 Vector<Vector<double> > Hole_centre;

 };

}

#endif
