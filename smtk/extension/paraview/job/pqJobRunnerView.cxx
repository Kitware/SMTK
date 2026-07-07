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
#include "smtk/extension/qt/qtLogView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/Stage.h"
#include "smtk/job/agents/JobAgent.h"
#include "smtk/job/operators/CancelJob.h"

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
#include <QFont>
#include <QLabel>
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
    QFont fontAwesome("Font Awesome 7 Free");
    m_lastRun = new QLabel("Last update: —");
    m_lastRun->setObjectName("LastRunLabel");
    m_lastRunStatus = new QLabel;
    m_lastRunStatus->setObjectName("LastRunStatus");
    m_lastRunStatus->setFont(fontAwesome);
    m_jobControl = new QPushButton("Run");
    m_jobControl->setObjectName("JobControl");
    m_stageGrid = new QGridLayout;
    m_stageGrid->setObjectName("StageGrid");
    m_stageGrid->setColumnMinimumWidth(0, 20);
    // m_stageGrid->setColumnStretch(0, 1);
    m_stageGrid->setColumnStretch(1, 1);
    m_artifactGrid = new QGridLayout;
    m_artifactGrid->setObjectName("ArtifactGrid");
    upperLayout->addWidget(m_lastRun);
    upperLayout->addStretch();
    upperLayout->addWidget(m_lastRunStatus);
    upperLayout->addWidget(m_jobControl);
    topLevelLayout->addLayout(upperLayout);
    topLevelLayout->addLayout(m_stageGrid);
    topLevelLayout->addLayout(m_artifactGrid);
    ww->setLayout(topLevelLayout);
    // Connect widgets.
    // One-time initialization:
    QObject::connect(m_jobControl, &QPushButton::clicked, self, &::pqJobRunnerView::onRunClicked);
    m_agent = smtk::job::agents::JobAgent::activeJobAgent(
      self->uiManager()->managers(), smtk::string::Token());
    // Stuff that must be updated with each job.
    this
      ->updateJobControls(); // Add per-stage status and log button. Add artifact vis controls. Update label of m_jobControl button.
  }

  void updateLastRunTime(smtk::job::Job* job)
  {
    if (!job)
    {
      m_lastRun->setText("Last update: —");
      return;
    }
    auto modificationTime = job->modificationTime();
    std::ostringstream text;
    text << "Last update: " << std::asctime(std::localtime(&modificationTime));
    m_lastRun->setText(QString::fromStdString(text.str()));
    QPalette palette = m_lastRunStatus->palette();
    switch (job->status())
    {
      default:
      case smtk::job::Status::Pending:
        // Spinner: f2f1 
        m_lastRunStatus->setText("");
        palette.setColor(QPalette::WindowText, QColor(100, 100, 100)); // medium grey
        break;
      case smtk::job::Status::Failed:
        // Exclamation: f06a 
        m_lastRunStatus->setText("");
        palette.setColor(QPalette::WindowText, QColor(210, 65, 34)); // dark red
        break;
      case smtk::job::Status::Succeeded:
        // Circle check: f058 
        m_lastRunStatus->setText("");
        palette.setColor(QPalette::WindowText, QColor(74, 166, 33)); // dark green
        break;
      case smtk::job::Status::Terminated:
        // Circle x-mark: f057
        text << "<b><span style=\"color: black; font-family:Font Awesome 7 "
                "Free\"></span></b>&nbsp;";
        break;
    }
    m_lastRunStatus->setPalette(palette);
  }

  void addStages(smtk::job::Definition* jobType)
  {
    if (!jobType)
    {
      return;
    }
    int ii = 0;
    QFont fontAwesome("Font Awesome 7 Free");
    for (const auto& stage : jobType->stages())
    {
      auto* stageLabel = new QLabel(QString::fromStdString(stage->name()));
      stageLabel->setToolTip(QString::fromStdString(stage->description()));
      m_stageGrid->addWidget(stageLabel, ii, 1);
      auto* stageDone = new QLabel;
      stageDone->setFont(fontAwesome);
      QPalette palette = stageDone->palette();
      palette.setColor(QPalette::WindowText, QColor(74, 166, 33)); // medium green
      stageDone->setPalette(palette);
      m_stageGrid->addWidget(stageDone, ii, 2);
      if (!stage->log().empty())
      {
        auto* stageLog = new QPushButton;
        stageLog->setFont(fontAwesome);
        stageLog->setText("");
        m_stageGrid->addWidget(stageLog, ii, 3);
        QObject::connect(stageLog, &QPushButton::clicked, [&]() {
          auto* job = m_agent->job();
          if (job)
          {
            auto* dialog = new smtk::qt::qtLogView(job->caseDirectory() / stage->log());
            // Keep this dialog modal until you can properly detect when the log is
            // truncated by another run.
            dialog->exec();
            delete dialog;
          }
        });
      }
      ++ii;
    }
    m_stageGrid->update();
  }

  void updateStages(smtk::job::Job* job)
  {
    if (!job)
    {
      return;
    }
    int stageIndex = job->stage();
    int ii = 0;
    for (const auto& stage : job->jobType()->stages())
    {
      if (auto* label = dynamic_cast<QLabel*>(m_stageGrid->itemAtPosition(ii, 2)->widget()))
      {
        label->setText(ii <= stageIndex ? "" : "");
      }
      ++ii;
    }
  }

  void updateJobControls()
  {
    auto job = m_agent->job();
    if (m_lastJob.lock().get() != job || !job)
    {
      // Reset grids.
      QLayoutItem* child;
      while ((child = m_stageGrid->takeAt(0)) != 0)
      {
        delete child;
      }
      while ((child = m_artifactGrid->takeAt(0)) != 0)
      {
        delete child;
      }
      // Populate grids.
      this->addStages(job ? job->jobType() : m_agent->jobType());
      // this->addArtifacts(job);
    }
    // Update m_lastRun to show the time the last job was started.
    this->updateLastRunTime(job);

    // Update m_jobControl to show "Run" or "Cancel" depending on the job state.
    if (!job || job->status() != smtk::job::Status::Pending)
    {
      m_jobControl->setText("Run");
    }
    else
    {
      m_jobControl->setText("Cancel");
    }
    // This method is only called when it is safe to click the button:
    m_jobControl->setEnabled(true);

    // Update the "job control" area to show the job stages, logs, and artifacts.
    this->updateStages(job);
  }

  ::pqJobRunnerView* m_view{ nullptr };
  QLabel* m_lastRun{ nullptr };
  QLabel* m_lastRunStatus{ nullptr };
  QPushButton* m_jobControl{ nullptr };
  QGridLayout* m_stageGrid{ nullptr };
  QGridLayout* m_artifactGrid{ nullptr };
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
  this->updateJobControls();
}

void pqJobRunnerView::onRunClicked()
{
  if (!m_p->m_agent)
  {
    return;
  }

  smtk::string::Token action = smtk::string::Token(m_p->m_jobControl->text().toStdString());
  switch (action.id())
  {
    case "Cancel"_hash:
    {
      if (auto job = m_p->m_agent->job())
      {
        if (auto operationManager = this->uiManager()->operationManager())
        {
          m_p->m_jobControl->setEnabled(false);
          auto op = operationManager->create<smtk::job::CancelJob>();
          op->parameters()->associate(job->shared_from_this());
          operationManager->launchers()(op);
        }
      }
    }
    break;
    default:
    case "Run"_hash:
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

      // Now that we are sure we will launch the operation, disable the "Run" button.
      // It will get re-enabled when the job is queued.
      m_p->m_jobControl->setEnabled(false);

      // Launch the operation with a handler so that when it completes we
      // can submit the input deck as a job in a separate process.
      QPointer<pqJobRunnerView> self(this);
      op->addHandler(
        [self,
         this](smtk::operation::Operation& op, const smtk::operation::Operation::Result& res) {
          // std::cerr << "Handler, op " << &op << " res " << res << "\n";
          if (!self)
          {
            return;
          }
          if (smtk::operation::outcome(res) != smtk::operation::Operation::Outcome::SUCCEEDED)
          {
            // The operation that was supposed to create the job did not succeed.
            // Reset the "Run" button so users can try again.
            m_p->m_jobControl->setText("Run");
            m_p->m_jobControl->setEnabled(true);
          }
          else
          {
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
                      this->updateJobControls();
                    },
                    true);
                }
              }
            }
          }
        },
        0);
      op->manager()->launchers()(op->shared_from_this());
    }
    break;
  }
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
