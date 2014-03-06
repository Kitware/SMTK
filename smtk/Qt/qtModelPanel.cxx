#include "smtk/Qt/qtModelPanel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"
#include "smtk/Qt/qtEntityItemDelegate.h"
#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtModelView.h"

#include <QPointer>

#include "ui_qtModelPanel.h"
namespace Ui { class qtModelPanel; }

// -----------------------------------------------------------------------------
namespace smtk {
  namespace model {


//-----------------------------------------------------------------------------
class qtModelPanel::qInternal : public Ui::qtModelPanel
{
public:
  QPointer<qtModelView> ModelView;

  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qtModelPanel::qtModelPanel(QWidget* p)
  : QWidget(p)
{
  this->Internal = new qInternal;
  this->Internal->ModelView = new qtModelView(p);
  this->Internal->setupUi(this);

  this->Internal->treeviewLayout->addWidget(this->Internal->ModelView);
  // signals/slots

  QObject::connect(this->Internal->AddDomainset,
    SIGNAL(clicked()), this, SLOT(onAddDomainset()));
  QObject::connect(this->Internal->AddBC,
    SIGNAL(clicked()), this, SLOT(onAddBC()));
  QObject::connect(this->Internal->RemoveButton,
    SIGNAL(clicked()), this, SLOT(onRemove()));
}

//-----------------------------------------------------------------------------
qtModelPanel::~qtModelPanel()
{
}

//-----------------------------------------------------------------------------
smtk::model::qtModelView* qtModelPanel::getModelView()
{
  return this->Internal->ModelView;
}

//-----------------------------------------------------------------------------
void qtModelPanel::onAddDomainset()
{
  this->getModelView()->addGroup(DIMENSION_3, "Domain Set");
}

//-----------------------------------------------------------------------------
void qtModelPanel::onAddBC()
{
  this->getModelView()->addGroup(DIMENSION_2, "BC Group");
}

//-----------------------------------------------------------------------------
void qtModelPanel::onRemove()
{
  this->getModelView()->removeSelected();
}

  } // namespace model
} // namespace smtk
