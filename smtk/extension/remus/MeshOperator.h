//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_remus_MeshOperator_h
#define __smtk_extension_remus_MeshOperator_h

#include "smtk/extension/remus/smtkRemusExtExports.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class Session;

/**\brief Mesh a model using remus.
  *
  * The source model is serialized to a JSON stream and submitted to remus,
  * with the resulting remus::proto::Job being stored in the string job
  * specification to be returned to the client.
  *
  * The operator's remusRequirements attribute holds the exact requirements
  * to be used to submit the job. These need to be added to a job submission
  * along side the meshingControlAttributes and all the control keys. See
  * bottom of this doc on what all the job submission keys are
  *
  * The operator's meshingControlAttributes attribute holds a serialized smtk attribute
  * instance that defines all the relevant meshing controls for the given worker
  *
  * The operator's endpoint attribute holds how to connect to the remus
  * server, this will generally looks like tcp://127.0.0.1:50505
  *
  * This operator submits a job to a remus server. The JobSubmission it
  * submits will have the following key/value pairs.
  *
  *
  * Required Keys:
  *
  * Key ["meshing_attributes"]: The contents of the meshingControlAttributes string
  *
  * Key ["model"]: The input model serialized as a json string OR the path
  *                to the model if we are unable to serialize it.
  *
  *
  *
  */
class SMTKREMUSEXT_EXPORT MeshOperator : public Operator
{
public:
  smtkTypeMacro(MeshOperator);
  smtkSuperclassMacro(Operator);
  smtkCreateMacro(MeshOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  MeshOperator();
  virtual smtk::model::OperatorResult operateInternal();
};

  } // namespace model
} // namespace smtk

#endif // __smtk_extension_remus_MeshOperator_h
