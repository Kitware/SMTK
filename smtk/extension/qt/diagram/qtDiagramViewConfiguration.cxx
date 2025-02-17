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

using namespace smtk::string::literals;

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
  m_selectionColor = QColor("#ff00ff");

  // TODO - Have the ability to load in the workflow
  // designer's custom palettes and indicate if
  // the workflow is using them, the defaults, or
  // user specified palettes

  // clang-format off
  m_lightPalette = {
    {"irrelevant",       QColor("#CCCCCC")},
    {"unavailable",      QColor("#DA7C7C")},
    {"incomplete",       QColor("#FFCC00")},
    {"completable",      QColor("#BCD35F")},
    {"completed",        QColor("#677821")},
    {"text",             QColor("#000000")},
    {"base",             QColor("#B8B8B8")},
    {"background",       QColor("#FFFFFF")},
    {"border",           QColor("#000000")},
    {"port",             QColor("#999999")},
    { "task dependency", QColor("#BF5B17")},
    { "task adaptor",    QColor("#386CB0") }
  };

  m_darkPalette = {
    {"irrelevant",       QColor("#999999")},
    {"unavailable",      QColor("#A83030")},
    {"incomplete",       QColor("#E1B400")},
    {"completable",      QColor("#7D8B45")},
    {"completed",        QColor("#AEC652")},
    {"text",             QColor("#E6E6E6")},
    {"base",             QColor("#737373")},
    {"background",       QColor("#000000")},
    {"border",           QColor("#000000")},
    {"port",             QColor("#B3B3B3")},
    { "task dependency", QColor("#BF5B17") },
    { "task adaptor",    QColor("#386CB0") }
    // clang-format on
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

  int cpindex = styleComp.findChild("ColorPalettes");
  if (cpindex >= 0)
  {
    std::string label;
    std::string color;

    int dpindex = styleComp.child(cpindex).findChild("Dark");
    if (dpindex >= 0)
    {
      const auto& dpalette = styleComp.child(cpindex).child(dpindex);
      for (const auto& entry : dpalette.children())
      {
        label = entry.attributeAsString("Label");
        color = entry.attributeAsString("Color");
        if (!(label.empty() || color.empty()))
        {
          m_darkPalette[label] = QColor(QString::fromStdString(color));
        }
      }
    }
    int lpindex = styleComp.child(cpindex).findChild("Light");
    if (lpindex >= 0)
    {
      const auto& lpalette = styleComp.child(cpindex).child(lpindex);
      for (const auto& entry : lpalette.children())
      {
        label = entry.attributeAsString("Label");
        color = entry.attributeAsString("Color");
        if (!(label.empty() || color.empty()))
        {
          m_lightPalette[label] = QColor(QString::fromStdString(color));
        }
      }
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

QColor qtDiagramViewConfiguration::colorFromToken(const smtk::string::Token& token) const
{
  // In addition to the light and dark palettes that contain mappings for
  // "known" tokens, create a palette that can be used consistently for
  // "unknown" tokens
  static std::array<QColor, 8> palette = { { QColor("#66C2A5"),
                                             QColor("#FC8D62"),
                                             QColor("#8DA0CB"),
                                             QColor("#E78AC3"),
                                             QColor("#A6D854"),
                                             QColor("#FFD92F"),
                                             QColor("#E5C494"),
                                             QColor("#B3B3B3") } };
  if (QApplication::palette().window().color().lightnessF() > 0.5)
  {
    auto it = m_lightPalette.find(token);
    if (it != m_lightPalette.end())
    {
      return it->second;
    }
  }
  else
  {
    auto it = m_darkPalette.find(token);
    if (it != m_darkPalette.end())
    {
      return it->second;
    }
  }
  // The token is not known so map in into one of
  // the entries in the above palette.
  auto entry = static_cast<int>(token.id());
  int idx = (entry < 0 ? -entry : entry) % 8;
  return palette[idx];
}

QColor qtDiagramViewConfiguration::baseNodeColor() const
{
  return this->colorFromToken("base"_token);
}

QColor qtDiagramViewConfiguration::textColor() const
{
  return this->colorFromToken("text"_token);
}

QColor qtDiagramViewConfiguration::backgroundColor() const
{
  return this->colorFromToken("background"_token);
}

QColor qtDiagramViewConfiguration::borderColor() const
{
  return this->colorFromToken("border"_token);
}

QColor qtDiagramViewConfiguration::portNodeColor() const
{
  return this->colorFromToken("port"_token);
}

QColor qtDiagramViewConfiguration::colorForState(smtk::task::State state) const
{
  if (QApplication::palette().window().color().lightnessF() > 0.5)
  {
    auto it = m_lightPalette.find(smtk::task::stateToken(state));
    if (it != m_lightPalette.end())
    {
      return it->second;
    }
  }
  else
  {
    auto it = m_darkPalette.find(smtk::task::stateToken(state));
    if (it != m_darkPalette.end())
    {
      return it->second;
    }
  }
  return QColor("#000000");
}

} // namespace extension
} // namespace smtk
