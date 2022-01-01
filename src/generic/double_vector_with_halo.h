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
#ifndef OOMPH_DOUBLE_VECTOR_WITH_HALO_CLASS_HEADER
#define OOMPH_DOUBLE_VECTOR_WITH_HALO_CLASS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#include<map>
#include "double_vector.h"

namespace oomph{

class DoubleVectorWithHaloEntries;

//=====================================================================
/// A class that stores the halo/haloed entries required when
/// using a DoubleVectorWithHaloEntries.
/// This is a separate class so thay many different Vectors can share
/// the same object.
/// The constructor requires the distribution of the DoubleVector 
/// (if you pass in a different distribution things will go badly wrong)
/// and a vector that specifies which GLOBAL eqn numbers are required
/// on each processor.
//=====================================================================
class DoubleVectorHaloScheme
{
 /// \short The DoubleVectorWithHaloEntries should be able to access the
 /// private data.
 friend class DoubleVectorWithHaloEntries;

 /// \short Storage for the translation scheme from global unknown
 /// to local index in the additional storage vector.
 std::map<unsigned,unsigned> Local_index;

 /// \short The haloed entries that will be sent in a format compatible
 /// with MPI_Alltoallv i.e. (send_to_proc0,send_to_proc1 ... send_to_procn)
 Vector<unsigned> Haloed_eqns;

 /// \short Storage for the number of haloed entries to be sent to each
 /// processor
 Vector<int> Haloed_n;

 /// \short Storage for the offsets of the haloed entries for each processor
 /// in the packed Haloed_eqns array
 Vector<int> Haloed_displacement;
 
 /// \short Storage for all the entries that are to be received from
 /// other processors 
 /// (received_from_proc0,received_from_proc1,...received_from_procn)
 Vector<unsigned> Halo_eqns;

 /// \short Storage for the number of entries to be received from each
 /// other processor
 Vector<int> Halo_n;

 /// \short Storage for the offsets of the processor data in the 
 /// receive buffer
 Vector<int> Halo_displacement;
 

 /// \short Store the distribution that was used to setup the halo scheme
 LinearAlgebraDistribution *Distribution_pt;

public:

 ///\short Constructor that sets up the required information communicating
 ///between all processors. Requires two "all to all" communications.
 ///Arguments are the distribution of the DoubleVector and a
 ///Vector of global unknowns required on this processor.
 DoubleVectorHaloScheme(LinearAlgebraDistribution* const &dist_pt,
                        const Vector<unsigned> &required_global_eqn);

 ///\short Return the number of halo values
 inline unsigned n_halo_values() const {return Local_index.size();}

 ///\short Return the pointer to the distirbution used to setup
 ///the halo information
 inline LinearAlgebraDistribution* &distribution_pt() {return Distribution_pt;}

 ///\short Function that sets up a vector of pointers to halo 
 /// data, index using the scheme in Local_index
 void setup_halo_dofs(const std::map<unsigned,double*> &halo_data_pt,
                      Vector<double*> &halo_dof_pt);


 ///\short Return the local index associated with the global equation
 inline unsigned local_index(const unsigned &global_eqn)
  {
   //Does the entry exist in the map
   std::map<unsigned,unsigned>::iterator it = Local_index.find(global_eqn);
   //If it does return it
   if(it!=Local_index.end()) {return it->second;}
   //Otherwise throw an error
   else
    {
     std::ostringstream error_stream;
     error_stream << "Global equation " << global_eqn << " "
                  << "has not been set as halo\n";
     throw OomphLibError(error_stream.str(),
                         OOMPH_CURRENT_FUNCTION,
                         OOMPH_EXCEPTION_LOCATION);
     return 0;
    }
  }

};


///=====================================================================
/// An extension of DoubleVector that allows access to certain
/// global entries that are not stored locally. Synchronisation of these
/// values must be performed manually by calling the synchronise() 
/// function. Synchronisation can only be from the haloed to the halo,
/// but the local halo entries can all be summed and stored in the
/// haloed value. 
///======================================================================
class DoubleVectorWithHaloEntries : public DoubleVector
{
 /// \short Pointer to the lookup scheme that stores information about
 /// on which processor the required information is haloed
 DoubleVectorHaloScheme* Halo_scheme_pt;

 /// \short Vector of the halo values
 Vector<double> Halo_value;
  
 public :                     
  
  /// \short Constructor for an uninitialized DoubleVectorWithHaloEntries
 DoubleVectorWithHaloEntries() : DoubleVector(), Halo_scheme_pt(0) {}
 
 /// \short Constructor. Assembles a DoubleVectorWithHaloEntries 
 /// with a prescribed
 /// distribution. Additionally every entry can be set (with argument v - 
 /// defaults to 0).
 DoubleVectorWithHaloEntries(const LinearAlgebraDistribution* const &dist_pt, 
                             DoubleVectorHaloScheme* const 
                             &halo_scheme_pt=0,
                             const double& v=0.0) :
  DoubleVector(dist_pt,v) 
  {
   //construct the halo scheme
   this->build_halo_scheme(halo_scheme_pt);
  }

 /// \short Constructor. Assembles a DoubleVectorWithHaloEntries
 /// with a prescribed
 /// distribution. Additionally every entry can be set (with argument v - 
 /// defaults to 0).
 DoubleVectorWithHaloEntries(const LinearAlgebraDistribution& dist, 
                             DoubleVectorHaloScheme* const 
                             &halo_scheme_pt=0,
                             const double& v=0.0)
  : DoubleVector(dist,v)
  {
   //construct the halo scheme
   this->build_halo_scheme(halo_scheme_pt);
  }
   
 /// Destructor 
 ~DoubleVectorWithHaloEntries() {}

 
 /// Copy constructor from any DoubleVector
 DoubleVectorWithHaloEntries(const DoubleVectorWithHaloEntries& new_vector)
  : DoubleVector(new_vector)
  {
   //Build the appropriate halo scheme
   this->build_halo_scheme(new_vector.halo_scheme_pt());
  }

 /// Copy constructor from any DoubleVector
 DoubleVectorWithHaloEntries(const DoubleVector& new_vector,
                             DoubleVectorHaloScheme* const &halo_scheme_pt=0)
  : DoubleVector(new_vector)
  {
   //Construct the halo scheme
   this->build_halo_scheme(halo_scheme_pt);
  }
 
 /// assignment operator
 void operator=(const DoubleVectorWithHaloEntries& old_vector)
  {
   this->build(old_vector);
   //Do some other stuff
   this->build_halo_scheme(old_vector.halo_scheme_pt());
 }

 /// Direct access to global entry
 inline double &global_value(const unsigned &i)
  {
   //Only need to worry about the distributed case if 
   //we have compiled with MPI
#ifdef OOMPH_HAS_MPI
   if(this->distributed())
    {
     const unsigned first_row_local = this->first_row();
     const unsigned n_row_local = this->nrow_local();

     //If we are in range then just call the local value
     if((i >= first_row_local) && (i < first_row_local + n_row_local))
      {
       return (*this)[i-first_row_local];
      }
     //Otherwise the entry is not stored in the local processor
     //and we must have haloed it
     else
      {
#ifdef PARANOID
       if(Halo_scheme_pt==0)
        {
         std::ostringstream error_stream;
         error_stream << 
          "Halo data requested, but no halo scheme has been setup\n" <<
          "You should call this->build_halo_scheme(halo_scheme_pt).\n" <<
          "You may wish to setup the scheme for the Problem using \n" <<
          "Problem::setup_dof_halo_scheme()\n";
         
         throw OomphLibError(error_stream.str(),
                             OOMPH_CURRENT_FUNCTION,
                             OOMPH_EXCEPTION_LOCATION);
        }
#endif
       return Halo_value[Halo_scheme_pt->local_index(i)];
      }
    }
   //If not distributed the global entry is
   //the local entry
   else
#endif
    {
     return (*this)[i];
    }
  }

 ///Direct access to the global entry (const version)
 const double& global_value(const unsigned &i)  const
  {
   //Only need to worry about the distributed case if 
   //we have compiled with MPI
#ifdef OOMPH_HAS_MPI
   if(this->distributed())
    {
     const unsigned first_row_local = this->first_row();
     const unsigned n_row_local = this->nrow_local();
     
     //If we are in range then just call the local value
     if((i >= first_row_local) && (i < first_row_local + n_row_local))
      {
       return (*this)[i-first_row_local];
      }
     //Otherwise the entry is not stored in the local processor
     //and we must have haloed it
     else
      {
#ifdef PARANOID
       if(Halo_scheme_pt==0)
        {
         std::ostringstream error_stream;
         error_stream << 
          "Halo data requested, but no halo scheme has been setup\n" <<
          "You should call this->build_halo_scheme(halo_scheme_pt).\n" <<
          "You may wish to setup the scheme for the Problem using \n" <<
          "Problem::setup_dof_halo_scheme()\n";
         
         throw OomphLibError(error_stream.str(),
                             OOMPH_CURRENT_FUNCTION,
                             OOMPH_EXCEPTION_LOCATION);
        }
#endif
       return Halo_value[Halo_scheme_pt->local_index(i)];
      }
    }
   //If not distributed the global entry is
   //the local entry
   else
#endif
    {
     return (*this)[i];
    }
  }


 ///Synchronise the halo data
 void synchronise();

 ///Sum all the data, store in the master (haloed) data and then
 ///synchronise
 void sum_all_halo_and_haloed_values();

 ///Access function for halo scheme
 DoubleVectorHaloScheme* &halo_scheme_pt() {return Halo_scheme_pt;}

 ///Access function for halo scheme (const version)
 DoubleVectorHaloScheme* const &halo_scheme_pt() const {return Halo_scheme_pt;}


 ///\short Construct the halo scheme and storage for the halo
 ///data
 void build_halo_scheme(DoubleVectorHaloScheme* const &halo_scheme_pt);

};

}
#endif
