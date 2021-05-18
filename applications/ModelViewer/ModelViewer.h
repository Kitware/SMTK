//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __ModelViewer_h
#define __ModelViewer_h

#include <QWidget>

class QTreeView;
class QModelIndex;

class ModelViewer : public QWidget
{
  Q_OBJECT
public:
  ModelViewer(QWidget* parent = nullptr);
  ~ModelViewer() override;

protected:
  class Internals;
  Internals* m_p;
};

#endif
