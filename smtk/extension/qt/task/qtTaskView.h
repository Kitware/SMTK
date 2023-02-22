//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskView_h
#define smtk_extension_qtTaskView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsView>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtTaskEditor;
class qtTaskScene;

/**\brief A widget that holds a Qt scene graph.
  *
  */
class SMTKQTEXT_EXPORT qtTaskView : public QGraphicsView
{
  Q_OBJECT

public:
  using Superclass = QGraphicsView;

  qtTaskView(qtTaskScene* scene, qtTaskEditor* widget = nullptr);
  ~qtTaskView() override;

  // qtTaskScene* taskScene() const;
  // qtTaskEditor* taskEditor() const;

protected:
  void wheelEvent(QWheelEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskView_h
