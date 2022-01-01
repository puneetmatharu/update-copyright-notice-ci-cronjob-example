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
#ifndef OOMPH_DOUBLE_VECTOR_CLASS_HEADER
#define OOMPH_DOUBLE_VECTOR_CLASS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

// c++ headers
#include <algorithm>
#include <ostream>

// oomph headers
#include "linear_algebra_distribution.h"


namespace oomph{

class CRDoubleMatrix;

//=============================================================================
/// \short A vector in the mathematical sense, initially developed for
/// linear algebra type applications.
/// If MPI then this vector can be distributed - its distribution is 
/// described by the LinearAlgebraDistribution object at Distribution_pt. 
/// Data is stored in a C-style pointer vector (double*)
//=============================================================================
 class DoubleVector : public DistributableLinearAlgebraObject
{                                                        
 
 public :                     
  
  /// \short Constructor for an uninitialized DoubleVector
  DoubleVector()
   : Values_pt(0), Internal_values(true), Built(false)
  {}
 
 /// \short Constructor. Assembles a DoubleVector with a prescribed
 /// distribution. Additionally every entry can be set (with argument v - 
 /// defaults to 0).
 DoubleVector(const LinearAlgebraDistribution* const &dist_pt, 
              const double& v=0.0)
  : Values_pt(0), Internal_values(true), Built(false)
  {
   this->build(dist_pt,v);
  }

 /// \short Constructor. Assembles a DoubleVector with a prescribed
 /// distribution. Additionally every entry can be set (with argument v - 
 /// defaults to 0).
 DoubleVector(const LinearAlgebraDistribution& dist, 
              const double& v=0.0)
  : Values_pt(0), Internal_values(true), Built(false)
  {
   this->build(dist,v);
  }
   
 /// Destructor - just calls this->clear() to delete the distribution and data
 ~DoubleVector()
  {
   this->clear();
  }                 
 
 /// Copy constructor
 DoubleVector(const DoubleVector& new_vector)
  : DistributableLinearAlgebraObject(),
  Values_pt(0), Internal_values(true), Built(false)
  {
   this->build(new_vector);
  }
 
 /// assignment operator
 void operator=(const DoubleVector& old_vector)
  {
   this->build(old_vector);
  }
 
 /// \short Just copys the argument DoubleVector
 void build(const DoubleVector& old_vector);

 /// \short Assembles a DoubleVector with distribution dist, if v is specified 
 /// each element is set to v, otherwise each element is set to 0.0
 void build(const LinearAlgebraDistribution& dist, 
            const double& v)
 {
  this->build(&dist,v);
 }

 /// \short Assembles a DoubleVector with distribution dist, if v is specified 
 /// each element is set to v, otherwise each element is set to 0.0
 void build(const LinearAlgebraDistribution* const &dist_pt, 
            const double& v);

 /// \short Assembles a DoubleVector with a distribution dist and coefficients
 /// taken from the vector v.
 /// Note. The vector v MUST be of length nrow()
 void build(const LinearAlgebraDistribution& dist,
            const Vector<double>& v)
 {
  this->build(&dist,v);
 }

 /// \short Assembles a DoubleVector with a distribution dist and coefficients
 /// taken from the vector v.
 /// Note. The vector v MUST be of length nrow()
 void build(const LinearAlgebraDistribution* const &dist_pt,
            const Vector<double>& v);

 /// \short initialise the whole vector with value v
 void initialise(const double& v);

 /// \short initialise the vector with coefficient from the vector v.
 /// Note: The vector v must be of length 
 void initialise(const Vector<double> v);

 /// \short wipes the DoubleVector
 void clear() 
  {
   if (Internal_values)
    {
     delete[] Values_pt;
    }
   Values_pt = 0;
   this->clear_distribution();
   Built=false;
  }

 // indicates whether this DoubleVector is built
 bool built() const { return Built; }

 /// \short Allows are external data to be used by this vector. 
 /// WARNING: The size of the external data must correspond to the 
 /// LinearAlgebraDistribution dist_pt argument.
 /// 1. When a rebuild method is called new internal values are created.
 /// 2. It is not possible to redistribute(...) a vector with external
 /// values .
 /// 3. External values are only deleted by this vector if
 /// delete_external_values = true.
 void set_external_values(const LinearAlgebraDistribution* const& dist_pt,
                          double* external_values,
                          bool delete_external_values)
  {
   // clean the memory
   this->clear();

   // Set the distribution
   this->build_distribution(dist_pt);
   // Say that it's built
   Built = true;

   // set the external values
   set_external_values(external_values,delete_external_values);
  }
 
 /// \short Allows are external data to be used by this vector. 
 /// WARNING: The size of the external data must correspond to the 
 /// distribution of this vector.
 /// 1. When a rebuild method is called new internal values are created.
 /// 2. It is not possible to redistribute(...) a vector with external
 /// values .
 /// 3. External values are only deleted by this vector if
 /// delete_external_values = true.
 void set_external_values(double* external_values, 
                          bool delete_external_values)
  {
#ifdef PARANOID
   // check that this distribution is setup
   if (!this->distribution_built())
    {
    // if this vector does not own the double* values then it cannot be
    // distributed.
    // note: this is not stictly necessary - would just need to be careful 
    // with delete[] below.
     std::ostringstream error_message;    
     error_message << "The distribution of the vector must be setup before "
                   << "external values can be set"; 
     throw OomphLibError(error_message.str(),
                         OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   if (Internal_values)
    {
     delete[] Values_pt;
    }
   Values_pt = external_values;
   Internal_values = delete_external_values;
  }

 /// \short The contents of the vector are redistributed to match the new
 /// distribution. In a non-MPI rebuild this method works, but does nothing. 
 /// \b NOTE 1: The current distribution and the new distribution must have
 /// the same number of global rows.
 /// \b NOTE 2: The current distribution and the new distribution must have
 /// the same Communicator.
 void redistribute(const LinearAlgebraDistribution* const& dist_pt);
   
 /// \short [] access function to the (local) values of this vector
 double& operator[](int i);

 /// \short == operator
 bool operator==(const DoubleVector& v);

 /// \short += operator with another vector
 void operator+=(const DoubleVector& v);

 /// \short -= operator with another vector
 void operator-=(const DoubleVector& v);

 /// \short multiply by a double
 void operator*=(const double& d);

 /// \short divide by a double
 void operator/=(const double& d);

 /// \short [] access function to the (local) values of this vector
 const double& operator[](int i) const;

 /// \short Ouput operator for DoubleVector
 friend std::ostream& operator<< (std::ostream &out, const DoubleVector& v);

 /// \short returns the maximum coefficient
 double max() const;

 /// access function to the underlying values
 double* values_pt() {return Values_pt;}

 /// \short access function to the underlying values (const version)
 double* values_pt() const {return Values_pt;}

 /// output the global contents of the vector
 void output(std::ostream &outfile,const int& output_precision = -1) const;

 /// output the global contents of the vector
 void output(std::string filename,const int& output_precision = -1) const
  {
    // Open file
    std::ofstream some_file;
    some_file.open(filename.c_str());
    output(some_file,output_precision);
    some_file.close();
  }

 /// output the local contents of the vector
 void output_local_values(std::ostream &outfile,
                          const int& output_precision = -1) const;

 /// output the local contents of the vector
 void output_local_values(std::string filename,
                          const int& output_precision = -1) const
  {
    // Open file
    std::ofstream some_file;
    some_file.open(filename.c_str());
    output_local_values(some_file,output_precision);
    some_file.close();
  }

 /// output the local contents of the vector
 void output_local_values_with_offset(std::ostream &outfile,
                                      const int& output_precision = -1) const;

 /// output the local contents of the vector
 void output_local_values_with_offset(std::string filename,
                                      const int& output_precision = -1) const
  {
    // Open file
    std::ofstream some_file;
    some_file.open(filename.c_str());
    output_local_values_with_offset(some_file,output_precision);
    some_file.close();
  }

 /// compute the dot product of this vector with the vector vec.
 double dot(const DoubleVector& vec) const;

 /// compute the 2 norm of this vector 
 double norm() const;

 /// compute the A-norm using the matrix at matrix_pt
 double norm(const CRDoubleMatrix* matrix_pt) const;

 private :
 
 /// the local vector
 double* Values_pt;

 /// \short Boolean flag to indicate whether the vector's data (values_pt) 
 /// is owned by this vector.
 bool Internal_values;

 /// indicates that the vector has been built and is usable
 bool Built;

}; //end of DoubleVector

//=================================================================
/// Namespace for helper functions for DoubleVectors
//=================================================================
namespace DoubleVectorHelpers
{
 /// \short Concatenate DoubleVectors.
 /// Takes a Vector of DoubleVectors. If the out vector is built, we will not
 /// build a new distribution. Otherwise we build a uniform distribution.
 /// 
 /// The rows of the out vector is seen "as it is" in the in vectors.
 /// For example, if we have DoubleVectors with distributions A and B, 
 /// distributed across two processors (p0 and p1),
 /// 
 /// A: [a0] (on p0)    B: [b0] (on p0)
 ///    [a1] (on p1)       [b1] (on P1),
 /// 
 /// then the out_vector is
 ///
 /// [a0  (on p0)
 ///  a1] (on p0)
 /// [b0]  (on p1)
 ///  b1] (on p1),
 ///
 /// Communication is required between processors. The sum of the global number
 /// of rows in the in vectors must equal to the global number of rows in the
 /// out vector. This condition must be met if one is to supply an out vector
 /// with a distribution, otherwise we can let the function generate the
 /// out vector distribution itself. 
 void concatenate(const Vector<DoubleVector*> &in_vector_pt,
                  DoubleVector &out_vector);

 /// \short Wrapper around the other concatenate(...) function.
 /// Be careful with Vector of vectors. If the DoubleVectors are resized,
 /// there could be reallocation of memory. If we wanted to use the function
 /// which takes a Vector of pointers to DoubleVectors, we would either have
 /// to invoke new and remember to delete, or create a temporary Vector to
 /// store pointers to the DoubleVector objects.
 /// This wrapper is meant to make life easier for the user by avoiding calls 
 /// to new/delete AND without creating a temporary vector of pointers to 
 /// DoubleVectors. 
 /// If we had C++ 11, this would be so much nicer since we can use smart 
 /// pointers which will delete themselves, so we do not have to remember 
 /// to delete!
 void concatenate(Vector<DoubleVector> &in_vector,
                  DoubleVector &out_vector);

 /// \short Split a DoubleVector into the out DoubleVectors.
 /// Let vec_A be the in Vector, and let vec_B and vec_C be the out vectors.
 /// Then the splitting of vec_A is depicted below:
 /// vec_A: [a0  (on p0)
 ///         a1] (on p0)
 ///        [a2  (on p1)
 ///         a3] (on p1)
 ///
 /// vec_B: [a0] (on p0)    vec_C: [a2] (on p0)
 ///        [a1] (on p1)           [a3] (on p1)
 /// 
 /// Communication is required between processors.
 /// The out_vector_pt must contain pointers to DoubleVector which has already
 /// been built with the correct distribution; the sum of the number of global 
 /// row of the out vectors must be the same the the number of global rows of
 /// the in vector.
 void split(const DoubleVector &in_vector, 
            Vector<DoubleVector*> &out_vector_pt);

 /// \short Wrapper around the other split(...) function.
 /// Be careful with Vector of vectors. If the DoubleVectors are resized,
 /// there could be reallocation of memory. If we wanted to use the function
 /// which takes a Vector of pointers to DoubleVectors, we would either have
 /// to invoke new and remember to delete, or create a temporary Vector to
 /// store pointers to the DoubleVector objects.
 /// This wrapper is meant to make life easier for the user by avoiding calls 
 /// to new/delete AND without creating a temporary vector of pointers to 
 /// DoubleVectors. 
 /// If we had C++ 11, this would be so much nicer since we can use smart 
 /// pointers which will delete themselves, so we do not have to remember 
 /// to delete!
 void split(const DoubleVector &in_vector, 
            Vector<DoubleVector> &out_vector);

 /// \short Concatenate DoubleVectors.
 /// Takes a Vector of DoubleVectors. If the out vector is built, we will not
 /// build a new distribution. Otherwise a new distribution will be built
 /// using LinearAlgebraDistribution::concatenate(...).
 /// 
 /// The out vector has its rows permuted according to the individual 
 /// distributions of the in vectors. For example, if we have DoubleVectors 
 /// with distributions A and B, distributed across two processors 
 /// (p0 and p1),
 /// 
 /// A: [a0] (on p0)    B: [b0] (on p0)
 ///    [a1] (on p1)       [b1] (on P1),
 /// 
 /// then the out_vector is
 ///
 /// [a0  (on p0)
 ///  b0] (on p0)
 /// [a1  (on p1)
 ///  b1] (on p1),
 ///
 /// as opposed to
 ///
 /// [a0  (on p0)
 ///  a1] (on p0)
 /// [b0  (on p1)
 ///  b1] (on p1).
 ///
 /// Note (1): The out vector may not be uniformly distributed even
 /// if the the in vectors have uniform distributions. The nrow_local of the
 /// out vector will be the sum of the nrow_local of the in vectors.
 /// Try this out with two distributions of global rows 3 and 5, uniformly
 /// distributed across two processors. Compare this against a distribution
 /// of global row 8 distributed across two processors.
 ///
 /// There are no MPI send and receive, the data stays on the processor
 /// as defined by the distributions from the in vectors.
 void concatenate_without_communication(
  const Vector<DoubleVector*> &in_vector_pt, DoubleVector &out_vector);

 /// \short Wrapper around the other concatenate_without_communication(...)
 /// function.
 /// Be careful with Vector of vectors. If the DoubleVectors are resized,
 /// there could be reallocation of memory. If we wanted to use the function
 /// which takes a Vector of pointers to DoubleVectors, we would either have
 /// to invoke new and remember to delete, or create a temporary Vector to
 /// store pointers to the DoubleVector objects.
 /// This wrapper is meant to make life easier for the user by avoiding calls 
 /// to new/delete AND without creating a temporary vector of pointers to 
 /// DoubleVectors. 
 /// If we had C++ 11, this would be so much nicer since we can use smart 
 /// pointers which will delete themselves, so we do not have to remember 
 /// to delete!
 void concatenate_without_communication(
  Vector<DoubleVector> &in_vector, DoubleVector &out_vector);

 /// \short Split a DoubleVector into the out DoubleVectors.
 /// Data stays on its current processor, no data is sent between processors.
 /// This results in our vectors which are a permutation of the in vector.
 /// 
 /// Let vec_A be the in Vector, and let vec_B and vec_C be the out vectors.
 /// Then the splitting of vec_A is depicted below:
 /// vec_A: [a0  (on p0)
 ///         a1] (on p0)
 ///        [a2  (on p1)
 ///         a3] (on p1)
 ///
 /// vec_B: [a0] (on p0)    vec_C: [a1] (on p0)
 ///        [a2] (on p1)           [a3] (on p1).
 ///
 /// This means that the distribution of the in vector MUST be a 
 /// concatenation of the out vector distributions, refer to
 /// LinearAlgebraDistributionHelpers::concatenate(...) to concatenate
 /// distributions.
 void split_without_communication(const DoubleVector &in_vector, 
                                  Vector<DoubleVector*> &out_vector_pt);

 /// \short Wrapper around the other split_without_communication(...) 
 /// function.
 /// Be careful with Vector of vectors. If the DoubleVectors are resized,
 /// there could be reallocation of memory. If we wanted to use the function
 /// which takes a Vector of pointers to DoubleVectors, we would either have
 /// to invoke new and remember to delete, or create a temporary Vector to
 /// store pointers to the DoubleVector objects.
 /// This wrapper is meant to make life easier for the user by avoiding calls 
 /// to new/delete AND without creating a temporary vector of pointers to 
 /// DoubleVectors. 
 /// If we had C++ 11, this would be so much nicer since we can use smart 
 /// pointers which will delete themselves, so we do not have to remember 
 /// to delete!
 void split_without_communication(const DoubleVector &in_vector, 
                                  Vector<DoubleVector> &out_vector);

} // end of DoubleVectorHelpers namespace

} // end of oomph namespace
#endif
