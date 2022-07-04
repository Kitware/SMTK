# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================

"""A utility class for generating and populating attribute resources.

Basic yml format:

# Create attribute (default "action: create")
- type: string  # must be instanced unless name provided
    name: string  # optional but when included has priority over type
    items:
    # Option 1: shorthand for setting value
    - name: float | int | str | list

    # Option 2: for more options
    - name: string
        value: float | int | str | list

# Use "edit" to find and modify existing attribute
- action: edit
  type: string
  name: string  # optional
"""

import smtk
import smtk.attribute
import smtk.model
import smtk.resource
import smtk.view


class AttributeBuilder:
    """A utility class for generating and populating attribute resources.

    Inputs a formatted object specifying the contents to generate.
    """

    def __init__(self, verbose=False):
        """"""
        self.att_resource = None
        self.instanced_types = set()  # to prevent duplicating instances
        self.model_resource = None
        self.pedigree_lookup = dict()  # <face/volume.pedigree-id, smtk.model.Entity>

        # Dictionary for resources for associations and reference items
        # New API and syntax initially for build_attribute() method
        self.resource_dict = dict()
        self.verbose = verbose

    def build_attributes(self, att_resource, spec, model_resource=None, verbose=None):
        """Builds attributes based on the view configuration and input specification.

        Args:
            att_resource: smtk::attribute::Resource
            spec: dict with attribute specfications
            model_resource: smtk::attribute::Model
            verbose: boolean to enable print statements

        The logic initializes the attribute resource
        """
        if verbose is not None:
            self.verbose = verbose

        self.att_resource = att_resource
        self.model_resource = model_resource
        atts = att_resource.attributes()
        self._post_message('Initial attribute count: {}'.format(len(atts)))

        if model_resource is not None:
            self._build_pedigree_lookup(model_resource)

        # Create instanced attributes first
        self.instanced_types.clear()
        before_set = set(self.att_resource.attributes())
        self.create_instanced_atts(self.att_resource)
        instanced_set = set(self.att_resource.attributes()) - before_set
        for att in instanced_set:
            self.instanced_types.add(att.type())

        # Create/edit attributes per the specification
        self.process_spec(spec)

    def build_attribute(self,
                        att: smtk.attribute.Attribute,
                        spec: dict,
                        resource_dict: [str, smtk.resource.Resource] = None) -> None:
        """Updates attribute contents based on input spec.

        Input resource_dict, if provided, is used for all reference lookup.
        Otherwise original/legacy logic is used.
        """
        if resource_dict is not None:
            self.resource_dict = resource_dict
        self._edit_attribute(att, spec)

    def process_spec(self, spec):
        """Creates and populates attributes based on the input specification.

        This method is called by build_attributes(). It can also be called directly
        to process additional specifications after the build_attributes() method has
        been called with an initial spec.
        """
        # Traverse the elements in the input spec
        assert isinstance(
            spec, list), 'top-level specification be a list not {}'.format(type(spec))
        for element in spec:
            assert isinstance(
                element, dict), 'top-level element must be a dict not {}'.format(type(element))

            # default action is to create attribute
            action = element.get('action', 'create')
            if action == 'create':
                att = self._create_attribute_from_spec(element)
                assert att is not None, 'failed to create attribute'
                self._edit_attribute(att, element)

            elif action == 'edit':
                att = self._find_attribute(element)
                assert att is not None, 'failed to find attribute for spec {}'.format(
                    element)
                self._post_message(
                    'Editing attribute \"{}\"'.format(att.name()))
                self._edit_attribute(att, element)
            else:
                raise RuntimeError('Unrecognized action \"{}\"'.format(action))

        att_count_end = len(self.att_resource.attributes())
        self._post_message('Final attribute count: {}'.format(att_count_end))

    def get_analysis_att(self, att_resource, view=None):
        """Returns analysis attribute, creating it if needed.

        Returns:
            smtk.attribute.Attribute, or None if not specified in the resource
        """
        # Requires Analysis view
        if view is None:
            view = att_resource.findViewByType('Analysis')
        if view is None:
            return None

        # Find by name
        view_atts = view.details().attributes()
        att_name = view_atts.get('AnalysisAttributeName')
        att = att_resource.findAttribute(att_name)
        if att is not None:
            return att

        # Get analysis definition and attribute
        defn_type = view_atts.get('AnalysisAttributeType')
        defn = att_resource.findDefinition(defn_type)
        if defn is None:
            defn = att_resource.analyses().buildAnalysesDefinition(att_resource, defn_type)
        att = att_resource.createAttribute(att_name, defn)
        return att

    def create_instanced_atts(self, att_resource):
        """Instantiates all attributes referenced in Instanced views.

        Traverses from top-level view to find all instanced views that
        can potentially be displayed, and creates all instanced attributes.
        """
        top_view = att_resource.findTopLevelView()
        if top_view is None:
            print('WARNING: Attribute resource has no top-level view')
            return

        self._post_message('Generating instanced attributes')
        self.att_resource = att_resource
        self._recursive_create_instanced_atts(att_resource, top_view.details())
        instanced_count = len(att_resource.attributes())
        self._post_message(
            'After creating instanced attributes, count: {}'.format(instanced_count))

    def _associate_attribute(self, att, spec):
        """"""
        assert isinstance(spec, list), 'association spec is not a list'

        # New syntax
        if len(spec) and (isinstance(spec[0], dict)) and 'resource' in spec[0]:
            ref_item = att.associations()
            self._set_reference_item(ref_item, spec)
            return

        # Old syntax
        for element in spec:
            assert len(element.keys()
                       ) == 1, 'association element not a single key/value'
            key, value = element.popitem()
            # association can be single or a list; convert single value to a list
            ref_list = value if isinstance(value, list) else [value]
            if key == 'attribute':
                for ref_name in ref_list:
                    ref_att = self.att_resource.findAttribute(ref_name)
                    assert ref_att is not None, 'attribute with name {} not found'.format(
                        ref_name)
                    assert att.associate(ref_att), \
                        'failed to associate attribute {} to {}'.format(
                            ref_name, att.name())
            elif key in ['face', 'volume']:
                for ref_id in ref_list:
                    entity = self._lookup_entity(key, ref_id)
                    assert att.associate(entity), \
                        'failed to associate entity {} to attribute {}'.format(
                            entity.name(), att.name())
            else:
                raise RuntimeError(
                    'Unrecognized association type ({})'.format(key))

    def _build_pedigree_lookup(self, model_resource):
        """Buils a dictionary mapping pedigree id to model entity."""
        self.pedigree_lookup.clear()
        type_info = [('face', smtk.model.FACE), ('volume', smtk.model.VOLUME)]
        for t in type_info:
            prefix, ent_type = t
            uuid_list = model_resource.entitiesMatchingFlags(ent_type, True)
            for uuid in uuid_list:
                if not model_resource.hasIntegerProperty(uuid, 'pedigree id'):
                    continue
                prop_list = model_resource.integerProperty(uuid, 'pedigree id')
                if not prop_list:
                    continue
                ped_id = prop_list[0]
                key = '{}.{}'.format(prefix, ped_id)
                ent = model_resource.findEntity(uuid)
                self.pedigree_lookup[key] = ent

    def _create_attribute(self, att_type, att_name=None, exists_ok=False):
        """"""
        # Check if name already used
        if att_name is not None:
            att = self.att_resource.findAttribute(att_name)
            if att is not None:
                if exists_ok:
                    message = 'Return existing attribute type \"{}\", name \"{}\"'.format(
                        att_type, att_name)
                    self._post_message(message)
                    return att
                else:
                    assert False, 'attribute name {} already used'.format(
                        att_name)

        self._post_message(
            'Create attribute type \"{}\", name \"{}\"'.format(att_type, att_name))
        defn = self.att_resource.findDefinition(att_type)
        assert defn is not None, 'missing att definition {}'.format(att_type)

        if att_name is None:
            att = self.att_resource.createAttribute(defn)
        else:
            att = self.att_resource.createAttribute(att_name, defn)

        assert att is not None, 'failed to create attribute type {}'.format(
            att_type)
        return att

    def _create_attribute_from_spec(self, spec):
        """"""
        att_type = spec.get('type')
        assert att_type is not None, 'missing attribute \"type\" specification'
        assert att_type not in self.instanced_types, \
            'cannot create attribute type \"{}\" because it is instanced'.format(att_type) + \
            '; did you forget to use \"action: edit\"?'

        att_name = spec.get('name')
        return self._create_attribute(att_type, att_name)

    def _create_instanced_view_atts(self, att_resource, view):
        """Create attributes specified in instanced view.

        """
        comp = view.details()
        atts_comp = comp.child(0)
        for i in range(atts_comp.numberOfChildren()):
            att_comp = atts_comp.child(i)
            att_name = att_comp.attributes().get('Name')
            att_type = att_comp.attributes().get('Type')
            self._create_attribute(att_type, att_name, exists_ok=True)

    def _create_selector_view_att(self, att_resource, view):
        """Create attribute specified in selector view."""
        comp = view.details()
        att_name = comp.attributes().get('SelectorName')
        att_type = comp.attributes().get('SelectorType')
        self._create_attribute(att_type, att_name)

    def _edit_attribute(self, att, spec):
        """Updates attribute as specified.

        Args:
            att: smtk.attribute.Attribute
            spec: dictionary specifying contents
        """
        for key, value in spec.items():
            if key in ['action', 'type', 'name']:
                continue
            elif key == 'items':
                self._edit_items(att, value)
            elif key in ['associate', 'associations']:
                self._associate_attribute(att, value)
            else:
                raise RuntimeError('Unrecognized spec key \"{}\"'.format(key))

    def _edit_items(self, parent, spec, index=None):
        """Updates items as specified.

        Args:
            parent: Attribute or GroupItem or ValueItem
            spec: list of item specifications
            index: specifies one subgroup within a group item
        """
        assert isinstance(
            spec, list), 'items spec must be a list, not {}'.format(type(spec))
        for element in spec:
            assert isinstance(
                element, dict), 'item element spec must be a dict, not {}'.format(type(element))
            expression = None
            value = None
            count = None
            # Checkfor for name/value shortcut
            if len(element.keys()) == 1:
                name, value = element.popitem()
                # print('name', name, 'value', value, type(value))
            elif 'path' in element:
                path = element.get('path')
                item = parent.itemAtPath(path)
                assert item is not None, 'parent {} has no item with path \"{}\"'.format(
                    parent.name(), path)
                value = element.get('value')
                expression = element.get('expression')
                count = element.get('count')
            else:
                name = element.get('name')
                value = element.get('value')
                expression = element.get('expression')
                count = element.get('count')
                assert name is not None, 'no name found for element {}'.format(
                    element)
                assert isinstance(
                    name, str), 'name element must be a string, not {}'.format(type(name))

                # Get the item
                if index is not None:
                    assert parent.type() == smtk.attribute.Item.GroupType, 'index only valid for GroupItem'
                    item = parent.find(
                        index, name, smtk.attribute.SearchStyle.IMMEDIATE)
                elif hasattr(parent, 'findChild'):  # value item
                    item = parent.findChild(
                        name, smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
                elif hasattr(parent, 'find'):  # attribute, group item, reference item
                    item = parent.find(
                        name, smtk.attribute.SearchStyle.IMMEDIATE)
                    if item is None:
                        item = parent.find(
                            name, smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
                else:
                    msg = 'item {} has no find() or findChild() method', format(item.name())
                    raise RuntimeError(msg)

                assert item is not None, 'parent {} has no item with name \"{}\"'.format(
                    parent.name(), name)

            # Set expression or value (expression takes precedence)
            if expression is not None:
                self._set_expression(item, expression)
            elif count is not None and hasattr(item, 'setNumberOfGroups'):
                # we could infer the number of groups, but this is very complex with paths
                assert item.setNumberOfGroups(count), \
                    f'failed to set item {item.name()} number of groups to {count}'
            elif value is not None and hasattr(item, 'setValue'):
                # Apply value if specified
                if self._is_reference_item(item):
                    self._set_reference_item(item, value)
                elif isinstance(value, (float, int, str)):
                    item.setValue(value)
                elif isinstance(value, list):
                    assert item.setNumberOfValues(len(value)), \
                        f'failed to set item {item.name()} number of values to {len(value)}'
                    for i in range(len(value)):
                        if value[i] is None:
                            item.unset(i)
                        else:
                            item.setValue(i, value[i])
                else:
                    raise RuntimeError(
                        'Unrecognized value type {}'.format(type(value)))

            # Check for enabled flag
            enable_key = element.get('enable')
            if enable_key is None:
                enable_key = element.get('enabled')  # alias
            if enable_key is not None:
                enable_state = bool(enable_key)
                item.setIsEnabled(enable_state)

            # Check for children item spec
            for key in ['items', 'children']:
                children_spec = element.get(key)
                if children_spec:
                    self._edit_items(item, children_spec)

            # Check for GroupItem subgroups
            if item.type() == smtk.attribute.Item.GroupType:
                subgroup_spec = element.get('subgroups')
                if subgroup_spec is not None:
                    self._edit_subgroups(item, subgroup_spec)

    def _edit_subgroups(self, group_item, spec):
        """Processes the subgroups keyword."""
        assert isinstance(spec, list), \
            'subgroups must be a list, not {}'.format(type(spec))
        num_subgroups = len(spec)
        assert group_item.setNumberOfGroups(num_subgroups), \
            f'failed to set group item {group_item.name()} number of groups to {len(num_subgroups)}'
        for i in range(num_subgroups):
            subgroup_spec = spec[i]
            self._edit_items(group_item, subgroup_spec, i)

    def _find_attribute(self, spec):
        """"""
        # Attribute name is optional, but if specified, takes precedence
        att_name = spec.get('name')
        if att_name is not None:
            att = self.att_resource.findAttribute(att_name)
            return att

        att_type = spec.get('type')
        if att_type is not None:
            att_list = self.att_resource.findAttributes(att_type)
            assert bool(att_list), 'did not find attribute of type {}'.format(
                att_type)
            assert len(att_list) <= 1, 'found multiple attributes of type {}'.format(
                att_type)
            if att_list:
                return att_list[0]

        # (else)
        raise('failed to find attribute with spec {}'.format(spec))

    def _is_reference_item(self, item):
        """"""
        return item.type() in [
            smtk.attribute.Item.ComponentType,
            smtk.attribute.Item.ResourceType]

    def _lookup_entity(self, entity_type, pedigree_id):
        """Retrieves model entity by its type and pedigree id.

        Args:
            ent_type: string, either 'face' or 'volume'
            pedigree_id: int
        """
        lookup_key = '{}.{}'.format(entity_type, pedigree_id)
        entity = self.pedigree_lookup.get(lookup_key)
        assert entity is not None, 'lookup {} returned None'.format(lookup_key)
        return entity

    def _post_message(self, msg):
        """"""
        if self.verbose:
            print(msg)

    def _recursive_create_instanced_atts(self, att_resource, comp):
        """Traverses view components to find/create instanced attributes.

        """
        if comp is None:
            raise RuntimeError('Component is None')

        if comp.name() == 'View':
            title = comp.attributes().get('Title')
            # Skip item views, which don't have a title
            if title is None:
                return
            view = att_resource.findView(title)
            if view is None:
                raise RuntimeError('View {} not found'.format(title))

            if view.type() == 'Instanced':
                self._create_instanced_view_atts(att_resource, view)
            elif view.type() == 'Analysis':
                self.get_analysis_att(att_resource, view)
            elif view.type() == 'Selector':
                self._create_selector_view_att(att_resource, view)
            else:
                self._recursive_create_instanced_atts(
                    att_resource, view.details())
            return

        # (else) process component children
        for i in range(comp.numberOfChildren()):
            child = comp.child(i)
            self._recursive_create_instanced_atts(att_resource, child)

    def _set_expression(self, item, spec):
        """Sets expression(s) to value item."""
        if isinstance(spec, list):
            # smtk MR 2329 circa 01-Oct-2020 only supports 1 expression per item
            raise RuntimeError(
                f'item {item.name()} has expression list - no longer supported')

        exp_att = self.att_resource.findAttribute(spec)
        assert exp_att is not None, 'failed to find expression attribute {}'.format(
            exp)
        item.setExpression(exp_att)

    def _set_reference_item(self, item, target):
        """Assigns a persistent object (target) to a reference item.

        Args:
            item: smtk.attribute.ReferenceItem
            target: one of
              { 'resource': str } for resource
              { 'resource': str, 'component': str } for component

            Legacy syntax:
              model for model_resource
              [face, int, int, int] for model entities
              [volume, int, int, int] for model entities
              [attribute, str, str, str] for attributes
              (legacy) str for attribute name

        Note that the target is a HACK to keep things simple, and only
        works with model resources that store a "pedigree id" property.
        """
        def set_reference_att(ref_item, i, att_resource, att_name):
            """Internal helper to set ReferenceItem value."""
            att = att_resource.findAttribute(att_name)
            assert att is not None, 'failed for find attribute with name {}'.format(
                att_name)
            assert item.setValue(i, att), 'failed to set reference value {} with attribute {}' \
                .format(item.name(), att_name)

        # New syntax uses list of dictionary instances to specify persistent object
        # with resource identifier and optional component name
        if isinstance(target, list):
            num_values = len(target)
            assert item.setNumberOfValues(num_values), \
                'failed to set reference item {} number of values to {}'.format(
                    item.name(), num_values)

            for i, po_spec in enumerate(target):
                res_key = po_spec.get('resource')
                resource = self.resource_dict.get(res_key)
                assert resource is not None, 'did not find resource with dictionary key {}'.format(
                    res_key)

                if 'component' in po_spec:
                    comp_name = po_spec.get('component')
                    filter = "*[string{{'name'='{}'}}]".format(comp_name)
                    comp_set = resource.filter(filter)
                    assert comp_set, 'component not found {}'.format(comp_name)
                    persistent_object = comp_set.pop()
                else:
                    persistent_object = resource

                was_set = item.setValue(i, persistent_object)
                assert was_set, 'failed to set reference item {} value[{}] to {}'.format(
                    item.name(), i, persistent_object.name())
            return

        # Legacy syntax
        elif target == 'model':
            # Special case
            assert self.model_resource is not None, 'model resource required to set item {}'.format(
                item.name())
            assert item.setValue(
                self.model_resource), 'failed to set model resource on item {}'.format(item.name())
        elif isinstance(target, str):  # legacy uses string for attribute name
            set_reference_att(item, 0, self.att_resource, target)
        elif isinstance(target, list):
            assert len(
                target) > 1, 'reference list too short {}, must be str, str|int,...'.format(target)
            ref_type = target[0]
            if ref_type == 'attribute':
                name_list = target[1:]
                assert item.setNumberOfValues(len(name_list)), \
                    f'failed to set item {item.name()} number of values to {len(name_list)}'
                for i in range(len(name_list)):
                    name = name_list[i]
                    set_reference_att(item, i, self.att_resource, name)
            elif ref_type in ['face', 'volume']:
                id_list = target[1:]
                assert item.setNumberOfValues(len(id_list)), \
                    f'failed to set item {item.name()} number of values to {len(id_list)}'
                for i in range(len(id_list)):
                    pedigree_id = id_list[i]
                    entity = self._lookup_entity(ref_type, pedigree_id)
                    assert item.setValue(i, entity), \
                        'failed to set ref_item {} to entity {}'.format(
                            item.name(), entity.name())
            else:
                raise RuntimeError(
                    'unrecognized refitem_type {}'.format(ref_type))

        else:
            raise RuntimeError(
                'Unexpected target type {}. Should be str or list[string, int, ...]'.format(type(target)))
