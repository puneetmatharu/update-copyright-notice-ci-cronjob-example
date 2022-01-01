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
#ifndef OOMPH_BIHARMONIC_FLUX_ELEMENTS_HEADER
#define OOMPH_BIHARMONIC_FLUX_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
//mpi headers
#include "mpi.h"
#endif

// Generic C++ headers
#include <iostream>
#include <math.h>

// oomph-lib headers
#include "../generic/matrices.h"
#include "../generic/elements.h"
#include "../generic/hermite_elements.h"
#include "biharmonic_elements.h"



// oomph namespace
namespace oomph
{

 //============================================================================
/// \short face geometry for biharmonic elements - template parameter 
/// indicates dimension of problem (i.e. bulk element),  not the face elements
//============================================================================
template<unsigned DIM>
class FaceGeometry<BiharmonicElement<DIM>  > : 
 public virtual QHermiteElement<DIM-1>
{

  public:
 
 /// \short Constructor: Call the constructor for the
 /// appropriate lower-dimensional BiharmonicElement
 FaceGeometry() : QHermiteElement<DIM-1>() {}

};

template<> class 
FaceGeometry<BiharmonicElement<1> > : public virtual PointElement
{

  public:
 
 /// \short Constructor: Call the constructor for the
 /// appropriate lower-dimensional BiharmonicElement
 FaceGeometry() : PointElement() {}

};


//=============================================================================
/// Biharmonic Flux Element
//=============================================================================
template<unsigned DIM>
class BiharmonicFluxElement : public virtual 
FaceGeometry<BiharmonicElement<2> >, 
 public virtual FaceElement
{
	
public :
 
 /// definition of flux function pointer, flux must be parametised in terms of
 /// the macro element coordinate position (s) along the problem edge
 typedef void (*FluxFctPt)(const double& s, double& flux);

 ///\short constructor
 BiharmonicFluxElement(FiniteElement* const &bulk_el_pt,
                       const int& face_index,
                       const unsigned& b);


 ///\short  Broken empty constructor
 BiharmonicFluxElement()
  {
   throw OomphLibError(
    "Don't call empty constructor for PoissonFluxElement",
    OOMPH_CURRENT_FUNCTION,
    OOMPH_EXCEPTION_LOCATION);
  }


 /// Broken copy constructor
 BiharmonicFluxElement(const BiharmonicFluxElement& dummy) 
  { 
   BrokenCopy::broken_copy("BiharmonicFluxElement<DIM>");
  } 
 

 /// Broken assignment operator
 void operator=(const BiharmonicFluxElement&) 
  {
   BrokenCopy::broken_assign("BiharmonicFluxElement<DIM>");
  }

 /// Access function for the flux0 function pointer
 FluxFctPt& flux0_fct_pt() {return Flux0_fct_pt;}

 /// Access function for the flux1 function pointer
 FluxFctPt& flux1_fct_pt() {return Flux1_fct_pt;}

 /// Add the element's contribution to its residual vector
 inline void fill_in_contribution_to_residuals(Vector<double> &residuals)
  {
   //Call the generic residuals function
   fill_in_generic_residual_contribution_biharmonic_flux(residuals);
  }


 /// \short Add the element's contribution to its residual vector and its 
 /// Jacobian matrix (note - no contributions are made to the jacobian)
 inline void fill_in_contribution_to_jacobian(Vector<double> &residuals,
                                              DenseMatrix<double> &jacobian)
  {
   //Call the generic residuals routine
   fill_in_generic_residual_contribution_biharmonic_flux(residuals);
  }


 /// \short Specify the value of nodal zeta from the face geometry
 /// The "global" intrinsic coordinate of the element when
 /// viewed as part of a geometric object should be given by
 /// the FaceElement representation, by default (needed to break
 /// indeterminacy if bulk element is SolidElement)
 double zeta_nodal(const unsigned &n, const unsigned &k,           
                   const unsigned &i) const 
 {return FaceElement::zeta_nodal(n,k,i);}     


 /// Output function -- does nothing
 void output(std::ostream &outfile) {}


 /// \short Output function -- does nothing
 void output(std::ostream &outfile, const unsigned &n_plot) {}
 

 /// C-style output function -- does nothing
 void output(FILE* file_pt) {}
 

 /// \short C-style output function -- does nothing
 void output(FILE* file_pt, const unsigned &n_plot) {}


 /// compute_error -- does nothing
 void compute_error(std::ostream &outfile, 
                    FiniteElement::SteadyExactSolutionFctPt 
                    exact_soln_pt, double& error, double& norm) {}
 

 virtual void output_fct(std::ostream &outfile,const unsigned &nplot, 
                 FiniteElement::SteadyExactSolutionFctPt exact_soln_pt)
  {}
  
private:

 /// \short Function to return the prescribed flux at a given macro element 
 /// coordinate position
 void get_flux0(const double& s, double& flux)
  {
   //If the function pointer is zero return zero
   if(Flux0_fct_pt == 0)
    {
     flux=0.0;
    }
   //Otherwise call the function
   else
    {
     (*Flux0_fct_pt)(s,flux);
    }
  }

 /// \short Function to return the prescribed flux at a given macro element 
 /// coordinate position
 void get_flux1(const double& s, double& flux)
  {
   //If the function pointer is zero return zero
   if(Flux1_fct_pt == 0)
    {
     flux=0.0;
    }
   //Otherwise call the function
   else
    {
     (*Flux1_fct_pt)(s,flux);
    }
  }

 /// \short Calculate the Jacobian of the mapping between local and global
 /// coordinates at the position s for face elements
 double J_eulerian(const Vector<double> &s) const;

 /// \short Add the element's contribution to its residual vector. Flux 
 /// elements only make contribution to the residual vector
 void fill_in_generic_residual_contribution_biharmonic_flux(
  Vector<double> &residuals);
 
 /// Function pointer to the prescribed flux
 FluxFctPt Flux0_fct_pt;

 /// Function pointer to the prescribed flux
 FluxFctPt Flux1_fct_pt;

 /// \short the number of nodal degrees of freedom for the face element basis 
 /// functions
 unsigned Nface_nodal_dof;

 /// \short
 unsigned Boundary;

};



}
#endif
