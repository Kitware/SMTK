//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ReferenceItemPhraseModel_h
#define smtk_view_ReferenceItemPhraseModel_h

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
  * The list is flat (i.e., no sub-phrase generator is provided by default).
  * The model provides access to a user-selected subset as an smtk::view::Selection.
  */
class SMTKCORE_EXPORT ReferenceItemPhraseModel : public ComponentPhraseModel
{
public:
  smtkTypeMacro(smtk::view::ReferenceItemPhraseModel);
  smtkSuperclassMacro(smtk::view::ComponentPhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);

  ReferenceItemPhraseModel();
  ReferenceItemPhraseModel(const Configuration*, Manager*);
  ~ReferenceItemPhraseModel() override;

  /**\brief Create a model and configure it given a view description.
    *
    * Note that this method, unlike the version that takes a
    * smtk::view::Configuration, is used by qtReferenceItem (where
    * the PhraseModel is a "sub-view" of the qtItem's view).
    */
  static PhraseModelPtr create(const smtk::view::Configuration::Component& viewComp);
  void setUseAttributeAssociations(bool val) { m_useAttributeAssociatons = val; }
  void setReferenceItem(smtk::attribute::ReferenceItemPtr& refItem);

protected:
  ///\brief Populate the root based on a set of appropriate resource components.
  void populateRoot() override;
  void handleModified(const smtk::resource::PersistentObjectSet& modifiedObjects) override;
  smtk::attribute::ReferenceItemPtr m_refItem;
  bool m_useAttributeAssociatons;
};
} // namespace view
} // namespace smtk
#endif
