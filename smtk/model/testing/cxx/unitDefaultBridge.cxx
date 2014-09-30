//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/options.h"
#include "smtk/AutoInit.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Bridge.h"
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "cJSON.h"

#include "unitForwardingOperator_xml.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

namespace {

template<class T>
void printItem(typename T::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::size_t ne = item->numberOfValues();
  std::cout << " " << tname << " (" << ne << ") [";
  for (std::size_t e = 0; e < ne; ++e)
    {
    std::cout << " " << item->value(e);
    }
  std::cout << " ]";
}

template<>
void printItem<smtk::attribute::GroupItem>(smtk::attribute::GroupItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

template<>
void printItem<smtk::attribute::VoidItem>(smtk::attribute::VoidItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

} // anonymous namespace

#define smtkAttributeItemTypeCase(ENUM, CLASSNAME, ENTRYTYPE, ITEM, CODE) \
  case smtk::attribute::Item:: ENUM : \
    { \
    smtk::attribute:: CLASSNAME ::Ptr typedItem = \
      smtk::dynamic_pointer_cast<smtk::attribute:: CLASSNAME >( ITEM ); \
    std::string enumName( # ENUM ); \
    typedef smtk::attribute:: CLASSNAME ItemTType; \
    if (typedItem) \
      { \
      CODE; \
      } \
    } \
    break

#define smtkAttributeItemTypeSwitch(CODE) \
    smtkAttributeItemTypeCase(INT,IntItem,int,item,CODE); \
    smtkAttributeItemTypeCase(DOUBLE,DoubleItem,double,item,CODE); \
    smtkAttributeItemTypeCase(STRING,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(FILE,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(DIRECTORY,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(ATTRIBUTE_REF,RefItem,smtk::attribute::AttributePtr,item,CODE); \
    smtkAttributeItemTypeCase(MODEL_ENTITY,ModelEntityItem,smtk::model::Cursor,item,CODE); \
    smtkAttributeItemTypeCase(GROUP,GroupItem,smtk::attribute::ItemPtr,item,CODE); \
    smtkAttributeItemTypeCase(VOID,VoidItem,void,item,CODE); \
    default: \
     break; \

static void printParams(smtk::attribute::AttributePtr attr, const std::string& msg)
{
  std::cout << msg << "\n";
  if (!attr)
    return;
  std::size_t ni = attr->numberOfItems();
  for (std::size_t i = 0; i < ni; ++i)
    {
    std::cout << "  ";
    smtk::attribute::ItemPtr item = attr->item(i);
    switch (item->type())
      {
      smtkAttributeItemTypeSwitch(printItem<ItemTType>(typedItem, enumName));
      }
    std::cout << "\n";
    }
}

// Implement a forwarding bridge that does no communication.
// Instead, it just copies operator parameters _to_ a "remote"
// bridge and results back _from_ the "remote" bridge.
class TestForwardingBridge : public smtk::model::DefaultBridge
{
public:
  smtkTypeMacro(TestForwardingBridge); // Provides typedefs for Ptr, SelfType, ...
  smtkCreateMacro(TestForwardingBridge); // Provides static create() method
  smtkSharedFromThisMacro(Bridge); // Provides shared_from_this() method
  smtkDeclareModelingKernel(); // Declares name() and utility methods/members.

  smtk::model::Manager::Ptr remoteModel;
  smtk::model::Bridge::Ptr remoteBridge;

  void addSomeRemoteDanglers(const UUIDs& danglers)
    {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
      {
      this->remoteBridge->declareDanglingEntity(
        smtk::model::Cursor(this->remoteModel, *it),
        smtk::model::BRIDGE_NOTHING);
      }
    }

  bool checkLocalDanglers(const UUIDs& danglers, smtk::model::ManagerPtr modelMgr)
    {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
      {
      smtk::model::DanglingEntities::const_iterator cit =
        this->m_dangling.find(
          smtk::model::Cursor(modelMgr, *it));
      if (cit == this->m_dangling.end())
        return false;
      }
    return true;
    }

protected:
  TestForwardingBridge()
    {
    this->initializeOperatorManager(TestForwardingBridge::s_operators);
    }

  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags)
    {
    return remoteBridge->transcribe(Cursor(remoteModel, entity.entity()), flags);
    }

  virtual bool ableToOperateDelegate(RemoteOperatorPtr oper)
    {
    OperatorPtr remOp = remoteBridge->op(oper->name(), remoteModel);
    remOp->setSpecification(remoteBridge->operatorManager()->copyAttribute(oper->specification()));
    return remOp->ableToOperate();
    }

  virtual OperatorResult operateDelegate(RemoteOperatorPtr localOp)
    {
    printParams(localOp->specification(), "local input");
    OperatorPtr remOp = remoteBridge->op(localOp->name(), remoteModel);
    remOp->setSpecification(remoteBridge->operatorManager()->copyAttribute(localOp->specification()));
    OperatorResult remResult = remOp->operate();
    OperatorResult localResult = this->operatorManager()->copyAttribute(remResult);

    // Kill remote operator and result.
    remOp->bridge()->operatorManager()->removeAttribute(remOp->specification());
    remResult->manager()->removeAttribute(remResult);

    printParams(localResult, "local output");
    return localResult;
    }
};
const char* noFileTypes[] = {
  NULL
};
smtkImplementsModelingKernel(forwarding, noFileTypes, TestForwardingBridge);

class TestForwardingOperator : public Operator
{
public:
  smtkTypeMacro(TestForwardingOperator);
  smtkCreateMacro(TestForwardingOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();


  virtual bool ableToOperate()
    {
    if (!this->ensureSpecification())
      return false;

    if (!this->specification()->isValid())
      return false;

    if (!this->specification()->find("addToCount")->isEnabled())
      {
      return false;
      }

    return true;
    }

protected:
  TestForwardingOperator() { }

  virtual OperatorResult operateInternal()
    {
    printParams(this->specification(), "actual input");

    OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
    this->s_state += this->specification()->findInt("addToCount")->value();
    result->findInt("state")->setValue(this->s_state);

    printParams(result, "actual output");
    return result;
    }

  // Note that any state to be preserved between
  // invocations of the operator must be kept in
  // smtk::model::Manager or class static. This is
  // because the operator is created by the bridge
  // each time it is requested.
  static Integer s_state;
};
smtkImplementsModelOperator(
  TestForwardingOperator, forwarding,
  "forwarding operator", unitForwardingOperator_xml,
  smtk::model::DefaultBridge); //TestForwardingBridge);

Integer TestForwardingOperator::s_state = 1;

void printBridgeOperatorNames(BridgePtr br, const std::string& msg)
{
  StringList opNames = br->operatorNames();
  StringList::const_iterator it;
  std::cout << "Bridge \"" << br->name() << "\" [" << br->className() << ", " << msg << "] operators:\n";
  for (it = opNames.begin(); it != opNames.end(); ++it)
    {
    smtk::model::OperatorPtr op = br->op(*it, smtk::model::ManagerPtr());
    std::cout
      << "  " << *it
      << " [" << op->className() << "]"
      << "\n";
    }
  std::cout << "\n";
}

// Test remote bridging: create 2 model::Manager instances,
// add a native operator to manager A's "native" bridge,
// serialize the bridge session into a DefaultBridge instance
// that backs it into manager B, and invoke the remote version
// of the operator attached to the DefaultBridge on manager B.
// Check that the operation was invoked on manager A and that
// the OperatorResult from both operations are identical (by
// having the native operator cache its result parameters).
int main()
{
  int status = 0;

  try {

    // Create the managers
    smtk::model::Manager::Ptr remoteMgr = smtk::model::Manager::create();
    smtk::model::Manager::Ptr localMgr = smtk::model::Manager::create();

    // The default bridge of the "remote" manager:
    Bridge::Ptr remoteBridge = remoteMgr->bridgeForModel(UUID::null());
    printBridgeOperatorNames(remoteBridge, "remote");

    // Now we want to mirror the remote manager locally.
    // Serialize the "remote" bridge session:
    cJSON* sessJSON = cJSON_CreateObject();
    ExportJSON::forManagerBridgeSession(remoteBridge->sessionId(), sessJSON, remoteMgr);
    // ... and import the session locally to a new bridge object.
    TestForwardingBridge::Ptr localBridge = TestForwardingBridge::create();
    localBridge->remoteBridge = remoteBridge;
    localBridge->remoteModel = remoteMgr;
    test(localBridge->operatorNames().size() == 0, "Forwarding bridge should have no operators by default.");
    printBridgeOperatorNames(localBridge, "local, pre-import");
    ImportJSON::ofRemoteBridgeSession(sessJSON->child, localBridge, localMgr);
    printBridgeOperatorNames(localBridge, "local, post-import");
    test(localBridge->operatorNames().size() == remoteBridge->operatorNames().size(),
      "Forwarding bridge operator count should match remote bridge after import.");

    // Run the local operator.
    // Examine the remote version to verify the operation was forwarded.
    OperatorPtr localOp = localBridge->op("forwarding operator", localMgr);
    test(
      smtk::dynamic_pointer_cast<smtk::model::RemoteOperator>(localOp) ?
      true : false, "Local forwarding operator was not an instance of RemoteOperator.");
    test(
      localOp->className() == "smtk::model::RemoteOperator",
      "Local operator did not return expected className() \"smtk::model::RemoteOperator\"");
    test(
      !localOp->specification(),
      "Local operator had a default specification. Expected a null shared pointer.");
    localOp->ensureSpecification();
    test(
      !!localOp->specification(),
      "Local operator's ensureSpecification did not work.");
    localOp->specification()->findInt("addToCount")->setValue(2);
    test(
      localOp->specification()->findInt("addToCount")->value() == 2,
      "Setting a valid parameter had no effect.");
    OperatorResult localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult->findInt("state")->value() == 3, "Operation should have yielded state == 3.");
    localOp->eraseResult(localResult);

    std::cout << "\n---\n\n";

    // Rerun the local operator.
    localOp->specification()->findInt("addToCount")->setValue(8);
    localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult->findInt("state")->value() == 11, "Operation should have yielded state == 11.");

    // Rerun the local operator but with an "improper" input (disabling a parameter that really
    // shouldn't be optional is a surrogate for situations where an operator may be unable to run)
    // This also tests the setParameterEnabled method.
    test(localOp->specification()->find("addToCount")->isOptional(), "Expected optional \"addToCount\".");

    localOp->specification()->findInt("addToCount")->setIsEnabled(false);
    test(!localOp->specification()->find("addToCount")->isEnabled(), "Did not disable \"addToCount\".");
    localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == UNABLE_TO_OPERATE, "Operator should have been unable to execute.");

    // Test transferring remote dangling entity list to local bridge.
    // (As a precursor to a full implementation of TestForwardingBridge::transcribeInternal)
    UUIDGenerator ugen;
    UUIDs danglers;
    for (int i = 0; i < 8; ++i)
      danglers.insert(ugen.random());
    localBridge->addSomeRemoteDanglers(danglers);
    cJSON* jsonDanglers = cJSON_CreateObject();
    smtk::io::ExportJSON::forDanglingEntities(remoteBridge->sessionId(), jsonDanglers, remoteMgr);
    //std::cout << "\n\n\n" << cJSON_Print(jsonDanglers) << "\n\n\n";
    smtk::io::ImportJSON::ofDanglingEntities(jsonDanglers, localMgr);
    test(localBridge->checkLocalDanglers(danglers, localMgr), "All generated danglers should have been serialized.");

  } catch (const std::string& msg) {
    (void) msg; // Ignore the message; it's already been printed.
    std::cerr << "Exiting...\n";
    status = -1;
  }

  return status;
}
