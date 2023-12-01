//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramLegendEntry_h
#define smtk_extension_qtDiagramLegendEntry_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/SharedFromThis.h" // For macros.

#include <QObject>

class QPainter;
class QStyleOptionViewItem;
class QWidget;

namespace smtk
{
namespace extension
{

class qtDiagramGenerator;
class qtDiagramScene;
class qtBaseNode;

/**\brief All the information needed to represent a symbol used in a diagram.
  *
  * This object holds an object group ("arc", "node", etc.),
  * an object type ("task dependency", "smtk::markup::arcs::BoundariesToShapes", etc.),
  * and a method used to draw a symbol in the legend to illustrate their
  * appearance in the diagram.
  *
  * This object is also used to control the visibility of its targets
  * in the diagram (i.e., clicking on the legend entry can toggle the visiblity
  * of arcs/nodes/… of the given object type).
  *
  * In the future, the legend may serve as a draggable UI element when the
  * legend has an operation to create new instances of itself.
  */
class SMTKQTEXT_EXPORT qtDiagramLegendEntry : public QObject
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QObject);
  smtkTypeMacroBase(smtk::extension::qtDiagramLegendEntry);

  qtDiagramLegendEntry(
    smtk::string::Token entryGroup,
    smtk::string::Token entryType,
    qtDiagramGenerator* generator,
    smtk::string::Token entryLabel = smtk::string::Token::Invalid);
  ~qtDiagramLegendEntry() override;

  qtDiagramGenerator* generator() const { return m_generator; }
  qtDiagramScene* scene() const;
  smtk::string::Token group() const { return m_group; }
  smtk::string::Token type() const { return m_type; }
  smtk::string::Token label() const { return m_label.valid() ? m_label : m_type; }

  virtual bool toggleVisibility(bool show)
  {
    (void)show;
    return false;
  }

  // Compare two entries by value.
  bool operator==(const qtDiagramLegendEntry& other) const
  {
    return m_generator == other.m_generator && m_group == other.m_group && m_type == other.m_type;
  }

  /// Draw a symbol into the legend. This method is called by the qtDiagramLegend's delegate.
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, QWidget* widget);

protected:
  qtDiagramGenerator* m_generator{ nullptr };
  smtk::string::Token m_group;
  smtk::string::Token m_type;
  smtk::string::Token m_label;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDiagramLegendEntry_h
