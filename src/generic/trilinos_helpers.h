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
#ifndef OOMPH_TRILINOS_HELPERS_HEADER
#define OOMPH_TRILINOS_HELPERS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif


// trilinos headers
#include "Epetra_Map.h"
#include "Epetra_LocalMap.h"
#include "Epetra_Vector.h"
#include "Epetra_CrsMatrix.h"
#include "EpetraExt_MatrixMatrix.h"
#include "ml_epetra_utils.h"

#ifdef OOMPH_HAS_MPI
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif

// oomph-lib headers
#include "matrices.h"

// mpi includes
#ifdef OOMPH_HAS_MPI
#include "double_vector.h"
#include "linear_algebra_distribution.h"
#endif

namespace oomph
{

//// forward declaration of oomph-lib compressed row matrix
class CRDoubleMatrix;


//=============================================================================
/// \short Helper namespace for use with the Trilinos Epetra package. 
/// Contains functions to generate two Epetra containers (Epetra_Vector and
/// Epetra_CrsMatrix) and provides access to the trilinos matrix-matrix
/// and matrix-vector product routines.
//=============================================================================
namespace TrilinosEpetraHelpers
{

 // VECTOR METHODS ============================================================

 /// \short create an Epetra_Vector from an oomph-lib DoubleVector.
 /// If oomph_vec is NOT distributed (i.e. locally replicated) and 
 /// on more than one processor, then the returned Epetra_Vector will be 
 /// uniformly distributed. If the oomph_vec is distributed then the 
 /// Epetra_Vector returned will have the same distribution as oomp_vec.
 Epetra_Vector* create_distributed_epetra_vector(const DoubleVector& 
                                                 oomph_vec);
 
 /// \short create an Epetra_Vector based on the argument oomph-lib 
 /// LinearAlgebraDistribution
 /// If dist is NOT distributed and 
 /// on more than one processor, then the returned Epetra_Vector will be 
 /// uniformly distributed. If dist is distributed then the Epetra_Vector 
 /// returned will have the same distribution as dist.
 /// The coefficient values are not set.
 Epetra_Vector* create_distributed_epetra_vector
  (const LinearAlgebraDistribution* dist_pt);

 /// \short create an Epetra_Vector equivalent of DoubleVector
 /// The argument DoubleVector must be built.
 /// The Epetra_Vector will point to, and NOT COPY the underlying data in the 
 /// DoubleVector.
 /// The oomph-lib DoubleVector and the returned Epetra_Vector will have the 
 /// the same distribution.
 Epetra_Vector* create_epetra_vector_view_data(DoubleVector& oomph_vec);

 /// \short Helper function to copy the contents of a Trilinos vector to an
 /// oomph-lib distributed vector. The distribution of the two vectors must
 /// be identical 
 void copy_to_oomphlib_vector(const Epetra_Vector* epetra_vec_pt,
                              DoubleVector& oomph_vec);

 // MATRIX METHODS ============================================================

 /// \short create an Epetra_CrsMatrix from an oomph-lib CRDoubleMatrix.
 /// If oomph_matrix_pt is NOT distributed (i.e. locally replicated) and 
 /// on more than one processor, then the returned Epetra_Vector will be 
 /// uniformly distributed. If the oomph_matrix_pt is distributed then the 
 /// Epetra_CrsMatrix returned will have the same distribution as
 /// oomph_matrix_pt.
 /// The LinearAlgebraDistribution argument dist_pt should specify the 
 /// distribution of the object this matrix will operate on.
 Epetra_CrsMatrix* create_distributed_epetra_matrix
  (const CRDoubleMatrix* oomph_matrix_pt,
   const LinearAlgebraDistribution* dist_pt); 
 
 /// \short create and Epetra_CrsMatrix from an oomph-lib CRDoubleMatrix.
 /// Specialisation for Trilinos AztecOO.
 /// If oomph_matrix_pt is NOT distributed (i.e. locally replicated) and 
 /// on more than one processor, then the returned Epetra_Vector will be 
 /// uniformly distributed. If the oomph_matrix_pt is distributed then the 
 /// Epetra_CrsMatrix returned will have the same distribution as
 /// oomph_matrix_pt.
 /// For AztecOO, the column map is ordered such that the local rows are
 /// first.
 Epetra_CrsMatrix* create_distributed_epetra_matrix_for_aztecoo
  (CRDoubleMatrix* oomph_matrix_pt);

 // MATRIX OPERATION METHODS ==================================================

 /// \short  Function to perform a matrix-vector multiplication on a 
 /// oomph-lib matrix and vector using Trilinos functionality.
 /// NOTE 1. the matrix and the vectors must have the same communicator.
 /// NOTE 2. The vector will be returned with the same distribution
 /// as the matrix, unless a distribution is predefined in the solution
 /// vector in which case the vector will be returned with that distribution.
 void multiply(const CRDoubleMatrix* matrix,
               const DoubleVector& x,
               DoubleVector &soln);

 /// \short Function to perform a matrix-matrix multiplication on oomph-lib
 /// matrices by using Trilinos functionality.
 /// \b NOTE 1. There are two Trilinos matrix-matrix multiplication methods 
 /// available, using either the EpetraExt::MatrixMatrix class (if use_ml == 
 /// false) or using ML (Epetra_MatrixMult method)
 /// \b NOTE 2. the solution matrix (matrix_soln) will be returned with the 
 /// same distribution as matrix1
 /// \b NOTE 3. All matrices must share the same communicator. 
 void multiply(const CRDoubleMatrix &matrix_1,
               const CRDoubleMatrix &matrix_2,
               CRDoubleMatrix &matrix_soln,
               const bool& use_ml = false);

 // HELPER METHODS ============================================================

 /// create an Epetra_Map corresponding to the LinearAlgebraDistribution
 Epetra_Map* create_epetra_map
  (const LinearAlgebraDistribution* const dist);
}; // end of trilinos epetra helpers namespace
} // end of namspace oomph
#endif
