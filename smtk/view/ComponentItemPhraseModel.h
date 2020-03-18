//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ComponentItemPhraseModel_h
#define smtk_view_ComponentItemPhraseModel_h

#include "smtk/view/ComponentPhraseModel.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace view
{
/**\brief Present phrases describing a set of acceptable components held by a single resource.
  *
  * This model maintains the list of acceptable components by
  * asking the resource for all matching components each time
  * an operation runs.
  *
  * The list is flat (i.e., no subphrase generator is provided by default).
  * The model provides access to a user-selected subset as an smtk::view::Selection.
  */
class SMTKCORE_EXPORT ComponentItemPhraseModel : public ComponentPhraseModel
{
public:
  smtkTypeMacro(smtk::view::ComponentItemPhraseModel);
  smtkSuperclassMacro(smtk::view::ComponentPhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);
  virtual ~ComponentItemPhraseModel();

  /**\brief Create a model and configure it given a view description.
    *
    * Note that this method, unlike the version with no parameters,
    * properly initializes its subphrase generator with a reference to
    * the created model so that subphrases are properly decorated.
    */
  static PhraseModelPtr create(const smtk::view::Configuration::Component& viewComp);
  void setUseAttributeAssociations(bool val) { m_useAttributeAssociatons = val; }
  void setComponentItem(smtk::attribute::ComponentItemPtr& compItem);

protected:
  ComponentItemPhraseModel();

  ///\brief Populate the root based on a set of appropriate resource components.
  void populateRoot() override;
  void handleModified(
    const Operation& op, const Operation::Result& res, const ComponentItemPtr& data) override;
  smtk::attribute::ComponentItemPtr m_compItem;
  bool m_useAttributeAssociatons;
};
}
}
#endif
