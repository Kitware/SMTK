//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qt_testing_cxx_UnitTestOperationLauncher_h
#define smtk_extension_qt_testing_cxx_UnitTestOperationLauncher_h

#include <QObject>

#include "smtk/extension/qt/qtOperationLauncher.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include <vector>

class TestOperationLauncher : public QObject
{
  Q_OBJECT

public:
  TestOperationLauncher(
    const smtk::resource::Manager::Ptr& resourceManager,
    const smtk::operation::Manager::Ptr& operationManager,
    QObject* parent)
    : QObject(parent)
    , m_resourceManager(resourceManager)
    , m_operationManager(operationManager)
    , m_nReadFiles(0)
  {
  }

  std::vector<std::future<smtk::operation::Operation::Result>> OpenFiles(
    const std::vector<std::string>& files);

  std::size_t numberOfReadFiles() const { return m_nReadFiles; }

Q_SIGNALS:
  void finished();

public Q_SLOTS:
  void run();

private:
  smtk::extension::qtOperationLauncher* GetLauncher();

  smtk::resource::Manager::Ptr m_resourceManager;
  smtk::operation::Manager::Ptr m_operationManager;
  std::size_t m_nReadFiles;
};

#endif
