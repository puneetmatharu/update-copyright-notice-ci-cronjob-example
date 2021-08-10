//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC// Copyright (C) 2006-2021 Matthias Heil and Andrew Hazel
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
//Include guards
#ifndef OOMPH_PRECONDITION_HEADER
#define OOMPH_PRECONDITION_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "matrices.h"



namespace oomph
{

 // Forward declaration of block preconditioners (for turn into subs.)
 template<typename MATRIX> class BlockPreconditioner;

 // Forward declaration of problem class (for obsolete setup function)
 class Problem;

 //=============================================================================
 /// \short Preconditioner base class. Gives an interface to call all other
 /// preconditioners through and stores the matrix and communicator
 /// pointers. All preconditioners should be derived from this class.
 //=============================================================================
 class Preconditioner : public DistributableLinearAlgebraObject
 {

 public:

  /// Constructor
  Preconditioner() : Silent_preconditioner_setup(false), Stream_pt(0),
		     Matrix_pt(0), Comm_pt(0), Setup_time(0){};

  /// Broken copy constructor
  Preconditioner(const Preconditioner&)
  {
   BrokenCopy::broken_copy("Preconditioner");
  }

  /// Broken assignment operator
  void operator=(const Preconditioner&)
  {
   BrokenCopy::broken_assign("Preconditioner");
  }

  /// Destructor (empty)
  virtual ~Preconditioner(){}

  /// \short Apply the preconditioner. Pure virtual generic interface
  /// function. This method should apply the preconditioner operator to the
  /// vector r and return the vector z.
  virtual void preconditioner_solve(const DoubleVector &r, DoubleVector &z)=0;

  /// \short Apply the preconditioner. Pure virtual generic interface
  /// function. This method should apply the preconditioner operator to the
  /// vector r and return the vector z. (broken virtual)
  virtual void preconditioner_solve_transpose(const DoubleVector &r,
					      DoubleVector &z)
  {
   // Throw an error
   throw OomphLibError("This function hasn't been implemented yet!",
		       OOMPH_CURRENT_FUNCTION,
		       OOMPH_EXCEPTION_LOCATION);
  }

  /// \short Setup the preconditioner: store the matrix pointer and the
  /// communicator pointer then call preconditioner specific setup()
  /// function.
  void setup(DoubleMatrixBase* matrix_pt)
  {
   // Store matrix pointer
   set_matrix_pt(matrix_pt);

   // Extract and store communicator pointer
   DistributableLinearAlgebraObject* dist_obj_pt=
    dynamic_cast<DistributableLinearAlgebraObject*>(matrix_pt);
   if (dist_obj_pt!=0)
    {
     set_comm_pt(dist_obj_pt->distribution_pt()->communicator_pt());
    }
   else
    {
     set_comm_pt(0);
    }

   double setup_time_start = TimingHelpers::timer();
   setup();
   double setup_time_finish = TimingHelpers::timer();
   Setup_time = setup_time_finish - setup_time_start;
  }

  /// \short Compatability layer for old preconditioners where problem
  /// pointers were needed. The problem pointer is only used to get a
  /// communicator pointer.
  void setup(const Problem* problem_pt, DoubleMatrixBase* matrix_pt)
  {
   ObsoleteCode::obsolete();
   setup(matrix_pt);
  }

  /// Set up the block preconditioner quietly!
  void enable_silent_preconditioner_setup()
  {
   // Set the appropriate (silencing) boolean to true
   Silent_preconditioner_setup=true;
  } // End of enable_silent_preconditioner_setup

  
  /// Be verbose in the block preconditioner setup 
  void disable_silent_preconditioner_setup()
  {
   // Set the appropriate (silencing) boolean to false
   Silent_preconditioner_setup=false;
  } // End of disable_silent_preconditioner_setup


  /// \short Setup the preconditioner. Pure virtual generic interface
  /// function.
  virtual void setup() = 0;

  /// \short Clean up memory (empty). Generic interface function.
  virtual void clean_up_memory(){}

  /// Get function for matrix pointer.
  virtual DoubleMatrixBase* matrix_pt() const
  {
#ifdef PARANOID
   if(Matrix_pt == 0)
    {
     std::ostringstream error_msg;
     error_msg << "Matrix pointer is null.";
     throw OomphLibError(error_msg.str(),
                         OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   return Matrix_pt;
  }

  /// \short Set the matrix pointer.
  virtual void set_matrix_pt(DoubleMatrixBase* matrix_pt) {Matrix_pt = matrix_pt;}

  /// Get function for comm pointer.
  virtual const OomphCommunicator* comm_pt() const 
  {
   // If we are using MPI then the comm pointer should not be null.
#ifdef OOMPH_HAS_MPI
#ifdef PARANOID
   if(Comm_pt == 0)
    {
     std::ostringstream error_msg;
     error_msg
      << "Tried to access a null communicator pointer. This might mean you are\n"
      << "trying to use it in a non-parallel case. Or it might mean you haven't\n"
      << "set it properly.";
     throw OomphLibError(error_msg.str(),
                         OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
#endif
   return Comm_pt;
  }
  
  /// \short Set the communicator pointer
  virtual void set_comm_pt(const OomphCommunicator* const comm_pt)
  {Comm_pt = comm_pt;}

  /// \short Returns the time to setup the preconditioner.
  double setup_time() const
  {return Setup_time;}

  /// Virtual interface function for making a preconditioner a subsidiary
  /// of a block preconditioner. By default nothing is needed, but if this
  /// preconditioner is also a block preconditioner then things need to
  /// happen. There's an assumption here that the block preconditioner will
  /// be in CR form but since that assumption is hard coded all over
  /// BlockPreconditioner we're safe.
  virtual void turn_into_subsidiary_block_preconditioner
  (BlockPreconditioner<CRDoubleMatrix>* master_block_prec_pt,
   const Vector<unsigned>& doftype_in_master_preconditioner_coarse) {}

  /// Virtual interface function for making a preconditioner a subsidiary
  /// of a block preconditioner. By default nothing is needed, but if this
  /// preconditioner is also a block preconditioner then things need to
  /// happen. Version for coarsening dof-types.
  virtual void turn_into_subsidiary_block_preconditioner
  (BlockPreconditioner<CRDoubleMatrix>* master_block_prec_pt,
   const Vector<unsigned>& doftype_in_master_preconditioner_coarse,
   const Vector<Vector<unsigned> > & doftype_coarsen_map_coarse) {}

 protected:  

  /// Boolean to indicate whether or not the build should be done silently
  bool Silent_preconditioner_setup;
  
  /// Pointer to the output stream -- defaults to std::cout
  std::ostream* Stream_pt;
  
 private:

  /// Storage for a pointer to the matrix.
  DoubleMatrixBase* Matrix_pt;

  /// \short Storage for a pointer to the communicator. Null 
  /// if the preconditioner should not be distributed.
  const OomphCommunicator* Comm_pt;

  /// The time it takes to set up this preconditioner.
  double Setup_time;

 };//Preconditioner


 //=============================================================================
 /// The Identity Preconditioner
 //=============================================================================
 class IdentityPreconditioner : public Preconditioner
 {

 public:

  // Constructor - nothing to construct
  IdentityPreconditioner() {}

  /// Broken copy constructor
  IdentityPreconditioner(const IdentityPreconditioner&)
  {
   BrokenCopy::broken_copy("IdentityPreconditioner");
  }

  /// Broken assignment operator
  void operator=(const IdentityPreconditioner&)
  {
   BrokenCopy::broken_assign("IdentityPreconditioner");
  }

  /// Destructor (empty)
  virtual ~IdentityPreconditioner(){}

  /// setup method - just sets the distribution
  virtual void setup()
  {
   // first attempt to cast to DistributableLinearAlgebraObject
   DistributableLinearAlgebraObject* dist_matrix_pt =
    dynamic_cast<DistributableLinearAlgebraObject*>(matrix_pt());

   // if it is a distributable matrix
   if (dist_matrix_pt != 0)
    {
     // store the distribution
     this->build_distribution(dist_matrix_pt->distribution_pt());
    }
   // else it is not a distributable matrix
   else
    {
     // # of rows in the matrix
     unsigned n_row=matrix_pt()->nrow();

     LinearAlgebraDistribution dist(comm_pt(),n_row,false);
     this->build_distribution(dist);
    }
  }

  /// \short Apply the preconditioner. This method should apply the
  /// preconditioner operator to the vector r and return the vector z.
  virtual void preconditioner_solve(const DoubleVector&r, DoubleVector &z)
  {
#ifdef PARANOID
   if (*r.distribution_pt() != *this->distribution_pt())
    {
     std::ostringstream error_message_stream;
     error_message_stream
      << "The r vector must have the same distribution as the preconditioner. "
      << "(this is the same as the matrix passed to setup())";
     throw OomphLibError(error_message_stream.str(), OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
    }
   if (z.built())
    {
     if (*z.distribution_pt() != *this->distribution_pt())
      {
       std::ostringstream error_message_stream;
       error_message_stream
        << "The z vector distribution has been setup; it must have the "
        << "same distribution as the r vector (and preconditioner).";
       throw OomphLibError(error_message_stream.str(), OOMPH_CURRENT_FUNCTION,
                           OOMPH_EXCEPTION_LOCATION);
      }
    }
#endif

   // apply
   z=r;
  }
  
  
  /// \short Apply the preconditioner. This method should apply the
  /// preconditioner operator to the vector r and return the vector z.
  void preconditioner_solve_transpose(const DoubleVector& r, DoubleVector &z)
  {
   // Applying the preconditioner to the transposed system is exactly the
   // same as applying the preconditioner to the original system
   preconditioner_solve(r,z);
  }
 };
}

#endif