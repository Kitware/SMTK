//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_CreateModel_h
#define __smtk_session_rgg_CreateModel_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/model/Model.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{
class Group;
}
}

/**\brief a type to define models used for core
  */
#define SMTK_SESSION_RGG_CORE "_rgg_core"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Create a rgg model which includes a rgg core.
 * The core is a group in smtk world which is only used as a container
 * to hold pin and duct instances. All other properties are stored on the model.
  */
class SMTKRGGSESSION_EXPORT CreateModel : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::CreateModel);
  smtkCreateMacro(CreateModel);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  static void populateCore(smtk::operation::Operation* op, smtk::model::Group& core);

  static size_t materialNum(smtk::model::Model = smtk::model::Model());
  static void getMaterial(
    const size_t& index, std::string& name, smtk::model::Model = smtk::model::Model());
  static void getMaterialColor(
    const size_t& index, std::vector<double>& rgba, smtk::model::Model = smtk::model::Model());

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_CreateModel_h
