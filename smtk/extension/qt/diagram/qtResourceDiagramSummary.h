//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtResourceDiagramSummary_h
#define smtk_extension_qtResourceDiagramSummary_h

#include "smtk/extension/qt/diagram/qtBaseNode.h"

#include "smtk/PublicPointerDefs.h"

#include <QLabel>
#include <QPointer>

namespace smtk
{
namespace extension
{

class qtDiagram;
class qtResourceDiagram;
class qtDiagramScene;

/**\brief A label each qtResourceDiagram owns and inserts into the qtDiagram's sidebar.
  *
  * The label is used to show the name and type of nodes as the user hovers
  * the pointer over them.
  */
class SMTKQTEXT_EXPORT qtResourceDiagramSummary : public QLabel
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QLabel);

  qtResourceDiagramSummary(qtResourceDiagram* scene, QWidget* parent = nullptr);
  ~qtResourceDiagramSummary() override;

  /// Deal with updates to the diagram (e.g., a change in the selection that the
  /// label should reflect).
  void dataUpdated();

  /// Set the subject node this node describes.
  bool setSubject(qtBaseNode* subject);

  /// Return the diagram-generator this summary widget belongs to.
  qtResourceDiagram* generator() const { return m_generator; }

  /// Return the diagram this summary widget belongs to.
  qtDiagram* diagram() const;

protected:
  /// Produce an HTML snippet describing a node.
  QString describe(qtBaseNode* node);

  /// The parent qtResourceDiagram
  QPointer<qtResourceDiagram> m_generator;
  /// The node which qtResourceDiagramSummary should show information about.
  QPointer<qtBaseNode> m_subject;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtResourceDiagramSummary_h
