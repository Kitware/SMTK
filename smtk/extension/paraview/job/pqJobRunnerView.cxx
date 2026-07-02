//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqJobRunnerView.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/agents/JobAgent.h"

#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

// ParaView includes
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqReloadFilesReaction.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqView.h"

#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMProxy.h"
#include "vtkSMSourceProxy.h"

// VTK includes
#include "vtkNew.h"

// Qt includes
#include <QDebug>
#include <QDir>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>

#include <chrono>
#include <ctime>
#include <sstream>

#include "moc_pqJobRunnerView.cpp"

Q_DECLARE_METATYPE(smtk::attribute::Attribute::Ptr);

using namespace smtk::string::literals;

namespace // anonmyous
{

std::map<std::filesystem::path, QPointer<pqPipelineSource>> s_artifacts;

template<typename TP>
std::time_t to_time_t(TP tp)
{
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
  return system_clock::to_time_t(sctp);
}

} // anonymous namespace

class pqJobRunnerView::Internal
{
public:
  Internal(pqJobRunnerView* self, const smtk::view::ConfigurationPtr& config)
    : m_view(self)
  {
    QPointer<pqJobRunnerView> selfp(self);
    QWidget* ww = m_view->widget();
    // Configure the widget.
    auto* topLevelLayout = new QVBoxLayout;
    auto* upperLayout = new QHBoxLayout;
    m_run = new QPushButton("Run");
    m_run->setObjectName("JobControl");
    upperLayout->addStretch();
    upperLayout->addWidget(m_run);
    topLevelLayout->addItem(upperLayout);
    ww->setLayout(topLevelLayout);
    // Connect widgets.
    // One-time initialization:
    QObject::connect(m_run, &QPushButton::clicked, self, &::pqJobRunnerView::onRunClicked);
    m_agent = smtk::job::agents::JobAgent::activeJobAgent(
      self->uiManager()->managers(), smtk::string::Token());
    // Stuff that must be updated with each job.
    this
      ->updateJobControls(); // Add per-stage status and log button. Add artifact vis controls. Update label of m_run button.
  }

  void updateJobControls() {}

  ::pqJobRunnerView* m_view{ nullptr };
  QPushButton* m_run{ nullptr };
  smtk::job::agents::JobAgent* m_agent{ nullptr };
  int m_jobObserver{ -1 };
  std::weak_ptr<smtk::job::Job> m_lastJob;
};

smtk::extension::qtBaseView* pqJobRunnerView::createViewWidget(const smtk::view::Information& info)
{
  pqJobRunnerView* view = new pqJobRunnerView(info);
  return view;
}

pqJobRunnerView::pqJobRunnerView(const smtk::view::Information& info)
  : Superclass(info)
{
  qRegisterMetaType<smtk::attribute::Attribute::Ptr>("smtk::attribute::Attribute::Ptr");
  this->createWidget();
  m_p =
    std::make_unique<Internal>(this, this->configuration()); // NB: Must come after createWidget().
}

void pqJobRunnerView::updateUI()
{
  std::cerr << "TODO: Update UI\n";
}

void pqJobRunnerView::onRunClicked()
{
  auto op = m_p->m_agent ? m_p->m_agent->operation() : nullptr;
  if (!op)
  {
    QString message("Internal Error: failed to fetch operation.");
    QMessageBox::critical(this->Widget, "Error", message);
    return;
  }

  // If the operation has a "task" parameter, populate it with the given task.
  auto taskItem = op->parameters()->findComponent("task");
  if (taskItem)
  {
    taskItem->appendValue(m_p->m_agent->parent()->shared_from_this());
  }

  {
    // Check if case directory exists
    auto caseDir(m_p->m_agent->caseDirectory());
    if (std::filesystem::exists(caseDir))
    {
      QString message;
      QTextStream qs(&message);
      qs << "This action may delete the current analysis folder "
         << QString::fromStdString(caseDir.string()) << " including any solution data there."
         << "\n\nAre you sure you want to proceed?";

      auto buttons = QMessageBox::Ok | QMessageBox::Cancel;
      auto reply =
        QMessageBox::warning(this->Widget, "Warning", message, buttons, QMessageBox::Cancel);
      if (reply != QMessageBox::Ok)
      {
        qInfo() << "Aborting Run";
        return;
      }
    } // if caseDir exists
  }   // if deleteCaseDirectory

  // Launch the operation with a handler so that when it completes we
  // can submit the input deck as a job in a separate process.
  QPointer<pqJobRunnerView> self(this);
  op->addHandler(
    [self, this](smtk::operation::Operation& op, const smtk::operation::Operation::Result& res) {
      // std::cerr << "Handler, op " << &op << " res " << res << "\n";
      if (!self)
      {
        return;
      }
      for (const auto& obj : *res->findComponent("created"))
      {
        if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(obj))
        {
          if (auto* queue = job->queue())
          {
            if (m_p->m_jobObserver >= 0)
            {
              if (auto lastJob = m_p->m_lastJob.lock())
              {
                lastJob->queue()->unobserve(lastJob.get(), m_p->m_jobObserver);
              }
            }
            m_p->m_lastJob = job;
            m_p->m_jobObserver = queue->observe(
              job.get(),
              [self, this](const smtk::job::Job& updatedJob) {
                if (!self)
                {
                  return;
                }
                // std::cerr << "Job updated to stage " << updatedJob.stage() << "\n";
                // if (auto* runner = smtk::openfoam::qtJobRunner::instance())
                // {
                //   auto logPath = updatedJob.stageLog().string();
                //   if (!logPath.empty())
                //   {
                //     runner->showLogFile(logPath.c_str());
                //   }
                // }
                this->updateUI();
              },
              true);
          }
        }
      }
    },
    0);
  op->manager()->launchers()(op->shared_from_this());
}

void pqJobRunnerView::updateJobControls()
{
  m_p->updateJobControls();
}

void pqJobRunnerView::createWidget()
{
  this->Widget = new QWidget(this->parentWidget());
  this->Widget->setObjectName("pqJobRunnerView");
}
