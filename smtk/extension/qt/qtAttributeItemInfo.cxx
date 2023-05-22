//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeItemInfo.h"

#include "smtk/extension/qt/qtItem.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/PathGrammar.h"

using namespace smtk::attribute;
using namespace smtk::extension;

qtAttributeItemInfo::qtAttributeItemInfo(
  ItemPtr item,
  smtk::view::Configuration::Component itemComp,
  QPointer<QWidget> parent,
  qtBaseView* bview)
  : m_item(item)
  , m_component(itemComp)
  , m_parentWidget(parent)
{
  m_baseView = qobject_cast<qtBaseAttributeView*>(bview);
}

qtUIManager* qtAttributeItemInfo::uiManager() const
{
  if (m_baseView)
  {
    return m_baseView->uiManager();
  }
  return nullptr;
}

bool qtAttributeItemInfo::buildFromComponent(
  smtk::view::Configuration::Component comp,
  qtBaseAttributeView* view,
  std::map<std::string, qtAttributeItemInfo>& dict)
{
  std::string iname, path;
  std::size_t i, n = comp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    // There are 2 forms supported one that have an Item Attribute (old style) and
    // one that has a Path attribute (new style).  Note that Item="foo" is equivalent to
    // Path="/foo"

    // Do we have an item attribute?
    if (comp.child(i).attribute("Item", iname))
    {
      // Is this the first time we have encountered this item>
      auto it = dict.find(iname);
      if (it == dict.end())
      {
        // Ok we need to "prep" it
        qtAttributeItemInfo info;
        info.m_baseView = view;
        info.m_component = comp.child(i);
        dict[iname] = info;
      }
      else
      {
        it->second.m_component = comp.child(i);
      }
    }
    else if (comp.child(i).attribute("Path", path))
    {
      //Lets parse this to get the leading component of the path
      std::string subPath;
      bool ok;
      pegtl::string_input<> in(path, "parsingPath");
      pegtl::parse<smtk::attribute::pathGrammar::grammar, smtk::attribute::pathGrammar::action>(
        in, iname, subPath, ok);
      if (!ok)
      {
        std::cerr << "Failed to parse item path: " << path << std::endl;
      }
      else
      {
        // Is this the first time we have encountered this item>
        auto it = dict.find(iname);
        if (it == dict.end())
        {
          // Ok we need to "prep" it
          qtAttributeItemInfo info;
          info.m_baseView = view;
          if (subPath.empty())
          {
            // The path refers to this item
            info.m_component = comp.child(i);
          }
          else
          {
            // this path refers to a child of this item
            info.m_childrenViewInfo[subPath] = comp.child(i);
          }
          dict[iname] = info;
        }
        else if (subPath.empty())
        {
          it->second.m_component = comp.child(i);
        }
        else
        {
          it->second.m_childrenViewInfo[subPath] = comp.child(i);
        }
      }
    }
  }
  return true;
}

bool qtAttributeItemInfo::createNewDictionary(std::map<std::string, qtAttributeItemInfo>& dict)
{

  std::string iname, path;
  for (auto const& entry : m_childrenViewInfo)
  {
    // Lets parse the child's path and add it to to dictionary
    std::string subPath;
    bool ok;
    pegtl::string_input<> in(entry.first, "parsingPath");
    pegtl::parse<smtk::attribute::pathGrammar::grammar, smtk::attribute::pathGrammar::action>(
      in, iname, subPath, ok);
    if (!ok)
    {
      std::cerr << "Failed to parse item path: " << entry.first << std::endl;
    }
    else
    {
      // Is this the first time we have encountered this item>
      auto it = dict.find(iname);
      if (it == dict.end())
      {
        // Ok we need to "prep" it
        qtAttributeItemInfo info;
        info.m_baseView = m_baseView;
        if (subPath.empty())
        {
          // The path refers to this item
          info.m_component = entry.second;
        }
        else
        {
          // this path refers to a child of this item
          info.m_childrenViewInfo[subPath] = entry.second;
        }
        dict[iname] = info;
      }
      else if (subPath.empty())
      {
        it->second.m_component = entry.second;
      }
      else
      {
        it->second.m_childrenViewInfo[subPath] = entry.second;
      }
    }
  }

  // Now add any new ItemView Information declared in this item's configuration
  // Does the component representing the attribute contain a Style block?
  int sindex = m_component.findChild("ItemViews");
  if (sindex == -1)
  {
    return true;
  }
  auto iviews = m_component.child(sindex);
  buildFromComponent(iviews, m_baseView, dict);

  return true;
}

qtBaseAttributeView* qtAttributeItemInfo::baseView() const
{
  return m_baseView;
}

bool qtAttributeItemInfo::toBeDisplayed() const
{
  // Is there an itemView Configuration that indicates
  // the item should not be displayed (Type set to null)
  std::string qtItemViewType;
  if (m_component.attribute("Type", qtItemViewType) && (qtItemViewType == "null"))
  {
    return false;
  }

  smtk::attribute::ItemPtr theItem = m_item.lock();
  return (!m_baseView || (m_baseView->displayItem(theItem)));
}
