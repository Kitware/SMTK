#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

""" export_to_pyarcHelper.py:

Helper classes for exporting a RGG model as PyARC geometry.

"""
import sys

import os
import smtk
import smtk.io
import smtk.model
import smtk.bridge.rgg
import uuid

# TODO: Use the new string format to dump out info
# '-' is a reserved keyword in pyarc. Do not use it.


def writeFile(filename, contents, mode="wt"):
    # wt = "write text"
    with open(filename, mode) as fout:
        fout.write(contents)


def prefixTabs(tabNum):
    return " " * 4 * tabNum


def rggOrderToNeamsOrder(ring, index):
    if (ring == 0):
        return 1, 1
    """Convert rgg ring index to neams ring index"""
    # AKA from 11 o'clock, counter clockwise to 3 o'clock clockwise
    if (index != 0):
        index = 6 * ring - index
    nIndex = (index + 2 * ring + 1) % (6 * ring)
    if (nIndex == 0):
        nIndex = 6 * ring
    nRing = ring + 1
    return nRing, nIndex


def surfacesToString(tabNum, surfaces):
    result = ""
    result += prefixTabs(tabNum) + "surfaces{\n"
    # Hexagon
    hexagons = sorted(list(surfaces["hexagon"]))
    for hexagon in hexagons:
        result += prefixTabs(tabNum + 1) + "hexagon ( %s ) {\n" % (hexagon[0])
        result += prefixTabs(tabNum + 2) + "orientation = %s\n" % (hexagon[1])
        result += prefixTabs(tabNum + 2) + "normal      = %s\n" % (hexagon[2])
        result += prefixTabs(tabNum + 2) + "pitch       = %f\n" % (hexagon[3])
        result += prefixTabs(tabNum + 1) + "}\n"
    # Cylinder
    cylinders = sorted(list(surfaces["cylinder"]))
    for cylinder in cylinders:
        result += prefixTabs(tabNum + 1) + \
            "cylinder ( %s ) {\n" % (cylinder[0])
        result += prefixTabs(tabNum + 2) + "axis   = %s\n" % (cylinder[1])
        result += prefixTabs(tabNum + 2) + "radius = %f\n" % (cylinder[2])
        result += prefixTabs(tabNum + 1) + "}\n"
    # Plane
    planes = sorted(list(surfaces["plane"]))
    for plane in planes:
        result += prefixTabs(tabNum + 1) + "plane ( %s ) {\n" % (plane[0])
        result += prefixTabs(tabNum + 2) + "z = %s\n" % (plane[1])
        result += prefixTabs(tabNum + 1) + "}\n"
    result += prefixTabs(tabNum) + "}\n"
    return result


class SubPin(object):

    def __init__(self, segmentType, z0, typeParameters):
        self.segmentType = segmentType
        self.z0 = z0
        self.z1 = z0 + typeParameters[0]
        self.baseRadius = typeParameters[1]
        self.topRadius = typeParameters[2]

    def __str__(self):
        segType = 'Cylinder'
        if self.segmentType == 1:
            segType = 'Frustum'
        return str((segType, self.z0, self.z1, self.baseRadius, self.topRadius))


class Pin(object):

    def __init__(self, auxGeo):
        self.auxGeo = auxGeo
        # Type(hex or rect)
        self.segmentTypes = auxGeo.integerProperty('pieces')
        # Length, base radius, top radius
        self.typeParameters = auxGeo.floatProperty('pieces')
        self.z0 = self.auxGeo.floatProperty('z origin')[0]
        self.zMax = self.z0 + sum(self.typeParameters[::3])

    def nParts(self):
        return len(self.segmentTypes)

    def subPins(self):
        segmentTypes = self.auxGeo.integerProperty('pieces')
        typeParameters = self.auxGeo.floatProperty('pieces')
        z = self.z0
        for i in xrange(self.nParts()):
            subPin = SubPin(
                segmentTypes[i], z, typeParameters[3 * i: 3 * (i + 1)])
            yield subPin
            z = subPin.z1

    def zRange(self):
        """Return z origin and z max of the pin"""
        return self.z0, self.zMax

    def exportPinRegion(self, tabNum, z0z1, indexOfAssySeg, owningAssembly, surface):
        result = ""
        if ((z0z1[0] >= self.zMax) or (z0z1[1] <= self.z0)):
            return result
        # QUESTION: Assume that the pin has no frustum involved
        materialNames = self.auxGeo.owningModel().stringProperty('materials')
        maxRadius = self.auxGeo.floatProperty("max radius")[0]
        label = self.auxGeo.stringProperty("label")[0]
        aLabel = owningAssembly.stringProperty("label")[0]
        subMs = self.auxGeo.integerProperty("layer materials")
        radiusNs = self.auxGeo.floatProperty("layer materials")
        radius = [v * maxRadius for v in radiusNs]
        result += prefixTabs(tabNum) + "pin_region ( {}_{}_{} ) {{\n".format(
            aLabel, label, indexOfAssySeg)
        for i in xrange(len(subMs)):
            subPinName = label + str(i)
            materialName = materialNames[subMs[i]]
            result += prefixTabs(
                tabNum + 1) + "sub_pin_region ( %s ) {\n" % (subPinName)
            result += prefixTabs(
                tabNum + 2) + "material   =%s\n" % (materialName)
            # Do not include inner_surf for the central region
            if (i != 0):
                innerName = "cylinder" + str(radius[i - 1])
                innerName = innerName.replace(".", "_")
                result += prefixTabs(
                    tabNum + 2) + "inner_surf = %s" % (innerName) + "\n"
                surface["cylinder"].add((innerName, "z", radius[i - 1]))
            outerName = "cylinder" + str(radius[i])
            outerName = outerName.replace(".", "_")
            result += prefixTabs(
                tabNum + 2) + "outer_surf = %s" % (outerName) + "\n"
            surface["cylinder"].add((outerName, "z", radius[i]))
            result += prefixTabs(tabNum + 1) + "}\n"
        result += prefixTabs(tabNum) + "}\n"
        return result


class SubDuctLayer(object):

    def __init__(self, parent, index):
        self.parent = parent
        self.index = index

    @property
    def z0(self):
        return self.parent.z0

    @property
    def z1(self):
        return self.parent.z1

    @property
    def material(self):
        return self.parent.materials[self.index]

    @property
    def thickness0(self):
        return self.parent.thicknesses[2 * self.index]

    @property
    def thickness1(self):
        return self.parent.thicknesses[2 * self.index + 1]

    def __str__(self):
        return str((self.z0, self.z1, self.material, self.thickness0, self.thickness1))


class SubDuct(object):

    def __init__(self, z, materials, thicknesses):
        self.z0 = z[0]
        self.z1 = z[1]
        self.materials = materials
        self.thicknesses = thicknesses

    def nLayers(self):
        return len(self.materials)

    def layers(self):
        for i in xrange(self.nLayers()):
            yield SubDuctLayer(self, i)

    def __str__(self):
        return str((self.z0, self.z1, self.materials, self.thicknesses))


class Duct(object):

    def __init__(self, auxGeo):
        self.auxGeo = auxGeo
        self.zValues = self.auxGeo.floatProperty('z values')
        self.zIntervals = [self.zValues[0]] + \
            self.zValues[1:-1:2] + [self.zValues[-1]]

    def nParts(self):
        return len(self.auxGeo.floatProperty('z values')) / 2

    def subDucts(self):
        zValues = self.auxGeo.floatProperty('z values')
        nMaterialsPerSegment = self.auxGeo.integerProperty(
            'material nums per segment')
        materials = self.auxGeo.integerProperty('materials')
        materialCounter = 0
        thicknesses = self.auxGeo.floatProperty('thicknesses(normalized)')
        for i in xrange(self.nParts()):
            subDuct = SubDuct(zValues[2 * i: 2 * (i + 1)],
                              materials[
                                  materialCounter: materialCounter + nMaterialsPerSegment[
                                      i]],
                              thicknesses[2 * materialCounter: 2 * (materialCounter + nMaterialsPerSegment[i])])
            yield subDuct
            materialCounter += nMaterialsPerSegment[i]

    def exportRadicalRegions(self, tabNum, z0z1, indexOfSubDuct, surfaces):
        """Based on z0 and z1, generate radical_region(s) string"""
        # Out of bound condition
        result = ""
        if (z0z1[0] >= self.zValues[-1] or z0z1[1] <= self.zValues[0]):
            return result, "null"
        nMaterialsPerSegment = self.auxGeo.integerProperty(
            'material nums per segment')
        materials = self.auxGeo.integerProperty('materials')
        thicknessesN = self.auxGeo.floatProperty('thicknesses(normalized)')
        # Figure out which interval z0z1 falls into in zValues
        interValIndex = next(
            i for i, v in enumerate(self.zIntervals) if v > z0z1[0]) - 1
        if (z0z1[1] > self.zIntervals[interValIndex + 1]):
            # Each sub duct should have only one set of materials along z axis
            print "ERROR: Duct %s's sub part from z %f to z %f is not segmented\
             properly." % (self.auxGeo.name(), z0z1[0], z0z1[1])
        materialOffset = sum(nMaterialsPerSegment[0:(interValIndex)])
        currentMs = materials[
            materialOffset: materialOffset + nMaterialsPerSegment[interValIndex]]
        currentTs = thicknessesN[2 * materialOffset:
                                 2 * (materialOffset + nMaterialsPerSegment[interValIndex])]
        _rRString, centerRegionMaterial = \
            self._radicalRegion(
                tabNum, currentMs, currentTs, indexOfSubDuct, surfaces)
        result += _rRString
        return result, centerRegionMaterial

    def _radicalRegion(self, tabNum, materials, thicknessesN, indexOfSubDoct, surfaces):
        """Helper function for exportRadicalRegions"""
        materialNames = self.auxGeo.owningModel().stringProperty('materials')
        ductThickness = self.auxGeo.owningModel().floatProperty(
            'duct thickness')[0]
        thicknesses = [v * ductThickness for v in thicknessesN[::2]]
        result = ""
        for i in xrange(len(materials)):
            # In neams, materials are exported from the outter region to inner region
            # PyARC requires each radial_region to have a unique name within an
            # assembly
            index = len(materials) - i - 1
            result += prefixTabs(tabNum) + "radial_region ( " +\
                self.auxGeo.name() + str(
                indexOfSubDoct) + "_" + str(i) + " ) {\n"
            result += prefixTabs(tabNum + 1) + "material   = " +\
                materialNames[materials[index]] + "\n"
            # Do not include inner_surf for the central region
            if (index != 0):
                innerName = "hexagon" + str(thicknesses[index - 1])
                innerName = innerName.replace(".", "_")
                result += prefixTabs(
                    tabNum + 1) + "inner_surf = " + innerName + "\n"
                surfaces["hexagon"].add(
                    (innerName, "y", "z", thicknesses[index - 1]))
            outerName = "hexagon" + str(thicknesses[index])
            outerName = outerName.replace(".", "_")
            result += prefixTabs(
                tabNum + 1) + "outer_surf = " + outerName + "\n"
            surfaces["hexagon"].add((outerName, "y", "z", thicknesses[index]))
            result += prefixTabs(tabNum) + "}\n"
        centerRegionMaterial = materialNames[materials[0]]
        return result, centerRegionMaterial


class Assembly(object):

    def __init__(self, group):
        self.assembly = group
        mgr = group.manager()
        # get all pins and ducts
        self.duct = Duct(smtk.model.EntityRef(mgr, uuid.UUID
                                              (group.stringProperty('associated duct')[0])))
        self.pins = [Pin(smtk.model.EntityRef(mgr, uuid.UUID(pinId)))
                     for pinId in group.stringProperty('pins')]
        self.pitch = self.assembly.floatProperty("pitches")[0]
        self.num_ring = self.assembly.integerProperty("lattice size")[0]

    def exportAssembly(self, tabNum, surfaces):
        """
        :param tabNum: number of tabs
        :return: the string of the assembly in son format
        """
        # QUESTION: Convert tab to space?
        # Calculate the segmentation of the assembly
        assemblySegs = self.calculateAssemblySegs()
        label = self.assembly.stringProperty("label")[0]

        result = prefixTabs(tabNum) + "assembly_hex ( " + label + " ) {\n"
        tabNum += 1
        for i in xrange(len(assemblySegs) / 2):
            # QUESTION: In file it's called sub_assembly but in the documentation
            #  it's called sub_assembly_hex
            z0z1 = assemblySegs[2 * i: 2 * (i + 1)]
            # TODO: a meaningful name should be assigned to a subAssembly so that
            # it can be referred properly in calculation
            subAssyName = label + "_" + "_".join(map(str, z0z1))
            subAssyName = subAssyName.replace(".", "_")  # Avoid . in the name
            result += prefixTabs(tabNum) + "sub_assembly ( " + subAssyName\
                + " ) {\n"
            result += self._generateSurfaceString(tabNum + 1, z0z1, surfaces)
            _gSDSring, centerRegionMaterial =\
                self._generateSubDuctString(tabNum + 1, z0z1, i, surfaces)
            result += _gSDSring
            result += self._generateLatticeString(tabNum + 1, z0z1,
                                                  i,
                                                  self.assembly,
                                                  centerRegionMaterial,
                                                  surfaces)
            result += prefixTabs(tabNum) + "}\n"
        result += prefixTabs(tabNum - 1) + "}\n"
        return result

    def calculateAssemblySegs(self):
        """Calculate the segmentation via slicing pins and the duct"""
        assemblySegs = self.duct.auxGeo.floatProperty('z values')
        # Extend assembly segments by the range of pins
        pinZValues = []
        for pin in self.pins:
            pinZValues.extend(pin.zRange())
        uniqueZValues = set(assemblySegs + pinZValues)
        result = sorted(list(set(assemblySegs + pinZValues)) * 2)[1:-1]
        return result

    def _generateSurfaceString(self, tabNum, z0z1, surfaces):
        """
        # Helper function for exportAssembly to generate lower/upper_axial_surf
        :param tabNum: the number of tabs
        :param z0z1: current lower and upper bound along z axis
        :param surfaces: ad new objects to surfaces set if needed
        :return: result string
        """
        result = ""
        z0Name, z1Name = "z" + str(z0z1[0]), "z" + str(z0z1[1])
        z0Name, z1Name = z0Name.replace(".", "_"), z1Name.replace(".", "_")
        result += prefixTabs(tabNum) + "lower_axial_surf=" + z0Name + "\n"
        result += prefixTabs(tabNum) + "upper_axial_surf=" + z1Name + "\n"
        surfaces["plane"].add((z0Name, z0z1[0]))
        surfaces["plane"].add((z1Name, z0z1[1]))
        return result

    def _generateSubDuctString(self, tabNum, z0z1, indexOfSubDuct, surfaces):
        stringResult, centerRegionMaterial =\
            self.duct.exportRadicalRegions(
                tabNum, z0z1, indexOfSubDuct, surfaces)
        return stringResult, centerRegionMaterial

    def _generateLatticeString(self, tabNum, z0z1, indexofAssySeg,
                               owningAssembly, cRMaterial, surfaces):
        result = prefixTabs(tabNum) + "assembly_hexlattice {\n"
        result += prefixTabs(tabNum + 1) + "pitch" + \
            " " * 9 + "= " + str(self.pitch) + "\n"
        result += prefixTabs(tabNum + 1) + "num_ring" + " " * 6 + "= "\
            + str(self.num_ring) + "\n"
        result += prefixTabs(tabNum + 1) + "outer" + " " * 9 + "= " + cRMaterial\
            + "\n"
        if (len(self.pins) > 0):
            result += self._generatePinLayoutString(
                tabNum + 1, z0z1, indexofAssySeg,
                                                    owningAssembly, surfaces)
        result += prefixTabs(tabNum) + "}\n"
        return result

    def _generatePinLayoutString(self, tabNum, z0z1, indexOfAssySeg,
                                 owningAssembly, surfaces):
        result = ""
        # TODO: Use the fill list to simplify the logic
        # QUESTION: The schema planner in SMTK only supports top view(2D). If we
        # allow accumulating pins along z axis, then what should happen?
        # Replace center pin is not allowed in son format. You need to specify
        # it in the fill list
        pinRegions, replaceString, centerPinLabel = "", "", "null"
        for i in xrange(len(self.pins)):
            uuid = str(self.pins[i].auxGeo.entity())
            pinRegion = self.pins[i].exportPinRegion(tabNum, z0z1,
                                                     indexOfAssySeg,
                                                     owningAssembly, surfaces)
            # In order to distinguish different pins, we use the unique label
            # here
            label = self.pins[i].auxGeo.stringProperty("label")[0]
            layout = self.assembly.integerProperty(uuid)
            # Only add the layout info if the pin exist
            if pinRegion:
                replaceStringsL = []
                for j in xrange(len(layout) / 2):
                    ring, index = rggOrderToNeamsOrder(
                        layout[2 * j], layout[2 * j + 1])
                    aLabel = owningAssembly.stringProperty("label")[0]
                    # the name in replace and pin_region should match
                    uniquePinName = "{}_{}_{}".format(aLabel, label,
                                                      indexOfAssySeg)
                    if (ring == 1 and index == 1):
                        centerPinLabel = uniquePinName
                    else:
                        replaceStringsL.append((ring, prefixTabs(tabNum) +
                                                "replace{ ring=%d index=%d name=%s }\n"
                                                % (ring, index, uniquePinName)))
                # Print out info in ascending order
                replaceStringsL = sorted(replaceStringsL)
                for rS in replaceStringsL:
                    replaceString += rS[1]
                pinRegions += pinRegion
        # TODO: understand how null value is handled in son format
        result += prefixTabs(tabNum) + "fill" + " " * 10 + "= [ " +\
            "{} ".format(centerPinLabel) * \
            (self.num_ring) + "]\n"
        result += replaceString
        result += pinRegions
        return result

    def getAllSubAssyNames(self):
        # Duplicated logic in exportAssembly
        names = []
        assemblySegs = self.calculateAssemblySegs()
        label = self.assembly.stringProperty("label")[0]

        for i in xrange(len(assemblySegs) / 2):
            # QUESTION: In file it's called sub_assembly but in the documentation
            #  it's called sub_assembly_hex
            z0z1 = assemblySegs[2 * i: 2 * (i + 1)]
            # TODO: a meaningful name should be assigned to a subAssembly so that
            # it can be referred properly in calculation
            subAssyName = label + "_" + "_".join(map(str, z0z1))
            subAssyName = subAssyName.replace(".", "_")  # Avoid . in the name
            names.append(subAssyName)
        return names


class Core(object):

    def __init__(self, group):
        self.core = group
        mgr = group.manager()
        model = group.owningModel()
        self.assys = [Assembly(smtk.model.EntityRef(mgr, uuid.UUID(assyId)))
                      for assyId in model.stringProperty("assemblies")]
        self.num_ring = model.integerProperty("lattice size")[0]

    def exportCore(self, tabNum, surfaces):
        result = prefixTabs(tabNum) + "regions_reactor{\n"
        result += self._calculateAxialSurfAndBoundaryCon(tabNum + 1, surfaces)
        result += self._generateCoreHexlatticeString(tabNum + 1, surfaces)
        result += self._generateRelatedAssysString(tabNum + 1, surfaces)
        result += prefixTabs(tabNum) + "}\n"
        return result

    def getAllSubAssyNames(self):
        model = self.core.owningModel()
        names = []
        for group in model.groups():
            if (group.stringProperty('rggType')[0] == '_rgg_assembly'):
                assembly = Assembly(group)
                names.extend(assembly.getAllSubAssyNames())
        return names

    def _calculateAxialSurfAndBoundaryCon(self, tabNum, surfaces):
        """Helper function for exportCore to calculate axial surface and define\
        # boundary conditions
        """
        result = ""
        model = self.core.owningModel()
        zRangesOfAssy = []
        for group in model.groups():
            rggType = group.stringProperty('rggType')[0]
            if (rggType == '_rgg_assembly'):
                assembly = Assembly(group)
                segs = assembly.calculateAssemblySegs()
                zRangesOfAssy.append(segs[0])
                zRangesOfAssy.append(segs[-1])
        if (len(zRangesOfAssy) == 0):
            return result
        zMin, zMax = min(zRangesOfAssy), max(zRangesOfAssy)
        zMinName, zMaxName = "z" + str(zMin), "z" + str(zMax)
        zMinName, zMaxName = zMinName.replace(
            ".", "_"), zMaxName.replace(".", "_")
        surfaces["plane"].add((zMinName, zMin))
        surfaces["plane"].add((zMaxName, zMax))
        result += prefixTabs(tabNum) + "lower_axial_surf = %s\n" % (zMinName)
        result += prefixTabs(tabNum) + \
            "lower_boundary_condition = extrapolated\n\n"
        result += prefixTabs(tabNum) + "upper_axial_surf = %s\n" % (zMaxName)
        result += prefixTabs(tabNum) + \
            "upper_boundary_condition = extrapolated\n"
        return result

    def _generateCoreHexlatticeString(self, tabNum, surfaces):
        model = self.core.owningModel()
        ductThickness = model.floatProperty("duct thickness")[0]
        surfName = "hexagon" + str(ductThickness)
        surfName = surfName.replace(".", "_")
        surfaces["hexagon"].add((surfName, "y", "z", ductThickness))
        result, replaceString = prefixTabs(tabNum) + "core_hexlattice {\n", ""
        # In neams workflow, assembly_surf refers to the largest duct in the core
        # In rgg workflow, duct's size is fixed per core
        result += prefixTabs(tabNum + 1) + "assembly_surf = %s\n" % (surfName)
        result += prefixTabs(tabNum + 1) + \
            "num_ring      = %d\n" % (self.num_ring)
        # Replace center assembly is not allowed in son format. You need to specify
        # it in the fill list
        centerAssyLabel = "null"
        for i in xrange(len(self.assys)):
            uuid = str(self.assys[i].assembly.entity())
            # In order to distinguish different assemblies, we use the unique
            # label here
            label = self.assys[i].assembly.stringProperty("label")[0]
            layout = model.integerProperty(uuid)
            replaceStringsL = []
            for j in xrange(len(layout) / 2):
                ring, index = rggOrderToNeamsOrder(
                    layout[2 * j], layout[2 * j + 1])
                if (ring == 1 and index == 1):
                    centerAssyLabel = label
                else:
                    replaceStringsL.append((ring, prefixTabs(tabNum + 2) +
                                            "replace{ ring=%d index=%d name=%s }\n"
                                            % (ring, index, label)))
            # Print out info in ascending order
            replaceStringsL = sorted(replaceStringsL)
            for rS in replaceStringsL:
                replaceString += rS[1]
        result += prefixTabs(tabNum + 1) + "fill" + " " * 10 + "= [ " +\
            "{} ".format(centerAssyLabel) * \
            (self.num_ring) + "]\n"
        result += replaceString
        result += prefixTabs(tabNum) + "}\n"
        return result

    def _generateRelatedAssysString(self, tabNum, surfaces):
        model = self.core.owningModel()
        result = ""
        for group in model.groups():
            if (group.stringProperty('rggType')[0] == '_rgg_assembly'):
                assembly = Assembly(group)
                result += assembly.exportAssembly(tabNum, surfaces)
        return result
