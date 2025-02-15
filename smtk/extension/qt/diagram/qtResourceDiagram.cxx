//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"

#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramLegend.h"
#include "smtk/extension/qt/diagram/qtDiagramLegendEntry.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"
#include "smtk/extension/qt/diagram/qtGroupingNode.h"
#include "smtk/extension/qt/diagram/qtObjectNodeFactory.h"
#include "smtk/extension/qt/diagram/qtResourceDiagramArc.h"
#include "smtk/extension/qt/diagram/qtResourceDiagramSummary.h"
#include "smtk/extension/qt/qtManager.h"

#include "smtk/view/Information.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/ResourceBase.h"
#include "smtk/graph/RuntimeArcEndpoint.h"

#include "smtk/common/Managers.h"
#include "smtk/common/StringUtil.h"
#include "smtk/common/json/jsonUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/string/json/jsonToken.h"

#include <QParallelAnimationGroup>
#include <QPolygon>
#include <QPropertyAnimation>

#include <cmath>

// MSVC does not provide M_PI for C++. Bleh.
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{
namespace // anonymous
{

// Prune unneeded entries from the inheritance hierarchy to prevent ugly diagrams.
std::vector<smtk::string::Token> pruneHierarchy(
  const std::vector<smtk::string::Token>& hierarchy,
  const std::unordered_set<smtk::string::Token>& exclusions)
{
  std::vector<smtk::string::Token> pruned;
  pruned.reserve(hierarchy.size());
  for (const auto& entry : hierarchy)
  {
    if (exclusions.find(entry) == exclusions.end())
    {
      pruned.push_back(entry);
    }
  }
  return pruned;
}

// From the \a start of a grouping-node's appearance in \a nodeTree,
// how many entries does is span?
std::size_t groupSpan(
  qtResourceDiagram::NodeTree& nodeTree,
  qtResourceDiagram::NodeTree::const_iterator start,
  std::size_t level)
{
  std::size_t dd = 0;
  for (qtBaseNode* node = (*start)[level];
       start != nodeTree.end() && start->size() > level && node == (*start)[level];
       ++start)
  {
    ++dd;
  }
  return dd;
}

void extractRules(
  const smtk::view::Configuration::Component& data,
  const std::string& ruleType,
  qtResourceDiagram::ObjectRules& ruleMap)
{
  for (const auto& filter : data.children())
  {
    std::string resourceType;
    std::string filterText;
    if (filter.name() == ruleType && filter.attribute("Resource", resourceType))
    {
      ruleMap[resourceType].resourceNode = !filter.attributeAsBool("ComponentsOnly");
      if (filter.attribute("Filter", filterText))
      {
        ruleMap[resourceType].componentRules.insert(filterText);
        // std::cout << ruleType << " " << resourceType << " " << filterText << "\n";
        if (!filter.children().empty())
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            ruleType << " entries with a Filter should have no children.");
        }
      }
      else
      {
        for (const auto& rule : filter.children())
        {
          if (rule.name() == "Filter" && !rule.contents().empty())
          {
            ruleMap[resourceType].componentRules.insert(rule.contents());
            // std::cout << ruleType << " " << resourceType << " " << rule.contents() << "\n";
          }
        }
      }
    }
  }
}

bool testMatch(
  const smtk::resource::PersistentObject* obj,
  const qtResourceDiagram::ObjectRules& rules,
  bool accepting)
{
  if (!rules.empty())
  {
    if (const auto* rsrc = dynamic_cast<const smtk::resource::Resource*>(obj))
    {
      for (const auto& entry : rules)
      {
        if (entry.first == "*"_token || rsrc->matchesType(entry.first))
        {
          return entry.second.resourceNode ? accepting : !accepting;
        }
      }
    }
    else if (const auto* comp = dynamic_cast<const smtk::resource::Component*>(obj))
    {
      auto* rsrc = comp->parentResource();
      for (const auto& entry : rules)
      {
        if (entry.first == "*"_token || (rsrc && rsrc->matchesType(entry.first)))
        {
          for (const auto& rule : entry.second.componentRules)
          {
            if (rule == "*"_token || comp->matchesType(rule))
            {
              return accepting;
            }
          }
        }
      }
    }
    return !accepting;
  }
  return true;
}

} // anonymous namespace

bool qtResourceDiagram::VisualComparator::operator()(
  const std::vector<qtBaseNode*>& aa,
  const std::vector<qtBaseNode*>& bb) const
{
  std::size_t nn = aa.size() < bb.size() ? aa.size() : bb.size();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (this->nodeLessThan(aa[ii], bb[ii]))
    {
      return true;
    }
    else if (this->nodeLessThan(bb[ii], aa[ii]))
    {
      return false;
    }
  }
  // To get here, one vector must be longer than the other and both
  // must have entries that compare as identical for the entire length
  // of the shortest vector.
  return nn == aa.size(); // If aa is shorter and matches bb its entire length, it comes first.
}

bool qtResourceDiagram::VisualComparator::nodeLessThan(const qtBaseNode* cc, const qtBaseNode* dd)
  const
{
  // Compare by type and then by name.
  // Fetch the types as string tokens and then look up their order
  // in a map (which could be made part of the diagram configuration
  // if needed).
  static std::unordered_map<smtk::string::Token, int> typeOrder{
    { "smtk::extension::qtGroupingNode"_token, 0 },
    { "smtk::extension::qtResourceNode"_token, 1 },
    { "smtk::extension::qtDefaultTaskNode"_token, 2 },
    { "smtk::extension::qtBaseTaskNode"_token, 3 },
    { "smtk::extension::qtComponentNode"_token, 4 },
    { "smtk::extension::qtBaseObjectNode"_token, 5 },
    { "smtk::extension::qtBaseNode"_token, 7 }
  };
  auto cct = cc->typeToken();
  auto ddt = dd->typeToken();
  if (cct != ddt)
  {
    auto ccoit = typeOrder.find(cct);
    auto ddoit = typeOrder.find(ddt);
    auto cco = ccoit == typeOrder.end() ? 11 : ccoit->second;
    auto ddo = ccoit == typeOrder.end() ? 11 : ddoit->second;
    if (cco != ddo)
    {
      return cco < ddo;
    }
  }
  else if (cct == "smtk::extension::qtGroupingNode"_token)
  {
    const auto* cg = dynamic_cast<const smtk::extension::qtGroupingNode*>(cc);
    const auto* dg = dynamic_cast<const smtk::extension::qtGroupingNode*>(dd);
    if (cg && dg)
    {
      // Both cct and ddt are groups; choose the order for groups.
      static std::unordered_map<smtk::string::Token, int> groupOrder{
        { "smtk::project::Project"_token, 0 },    { "smtk::task::Task"_token, 1 },
        { "smtk::task::Adaptor"_token, 2 },       { "smtk::task::Worklet"_token, 3 },

        { "smtk::attribute::Resource"_token, 4 }, { "smtk::attribute::Attribute"_token, 5 },

        { "smtk::model::Resource"_token, 6 },     { "smtk::model::Entity"_token, 7 },

        { "smtk::markup::Resource"_token, 8 },    { "smtk::markup::Ontology"_token, 9 },
        { "smtk::markup::Label"_token, 10 },      { "smtk::markup::SpatialData"_token, 11 },
        { "smtk::markup::Field"_token, 12 }
      };
      auto cgit = groupOrder.find(cg->groupName());
      auto dgit = groupOrder.find(dg->groupName());
      auto cgo = (cgit != groupOrder.end() ? cgit->second : 100);
      auto dgo = (dgit != groupOrder.end() ? dgit->second : 100);
      if (cgo != dgo)
      {
        return cgo < dgo;
      }
    }
  }
  // The types compare as identical. Examine the names.
  return smtk::common::StringUtil::mixedAlphanumericComparator(cc->name(), dd->name());
}

qtResourceDiagram::qtResourceDiagram(
  const smtk::view::Information& info,
  const smtk::view::Configuration::Component& config,
  qtDiagram* parent)
  : Superclass(info, config, parent)
{
  m_summarizer = new qtResourceDiagramSummary(this);
  m_summarizer->setObjectName("SummaryInfo");
  parent->sidebar()->layout()->addWidget(m_summarizer);

  // Grab our configuration
  double value;
  if (config.attributeAsDouble("Beta", value))
  {
    this->setBeta(value);
  }
  if (config.attributeAsDouble("NodeSpacing", value))
  {
    this->setNodeSpacing(value);
  }
  if (config.attributeAsDouble("ShortArcOpacity", value))
  {
    this->setShortArcOpacity(value);
  }
  if (config.attributeAsDouble("LongArcOpacityAdjustment", value))
  {
    this->setLongArcOpacityAdjustment(value);
  }
  int child;
  if ((child = config.findChild("ClassExclusions")) >= 0)
  {
    const auto& classExclusions = config.child(child);
    for (const auto& exclusion : classExclusions.children())
    {
      if (exclusion.name() == "Exclude" && !exclusion.contents().empty())
      {
        m_classExclusions.insert(exclusion.contents());
      }
    }
  }
  else
  {
    m_classExclusions = { { "smtk::resource::Resource"_token,
                            "smtk::resource::Component"_token,
                            "smtk::geometry::Resource"_token,
                            "smtk::graph::ResourceBase"_token,
                            "smtk::graph::Component"_token } };
  }
  if ((child = config.findChild("ObjectFilters")) >= 0)
  {
    const auto& objFilters = config.child(child);
    extractRules(objFilters, "Accepts", m_acceptsRules);
    extractRules(objFilters, "Rejects", m_rejectsRules);
  }
}

void qtResourceDiagram::removeFromLayout(
  const std::unordered_set<smtk::resource::PersistentObject*>& expunged)
{
  NodeTree::iterator next = m_diagramNodes.begin();
  for (auto it = next; it != m_diagramNodes.end(); /* do nothing */)
  {
    ++next;
    const auto& treeEntry(*it);
    if (!treeEntry.empty())
    {
      if (auto* objNode = dynamic_cast<qtBaseObjectNode*>(treeEntry.back()))
      {
        if (expunged.find(objNode->object()) != expunged.end())
        {
          // Remove from the layout.
          m_diagramNodes.erase(it);
        }
      }
    }
    it = next;
  }
}

template<bool HandleReparentedObjects>
bool qtResourceDiagram::updateParentArc(
  smtk::resource::PersistentObject* object,
  ArcLegendEntries& registeredArcTypes)
{
  bool didModify = false;
  // Make components point to their owning resource.
  if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
  {
    auto* cnode = m_diagram->findNode(comp->id());
    if (!cnode)
    {
      return didModify;
    }
    auto* rsrc = comp->parentResource();
    auto* rnode = rsrc ? m_diagram->findNode(rsrc->id()) : nullptr;
    if (!rnode)
    {
      return didModify;
    }
    const auto* arcMapSet = m_diagram->arcsFromNode(cnode);
    if (arcMapSet)
    {
      if (HandleReparentedObjects)
      {
        // First, see if cnode already has parent arc(s).
        // If it has any arc that arc doesn't lead to rnode,
        // then delete it; we'll create a new arc below.
        bool haveCorrectParent = false;
        std::unordered_set<qtBaseArc*> arcsToErase;
        for (const auto& successorEntry : *arcMapSet)
        {
          auto it = successorEntry.second.find("parent"_token);
          if (it != successorEntry.second.end())
          {
            if (successorEntry.first == rnode)
            {
              haveCorrectParent = true;
            }
            else
            {
              // Delete all these parent arcs; they are invalid.
              arcsToErase.insert(it->second.begin(), it->second.end());
            }
          }
        }
        if (!arcsToErase.empty())
        {
          didModify = true;
          for (auto* arc : arcsToErase)
          {
            m_diagramArcs.erase(arc);
            this->diagram()->removeArc(arc);
          }
        }
        if (haveCorrectParent)
        {
          return didModify;
        }
      }
      else
      {
        // Just verify that no parent arc exists.
        auto parentIt = arcMapSet->find(rnode);
        if (parentIt != arcMapSet->end())
        {
          auto typeIt = parentIt->second.find("parent"_token);
          if (typeIt != parentIt->second.end())
          {
            // We already have an arc of type "parent" between cnode and rnode.
            return didModify;
          }
        }
      }
    }
    auto* arc = new qtResourceDiagramArc(this, cnode, rnode, "parent"_token);
    m_diagram->addArc(arc); // Index the arc.
    m_diagramArcs.insert(arc);
    if (registeredArcTypes.find("parent"_token) == registeredArcTypes.end())
    {
      auto* entry = new qtDiagramLegendEntry("arc"_token, "parent"_token, this);
      if (m_diagram->legend()->addEntry(entry))
      {
        registeredArcTypes["parent"_token] = entry;
      }
    }
    didModify = true;
  }
  return didModify;
}

template<bool RemoveUnusedArcs>
bool qtResourceDiagram::updateGraphArcs(
  smtk::resource::PersistentObject* object,
  ArcLegendEntries& registeredArcTypes)
{
  bool didModify = false;
  // Also, if component is a graph component, iterate over arcs and add them.
  if (auto* node = dynamic_cast<smtk::graph::Component*>(object))
  {
    auto* predecessor = m_diagram->findNode(node->id());
    if (!predecessor)
    {
      return false;
    }
    const auto* arcMapSet = m_diagram->arcsFromNode(predecessor);
    const auto& graphArcTypes = node->parentResourceAs<smtk::graph::ResourceBase>()->arcTypes();

    if (arcMapSet && RemoveUnusedArcs)
    {
      std::unordered_set<qtBaseArc*> arcsToErase;
      // Iterate over arc items and remove any that do not exist
      // on the underlying component any longer.
      for (const auto& successorEntry : *arcMapSet)
      {
        auto* successor = dynamic_cast<qtBaseObjectNode*>(successorEntry.first);
        if (!successor)
        {
          continue; // The successor is not a persistent object.
        }
        auto* toNode = dynamic_cast<smtk::graph::Component*>(successor->object());
        if (!toNode)
        {
          continue; // The successor is not a graph-resource node.
        }
        for (const auto& arcTypeEntry : successorEntry.second)
        {
          if (graphArcTypes.find(arcTypeEntry.first) == graphArcTypes.end())
          {
            // These arcs are not graph-resource arcs. Skip them.
            continue;
          }
          if (!node->outgoing(arcTypeEntry.first).contains(toNode))
          {
            arcsToErase.insert(arcTypeEntry.second.begin(), arcTypeEntry.second.end());
          }
        }
      }
      // If there are arcs to remove, remove them.
      for (auto* arc : arcsToErase)
      {
        // De-index arc
        m_diagramArcs.erase(arc);
        // Remove the arc from the scene and destroy it.
        m_diagram->removeArc(arc);
        didModify = true;
      }
    }

    const auto& arcTypes = node->parentResourceAs<smtk::graph::ResourceBase>()->arcTypes();
    // We already have predecessor. Just find other nodes from outgoing arcs
    for (const auto& arcType : arcTypes)
    {
      node->outgoing(arcType).visit([&](const smtk::graph::Component* nodeB) {
        auto* successor = m_diagram->findNode(nodeB->id());
        if (!successor)
        {
          return;
        }
        if (arcMapSet)
        {
          auto successorIt = arcMapSet->find(successor);
          if (successorIt != arcMapSet->end())
          {
            auto typeIt = successorIt->second.find(arcType);
            if (typeIt != successorIt->second.end())
            {
              if (!typeIt->second.empty())
              {
                // An arc of this type already exists between the two nodes.
                // NB: We do not model multi-arcs at this point.
                return;
              }
            }
          }
        }
        // No existing arc; add one.
        // Register the arc type if needed.
        if (registeredArcTypes.find(arcType) == registeredArcTypes.end())
        {
          qtDiagramLegendEntry* entry{ nullptr };
          auto rsrcMgr = m_diagram->information()
                           .get<smtk::common::Managers::Ptr>()
                           ->get<smtk::resource::Manager::Ptr>();
          if (rsrcMgr)
          {
            const auto& typeLabels = rsrcMgr->objectTypeLabels();
            auto labelIt = typeLabels.find(arcType);
            if (labelIt != typeLabels.end())
            {
              entry = new qtDiagramLegendEntry("arc"_token, arcType, this, labelIt->second);
            }
          }
          if (!entry)
          {
            entry = new qtDiagramLegendEntry("arc"_token, arcType, this);
          }
          if (m_diagram->legend()->addEntry(entry))
          {
            registeredArcTypes[arcType] = entry;
          }
        }
        // std::cout << "Add arc " << node->name() << "→" << nodeB->name() << " (" << arcType.data() << ")\n";
        auto* arc = new qtResourceDiagramArc(this, predecessor, successor, arcType);
        m_diagram->addArc(arc); // Index the arc.
        m_diagramArcs.insert(arc);
        didModify = true;
      });
    }
  }
  return didModify;
}

void qtResourceDiagram::updateSceneNodes(
  std::unordered_set<smtk::resource::PersistentObject*>& created,
  std::unordered_set<smtk::resource::PersistentObject*>& modified,
  std::unordered_set<smtk::resource::PersistentObject*>& expunged,
  const smtk::operation::Operation& operation,
  const smtk::operation::Operation::Result& result)
{
  (void)modified;
  (void)expunged;
  (void)operation;
  (void)result;

  auto mgr = m_diagram->information()
               .get<smtk::common::Managers::Ptr>()
               ->get<smtk::extension::qtManager::Ptr>();
  auto rsrcMgr = m_diagram->information()
                   .get<smtk::common::Managers::Ptr>()
                   ->get<smtk::resource::Manager::Ptr>();
  auto& objectNodeFactory = mgr->objectNodeFactory();
  m_sceneModified = false;
  for (auto* object : created)
  {
    if (!this->acceptObject(object))
    {
      continue;
    }
    std::vector<qtBaseNode*> treeEntry;
    qtBaseNode* node = m_diagram->findNode(object->id());
    if (!node)
    {
      QGraphicsItem* parent = nullptr; // TODO: initialize to m_groupingNodes["root"_token]?
      // We need to create this object's parent-nodes if they do not exist.
      auto parents = pruneHierarchy(object->classHierarchy(), m_classExclusions);
      for (auto it = parents.rbegin(); it != parents.rend(); ++it)
      {
        auto git = m_groupItemData.find(it->data());
        auto nit = m_groupingNodes.find(*it);
        if (nit == m_groupingNodes.end())
        {
          m_sceneModified = true;
          qtGroupingNode* gnode;
          if (git == m_groupItemData.end())
          {
            gnode = new qtGroupingNode(this, *it, parent);
          }
          else
          {
            gnode = new qtGroupingNode(
              (*git)["id"].get<smtk::common::UUID>(),
              (*git)["pt"].get<std::array<double, 2>>(),
              this,
              *it,
              parent);
          }
          node = gnode;
          // See if there is a label for the object type.
          if (rsrcMgr)
          {
            const auto& typeLabels = rsrcMgr->objectTypeLabels();
            auto labelIt = typeLabels.find(*it);
            if (labelIt != typeLabels.end())
            {
              gnode->setLabel(labelIt->second);
            }
          }
          QObject::connect(
            node,
            &qtBaseNode::nodeMoved,
            this,
            &qtResourceDiagram::updateArcsOfSendingNodeRecursive);
          m_diagram->addNode(node);
          treeEntry.push_back(node);
          m_groupingNodes[*it] = node;
          parent = node;
        }
        else
        {
          parent = nit->second;
          treeEntry.push_back(nit->second);
          if (git != m_groupItemData.end())
          {
            // Update the location of the node only; the UUID should not vary.
            auto xy = (*git)["pt"].get<std::array<double, 2>>();
            nit->second->setPos(xy[0], xy[1]);
          }
        }
      }

      // Determine the type of this object's node.
      // TODO: Fetch view style and choose item type from it.
      // One idea: iterate over object->classHierarchy() and choose first type-name that has
      //           a style associated with it; use that as the node style.
      if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
      {
        (void)comp;
        auto* cnode = objectNodeFactory
                        .createFromName(
                          "smtk::extension::qtComponentNode",
                          static_cast<qtDiagramGenerator*>(this),
                          object,
                          parent)
                        .release();
        node = cnode;
      }
      else if (auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object))
      {
        (void)rsrc;
        node = objectNodeFactory
                 .createFromName(
                   "smtk::extension::qtResourceNode",
                   static_cast<qtDiagramGenerator*>(this),
                   object,
                   parent)
                 .release();
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Unhandled object type " << object->typeName() << ".");
      }
      // std::cout << "  Add " << node << " obj " << object->typeName() << "\n";
      if (node)
      {
        m_addedToScene = true;
        m_sceneModified = true;
        treeEntry.push_back(node);
        QObject::connect(
          node,
          &qtBaseNode::nodeMoved,
          this,
          &qtDiagramGenerator::updateArcsOfSendingNodeRecursive);
        m_diagram->addNode(node);
        m_diagramNodes.insert(treeEntry);
      }
    }
  }
}

void qtResourceDiagram::updateSceneArcs(
  std::unordered_set<smtk::resource::PersistentObject*>& created,
  std::unordered_set<smtk::resource::PersistentObject*>& modified,
  std::unordered_set<smtk::resource::PersistentObject*>& expunged,
  const smtk::operation::Operation& operation,
  const smtk::operation::Operation::Result& result)
{
  (void)operation;
  (void)result;

  auto mgr = m_diagram->information()
               .get<smtk::common::Managers::Ptr>()
               ->get<smtk::extension::qtManager::Ptr>();
  auto rsrcMgr = m_diagram->information()
                   .get<smtk::common::Managers::Ptr>()
                   ->get<smtk::resource::Manager::Ptr>();
  bool didModify = m_sceneModified;

  // All the nodes exist; add arcs as needed between them.
  //
  // NB: We do not check acceptObject(object) inside this loop since
  // we wish to create arcs to nodes created by other diagram generators
  // (e.g., tasks in a task diagram).

  // Don't add arc types previously registered:
  ArcLegendEntries registeredArcTypes = m_diagram->legend()->typesInGroup("arc"_token);

  // Handle arcs attached to objects that have just been created.
  // This will only create arcs, not remove or redirect existing arcs.
  for (auto* object : created)
  {
    didModify |= this->updateParentArc<false>(object, registeredArcTypes);
    didModify |= this->updateGraphArcs<false>(object, registeredArcTypes);
  }

  // Handle arcs connecting existing (modified) components.
  // This may create, destroy, or redirect arcs.
  for (auto* object : modified)
  {
    didModify |= this->updateParentArc<true>(object, registeredArcTypes);
    didModify |= this->updateGraphArcs<true>(object, registeredArcTypes);
  }

  // Remove entries from m_diagramNodes, m_diagramArcs from the
  // layout before removing them from the scene. This will
  // also remove unused grouping nodes.
  this->removeFromLayout(expunged);
  // Now remove expunged nodes from the scene.
  for (const auto& object : expunged)
  {
    if (!acceptObject(object))
    {
      continue;
    } // Do not remove nodes we do not own.
    if (auto* node = m_diagram->findNode(object->id()))
    {
      m_diagram->removeNode(node); // Also removes arcs.
      didModify = true;
    }
  }

  // Compute new positions for every node except the root node
  // (which is the qtGroupingNode for "smtk::resource::PersistentObject").
  // This also hides any referenced grouping nodes.
  // The boolean parameter instructs generateLayout whether to force
  // the diagram to zoom out so the entire layout is in view (which should
  // only happen when components are added to (not removed from or modified)
  // the diagram.
  this->generateLayout(didModify && m_addedToScene);

  // We should now be able to clear the JSON data provided by the operation.
  // Do this so that subsequent operations which do not provide JSON do not
  // end up using stale data to reset node locations.
  m_groupItemData.clear();
}

qtBaseNode* qtResourceDiagram::root() const
{
  auto it = m_groupingNodes.find("smtk::resource::PersistentObject"_token);
  if (it == m_groupingNodes.end())
  {
    return nullptr;
  }
  return it->second;
}

bool qtResourceDiagram::addConfiguration(nlohmann::json& config) const
{
  nlohmann::json groupingNodes;
  for (const auto& entry : m_groupingNodes)
  {
    nlohmann::json nodeRecord = { { "id", entry.second->nodeId() },
                                  { "pt", { entry.second->pos().x(), entry.second->pos().y() } } };
    if (auto* nodeParent = dynamic_cast<qtBaseNode*>(entry.second->parentItem()))
    {
      nodeRecord["parentId"] = nodeParent->nodeId();
      if (auto* groupParent = dynamic_cast<qtGroupingNode*>(nodeParent))
      {
        nodeRecord["parentGroup"] = groupParent->groupName().data();
      }
    }
    groupingNodes[entry.first.data()] = nodeRecord;
  }
  if (!groupingNodes.empty())
  {
    config["grouping-nodes"] = groupingNodes;
  }
  return true;
}

bool qtResourceDiagram::configure(const nlohmann::json& config)
{
  auto git = config.find("grouping-nodes");
  if (git != config.end())
  {
    m_groupItemData = *git;
  }
  return true;
}

bool qtResourceDiagram::setShortArcOpacity(double value)
{
  if (m_shortArcOpacity == value || value < 0.0 || value > 1.0)
  {
    return false;
  }
  m_shortArcOpacity = value;
  return true;
}

bool qtResourceDiagram::setLongArcOpacityAdjustment(double value)
{
  if (m_longArcOpacityAdjustment == value || value < 0.0 || value > 1.0)
  {
    return false;
  }
  m_longArcOpacityAdjustment = value;
  return true;
}

bool qtResourceDiagram::setBeta(double value)
{
  if (m_beta == value || value < 0.0 || value > 1.0)
  {
    return false;
  }
  m_beta = value;
  return true;
}

bool qtResourceDiagram::setNodeSpacing(double value)
{
  if (m_nodeSpacing == value || value <= 0.0)
  {
    return false;
  }
  m_nodeSpacing = value;
  return true;
}

void qtResourceDiagram::generateLayout(bool zoomToLayout)
{
  // I. Create/update a linear ordering of all leaf nodes, sorting
  //    first by type-hierarchy and then by name. (This is m_diagramNodes.)
  // II. Place grouping nodes at their appropriate angle (theta_i), averaged
  //     across all the children's positions.
  // III. Lay nodes out along a circle (uniform spacing? based on node width?)
  //      This is just a simple formula applied via iteration of the set from step I.
  //      Accumulate offsets into set of the start/end theta of each grouping node.
  // IV. Lay grouping nodes out by by computing the average theta and the radial
  //     position based on the length of its position is the vector compared to the
  //     maximum vector length.
  // V. Create a QParallelAnimationGroup and call addAnimation() on each node's
  //    positition/rotation difference relative to the node's current location/rotation.
  // VI. Call QParallelAnimationGroup::start(QAbstractAnimation::DeleteWhenStopped);

  auto* transition = new QParallelAnimationGroup;

  // The circumference of the diagram scales with the number of leaf nodes.
  // The arc length of the circle (2*pi*radius) must match the sum of the leaf-node heights
  // since the nodes will be rotated.
  double radius = m_diagramNodes.size() * 15.0 / 2.0 * m_nodeSpacing; // 15 == height of node.
  // TODO: accumulate leaf-node heights in radius calculation?
  double theta = 0.0;
  double deltaTheta = 360.0 / m_diagramNodes.size(); // degrees per node
  // Position+rotation for all nodes in scene coords (not local):
  std::unordered_map<qtBaseNode*, std::pair<QPointF, qreal>> placements;
  QPointF origin(0, 0);
  if (this->root())
  {
    // Force the root node to stay in its current location.
    placements[this->root()] = std::make_pair(this->root()->pos(), qreal(0.0));
    origin = this->root()->pos();
  }
  std::unordered_set<qtBaseNode*> visited;
  for (auto it = m_diagramNodes.begin(); it != m_diagramNodes.end(); ++it)
  {
    const auto& treeEntry(*it);
    if (treeEntry.empty())
    {
      continue;
    } // TODO: Warn.

    double level = 0;
    double levelDelta = radius / (treeEntry.size() - 1);
    for (auto rit = treeEntry.begin(); rit != treeEntry.end(); ++rit)
    {
      qtBaseNode* node = *rit;
      visited.insert(node);
      if (placements.find(node) == placements.end())
      {
        // Find the spanned angle for all nodes which this node groups:
        auto span = groupSpan(m_diagramNodes, it, rit - treeEntry.begin());
        double angRad = (theta + (span / 2.0 * deltaTheta)) * M_PI / 180.0;
        // NB: We use -sin(angRad) here because the scene's y-axis is downward-pointing and we
        //     want the diagram to be laid out in a counterclockwise direction.
        QPointF nodeInScene(
          origin.x() + level * std::cos(angRad), origin.y() - level * std::sin(angRad));
        // Now because we are flipping about the y axis, the nodes must be rotated by a negative angle:
        placements[node] =
          std::make_pair(nodeInScene, static_cast<qreal>(-theta - span / 2.0 * deltaTheta));
      }
      level += levelDelta;
    }
    theta += deltaTheta;
  }

  // Hide unreferenced grouping nodes.
  for (const auto& entry : m_groupingNodes)
  {
    if (visited.find(entry.second) == visited.end())
    {
      entry.second->hide();
    }
    else
    {
      entry.second->show();
    }
  }

  // Create animations for all placements.
  for (const auto& placement : placements)
  {
    auto* node = placement.first;
    QPointF spot = placement.second.first;
    qreal rot = placement.second.second;
    if (node->parentItem())
    {
      if (auto* parentNode = dynamic_cast<qtBaseNode*>(node->parentItem()))
      {
        // Nodes with parents inherit a transform with a rotation;
        // move the scene coordinates into the parent's coordinates.
        QPointF xx = spot - placements[parentNode].first;
        QTransform m;
        m.rotate(-placements[parentNode].second);
        spot = xx * m;
        rot -= placements[parentNode].second;
      }
    }

    auto* posTween = new QPropertyAnimation(node, "pos", transition);
    posTween->setStartValue(node->pos());
    posTween->setEndValue(spot);

    auto* rotTween = new QPropertyAnimation(node, "rotation", transition);
    rotTween->setStartValue(node->rotation());
    rotTween->setEndValue(rot);

    posTween->setDuration(250);
    posTween->setEasingCurve(QEasingCurve::OutExpo);
    rotTween->setDuration(250);
    rotTween->setEasingCurve(QEasingCurve::InOutExpo);
  }

  QPointF sceneDelta(radius + /* node height */ 15, radius + /* node height */ 15);
  QRectF futureSceneRect(origin - sceneDelta, origin + sceneDelta);
  // Hint to the diagram that the entirety of \a futureSceneRect should be in
  // view by the time our transition completes. The view should not zoom it,
  // but should zoom out as needed to keep both futureSceneRect and the current
  // viewport window in the scene.
  //
  // If another generator requests something different, the higher priority wins.
  // If multiple generators provide rectangles to be included, they are all
  // unioned (regardless of priority).
  if (zoomToLayout)
  {
    this->diagram()->includeInView(futureSceneRect, /* priority */ 0);
  }
  transition->start(QAbstractAnimation::DeleteWhenStopped);
}

bool qtResourceDiagram::acceptObject(const smtk::resource::PersistentObject* obj) const
{
  if (!obj)
  {
    return false;
  }

  bool ok = testMatch(obj, m_acceptsRules, true) && testMatch(obj, m_rejectsRules, false);

  return ok;
}

} // namespace extension
} // namespace smtk
