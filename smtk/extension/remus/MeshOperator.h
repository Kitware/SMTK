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
  * to be used to submit the job. These need to be given to the SMTKMeshSubmission,
  * during construction alongside the serialized model, serialized meshingControlAttributes,
  * and the UUIDS of models that you want meshed.
  *
  * Required Input:
  *
  * Key ["endpoint"]:  Holds how to connect to the remus server, this will
  *                    generally looks like tcp://127.0.0.1:50505
  *
  * Key ["remusRequirements"]: The filled out requirements of the worker
  *
  * Key ["meshingControlAttributes"]: The attribute system of the worker
  *
  *
  *
  * This operator submits a job to a remus server. The SMTKMeshSubmission it
  * submits needs to have the model, attribute, and modelItemsToMesh member
  * data filled in.
  *
  *
  *
  * Required SMTKMeshSubmission components:
  *
  * model(): The input model serialized as a JSON string.
  * attributes(): The contents of the meshingControlAttributes string
  * modelItemsToMesh(): an JSON string of the form
  *   {
  *     "ids":  ["2add8c09-01f6-457e-9ed9-a75cc833411a", "5ade8c11-6f06-e754-9ad3-e75dd8334325"]
  *   }
  *   that lists all the models that you want meshed. Currently this is all
  *   the models in the manager
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
