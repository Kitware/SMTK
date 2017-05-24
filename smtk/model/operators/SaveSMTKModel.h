//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SaveSMTKModel_h
#define __smtk_model_SaveSMTKModel_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT SaveSMTKModel : public Operator
{
public:
  smtkTypeMacro(SaveSMTKModel);
  smtkCreateMacro(SaveSMTKModel);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  void extractChanges();

  /**\brief Apply changes (or unapply temporary changes) in preparation for saving data.
    *
    * Call this method just before saving and, if entities are to be reverted
    * to their original state, then again just after saving.
    *
    * Entries will be added to \a modified for each entity being modified and for
    * each model begin saved (regardless of whether it is modified), **unless**
    * entities are to be reverted to their original state (in which case no entities
    * are modified).
    */
  bool applyChanges(smtk::model::EntityRefs& modified);

protected:
  class Internals;
  Internals* m_data;

  SaveSMTKModel();
  virtual OperatorResult operateInternal();
  virtual void generateSummary(OperatorResult&);
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_SaveSMTKModel_h
