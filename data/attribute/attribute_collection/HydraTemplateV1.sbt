<?xml version="1.0"?>
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories Default="General">
    <Cat>General</Cat>
    <Cat>Incompressible Navier-Stokes</Cat>
    <Cat>Energy Equation</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="Incompressible Navier-Stokes Analysis">
      <Cat>General</Cat>
      <Cat>Incompressible Navier-Stokes</Cat>
    </Analysis>
    <Analysis Type="NS and Energy Equation Analysis"><!-- can compute in enthalpy form or temperature form of energy equation -->
      <Cat>General</Cat>
      <Cat>Incompressible Navier-Stokes</Cat>
      <Cat>Energy Equation</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--***  Problem definition  ***-->
    <AttDef Type="energy" BaseType="" Version="0">
      <ItemDefinitions>
        <String Name="energy" Label="Energy Equation Formulation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(energy)</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Isothermal">isothermal</Value>
            <Value Enum="Temperature">temperature</Value>
            <Value Enum="Enthalpy">enthalpy</Value>
            <Value Enum="Internal Energy">int_energy</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="hydrostat" BaseType="" Version="0" Unique="true"> <!-- acbauer - this needs a node set id -->
      <ItemDefinitions>
        <Group Name="Hydrostat" Label="Hydrostatic pressure" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value" Label="Load Curve" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>(hydrostat::nodeset [amplitude])</BriefDescription>
              <DetailedDescription>Prescribe the hydrostatic pressure. This may be used in conjunction with prescribed pressure
              boundary conditions, or by itself. When used by itself, the hstat keyword plays two roles. It makes
              the pressure-Poisson equation non-singular and it permits the pressure for the system to be uniquely
              determined. When the hstat keyword is used with prescribed pressure boundary conditions, then
              it only specifies the unique hydrostatic pressure level for the system. In either case, the pressure
              time-history and field output is adjusted to reflect the specified hydrostatic pressure level.</DetailedDescription>
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
            <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(hydrostat::nodeset [loadCurveId])</BriefDescription>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--***  Materials Definitions ***-->
    <AttDef Type="Material" Label="Material" BaseType="" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="Density" Label="Density" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(material::rho)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <!-- acbauer - may want to change this when we get children of enumerated types working
             also for Hydra-TH, it's safe to assume constant-pressure really, so maybe the radio button is just a sort of generalization for Hydra in general where we use C_v for things like compressible flows or Lagrangian hydro.-->
        <String Name="specificheattype" Label="Specific Heat Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(material::Cp or material::Cv type)</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Constant Pressure">Cp</Value>
            <Value Enum="Constant Volume">Cv</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </String>
        <Double Name="specificheatvalue" Label="Specific Heat Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(material::Cp or material::Cv value)</BriefDescription>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="k" Label="Thermal conductivity" Version="0" AdvanceLevel="0" NumberOfRequiredValues="6">
          <BriefDescription>Symmetric thermal conductivity tensor (material::{k11,k12,k13,k22,k23,k33})</BriefDescription>
          <ComponentLabels>
            <Label>11</Label>
            <Label>12</Label>
            <Label>13</Label>
            <Label>22</Label>
            <Label>23</Label>
            <Label>33</Label>
          </ComponentLabels>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="mu" Label="Mu" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1.0</DefaultValue>
          <BriefDescription>Molecular viscosity (material::mu)</BriefDescription>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <!-- don't show gamma for now, it is needed for compressible Eulerian and Lagrangian shock hydrodynamics solvers and not really used anywhere in Hydra-TH.   I expect this to change pretty soon though so we will need to resolve this
             <Double Name="gamma" Label="Gamma - what is this?" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
             <DefaultValue>0.0</DefaultValue>
             <BriefDescription>i don't know what this is - acbauer</BriefDescription>
             <Categories>
             <Cat>Energy Equation</Cat>
             </Categories>
             </Double>
        -->
        <Double Name="beta" Label="Beta" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>0.0</DefaultValue>
          <BriefDescription>Coefficient of thermal expansion (material::beta)</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="Tref" Label="Material Reference Temperature" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>0.0</DefaultValue>
          <BriefDescription>(material::Tref)</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Void Name="rigid" Label="Rigid body specification" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>Rigid body material specification for native conjugate heat transfer (material::rigid)</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Void>
        <Double Name="velxyz" Label="Rigid body velocity" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
          <BriefDescription>(material::{velx,vely,velz})</BriefDescription>
          <DefaultValue>0.0</DefaultValue>
          <ComponentLabels>
            <Label>1</Label>
            <Label>2</Label>
            <Label>3</Label>
          </ComponentLabels>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!-- temporarily removed from the Hydra User's Manual on 12/17/2013
         <AttDef Type="viscosity" Label="Viscosity Model" BaseType="" Version="0" Unique="true">
           <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
             <MembershipMask>group|domain|volume</MembershipMask>
           </AssociationsDef>
         <ItemDefinitions>
         <Group Name="viscosity" Label="Viscosity Model" NumberOfRequiredGroups="1">
         <ItemDefinitions>
         <String Name="type" Label="Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
         <DefaultValue>Newtonian</DefaultValue>
         <BriefDescription>(viscosity::type)</BriefDescription>
         </String>
         <Double Name="Tref" Label="Reference Temperature" Version="0" NumberOfRequiredValues="1">
         <BriefDescription>(viscosity::Tref)</BriefDescription>
         <Categories>
         <Cat>Incompressible Navier-Stokes</Cat>
         <Cat>Energy Equation</Cat>
         </Categories>
         </Double>
         </ItemDefinitions>
         </Group>
         </ItemDefinitions>
         </AttDef>
    -->

    <!--*** Body force definitions ***-->
    <AttDef Type="BodyForce" BaseType="" Abstract="1" Version="0" Unique="false">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
    </AttDef>
    <AttDef Type="GravityForce" Label="Gravity Force" BaseType="BodyForce" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="GravityForce" Label="Load Curve" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(body_force::lcid)</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
          <BriefDescription>(body_force::{fx,fy,fz})</BriefDescription>
          <DefaultValue>0</DefaultValue>
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="BoussinesqForce" Label="Boussinesq Force" BaseType="BodyForce" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="BoussinesqForce" Label="Load Curve" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(boussinesqforce::lcid)</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Energy Equation</Cat>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
          <BriefDescription>(boussinesqforce::{gx,gy,gz})</BriefDescription>
          <DefaultValue>0</DefaultValue>
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
          <Categories>
            <Cat>Energy Equation</Cat>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="porous_drag" Label="Porous Drag" BaseType="BodyForce" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="porous_drag" Label="Load Curve" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(porous_drag::lcid)</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <BriefDescription>(porous_drag::amp)</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="HeatSource" Label="Heat Source" BaseType="BodyForce" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|domain|volume</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="HeatSource" Label="Load Curve" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(heat_source::lcid)</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(heat_source::Q)</BriefDescription>
          <DefaultValue>0</DefaultValue>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Initial Condition Definitions ***-->
    <AttDef Type="InitialConditions" BaseType="" Version="0" Abstract="0">
      <ItemDefinitions>
        <Group Name="InitialConditions" Label="Initial Conditions" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Velocity" Label="Initial velocity" Version="0" NumberOfRequiredValues="3">
              <BriefDescription>(initial::{velx,vely,velz})</BriefDescription>
              <DefaultValue>0</DefaultValue>
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="tke" Label="Initial turbulent kinetic energy" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>Turbulent kinetic energy for k-e and k-w models (initial::tke)</BriefDescription>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="itdr" Label="Initial turbulent dissipation rate" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Initial turbulent dissipation rate for k-e models (initial::eps)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="idts" Label="Inverse dissipation time scale" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Initial turbulent dissipation time scale for k-w models (initial::omega)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="tv" Label="Turbulent viscosity" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Turbulent viscosity for Spalart-Allmaras and DES models (initial::turbnu)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="temperature" Label="Temperature" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>(initial::temperature)</BriefDescription>
              <Categories>
                <Cat>Energy Equation</Cat>
              </Categories>
            </Double>
            <Double Name="internalenergy" Label="Internal Energy" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>(initial::int_energy)</BriefDescription>
              <Categories>
                <Cat>Energy Equation</Cat>
              </Categories>
            </Double>
            <Double Name="enthalpy" Label="Enthalpy" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>(initial::enthalpy)</BriefDescription>
              <Categories>
                <Cat>Energy Equation</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--***  Boundary Condition Definitions ***-->
    <AttDef Type="BoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="false">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
    </AttDef>
    <!-- BC structure to enforce uniqueness/prevent overspecification of BCs on a boundary -->
    <AttDef Type="EnthalpyBoundaryCondition" Label="Enthalpy" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(enthalpybc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(enthalpybc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Turbulent Dissipation" Label="Turbulent Dissipation" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Version="0" Label="Load Curve" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>Used with the RNG k-e turbulence model (epsbc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>Used with the RNG k-e turbulence model (epsbc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="distancebc" Label="Distance Function" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Version="0" Label="Load Curve" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(distancebc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(distancebc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Pressure" Label="Pressure" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(pressurebc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(pressurebc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Temperature" Label="Temperature" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(temperaturebc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(temperaturebc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="TurbulentViscosity" Label="Turbulent Viscosity" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>Used with the RNG k-e turbulence model (turbnubc::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>Used with the RNG k-e turbulence model (turbnubc::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>


    <!-- not in the Hydra User's Manual so I'm not putting it in here yet until I better understand it
         <AttDef Type="Wall" Label="Wall" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false">
           <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
             <MembershipMask>group|bdy|face</MembershipMask>
           </AssociationsDef>
         <ItemDefinitions>
         <Void Name="void" Label="Not used - do not show" Version="0" NumberOfRequiredValues="0" AdvanceLevel="1">
         <Categories>
         <Cat>Incompressible Navier-Stokes</Cat>
         </Categories>
         </Void>
         </ItemDefinitions>
         </AttDef>
         <AttDef Type="Penetration" Label="Penetration" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
           <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
             <MembershipMask>group|bdy|face</MembershipMask>
           </AssociationsDef>
         <ItemDefinitions>
         <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
         <ExpressionType>PolyLinearFunction</ExpressionType>
         <Categories>
         <Cat>Incompressible Navier-Stokes</Cat>
         </Categories>
         </Double>
         <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
         <Categories>
         <Cat>Incompressible Navier-Stokes</Cat>
         </Categories>
         </Double>
         </ItemDefinitions>
         </AttDef>
    -->

    <!--*** Velocity boundary conditions ***-->
    <AttDef Type="VelXBoundaryCondition" Label="X Velocity" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(velocitybc::velx sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(velocitybc::velx sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="VelYBoundaryCondition" Label="Y Velocity" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(velocitybc::vely sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(velocitybc::vely sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="VelZBoundaryCondition" Label="Z Velocity" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(velocitybc::velz sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(velocitybc::velz sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="SymmetryVelXBoundaryCondition" Label="X Symmetry Velocity" BaseType="VelXBoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
    </AttDef>
    <AttDef Type="SymmetryVelYBoundaryCondition" Label="Y Symmetry Velocity" BaseType="VelYBoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
    </AttDef>
    <AttDef Type="SymmetryVelZBoundaryCondition" Label="Z Symmetry Velocity" BaseType="VelZBoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
    </AttDef>

    <AttDef Type="HeatFlux" Label="Heat Flux" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="LoadCurve" Label="Load Curve" Version="0" NumberOfRequiredValues="1" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>(heatflux::sideset [loadCurveId])</BriefDescription>
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
        <Double Name="Scale" Label="Scale" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(heatflux::sideset [amplitude])</BriefDescription>
          <Categories>
            <Cat>Energy Equation</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="passiveoutflowbc" Label="Passive Outflow" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="void" Label="Not used - do not show" Version="0" NumberOfRequiredValues="0" AdvanceLevel="1">
          <BriefDescription>(passiveoutflowbc::sideset)</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="pressureoutflowbc" Label="Pressure Outflow" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="void" Label="Not used - do not show" Version="0" NumberOfRequiredValues="0" AdvanceLevel="1">
          <BriefDescription>(pressureoutflowbc::sideset)</BriefDescription>
          <Categories>
            <Cat>Incompressible Navier-Stokes</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <!--***  Execution Definitions ***-->
    <AttDef Type="LoadBalancer" BaseType="" Abstract="0" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="Method" Label="Load Balance Method" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>(load_balance::method)</BriefDescription>
          <DiscreteInfo DefaultIndex="4">
            <Value Enum="RCB">rcb</Value>
            <Value Enum="RIB">rib</Value>
            <Value Enum="SFC">sfc</Value>
            <Value Enum="Hypergraph">hg</Value>
            <Value Enum="SFC and HG">sfc_and_hg</Value>
          </DiscreteInfo>
        </String>
        <Void Name="Load Balance Diagnostics" Label="Load Balance Diagnostics" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>Load Balance Verbose Level (load_balance::diagnostics)</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="ExecutionControl" BaseType="" Abstract="0" Version="0" Unique="true">
      <ItemDefinitions>
        <Int Name="ExecutionControl" Label="Execution Control Frequency Checking"  Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>Frequency of checking execution control file (exe_control)</BriefDescription>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Output" BaseType="" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="type" Label="Output Type" Version="0" NumberOfRequiredValues="1">
          <BriefDescription>Output format for restart and field files (filetype)</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Serial">serial</Value>
            <Value Enum="Distributed">distributed</Value>
          </DiscreteInfo>
        </String>
        <Group Name="RestartOutput" Label="Restart Output" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Int Name="frequency" Label="Frequency" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>Output dump file frequency. 0 implies do not output. (dump)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <DefaultValue>0</DefaultValue>
            </Int>
          </ItemDefinitions>
        </Group>
        <Group Name="FieldOutput" Label="Field Output" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Int Name="frequency" Label="Frequency" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>Output plot file frequency. 0 implies do not output. (plti)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <DefaultValue>20</DefaultValue>
            </Int>
            <String Name="type" Label="Type" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>Output field format (pltype)</BriefDescription>
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="GMV ASCII">gmv_ascii</Value>
                <Value Enum="ExodusII">exodusii</Value>
                <Value Enum="ExodusII (64 bit)">exodusii64</Value>
                <Value Enum="ExodusII (HDF5)">exodusii_hdf5</Value>
                <Value Enum="ExodusII (64 bit HDF5)">exodusii64_hdf5</Value>
                <Value Enum="VTK ASCII">vtk_ascii</Value>
              </DiscreteInfo>
            </String>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="StatusInformation" BaseType="" Abstract="0" Version="0" Unique="true">
      <ItemDefinitions>
        <Int Name="minmaxfrequency" Label="Interval to report min/max velocity values" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>(ttyi)</BriefDescription>
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="yes">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="tifrequency" Label="Time history write frequency" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>Interval to write time-history data to the time-history files (thti)</BriefDescription>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="yes">0</Min>
          </RangeInfo>
        </Int>
        <String Name="PrintLevel" Label="ASCII Print Level" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>Controls the amount of data written to the ASCII (human-readable) output file (prtlevel)</BriefDescription>
          <ChildrenDefinitions>
            <Int Name="hcfrequency" Label="ASCII Verbose Print Interval" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(prti)</BriefDescription>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="yes">0</Min>
              </RangeInfo>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="param">param</Value>
            <Value Enum="results">results</Value>
            <Structure>
              <Value Enum="verbose">verbose</Value>
              <Items>
                <Item>hcfrequency</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="VarOutput" BaseType="" Abstract="1" Version="0" Unique="false" AdvanceLevel="0" NumberOfRequiredValues="1"/>
    <!-- histvar Hydra keyword for elements - there are no nodal variables for histvar output -->
    <AttDef Type="ElemHistVarOutput" Label="Element Time History Output" BaseType="VarOutput" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <Int Name="Id" Label="Id" Version="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <Value Enum="density">density</Value>
            <Value Enum="div">div</Value>
            <Value Enum="enstrophy">enstrophy</Value>
            <Value Enum="enthalpy">enthalpy</Value>
            <Value Enum="helicity">helicity</Value>
            <Value Enum="pressure">pressure</Value>
            <Value Enum="temp">temp</Value>
            <Value Enum="turbeps">turbeps</Value>
            <Value Enum="turbke">turbke</Value>
            <Value Enum="turbnu">turbnu</Value>
            <Value Enum="vel">vel</Value>
            <Value Enum="vorticity">vorticity</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- histvar Hydra keyword for sidesets -->
    <AttDef Type="SideSetHistVarOutput" Label="Sideset Time History Output" BaseType="VarOutput" Abstract="0" Version="0" Unique="false">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1">
          <DiscreteInfo>
            <Value Enum="avgpress">avgpress</Value>
            <Value Enum="avgtemp">avgtemp</Value>
            <Value Enum="avgvel">avgvel</Value>
            <Value Enum="force">force</Value>
            <Value Enum="fvol">fvol</Value>
            <Value Enum="heatflow">heatflow</Value>
            <Value Enum="massflow">massflow</Value>
            <Value Enum="pressforce">pressforce</Value>
            <Value Enum="surfarea">surfarea</Value>
            <Value Enum="viscforce">viscforce</Value>
            <Value Enum="volumeflow">volumeflow</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- plotvar Hydra keyword for nodes and elements -->
    <AttDef Type="NodePlotVarOutput" Label="Node Variable Output" BaseType="VarOutput" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <!-- can probably be either way but acbauer should check on this for exporting -->
            <Value Enum="density">density</Value>
            <Value Enum="dist">dist</Value>
            <Value Enum="enthalpy">enthalpy</Value>
            <Value Enum="enstrophy">enstrophy</Value>
            <Value Enum="helicity">helicity</Value>
            <Value Enum="pressure">pressure</Value>
            <Value Enum="procid">procid</Value>
            <Value Enum="temp">temp</Value>
            <Value Enum="turbeps">turbeps</Value>
            <Value Enum="turbke">turbke</Value>
            <Value Enum="turbnu">turbnu</Value>
            <Value Enum="u">u</Value>
            <Value Enum="vel">vel</Value>
            <Value Enum="vginv2">vginv2</Value>
            <Value Enum="vorticity">vorticity</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="ElemPlotVarOutput" Label="Element Variable Output" BaseType="VarOutput" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <!-- can probably be either way but acbauer should check on this for exporting -->
            <Value Enum="cfl">cfl</Value>
            <Value Enum="density">density</Value>
            <Value Enum="dist">dist</Value>
            <Value Enum="div">div</Value>
            <Value Enum="enthalpy">enthalpy</Value>
            <Value Enum="enstrophy">enstrophy</Value>
            <Value Enum="helicity">helicity</Value>
            <Value Enum="pressure">pressure</Value>
            <Value Enum="procid">procid</Value>
            <Value Enum="temp">temp</Value>
            <Value Enum="turbeps">turbeps</Value>
            <Value Enum="turbke">turbke</Value>
            <Value Enum="turbnu">turbnu</Value>
            <Value Enum="vel">vel</Value>
            <Value Enum="vginv2">vginv2</Value>
            <Value Enum="volume">volume</Value>
            <Value Enum="vorticity">vorticity</Value>
            <Value Enum="ystar">ystar</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- plotvar Hydra keyword for sidesets -->
    <AttDef Type="SideSetPlotVarOutput" Label="Sideset Variable Output" BaseType="VarOutput" Abstract="0" Version="0" Unique="false">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1">
          <DiscreteInfo>
            <Value Enum="traction">traction</Value>
            <Value Enum="straction">straction</Value>
            <Value Enum="ntraction">ntraction</Value>
            <Value Enum="wallshear">wallshear</Value>
            <Value Enum="yplus">yplus</Value>
            <Value Enum="ystar">ystar</Value>
            <Value Enum="varyplus">varyplus</Value>
            <Value Enum="heatflux">heatflux</Value>
            <Value Enum="nheatflux">nheatflux</Value>
            <Value Enum="surfarea">surfarea</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!--*** Temporal statistics options - these are output in the plotstatvar section ***-->
    <AttDef Type="TempStatVarOutput" BaseType="" Abstract="1" Version="0" Unique="false"/>
    <AttDef Type="NodeTempStatVarOutput" Label="Node plotstatvar" BaseType="TempStatVarOutput" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <Value Enum="&lt;density&gt;">&lt;density&gt;</Value>
            <Value Enum="&lt;pressure&gt;">&lt;pressure&gt;</Value>
            <Value Enum="&lt;velocity&gt;">&lt;velocity&gt;</Value>
            <Value Enum="&lt;temperature&gt;">&lt;temperature&gt;</Value>
            <Value Enum="&lt;enstrophy&gt;">&lt;enstrophy&gt;</Value>
            <Value Enum="&lt;helicity&gt;">&lt;helicity&gt;</Value>
            <Value Enum="&lt;vorticity&gt;">&lt;vorticity&gt;</Value>
            <Value Enum="&lt;pressure',pressure'&gt;">&lt;pressure',pressure'&gt;</Value>
            <Value Enum="&lt;temp',temp'&gt;">&lt;temp',temp'&gt;</Value>
            <Value Enum="&lt;density',pressure'&gt;">&lt;density',pressure'&gt;</Value>
            <Value Enum="&lt;pressure',velocity'&gt;">&lt;pressure',velocity'&gt;</Value>
            <Value Enum="rms-pressure">rms-pressure</Value>
            <Value Enum="rms-temp">rms-temp</Value>
            <Value Enum="tke">tke</Value>
            <Value Enum="reynoldsstress">reynoldsstress</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="ElemTempStatVarOutput" Label="Elem plotstatvar" BaseType="TempStatVarOutput" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <Value Enum="&lt;density&gt;">&lt;density&gt;</Value>
            <Value Enum="&lt;pressure&gt;">&lt;pressure&gt;</Value>
            <Value Enum="&lt;velocity&gt;">&lt;velocity&gt;</Value>
            <Value Enum="&lt;temperature&gt;">&lt;temperature&gt;</Value>
            <Value Enum="&lt;enstrophy&gt;">&lt;enstrophy&gt;</Value>
            <Value Enum="&lt;helicity&gt;">&lt;helicity&gt;</Value>
            <Value Enum="&lt;vorticity&gt;">&lt;vorticity&gt;</Value>
            <Value Enum="&lt;turbnu&gt;">&lt;turbnu&gt;</Value>
            <Value Enum="&lt;pressure',pressure'&gt;">&lt;pressure',pressure'&gt;</Value>
            <Value Enum="&lt;temp',temp'&gt;">&lt;temp',temp'&gt;</Value>
            <Value Enum="&lt;density',pressure'&gt;">&lt;density',pressure'&gt;</Value>
            <Value Enum="&lt;pressure',velocity'&gt;">&lt;pressure',velocity'&gt;</Value>
            <Value Enum="rms-pressure">rms-pressure</Value>
            <Value Enum="rms-temp">rms-temp</Value>
            <Value Enum="tke">tke</Value>
            <Value Enum="reynoldsstress">reynoldsstress</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="SideSetTempStatVarOutput" Label="SideSet plotstatvar" BaseType="TempStatVarOutput" Abstract="0" Version="0" Unique="false">
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>group|bdy|face</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="varname" Label="Variable Name" Version="0" NumberOfRequiredValues="1" >
          <DiscreteInfo>
            <Value Enum="&lt;density&gt;">&lt;density&gt;</Value>
            <Value Enum="&lt;pressure&gt;">&lt;pressure&gt;</Value>
            <Value Enum="&lt;velocity&gt;">&lt;velocity&gt;</Value>
            <Value Enum="&lt;temperature&gt;">&lt;temperature&gt;</Value>
            <Value Enum="&lt;heatflux&gt;">&lt;heatflux&gt;</Value>
            <Value Enum="&lt;straction&gt;">&lt;straction&gt;</Value>
            <Value Enum="&lt;ntraction&gt;">&lt;ntraction&gt;</Value>
            <Value Enum="&lt;traction&gt;">&lt;traction&gt;</Value>
            <Value Enum="&lt;pressure',pressure'&gt;">&lt;pressure',pressure'&gt;</Value>
            <Value Enum="&lt;temp',temp'&gt;">&lt;temp',temp'&gt;</Value>
            <Value Enum="&lt;density',pressure'&gt;">&lt;density',pressure'&gt;</Value>
            <Value Enum="&lt;pressure',velocity'&gt;">&lt;pressure',velocity'&gt;</Value>
            <Value Enum="rms-pressure">rms-pressure</Value>
            <Value Enum="rms-temp">rms-temp</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="TempStatVarStatistics" Label="TempStatVarStatistics" BaseType="" Abstract="0" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="TemporalStatistics" Label="Temporal Statistics" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="StartTime" Label="Start Time" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(statistics::starttime)</BriefDescription>
              <DefaultValue>0.0</DefaultValue>
            </Double>
            <Double Name="EndTime" Label="End Time" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(statistics::endtime)</BriefDescription>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="PlotWinSize" Label="Time Window Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.1</DefaultValue>
              <BriefDescription>(statistics::plotwinsize)</BriefDescription>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--*** Solver Definitions ***-->
    <AttDef Type="ppesolver" Label="ppesolver" BaseType="" Version="0" Unique="" Associations="">
      <ItemDefinitions>
        <Group Name="PressurePoissonSolver" Label="Pressure Poisson Solver" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="ppetype" Label="Type" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(ppesolver::type)</BriefDescription>
              <ChildrenDefinitions>
                <!-- preconditioner with a lot of child items -->
                <String Name="preconditioner" Label="Preconditioner" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"  Optional="true">
                  <BriefDescription>(ppesolver::amgpc)</BriefDescription>
                  <ChildrenDefinitions>
                    <String Name="hypre_coarsen_type" Label="Coarsening Algorithm" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::hypre_coarsen_type)</BriefDescription>
                      <DiscreteInfo DefaultIndex="3">
                        <Value Enum="Cleary-Luby-Jones-Plassman">CLJP</Value>
                        <Value Enum="Classical Ruge-Steuben on each processor, no boundary treatment">RUGE STEUBEN</Value>
                        <Value Enum="Classical Ruge-Steuben on each processor, followed by a third pass">MODIFIED RUGE STEUBEN</Value>
                        <Value Enum="Falgout coarsening">FALGOUT</Value>
                        <Value Enum="PMIS-coarsening">PMIS</Value>
                        <Value Enum="HMIS-coarsening">HMIS</Value>
                      </DiscreteInfo>
                    </String>
                    <!-- hypre_smoother is the way to set the smoother for all operations, hypre_smoother_do, hypre_smoother_up and hypre_smoother_co overrides these settings so I made them optional -->
                    <String Name="hypre_smoother" Label="Smoother" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::hypre_smoother)</BriefDescription>
                      <DiscreteInfo DefaultIndex="4">
                        <Value Enum="Jacobi">JACOBI</Value>
                        <Value Enum="Gauss-Seidel, Sequential">SEQ_SGS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Forward Solve">HYB_GS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Backward Solve">BACK_HYB_GS</Value>
                        <Value Enum="Hybrid Symmetric Gauss-Seidel or SSOR">HYB_SGS</Value>
                        <Value Enum="Gaussian elimination">GE</Value>
                      </DiscreteInfo>
                    </String>
                    <String Name="hypre_smoother_dn" Label="Smoother Down" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
                      <BriefDescription>(ppesolver::hypre_smoother_dn)</BriefDescription>
                      <DiscreteInfo DefaultIndex="4">
                        <Value Enum="Jacobi">JACOBI</Value>
                        <Value Enum="Gauss-Seidel, Sequential">SEQ_SGS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Forward Solve">HYB_GS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Backward Solve">BACK_HYB_GS</Value>
                        <Value Enum="Hybrid Symmetric Gauss-Seidel or SSOR">HYB_SGS</Value>
                        <Value Enum="Gaussian elimination">GE</Value>
                      </DiscreteInfo>
                    </String>
                    <String Name="hypre_smoother_up" Label="Smoother Up" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
                      <BriefDescription>(ppesolver::hypre_smoother_up)</BriefDescription>
                      <DiscreteInfo DefaultIndex="4">
                        <Value Enum="Jacobi">JACOBI</Value>
                        <Value Enum="Gauss-Seidel, Sequential">SEQ_SGS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Forward Solve">HYB_GS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Backward Solve">BACK_HYB_GS</Value>
                        <Value Enum="Hybrid Symmetric Gauss-Seidel or SSOR">HYB_SGS</Value>
                        <Value Enum="Gaussian Elimination">GE</Value>
                      </DiscreteInfo>
                    </String>
                    <String Name="hypre_smoother_co" Label="Smoother Coarsest" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
                      <BriefDescription>(ppesolver::hypre_smoother_co)</BriefDescription>
                      <DiscreteInfo DefaultIndex="5">
                        <Value Enum="Jacobi">JACOBI</Value>
                        <Value Enum="Gauss-Seidel, Sequential">SEQ_SGS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Forward Solve">HYB_GS</Value>
                        <Value Enum="Hybrid Gauss-Seidel or SOR, Backward Solve">BACK_HYB_GS</Value>
                        <Value Enum="Hybrid Symmetric Gauss-Seidel or SSOR">HYB_SGS</Value>
                        <Value Enum="Gaussian Elimination">GE</Value>
                      </DiscreteInfo>
                    </String>
                    <String Name="interp_type" Label="Parallel Interpolation Operator" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::interp_type)</BriefDescription>
                      <DiscreteInfo DefaultIndex="0">
                        <Value Enum="Classical">CLASSICAL</Value>
                        <Value Enum="Direct">DIRECT</Value>
                        <Value Enum="Multipass">MULTIPASS</Value>
                        <Value Enum="Multipass with Separation of Weights">MULTIPASS WTS</Value>
                        <Value Enum="Extended+i">EXT+I</Value>
                        <Value Enum="Extended+i (if no Common C Neighbor)">EXT+I-CC</Value>
                        <Value Enum="Standard">STANDARD</Value>
                        <Value Enum="Standard with Separation of Weights">STANDARD WTS</Value>
                        <Value Enum="FF">FF</Value>
                        <Value Enum="FF1">FF1</Value>
                      </DiscreteInfo>
                    </String>
                    <Void Name="hypre_nodal" Label="Enable Nodal Systems Coarsening" Version="0" AdvanceLevel="1" Optional="false" IsEnabledByDefault="false" NumberOfRequiredValues="0">
                      <BriefDescription>(ppseolver::hypre_nodal)</BriefDescription>
                    </Void>
                    <Double Name="trunc_factor" Label="Interpolation Truncation Factor" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::trunc_factor)</BriefDescription>
                      <DefaultValue>0.</DefaultValue>
                    </Double>
                    <Int Name="pmax_elements" Label="Maximum Element/Row for the Interpolation" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::pmax_elements)</BriefDescription>
                      <DefaultValue>0</DefaultValue>
                    </Int>
                    <Int Name="agg_num_levels" Label="Number of Levels of Aggressive Coarsening" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::agg_num_levels)</BriefDescription>
                      <DefaultValue>0</DefaultValue>
                    </Int>
                    <Int Name="agg_num_paths" Label="Number of Paths of Aggressive Coarsening" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::agg_num_paths)</BriefDescription>
                      <DefaultValue>1</DefaultValue>
                    </Int>
                    <Double Name="strong_threshold" Label="Strength Threshold" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::strong_threshold)</BriefDescription>
                      <DefaultValue>0.85</DefaultValue>
                    </Double>
                    <Double Name="max_rowsum" Label="Parameter to Modify the Definition of Strength for Diagonally Dominant Portions of the Matrix" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::max_rowsum)</BriefDescription>
                      <DefaultValue>0.9</DefaultValue>
                    </Double>

                    <!-- ml preconditioner -->
                    <String Name="smoother" Label="Smoother" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::smoother)</BriefDescription>
                      <DiscreteInfo DefaultIndex="0">
                        <Value Enum="Incomplete Cholesky">ICC</Value>
                        <Value Enum="Incomplete LU">ILU</Value>
                        <Value Enum="Successive Over-Relaxation">SSOR</Value>
                        <Value Enum="Chebychev Polynomial">CHEBYCHEV</Value>
                      </DiscreteInfo>
                    </String>

                    <!-- acbauer - which discrete values for the following??? -->
                    <String Name="cycle" Label="AMG Cycle" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::cycle)</BriefDescription>
                      <DiscreteInfo DefaultIndex="0">
                        <Value Enum="V">V</Value>
                        <Value Enum="W">W</Value>
                      </DiscreteInfo>
                    </String>
                    <String Name="solver" Label="AMG Krylov Solver" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                      <BriefDescription>(ppesolver::solver)</BriefDescription>
                      <DiscreteInfo DefaultIndex="0">
                        <Value Enum="Conjugate Gradient">CG</Value>
                        <Value Enum="Stabilized Bi-Conjugate Gradient Squared">BCGS</Value>
                        <Value Enum="Flexible Generalized Minimum Residual">FGMRES</Value>
                      </DiscreteInfo>
                    </String>
                    <Int Name="pre_smooth" Label="Number of Presmoothing Sweeps" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::pre_smooth)</BriefDescription>
                      <DefaultValue>1</DefaultValue>
                    </Int>
                    <Int Name="post_smooth" Label="Number of Postsmoothing Sweeps" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::post_smooth)</BriefDescription>
                      <DefaultValue>1</DefaultValue>
                    </Int>
                    <Int Name="coarse_size" Label="Minimum size of coarsest problem in AMG preconditioner" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::coarse_size)</BriefDescription>
                      <DefaultValue>500</DefaultValue>
                    </Int>
                    <Int Name="levels" Label="Maximum Number of AMG Levels" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"  >
                      <BriefDescription>(ppesolver::levels)</BriefDescription>
                      <DefaultValue>20</DefaultValue>
                    </Int>
                  </ChildrenDefinitions>
                  <DiscreteInfo DefaultIndex="0">
                    <Structure>
                      <Value Enum="Multilevel Preconditioner (ML)">ml</Value>
                      <Items>
                        <Item>smoother</Item>
                        <Item>cycle</Item>
                        <Item>solver</Item>
                        <Item>pre_smooth</Item>
                        <Item>post_smooth</Item>
                        <Item>coarse_size</Item>
                        <Item>levels</Item>
                      </Items>
                    </Structure>
                    <Structure>
                      <Value Enum="Hypre">hypre</Value>
                      <Items>
                        <Item>hypre_coarsen_type</Item>
                        <Item>hypre_smoother</Item>
                        <Item>hypre_smoother_dn</Item>
                        <Item>hypre_smoother_up</Item>
                        <Item>hypre_smoother_co</Item>
                        <Item>interp_type</Item>
                        <Item>hypre_nodal</Item>
                        <Item>trunc_factor</Item>
                        <Item>pmax_elements</Item>
                        <Item>agg_num_levels</Item>
                        <Item>agg_num_paths</Item>
                        <Item>strong_threshold</Item>
                        <Item>max_rowsum</Item>

                        <Item>cycle</Item>
                        <Item>solver</Item>
                        <Item>pre_smooth</Item>
                        <Item>levels</Item>
                      </Items>
                    </Structure>
                  </DiscreteInfo>
                </String>
              </ChildrenDefinitions>
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="AMG">AMG</Value>
                  <Items>
                    <Item>preconditioner</Item>
                  </Items>
                </Structure>
                <Value Enum="SSORCG">SSORCG</Value>
                <Value Enum="JPCG">JPCG</Value>
              </DiscreteInfo>
            </String>
            <Int Name="itmax" Label="Maximum number of iterations" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The maximum number of iterations. In the case of AMG, this is the maximum number of V or W cycles (ppesolver::itmax).</BriefDescription>
              <DefaultValue>500</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">500</Max>
              </RangeInfo>
            </Int>
            <Int Name="itchk" Label="Convergence criteria checking frequency" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The number of iterations to take before checking convergence criteria (ppesolver::itchk).</BriefDescription>
              <DefaultValue>2</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">25</Max>
              </RangeInfo>
            </Int>
            <Void Name="diagnostics" Label="Diagnostics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the diagnostic information from the linear solver (ppesolver::diagnostics)</BriefDescription>
            </Void>
            <Void Name="convergence" Label="Convergence metrics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the convergence metrics or the linear solver (ppesolver::convergence)</BriefDescription>
            </Void>
            <Double Name="eps" Label="Convergence criteria" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(ppesolver::eps)</BriefDescription>
              <DefaultValue>1e-05</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Double Name="pivot" Label="Preconditioner zero pivot value" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(ppesolver::zeropivot)</BriefDescription>
              <DefaultValue>1e-16</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">0.1</Max>
              </RangeInfo>
            </Double>








          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="momentumsolver" Label="Momentum Solver" BaseType="" Version="0">
      <ItemDefinitions>
        <Group Name="MomentumSolver" Label="Momentum Solver" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="type" Label="Type" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(momentumsolver::type)</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Flexible GMRES">FGMRES</Value>
                <Value Enum="ILU-Preconditioned FGMRES">ILUFGMRES</Value>
                <Value Enum="GMRES">GMRES</Value>
                <Value Enum="ILU-Preconditioned GMRES">ILUGMRES</Value>
              </DiscreteInfo>
            </String>
            <Int Name="restart" Label="Number of restart vectors" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(momentumsolver::restart)</BriefDescription>
              <DefaultValue>30</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">100</Max>
              </RangeInfo>
            </Int>
            <Int Name="itmax" Label="Maximum number of iterations" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The maximum number of iterations (momentumsolver::itmax).</BriefDescription>
              <DefaultValue>500</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">500</Max>
              </RangeInfo>
            </Int>
            <Int Name="itchk" Label="Convergence criteria checking frequency" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The number of iterations to take before checking convergence criteria (momentumsolver::itchk)</BriefDescription>
              <DefaultValue>2</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">25</Max>
              </RangeInfo>
            </Int>
            <Void Name="diagnostics" Label="Diagnostics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the diagnostic information from the solver (momentumsolver::diagnostics)</BriefDescription>
            </Void>
            <Void Name="convergence" Label="Convergence metrics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the convergence metrics or the solver (momentumsolver::convergence)</BriefDescription>
            </Void>
            <Double Name="eps" Label="Convergence criteria" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(momentumsolver::eps)</BriefDescription>
              <DefaultValue>1e-05</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!--*** The transport solver is the same as the momentum solver but i copy it so that I can set the group label properly***-->
    <AttDef Type="transportsolver" Label="Transport Solver" BaseType="" Version="0">
      <ItemDefinitions>
        <Group Name="TransportSolver" Label="Transport Solver" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="type" Label="Type" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(transportsolver::type)</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Flexible GMRES">FGMRES</Value>
                <Value Enum="ILU-Preconditioned FGMRES">ILUFGMRES</Value>
                <Value Enum="GMRES">GMRES</Value>
                <Value Enum="ILU-Preconditioned GMRES">ILUGMRES</Value>
              </DiscreteInfo>
            </String>
            <Int Name="restart" Label="Number of restart vectors" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(transportsolver::restart)</BriefDescription>
              <DefaultValue>30</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">100</Max>
              </RangeInfo>
            </Int>
            <Int Name="itmax" Label="Maximum number of iterations" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The maximum number of iterations (transportsolver::itmax)</BriefDescription>
              <DefaultValue>500</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">500</Max>
              </RangeInfo>
            </Int>
            <Int Name="itchk" Label="Convergence criteria checking frequency" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The number of iterations to take before checking convergence criteria (transportsolver::itchk)</BriefDescription>
              <DefaultValue>2</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
                <Max Inclusive="true">25</Max>
              </RangeInfo>
            </Int>
            <Void Name="diagnostics" Label="Diagnostics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the diagnostic information from the solver (transportsolver::diagnostics)</BriefDescription>
            </Void>
            <Void Name="convergence" Label="Convergence metrics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable the convergence metrics or the solver (transportsolver::convergence)</BriefDescription>
            </Void>
            <Double Name="eps" Label="Convergence criteria" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(transportsolver::eps)</BriefDescription>
              <DefaultValue>1e-05</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--*** Solution method ***-->
    <AttDef Type="solution_method" BaseType="" Version="0">
      <ItemDefinitions>
        <Group Name="SolutionMethod" Label="Solution Method" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="strategy" Label="Strategy" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>solution_method::strategy</BriefDescription>
              <ChildrenDefinitions>
                <Int Name="nvec" Label="NVec" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                  <BriefDescription>Maximum number of vectors used for non-linear Krylov acceleration (solution_method::nvec).</BriefDescription>
                  <DefaultValue>0</DefaultValue>
                </Int>
                <String Name="error_norm" Label="Non-linear convergence norm" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                  <BriefDescription>(solution_method::error_norm).</BriefDescription>
                  <DiscreteInfo DefaultIndex="0">
                    <Value Enum="Composite RMS">composite</Value>
                    <Value Enum="Max">max</Value>
                    <!-- Will eventually put 'weight' and 'residual' in but not quite ready yet
                    <Value Enum="Weight">weight</Value>
                    <Value Enum="Residual">residual</Value>
                    -->
                  </DiscreteInfo>
                </String>
              </ChildrenDefinitions>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Projection">projection</Value>
                <Structure>
                  <Value Enum="Picard">picard</Value>
                  <Items>
                    <Item>error_norm</Item>
                    <Item>nvec</Item>
                  </Items>
                </Structure>
              </DiscreteInfo>
            </String>
            <Int Name="itmax" Label="Maximum number of iterations" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>The maximum number of non-linear iterations to be taken during each time step (solution_method::itmax)</BriefDescription>
              <DefaultValue>5</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Int>
            <Double Name="eps" Label="Convergence criteria" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(solution_method::eps)</BriefDescription>
              <DefaultValue>1e-04</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Double Name="eps_dist" Label="Convergence criteria for normal-distance function" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(solution_method::eps_dist)</BriefDescription>
              <DefaultValue>1e-05</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Double Name="eps_p0" Label="Convergence criteria for initial div-free projection and initial pressure computation" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(solution_method::eps_p0)</BriefDescription>
              <DefaultValue>1e-05</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Void Name="subcycle" Label="Subcycle the pressure solves" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Ignore pressure up-dates if the pressure variable has already converged (solution_method::subcycle)</BriefDescription>
            </Void>
            <Void Name="timestep_control" Label="Activate time step control" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>(solution_method::timestep_control)</BriefDescription>
            </Void>
            <Void Name="convergence" Label="Convergence" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable writing information about the non-linear convergence history to the conv file when using the Picard solution method (solution_method::convergence)</BriefDescription>
            </Void>
            <Void Name="diagnostics" Label="Diagnostics" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
              <BriefDescription>Enable/disable printing diagnostic information to the screen about the non-linear convergence history to the conv file when using the Picard solution method (solution_method::diagnostics)</BriefDescription>
            </Void>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--*** Time information ***-->
    <AttDef Type="simulationtime" BaseType="" Version="0">
      <ItemDefinitions>
        <Int Name="nsteps" Label="Maximum number of time steps" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>The maximum number of time steps to be taken during a single simulation (nsteps)</BriefDescription>
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
            <Min Inclusive="true">10e9</Min>
          </RangeInfo>
          <DefaultValue>0</DefaultValue>
        </Int>
        <Double Name="term" Label="Simulation termination time" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>Define the simulation termination time, in units consistent with the problem definition (term)</BriefDescription>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <DefaultValue>0</DefaultValue>
        </Double>
        <Double Name="deltat" Label="Time step size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>Define the time step size to be used. This value may be over-ridden by physics specific constraints on the time step (deltat)</BriefDescription>
          <DefaultValue>0.01</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <DefaultValue>0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--*** Time integration  method ***-->
    <AttDef Type="time_integration" BaseType="" Version="0">
      <ItemDefinitions>
        <Group Name="TimeIntegration" Label="Time Integration" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="type" Label="Method" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(time_integration::type)</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Fixed CFL">fixed_cfl</Value>
                <Value Enum="Fixed time step">fixed_dt</Value>
              </DiscreteInfo>
            </String>
            <!-- CFLinit, CFLmax, dtmax and dtscale are only used for fixed_cfl -->
            <Double Name="CFLinit" Label="Initial CFL number to use at startup" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(time_integration::CFLinit)</BriefDescription>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
                <Max Inclusive="true">100</Max>
              </RangeInfo>
            </Double>
            <Double Name="CFLmax" Label="Maximum CFL number to use at startup" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(time_integration::CFLmax)</BriefDescription>
              <DefaultValue>2</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
            </Double>
            <Double Name="dtmax" Label="Maximum time step" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(time_integration::dtmax)</BriefDescription>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
            </Double>
            <Double Name="dtscale" Label="Factor used to increase the time step" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(time_integration::dtscale)</BriefDescription>
              <DefaultValue>1.025</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
            </Double>
            <Double Name="thetaa" Label="Time weight for advective terms" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>Value of 0 indicates explicit advection and value of 1 indicates implicit (time_integration::thetaa)</BriefDescription>
              <DefaultValue>0.5</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Double Name="thetaK" Label="Time weight for viscous/diffusive terms" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>Value of 0 indicates explicit advection and value of 1 indicates implicit (time_integration::thetak)</BriefDescription>
              <DefaultValue>0.5</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <Double Name="thetaf" Label="Time weight for source terms" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>Value of 0 indicates explicit treatment and value of 1 indicates implicit (time_integration::thetaf)</BriefDescription>
              <DefaultValue>0.5</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
                <Max Inclusive="true">1.0</Max>
              </RangeInfo>
            </Double>
            <!-- thetap needs to be hidden based on email from M.C. on 12/9/2013, the info below is incomplete as well.
                 <Double Name="thetap" Label="" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
                 <BriefDescription>(time_integration::thetap)</BriefDescription>
                 <DefaultValue>1</DefaultValue>
                 </Double>
            -->
            <Void Name="trimlast" Label="Use exact termination time" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(time_integration::trimlast)</BriefDescription>
            </Void>

          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--*** Turbulence models ***-->
    <!-- if this is not selected, output should be "tmodel no_turbmodel" -->
    <AttDef Type="BasicTurbulenceModel" Label="Turbulence Model" BaseType="" Version="0" Unique="" Associations="">
      <ItemDefinitions>
        <String Name="Method" Label="Turbulence Model" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"  Optional="true">
          <BriefDescription>(tmodel or turbulence)</BriefDescription>
          <ChildrenDefinitions>
            <Void Name="timescale_limiter" Label="Time Scale Limiter" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0"/>
            <Double Name="c_s" Label="Smagorinsky Model Constant" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>Smagorinsky model constant.</BriefDescription>
              <DefaultValue>0.18</DefaultValue>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="c_w" Label="WALE Model Constant" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>WALE model constant.</BriefDescription>
              <DefaultValue>0.18</DefaultValue>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="prandtl" Label="Turbulent Prandtl Number" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <DefaultValue>0.889</DefaultValue>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
            <Double Name="schmidt" Label="Turbulent Schmidt Number" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>Incompressible Navier-Stokes</Cat>
              </Categories>
            </Double>
          </ChildrenDefinitions>

          <DiscreteInfo>
            <Value Enum="Spalart-Allmaras">spalart_allmaras</Value>
            <Value Enum="Detached Eddy Spalart-Allmaras">spalart_allmaras_des</Value>
            <Structure>
              <Value Enum="Smagorinsky">smagorinsky</Value>
              <Items>
                <Item>c_s</Item>
                <Item>prandtl</Item>
                <Item>schmidt</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="WALE">WALE</Value>
              <Items>
                <Item>c_w</Item>
                <Item>prandtl</Item>
                <Item>schmidt</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="RNG k-e">rng_ke</Value>
              <Items>
                <Item>timescale_limiter</Item>
              </Items>
            </Structure>
            <!--
                <Value Enum="Smagorinsky">smagorinsky</Value>
                <Value Enum="Wall-Adapted Large Eddy">wale</Value>
                <Value Enum="RNG k-e">rng_ke</Value>
                <Value Enum="SST k-w (under development)">sst_kw</Value>
                <Value Enum="Ksgs (under development)">ksgs</Value>
                <Value Enum="LDKM ksgs (under development)">ldkm_ksgs</Value>
            -->
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!--***  Expression Definitions ***-->
    <!--    <AttDef Type="SimExpression" Abstract="1" Association="None"/> -->
    <AttDef Type="SimExpression" Abstract="1"/>
    <AttDef Type="SimInterpolation" BaseType="SimExpression" Abstract="1"/>
    <AttDef Type="PolyLinearFunction" Label="Expression" BaseType="SimInterpolation" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Group Name="ValuePairs" Label="Value Pairs" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="X" Version="0" AdvanceLevel="0" Extensible="true" NumberOfRequiredValues="0"/>
            <Double Name="Value" Version="0" AdvanceLevel="0" Extensible="true" NumberOfRequiredValues="0"/>
          </ItemDefinitions>
        </Group>
        <!-- acbauer check on name here -->
        <String Name="Sim1DLinearExp" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
      </ItemDefinitions>
    </AttDef>


  </Definitions>



  <!--**********  Attribute Instances ***********-->
  <Attributes>
  </Attributes>

  <!--********** Workflow Views ***********-->
  <RootView Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>
    <AdvancedFontEffects Bold="0" Italic="1" />

    <AttributeView Title="Materials" ModelEntityFilter="r">
      <AttributeTypes>
        <Type>Material</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Source Terms" ModelEntityFilter="r">
      <AttributeTypes>
        <Type>BodyForce</Type>
      </AttributeTypes>
    </AttributeView>

    <InstancedView Title="Execution">
      <InstancedAttributes>
        <Att Type="LoadBalancer">Load Balancer</Att>
        <Att Type="ExecutionControl">Execution Control</Att>
        <Att Type="Output">Output</Att>
        <Att Type="StatusInformation">Status Information</Att>
      </InstancedAttributes>
    </InstancedView>

    <InstancedView Title="Problem Definition">
      <InstancedAttributes>
        <Att Type="simulationtime">Simulation Time</Att>
        <Att Type="solution_method">Solution Method</Att>
        <Att Type="time_integration">Time Integration</Att>
        <Att Type="BasicTurbulenceModel">Turbulence Model</Att>
        <Att Type="energy">energy</Att>
        <Att Type="hydrostat">Hydrostatic Pressure</Att>
        <Att Type="InitialConditions">Initial Conditions</Att>
      </InstancedAttributes>
    </InstancedView>

    <InstancedView Title="Solvers">
      <InstancedAttributes>
        <Att Type="ppesolver">Pressure Poisson Solver</Att>
        <Att Type="momentumsolver">Momentum Solver</Att>
        <Att Type="transportsolver">Transport Solver</Att>
      </InstancedAttributes>
    </InstancedView>

    <!--    <AttributeView Title="Field output" ModelEntityFilter="" > -->
    <AttributeView Title="Field Output">
      <AttributeTypes>
        <Type>VarOutput</Type>
      </AttributeTypes>
    </AttributeView>

    <GroupView Title="Statistics" Style="Tiled">
      <InstancedView Title="Plot Window Parameters">
        <InstancedAttributes>
          <Att Type="TempStatVarStatistics">Parameters</Att>
        </InstancedAttributes>
      </InstancedView>
      <AttributeView Title="Variables">
        <AttributeTypes>
          <Type>TempStatVarOutput</Type>
        </AttributeTypes>
      </AttributeView>
    </GroupView>

    <AttributeView Title="Boundary Conditions" ModelEntityFilter="f">
      <AttributeTypes>
        <Type>BoundaryCondition</Type>
      </AttributeTypes>
    </AttributeView>

    <SimpleExpressionView Title="Functions">
      <Definition>PolyLinearFunction</Definition>
    </SimpleExpressionView>

  </RootView>
</SMTK_AttributeManager>
