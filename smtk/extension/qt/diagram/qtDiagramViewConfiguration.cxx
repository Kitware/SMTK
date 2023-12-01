//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"

#include "smtk/io/Logger.h"
#include "smtk/string/Token.h"
#include "smtk/task/State.h"
#include "smtk/view/Configuration.h"

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace smtk
{
namespace extension
{

qtDiagramViewConfiguration::qtDiagramViewConfiguration(const smtk::view::Configuration& viewConfig)
{
  using namespace smtk::string::literals;
  // Create default colors from QApplication's palette.
  auto palette = qApp->palette(static_cast<QWidget*>(nullptr));
  m_backgroundFillColor = palette.base().color();
  m_backgroundGridColor = palette.midlight().color();
  m_activeTaskColor = palette.highlight().color();
  m_colorForState = {
    QColor("#e7e7e7"), // irrelevant
    QColor("#ff9898"), // unavailable
    QColor("#ffeca3"), // incomplete
    QColor("#e0f1ba"), // completable
    QColor("#7ed637")  // completed
  };
  m_colorForArcType = {
    { "task dependency", QColor("#BF5B17") }, // dependency
    { "task adaptor", QColor("#386CB0") }     // adaptor
  };
  // NB!!! QColor uses ARGB rather than RGBA if 4-byte color is provided:
  m_colorForArcStatus = {
    QColor("#ffff5555"), // invalid
    QColor("#7f89b32d")  // valid
  };

  // Look for overrides from the workflow designer.
  int styleIdx = viewConfig.details().findChild("Style");
  if (styleIdx < 0)
  {
    return;
  }
  const auto& styleComp = viewConfig.details().child(styleIdx);

  int statusColorIdx = styleComp.findChild("StatusPalette");
  if (statusColorIdx >= 0)
  {
    using smtk::task::State;
    const auto& statusColor = styleComp.child(statusColorIdx);
    for (const auto& entry : statusColor.attributes())
    {
      bool validName;
      State state = smtk::task::stateEnum(entry.first, &validName);
      // Skip invalid attributes:
      if (!validName)
      {
        continue;
      }
      m_colorForState[static_cast<int>(state)] = QColor(QString::fromStdString(entry.second));
    }
  }

  int arcColorIdx = styleComp.findChild("ArcPalette");
  if (arcColorIdx >= 0)
  {
    const auto& arcColor = styleComp.child(arcColorIdx);
    for (const auto& entry : arcColor.attributes())
    {
      m_colorForArcType[entry.first] = QColor(QString::fromStdString(entry.second));
    }
  }

  int viewPaletteIdx = styleComp.findChild("ViewPalette");
  if (viewPaletteIdx >= 0)
  {
    const auto& viewPalette = styleComp.child(viewPaletteIdx);
    for (const auto& entry : viewPalette.attributes())
    {
      smtk::string::Token attName(entry.first);
      auto val = QString::fromStdString(entry.second);
      switch (attName.id())
      {
          // clang-format off
      case "ActiveTask"_hash:          m_activeTaskColor     = QColor(val); break;
      case "BackgroundGrid"_hash:      m_backgroundGridColor = QColor(val); break;
      case "BackgroundFill"_hash:      m_backgroundFillColor = QColor(val); break;
      default:
        smtkWarningMacro(smtk::io::Logger::instance(),
          "Unrecognized attribute \"" << entry.first << "\" in ViewPalette.");
        break;
          // clang-format on
      }
    }
  }

  int nodeLayoutIdx = styleComp.findChild("NodeLayout");
  if (nodeLayoutIdx >= 0)
  {
    const auto& nodeLayout = styleComp.child(nodeLayoutIdx);
    for (const auto& entry : nodeLayout.attributes())
    {
      smtk::string::Token attName(entry.first);
      switch (attName.id())
      {
          // clang-format off
      case "Width"_hash:           m_nodeWidth           = QString::fromStdString(entry.second).toDouble(); break;
      case "Radius"_hash:          m_nodeRadius          = QString::fromStdString(entry.second).toDouble(); break;
      case "HeadlineHeight"_hash:  m_nodeHeadlineHeight  = QString::fromStdString(entry.second).toDouble(); break;
      case "HeadlinePadding"_hash: m_nodeHeadlinePadding = QString::fromStdString(entry.second).toDouble(); break;
      case "BorderThickness"_hash: m_nodeBorderThickness = QString::fromStdString(entry.second).toDouble(); break;
      case "FontSize"_hash:        m_nodeFontSize        = QString::fromStdString(entry.second).toInt(); break;
      case "Layer"_hash:           m_nodeLayer           = QString::fromStdString(entry.second).toInt(); break;
      default:
        smtkWarningMacro(smtk::io::Logger::instance(),
          "Unrecognized attribute \"" << entry.first << "\" in NodeLayout.");
        break;
          // clang-format on
      }
    }
  }

  int arcLayoutIdx = styleComp.findChild("ArcLayout");
  if (arcLayoutIdx >= 0)
  {
    const auto& arcLayout = styleComp.child(arcLayoutIdx);
    for (const auto& entry : arcLayout.attributes())
    {
      smtk::string::Token attName(entry.first);
      switch (attName.id())
      {
          // clang-format off
      case "Width"_hash:               m_arcWidth            = QString::fromStdString(entry.second).toDouble(); break;
      case "Outline"_hash:             m_arcOutline          = QString::fromStdString(entry.second).toDouble(); break;
      case "ArrowStemLength"_hash:     m_arrowStemLength     = QString::fromStdString(entry.second).toDouble(); break;
      case "ArrowHeadLength"_hash:     m_arrowHeadLength     = QString::fromStdString(entry.second).toDouble(); break;
      case "ArrowTipAspectRatio"_hash: m_arrowTipAspectRatio = QString::fromStdString(entry.second).toDouble(); break;
      case "Layer"_hash:               m_arcLayer            = QString::fromStdString(entry.second).toInt(); break;
      default:
        smtkWarningMacro(smtk::io::Logger::instance(),
          "Unrecognized attribute \"" << entry.first << "\" in ArcLayout.");
        break;
          // clang-format on
      }
    }
  }

  // TODO: Look for overrides from the user (via QSettings).
}

QColor qtDiagramViewConfiguration::colorFromPalette(int entry) const
{
  static std::array<QColor, 8> palette = { { QColor("#66C2A5"),
                                             QColor("#FC8D62"),
                                             QColor("#8DA0CB"),
                                             QColor("#E78AC3"),
                                             QColor("#A6D854"),
                                             QColor("#FFD92F"),
                                             QColor("#E5C494"),
                                             QColor("#B3B3B3") } };
  int idx = (entry < 0 ? -entry : entry) % 8;
  return palette[idx];
}

} // namespace extension
} // namespace smtk
