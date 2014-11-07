//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_bridge_discrete_ImportOperator_h
#define __smtk_bridge_discrete_ImportOperator_h

#include "smtk/bridge/discrete/discreteBridgeExports.h"
#include "smtk/model/Operator.h"
#include "vtkCMBModelBuilder.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Bridge;

/**\brief Read a CMB discrete model file.
  *
  * The supported file extensions currently:
  *    "Moab files (*.h5m *.sat *.brep *.stp *.cub *.exo)
  *
  * NOT YET:
  *    "VTK data files (*.vtk *.vtu *.vtp)
  *    "Solids (*.2dm *.3dm *.sol *.stl *.tin *.obj)
  *    "SimBuilder files (*.crf *.sbt *.sbi *.sbs)
  *    "Map files (*.map)
  *    "Poly files (*.poly *.smesh)
  *    "Shape files (*.shp)
  */
class SMTKDISCRETEBRIDGE_EXPORT ImportOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(ImportOperator);
  smtkCreateMacro(ImportOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  ImportOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* discreteBridge() const;

  vtkNew<vtkCMBModelBuilder> m_op;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_discrete_ImportOperator_h
