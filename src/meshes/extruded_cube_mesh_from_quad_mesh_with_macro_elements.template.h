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
// Header file for ExtrudedCubeMeshFromQuadMesh class
#ifndef OOMPH_EXTRUDED_CUBE_MESH_FROM_QUAD_MESH_WITH_MACRO_ELEMENTS_HEADER
#define OOMPH_EXTRUDED_CUBE_MESH_FROM_QUAD_MESH_WITH_MACRO_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// Oomph-lib headers
#include "../generic/mesh.h"
#include "../generic/quad_mesh.h"
#include "../generic/brick_mesh.h"
#include "../generic/refineable_brick_mesh.h"
#include "../generic/extruded_domain.h"
#include "../generic/extruded_macro_element.h"

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

namespace oomph
{
  //===============================================================
  // Namespace for the mesh extrusion machinery
  //===============================================================
  namespace MeshExtrusionHelpers
  {
    /// \short Class to contain any helpers for the mesh extrusion. At
    /// the moment this only used to decide on whether or not to
    /// doc the mesh setup time
    class ExtrusionHelper
    {
    public:

      /// Constructor (empty)
      ExtrusionHelper() : Doc_mesh_setup_time(false)
      {}


      /// Return the value of the Doc_mesh_setup_time flag
      bool doc_mesh_setup_time()
      {
        // Return the boolean flag
        return Doc_mesh_setup_time;
      } // End of doc_mesh_setup_time


      /// Enable doc-ing of the mesh setup
      void enable_doc_mesh_setup_time()
      {
        // Enable the appropriate boolean flag
        Doc_mesh_setup_time=true;
      } // End of enable_doc_mesh_setup_time


      /// Disable doc-ing of the mesh setup
      void disable_doc_mesh_setup_time()
      {
        // Disable the appropriate boolean flag
        Doc_mesh_setup_time=false;
      } // End of disable_doc_mesh_setup_time

    private:

      /// Boolean to indicate whether or not to doc the progress
      bool Doc_mesh_setup_time;
    } Mesh_extrusion_helper;
  } // End of namespace MeshExtrusionDocTimeHelper

  //=======================================================================
  /// \short Mesh class that takes a 2D mesh consisting of quadrilateral
  /// elements and "extrudes" it in the z-direction.
  //=======================================================================
  template <class ELEMENT>
  class ExtrudedCubeMeshFromQuadMesh : public virtual BrickMeshBase
  {

  public:

    /// \short Constructor: Pass a mesh consisting of quad elements,
    /// specify the number of elements in the z direction, and the
    /// corresponding length in this direction. Assumes that the back
    /// lower left corner is located at (0,0,0). Timestepper defaults
    /// to the Steady timestepper.
    ExtrudedCubeMeshFromQuadMesh(QuadMeshBase* quad_mesh_pt,
                                 const unsigned& nz,
                                 const double& lz,
                                 TimeStepper* time_stepper_pt=
                                   &Mesh::Default_TimeStepper) :
      Nz(nz), Zmin(0.0), Zmax(lz)
    {
      // Call the generic build function
      build_mesh(quad_mesh_pt,time_stepper_pt);
    }


    /// \short Constructor: Pass a mesh consisting of quad elements,
    /// specify the number of elements in the z direction, and the
    /// corresponding minimum and maximum z-value of the mesh. Again,
    /// timestepper defaults to Steady.
    ExtrudedCubeMeshFromQuadMesh(QuadMeshBase* quad_mesh_pt,
                                 const unsigned& nz,
                                 const double& zmin,
                                 const double& zmax,
                                 TimeStepper* time_stepper_pt=
                                   &Mesh::Default_TimeStepper) :
      Nz(nz), Zmin(zmin), Zmax(zmax)
    {
      // Call the generic mesh constructor
      build_mesh(quad_mesh_pt,time_stepper_pt);
    }


    /// \short Destructor: If the underlying spatial domain was made up of
    /// any macro elements then we will have created an extruded domain
    /// which in turn creates extruded macro elements. As we're responsible
    /// for creating the domain, we need to kill it and it'll kill the
    /// extruded macro elements...
    ~ExtrudedCubeMeshFromQuadMesh()
    {
      // Are there any extruded domains?
      if (Extruded_domain_pt.size()>0)
      {
        // How many extruded domains did we create?
        unsigned n_extruded_domain=Extruded_domain_pt.size();

        // Loop over the extruded domains
        for (unsigned i=0; i<n_extruded_domain; i++)
        {
          // Delete the i-th extruded domain
          delete Extruded_domain_pt[i];

          // Make it a null pointer
          Extruded_domain_pt[i]=0;
        }
      } // if (Extruded_domain_pt.size()>0)
    } // End of ExtrudedCubeMeshFromQuadMesh


    /// \short Return the value of the z-coordinate at the node given by the
    /// local node number, znode.
    virtual double z_spacing_function(const unsigned& z_element,
                                      const unsigned& z_node) const
    {
      // Calculate the values of equal increments in nodal values
      double z_step=(Zmax-Zmin)/((N_node_1d-1)*Nz);

      // Return the appropriate value
      return (Zmin+z_step*((N_node_1d-1)*z_element+z_node));
    } // End of z_spacing_function


    /// \short Get all the boundary information of an element using the
    /// input (quad_mesh_pt) mesh. If the element lies on a boundary then
    /// the user will be given the corresponding boundary index and the
    /// index of the face of quad_el_pt attached to the boundary. If the
    /// element does NOT lie on any boundaries, this function simply
    /// returns a vector of size zero.
    Vector<std::pair<unsigned,int> >
    get_element_boundary_information(QuadMeshBase* quad_mesh_pt,
                                     FiniteElement* quad_el_pt);

    /// Access function for number of elements in z-direction (const version)
    const unsigned& nz() const
    {
      // Return the value of the appropriate private variable
      return Nz;
    } // End of nz

  protected:

    /// The number of nodes in each direction
    unsigned N_node_1d;

    /// Number of elements in the z-direction
    unsigned Nz;

    /// Minimum value of z coordinate
    double Zmin;

    /// Maximum value of z coordinate
    double Zmax;

    /// Vector of pointers to extruded domain objects
    Vector<ExtrudedDomain*> Extruded_domain_pt;

    /// Generic mesh construction function: contains all the hard work
    void build_mesh(QuadMeshBase* quad_mesh_pt,
                    TimeStepper* time_stepper_pt=&Mesh::Default_TimeStepper);
  };

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////

  //=======================================================================
  /// Mesh class that takes a 2D mesh, "extrudes" it in the z-direction and
  /// also makes the resulting mesh refineable.
  //=======================================================================
  template <class ELEMENT>
  class RefineableExtrudedCubeMeshFromQuadMesh :
    public virtual ExtrudedCubeMeshFromQuadMesh<ELEMENT>,
    public virtual RefineableBrickMesh<ELEMENT>
  {
  public:

    /// \short Constructor: Pass a mesh consisting of quad elements,
    /// specify the number of elements in the z direction, and the
    /// corresponding length in this direction. Assumes that the back
    /// lower left corner is located at (0,0,0). Timestepper defaults
    /// to the Steady timestepper.
    RefineableExtrudedCubeMeshFromQuadMesh(QuadMeshBase* quad_mesh_pt,
                                           const unsigned& nz,
                                           const double& lz,
                                           TimeStepper* time_stepper_pt=
                                               &Mesh::Default_TimeStepper) :
      ExtrudedCubeMeshFromQuadMesh<ELEMENT>(quad_mesh_pt,nz,lz,time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // base class Only need to setup octree forest
      this->setup_octree_forest();
    }


    /// \short Constructor: Pass a mesh consisting of quad elements,
    /// specify the number of elements in the z direction, and the
    /// corresponding minimum and maximum z-value of the mesh. Again,
    /// timestepper defaults to Steady.
    RefineableExtrudedCubeMeshFromQuadMesh(QuadMeshBase* quad_mesh_pt,
                                           const unsigned& nz,
                                           const double& zmin,
                                           const double& zmax,
                                           TimeStepper* time_stepper_pt=
                                               &Mesh::Default_TimeStepper) :
      ExtrudedCubeMeshFromQuadMesh<ELEMENT>(quad_mesh_pt,nz,zmin,
                                            zmax,time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // base class Only need to setup octree forest
      this->setup_octree_forest();
    }
  };

} // End of namespace oomph

#endif
