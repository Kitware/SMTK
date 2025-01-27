//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramViewConfiguration_h
#define smtk_extension_qtDiagramViewConfiguration_h

#include "smtk/common/Deprecation.h"
#include "smtk/extension/qt/diagram/qtPreviewArc.h"
#include "smtk/extension/qt/diagram/qtTaskArc.h"
#include "smtk/string/Token.h"
#include "smtk/task/State.h"

#include <QColor>

#include <array>
#include <unordered_map>

namespace smtk
{
namespace view
{
class Configuration;
}
namespace extension
{

/**\brief An object to hold view configuration settings.
  *
  * This class extracts settings used to render the task view from a
  * `view::Configuration` instance and makes them quickly accessible
  * to the Qt classes that render tasks.
  */
class SMTKQTEXT_EXPORT qtDiagramViewConfiguration
{
public:
  qtDiagramViewConfiguration(const smtk::view::Configuration& viewConfig);

  QColor backgroundFillColor() const { return m_backgroundFillColor; }
  QColor backgroundGridColor() const { return m_backgroundGridColor; }

  ///\brief Return the color used to indicate the task is active.
  QColor activeTaskColor() const { return m_activeTaskColor; }

  ///\brief Return the color used to indicate the item is selected.
  QColor selectionColor() const { return m_selectionColor; }

  ///\brief Return the color used to indicate the task's state.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor colorForState(smtk::task::State state) const;

  ///\brief Return the base color used for a node.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor baseNodeColor() const;
  ///\brief Return the base color used for a task port node.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor portNodeColor() const;
  ///\brief Return the color used to display text.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor textColor() const;
  ///\brief Return the color used for the background.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor backgroundColor() const;
  ///\brief Return the color used for the node's border when
  /// not selected or active.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor borderColor() const;
  ///\brief Return the color associated with a string token.
  /// If the token is not found in the configuration's palette,
  /// a consistent color will be returned based on its integer value.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  QColor colorFromToken(const smtk::string::Token& token) const;

  // Returns the color based on the arc type.
  ///
  /// Note that this method will take light/dark mode into
  /// consideration.
  SMTK_DEPRECATED_IN_NEXT("Use qtDiagramViewConfiguration::colorFromToken instead.")
  QColor colorForArcType(smtk::string::Token arcType) const
  {
    return this->colorFromToken(arcType);
  }

  QColor colorForArcStatus(qtPreviewArc::ArcStatus status) const
  {
    return m_colorForArcStatus[static_cast<int>(status)];
  }

  qreal nodeWidth() const { return m_nodeWidth; }
  qreal nodeRadius() const { return m_nodeRadius; }
  qreal nodeHeadlineHeight() const { return m_nodeHeadlineHeight; }
  qreal nodeHeadlinePadding() const { return m_nodeHeadlinePadding; }
  qreal nodeBorderThickness() const { return m_nodeBorderThickness; }
  int nodeFontSize() const { return m_nodeFontSize; }
  int nodeLayer() const { return m_nodeLayer; }

  qreal arcWidth() const { return m_arcWidth; }
  qreal arcOutline() const { return m_arcOutline; }
  int arcLayer() const { return m_arcLayer; }

  qreal arrowStemLength() const { return m_arrowStemLength; }
  qreal arrowHeadLength() const { return m_arrowHeadLength; }
  qreal arrowTipAspectRatio() const { return m_arrowTipAspectRatio; }

  int constructionLayer() const { return m_constructionLayer; }

protected:
  std::unordered_map<smtk::string::Token, QColor> m_lightPalette;
  std::unordered_map<smtk::string::Token, QColor> m_darkPalette;
  QColor m_backgroundFillColor;
  QColor m_backgroundGridColor;
  QColor m_selectionColor;
  QColor m_activeTaskColor;

  std::array<QColor, static_cast<int>(qtPreviewArc::ArcStatus::Valid) + 1> m_colorForArcStatus;

  std::unordered_map<smtk::string::Token, QColor> m_colorForArcType;

  qreal m_nodeWidth{ 300. };
  qreal m_nodeRadius{ 4. };
  qreal m_nodeHeadlineHeight{ 13 };
  qreal m_nodeHeadlinePadding{ 4. };
  qreal m_nodeBorderThickness{ 4. };
  int m_nodeFontSize{ 13 };
  int m_nodeLayer{ 10 };

  qreal m_arcWidth{ 4. };
  qreal m_arcOutline{ 1. };
  int m_arcLayer{ 5 };

  qreal m_arrowStemLength{ 16. };    // The length of the path guaranteed to be a straight line.
  qreal m_arrowHeadLength{ 12. };    // The length of the arrow head along the linear stem.
  qreal m_arrowTipAspectRatio{ 2. }; // The width of the arrow head as a fraction of head length.

  int m_constructionLayer{ 15 };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDiagramViewConfiguration_h
