//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtBaseObjectNode_h
#define smtk_extension_qtBaseObjectNode_h

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace extension
{

class qtDiagramScene;

/**\brief A Graphical Item that represents a persistent object as a node in a scene.
  *
  */
class SMTKQTEXT_EXPORT qtBaseObjectNode : public qtBaseNode
{
  Q_OBJECT
  Q_PROPERTY(smtk::resource::PersistentObject* object READ object);

public:
  smtkSuperclassMacro(qtBaseNode);
  smtkTypeMacro(smtk::extension::qtBaseObjectNode);

  qtBaseObjectNode(
    qtDiagramGenerator* generator,
    smtk::resource::PersistentObject* obj,
    QGraphicsItem* parent = nullptr);
  ~qtBaseObjectNode() override;

  /// Return the task this node represents.
  virtual smtk::resource::PersistentObject* object() const = 0;

  /// Return the object's name as the node's name.
  std::string name() const override;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtBaseObjectNode_h
