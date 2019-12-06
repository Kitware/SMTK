//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtResourceItem.h"
#include "smtk/extension/qt/qtReferenceItemComboBox.h"
#include "smtk/extension/qt/qtReferenceItemData.h"

#include "smtk/common/Paths.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/model/Resource.h"

namespace smtk
{
namespace extension
{

qtItem* qtResourceItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  auto item = info.itemAs<smtk::attribute::ResourceItem>();
  if (item == nullptr)
  {
    return nullptr;
  }
  // If we are dealing with a non-extensible item with only 1 required value lets
  // use a simple combobox UI else we will use the more advance UI.
  auto itemDef = item->definitionAs<smtk::attribute::ReferenceItemDefinition>();
  if ((itemDef->numberOfRequiredValues() == 1) && !itemDef->isExtensible())
  {
    return new qtReferenceItemComboBox(info);
  }
  auto qi = new qtResourceItem(info);
  // Unlike other classes, qtResourceItem does not call createWidget()
  // in its constructor since the base class, qtReferenceItem, is
  // concrete and cannot call virtual methods of subclases. So, for
  // qtReferenceItem and its subclasses, we create the widget after
  // constructing the item.
  qi->createWidget();
  return qi;
}

qtResourceItem::qtResourceItem(const qtAttributeItemInfo& info)
  : qtReferenceItem(info)
{
}

qtResourceItem::~qtResourceItem()
{
}

smtk::view::PhraseModelPtr qtResourceItem::createPhraseModel() const
{
  // Constructing the PhraseModel with a View properly initializes the SubphraseGenerator
  // to point back to the model (thus ensuring subphrases are decorated). This is required
  // since we need to decorate phrases to show+edit "visibility" as set membership:
  auto config = smtk::view::Configuration::New("ResourceItem", "stuff");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(config);
  phraseModel->root()->findDelegate()->setModel(phraseModel);
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ResourceItemDefinition>(
    m_itemInfo.item()->definition());
  std::static_pointer_cast<smtk::view::ResourcePhraseModel>(phraseModel)
    ->setResourceFilters(def->acceptableEntries());

  return phraseModel;
}

std::string qtResourceItem::synopsis(bool& ok) const
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ResourceItem>();
  if (!item)
  {
    ok = false;
    return "uninitialized item";
  }

  std::size_t numRequired = item->numberOfRequiredValues();
  std::size_t maxAllowed = (item->isExtensible() ? item->maxNumberOfValues() : numRequired);
  std::ostringstream label;
  std::size_t numSel = 0;
  for (auto entry : m_p->m_members)
  {
    if (entry.second > 0)
    {
      ++numSel;
    }
  }
  ok = true;
  if (numRequired < 2 && maxAllowed == 1)
  {
    auto resource = (m_p->m_members.empty()
        ? smtk::resource::ResourcePtr()
        : std::static_pointer_cast<smtk::resource::Resource>(m_p->m_members.begin()->first.lock()));

    if (resource != nullptr)
    {
      // TODO: this routine was lifted from ResourcePhraseContent. It should not
      // be duplicated here.
      std::string locn = resource->location();
      std::string file = smtk::common::Paths::filename(locn);
      std::string dir = smtk::common::Paths::directory(locn);
      if (dir.size() > 31)
      {
        dir = dir.substr(0, 14) + "..." + dir.substr(dir.size() - 14, 14);
      }
      std::string name = dir.empty() ? "New Resource" : (file + " (" + dir + ")");
      label << (numSel == 1 ? (resource ? name : "NULL!!") : (numSel > 0 ? "too many" : "(none)"));
    }
    else
    {
      label << (numSel == 1 ? "(none)" : (numSel > 0 ? "too many" : "(none)"));
    }
    ok = numSel >= numRequired && numSel <= maxAllowed;
  }
  else
  {
    label << numSel;
    if (numRequired > 0)
    {
      label << " of ";
      if (numRequired == maxAllowed)
      { // Exactly N values are allowed and required.
        label << numRequired;
      }
      else if (maxAllowed > 0)
      { // There is a minimum required, but a limited additional number are acceptable
        label << numRequired << "—" << maxAllowed;
      }
      else
      { // Any number are allowed, but there is a minimum.
        label << numRequired << "+";
      }
      ok &= (numSel >= numRequired);
    }
    else
    { // no values are required, but there may be a cap on the maximum number.
      if (maxAllowed > 0)
      {
        label << " of 0–" << maxAllowed;
      }
      else
      {
        label << " chosen";
      }
    }
  }
  ok &= (maxAllowed == 0 || numSel <= maxAllowed);
  return label.str();
}
}
}
