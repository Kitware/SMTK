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

qtItem* qtResourceItem::createItemWidget(const AttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ResourceItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtResourceItem(info);
}

qtResourceItem::qtResourceItem(const AttributeItemInfo& info)
  : qtReferenceItem(info)
{
  this->createWidget();
}

qtResourceItem::~qtResourceItem()
{
}

void qtResourceItem::setLabelVisible(bool visible)
{
  m_p->m_label->setVisible(visible);
}

void qtResourceItem::updateItemData()
{
}

smtk::view::PhraseModelPtr qtResourceItem::createPhraseModel() const
{
  // Constructing the PhraseModel with a View properly initializes the SubphraseGenerator
  // to point back to the model (thus ensuring subphrases are decorated). This is required
  // since we need to decorate phrases to show+edit "visibility" as set membership:
  auto view = smtk::view::View::New("ResourceItem", "stuff");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(view);
  phraseModel->root()->findDelegate()->setModel(phraseModel);
  auto def = std::dynamic_pointer_cast<const smtk::attribute::ResourceItemDefinition>(
    m_itemInfo.item()->definition());
  std::static_pointer_cast<smtk::view::ResourcePhraseModel>(phraseModel)
    ->setResourceFilters(def->acceptableEntries());

  return phraseModel;
}

void qtResourceItem::createWidget()
{
  // Let our subclass do the UI work.
  this->Superclass::createWidget();

  // Now add in ResourceItem specifics.
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj || !this->passAdvancedCheck() ||
    (m_itemInfo.uiManager() &&
      !m_itemInfo.uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }

  this->updateItemData();
}

std::string qtResourceItem::synopsis(bool& ok) const
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ResourceItem>();
  if (!item)
  {
    ok = false;
    return "uninitialized item";
  }

  if (m_p->m_members.size())
  {
    auto foo = m_p->m_members.begin()->first;
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
        : std::static_pointer_cast<smtk::resource::Resource>(m_p->m_members.begin()->first));

    if (resource != nullptr)
    {
      // TODO: this routine was lifted from ResourcePhraseContent. It should not
      // be duplicated here.
      std::string locn = resource->location();
      std::string file = smtk::common::Paths::filename(locn);
      std::string dir = smtk::common::Paths::directory(locn);
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

int qtResourceItem::decorateWithMembership(smtk::view::DescriptivePhrasePtr phr)
{
  smtk::view::VisibilityContent::decoratePhrase(
    phr, [this](smtk::view::VisibilityContent::Query qq, int val,
           smtk::view::ConstPhraseContentPtr data) {
      auto resource = data->relatedResource();
      switch (qq)
      {
        case smtk::view::VisibilityContent::DISPLAYABLE:
          return resource == nullptr ? 1 : 0;
        case smtk::view::VisibilityContent::EDITABLE:
          return resource == nullptr ? 1 : 0;
        case smtk::view::VisibilityContent::GET_VALUE:
          if (resource)
          {
            auto valIt = m_p->m_members.find(resource);
            if (valIt != m_p->m_members.end())
            {
              return valIt->second;
            }
            return 0; // visibility is assumed if there is no entry.
          }
          return 0; // visibility is false if the resource is not a model entity or NULL.
        case smtk::view::VisibilityContent::SET_VALUE:
          if (resource)
          {
            if (val && !m_p->m_members.empty())
            {
              auto item = m_itemInfo.itemAs<attribute::ResourceItem>();
              if (item->numberOfRequiredValues() <= 1 && item->maxNumberOfValues() == 1)
              { // Clear all other members since only 1 is allowed and the user just chose it.
                m_p->m_members.clear();
                m_p->m_phraseModel->triggerDataChanged();
              }
            }
            m_p->m_members[resource] = val ? 1 : 0;
            this->updateSynopsisLabels();
            return 1;
          }
      }
      return 0;
    });
  return 0;
}

void qtResourceItem::toggleCurrentItem()
{
  auto cphr = m_p->m_qtModel->getItem(m_p->m_popupList->currentIndex());
  if (cphr)
  {
    auto currentMembership = cphr->relatedVisibility();
    // Selecting a new item when only 1 is allowed should reset all other membership.
    if (!currentMembership && !m_p->m_members.empty())
    {
      auto item = m_itemInfo.itemAs<attribute::ResourceItem>();
      if (item->numberOfRequiredValues() <= 1 && item->maxNumberOfValues() == 1)
      {
        m_p->m_members.clear();
        m_p->m_phraseModel->triggerDataChanged();
      }
    }
    cphr->setRelatedVisibility(!currentMembership);
    this->updateSynopsisLabels();
  }
}

bool qtResourceItem::synchronize(UpdateSource src)
{
  auto item = m_itemInfo.itemAs<attribute::ResourceItem>();
  if (!item)
  {
    return false;
  }

  std::size_t uiMembers = 0;
  for (auto member : m_p->m_members)
  {
    if (member.second)
    {
      ++uiMembers;
    }
  }
  static const std::size_t numRequired = item->numberOfRequiredValues();
  static const std::size_t maxAllowed = item->maxNumberOfValues();
  switch (src)
  {
    case UpdateSource::ITEM_FROM_GUI:
    {
      if ((numRequired > 0 && uiMembers < numRequired) ||
        (maxAllowed > 0 && uiMembers > maxAllowed))
      {
        // UI cannot satisfy item. Return.
        return false;
      }
      // Everything else in this case statement should really be
      // a single, atomic operation executed on the attribute/item:
      if (!item->setNumberOfValues(uiMembers))
      {
        return false;
      }
      int idx = 0;
      for (auto member : m_p->m_members)
      {
        if (member.second)
        {
          if (!item->setValue(
                idx, std::dynamic_pointer_cast<smtk::resource::Resource>(member.first)))
          {
            return false; // Huh!?!
          }
          ++idx;
        }
      }
      emit modified();
    }
    break;

    case UpdateSource::GUI_FROM_ITEM:
      m_p->m_members.clear();
      m_p->m_phraseModel->triggerDataChanged();
      for (auto vit = item->begin(); vit != item->end(); ++vit)
      {
        // Only allow non-null pointers into the set of selected items;
        // null pointers indicate that the item's entry is invalid and
        // the size of m_members is used to determine whether the
        // association's rules are met, so an extra entry can prevent
        // the association from being edited by the user.
        if (*vit)
        {
          m_p->m_members[*vit] = 1; // FIXME: Use a bit specified by the application.
        }
      }
      break;
  }
  return true;
}
}
}
