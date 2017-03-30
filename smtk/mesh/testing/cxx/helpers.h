//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef __smtk_mesh_testing_cxx_helpers_h
#define __smtk_mesh_testing_cxx_helpers_h

#include "smtk/mesh/CellTraits.h"

#include "smtk/common/testing/cxx/helpers.h"
#include <iostream>

namespace smtk
{
namespace mesh
{
namespace testing
{

struct Testing
{
  template <class FunctionType>
  struct InternalPrintOnInvoke
  {
    InternalPrintOnInvoke(FunctionType function, std::string toprint)
      : Function(function)
      , ToPrint(toprint)
    {
    }
    template <typename T>
    void operator()(T t)
    {
      std::cout << this->ToPrint << std::endl;
      this->Function(t);
    }

  private:
    FunctionType Function;
    std::string ToPrint;
  };

  struct CellCheckAll
  {
    template <class Tag, class Functor>
    void operator()(Tag t, Functor function) const
    {
      function(t);
    }
  };

  struct CellCheckFixedTypes
  {
    template <class Tag, class Functor>
    void operator()(Tag t, Functor function) const
    {
      this->Do(typename smtk::mesh::CellTraits<Tag>::PointNumberTag(), t, function);
    }

  private:
    template <class Tag, typename T, class Functor>
    void Do(Tag, T, const Functor&) const
    {
    }

    template <class Tag, class Functor>
    void Do(smtk::mesh::CellFixedPointNumberTag, Tag t, Functor function) const
    {
      function(t);
    }
  };
};

/// Runs templated \p function on all the cell tags defined in smtk::mesh. This is
/// helpful to test templated functions that should work on all cell types.
/// If the function is supposed to work on some subset of cell types, then \p
/// check can be set to restrict the types used. This Testing class contains
/// several helpful check functors.
///
template <class FunctionType, class CheckType>
static void TryAllCells(FunctionType function, CheckType check)
{
  check(smtk::mesh::CellHexahedron(), Testing::InternalPrintOnInvoke<FunctionType>(function,
                                        "*** smtk::mesh::CellHexahedron ******************"));

  check(smtk::mesh::CellLine(), Testing::InternalPrintOnInvoke<FunctionType>(
                                  function, "*** smtk::mesh::CellLine *****************"));

  check(smtk::mesh::CellPolygon(), Testing::InternalPrintOnInvoke<FunctionType>(
                                     function, "*** smtk::mesh::CellPolygon *******************"));

  check(smtk::mesh::CellPyramid(), Testing::InternalPrintOnInvoke<FunctionType>(
                                     function, "*** smtk::mesh::CellPyramid ***************"));

  check(smtk::mesh::CellQuad(),
    Testing::InternalPrintOnInvoke<FunctionType>(function, "*** smtk::mesh::CellQuad **********"));

  check(smtk::mesh::CellTetrahedron(), Testing::InternalPrintOnInvoke<FunctionType>(
                                         function, "*** smtk::mesh::CellTetrahedron **********"));

  check(smtk::mesh::CellTriangle(), Testing::InternalPrintOnInvoke<FunctionType>(
                                      function, "*** smtk::mesh::CellTriangle *************"));

  check(smtk::mesh::CellVertex(), Testing::InternalPrintOnInvoke<FunctionType>(
                                    function, "*** smtk::mesh::CellVertex ************"));

  check(smtk::mesh::CellWedge(), Testing::InternalPrintOnInvoke<FunctionType>(
                                   function, "*** smtk::mesh::CellWedge ******************"));
}

/// Runs templated \p function on all the cell tags defined in smtk::mesh. This is
/// helpful to test templated functions that should work on all cell types.
/// If the function is supposed to work on some subset of cell types, then \p
/// check can be set to restrict the types used. This Testing class contains
/// several helpful check functors.
///
template <class FunctionType, class CheckType>
static void TryAllCellEnums(FunctionType function, CheckType check)
{
  check(smtk::mesh::Hexahedron, Testing::InternalPrintOnInvoke<FunctionType>(
                                  function, "*** smtk::mesh::CellHexahedron ******************"));

  check(smtk::mesh::Line, Testing::InternalPrintOnInvoke<FunctionType>(
                            function, "*** smtk::mesh::CellLine *****************"));

  check(smtk::mesh::Polygon, Testing::InternalPrintOnInvoke<FunctionType>(
                               function, "*** smtk::mesh::CellPolygon *******************"));

  check(smtk::mesh::Pyramid, Testing::InternalPrintOnInvoke<FunctionType>(
                               function, "*** smtk::mesh::CellPyramid ***************"));

  check(smtk::mesh::Quad,
    Testing::InternalPrintOnInvoke<FunctionType>(function, "*** smtk::mesh::CellQuad **********"));

  check(smtk::mesh::Tetrahedron, Testing::InternalPrintOnInvoke<FunctionType>(
                                   function, "*** smtk::mesh::CellTetrahedron **********"));

  check(smtk::mesh::Triangle, Testing::InternalPrintOnInvoke<FunctionType>(
                                function, "*** smtk::mesh::CellTriangle *************"));

  check(smtk::mesh::Vertex, Testing::InternalPrintOnInvoke<FunctionType>(
                              function, "*** smtk::mesh::CellVertex ************"));

  check(smtk::mesh::Wedge, Testing::InternalPrintOnInvoke<FunctionType>(
                             function, "*** smtk::mesh::CellWedge ******************"));
}

template <class FunctionType>
static void TryAllCells(FunctionType function)
{
  TryAllCells(function, Testing::CellCheckAll());
}

template <class FunctionType>
static void TryAllCellEnums(FunctionType function)
{
  TryAllCellEnums(function, Testing::CellCheckAll());
}

template <class FunctionType>
static void TryFIXEDCells(FunctionType function)
{
  TryAllCells(function, Testing::CellCheckFixedTypes());
}
}
}
} //namespace smtk::mesh::testing

#endif //__smtk_mesh_testing_cxx_helpers_h
