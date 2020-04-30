//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_qt_TypeAndColorBadge_h
#define __smtk_extension_RedirectOutput_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"
#include "smtk/io/Logger.h"

#include "smtk/extension/qt/TypeAndColorBadge.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"
#include "smtk/view/ObjectIconBadge.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FloatData.h"

#include <QColor>
#include <QColorDialog>

namespace smtk
{
namespace extension
{
namespace qt
{

TypeAndColorBadge::TypeAndColorBadge()
  : Superclass()
{
}

TypeAndColorBadge::TypeAndColorBadge(
  smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component& comp)
  : Superclass(parent, comp)
{
}

TypeAndColorBadge::~TypeAndColorBadge() = default;

QColor getPhraseColor(smtk::view::DescriptivePhrase* item)
{
  QColor color;
  if (!(item->relatedComponent() || item->phraseType() == smtk::view::DescriptivePhraseType::LIST))
  {
    return color;
  }

  smtk::model::FloatList rgba = item->relatedColor();
  bool gotColor = false;
  if (rgba.size() >= 4 && rgba[3] < 0)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(item->relatedComponent());
    if (modelComp)
    {
      smtk::model::EntityRef ent(modelComp);
      if (ent.isFace())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Face");
        gotColor = true;
      }
      else if (ent.isEdge())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Edge");
        gotColor = true;
      }
      else if (ent.isVertex())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Vertex");
        gotColor = true;
      }
    }
    if (!gotColor)
    {
      // return an invalid color by default
      color = QColor();
    }
  }
  else
  {
    // Color may be luminance, luminance+alpha, rgb, or rgba:
    switch (rgba.size())
    {
      case 0:
        color = QColor(0, 0, 0, 0);
        break;
      case 1:
        color.setHslF(0., 0., rgba[0], 1.);
        break;
      case 2:
        color.setHslF(0., 0., rgba[0], rgba[1]);
        break;
      case 3:
        color.setRgbF(rgba[0], rgba[1], rgba[2], 1.);
        break;
      case 4:
      default:
        color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
        break;
    }
  }
  return color;
}

void TypeAndColorBadge::action(smtk::view::DescriptivePhrase* phrase) const
{
  if ((phrase->phraseModel() == nullptr) || (phrase->phraseModel()->operationManager() == nullptr))
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Can not access Operation Manager for editing color!");
    return;
  }
  std::string dialogInstructions =
    "Choose Color for " + phrase->title() + " (click Cancel to remove color)";
  // If the currentColor is invalid lets set it to opaque white
  QColor currentColor = getPhraseColor(phrase);
  if (!currentColor.isValid())
  {
    currentColor.setRed(255);
    currentColor.setGreen(255);
    currentColor.setBlue(255);
    currentColor.setAlpha(255);
  }

  QColor nextColor = QColorDialog::getColor(currentColor, nullptr, dialogInstructions.c_str(),
    QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
  bool canceled = !nextColor.isValid();
  if (!canceled)
  {
    smtk::model::FloatList rgba{ nextColor.red() / 255.0, nextColor.green() / 255.0,
      nextColor.blue() / 255.0, nextColor.alpha() / 255.0 };
    phrase->setRelatedColor(rgba);
  }
}
}
}
}

#endif
