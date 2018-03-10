<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenFOAM "set_main_controls" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="set main controls" Label="Model - Set Main Controls" BaseType="operator">
      <BriefDescription>
        Set the main controls for OpenFOAM
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Set the main controls for OpenFOAM.
        &lt;p&gt;This operator constructs the controlDict dictionary
        for an OpenFOAM simulation. The controlDict dictionary is used
        to specify the main case controls. This includes, e.g. timing
        information, write format, and optional libraries that can be
        loaded at run time.
      </DetailedDescription>
      <ItemDefinitions>

        <String Name="application" Label="Solver">
          <BriefDescription>The OpenFOAM solver to use</BriefDescription>

          <DetailedDescription>
            The OpenFOAM solver to use.

            OpenFOAM does not have a generic solver applicable to all
            cases. Instead, users must choose a specific solver for a class of
            problems to solve. The solvers with the OpenFOAM distribution are in
            the $FOAM_SOLVERS directory, reached quickly by typing app at the
            command line. This directory is further subdivided into several
            directories by category of continuum mechanics, e.g. incompressible
            flow, heat transfer, multiphase, lagrangian, combustion. Each solver
            is given a name that is descriptive. For some, mainly incompressible
            solvers, it reflects the algorithm, e.g.simpleFoam using the SIMPLE
            algorithm, pimpleFoam using the PIMPLE algorithm. More often the name
            reflects the physical models or type of problem it is designed to
            solve, e.g.shallowWaterFoam, sonicFoam, cavitatingFoam.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="simpleFoam">simpleFoam</Value>
	    </Structure>
	    <Structure>
              <Value Enum="potentialFoam">potentialFoam</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <String Name="startFrom" Label="Simulation Start Time Mode">
          <BriefDescription>Controls the start time of the simulation</BriefDescription>

          <DetailedDescription>
            Controls the start time of the simulation.

            firstTime: Earliest time step from the set of time directories.
            startTime: Time specified by the startTime keyword entry.
            latestTime: Most recent time step from the set of time directories.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="firstTime">firstTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="startTime">startTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="latestTime">latestTime</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Int Name="startTime" Label="Start Time">
          <BriefDescription>Start time for the simulation when Start
          Time Mode set is to Start Time</BriefDescription>
          <DefaultValue>0</DefaultValue>
        </Int>

        <String Name="stopAt" Label="Simulation Stop Time Mode">
          <BriefDescription>Controls the end time of the simulation</BriefDescription>

          <DetailedDescription>
            Controls the end time of the simulation.

            endTime: Time specified by the endTime keyword entry.
            writeNow: Stops simulation on completion of current time step and writes data.
            noWriteNow: Stops simulation on completion of current time step and does not write out data.
            nextWrite: Stops simulation on completion of next scheduled write time, specified by writeControl.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="endTime">endTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="writeNow">writeNow</Value>
	    </Structure>
	    <Structure>
              <Value Enum="noWriteNow">noWriteNow</Value>
	    </Structure>
	    <Structure>
              <Value Enum="nextWrite">nextWrite</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Int Name="endTime" Label="End Time">
          <BriefDescription>End time for the simulation when End
          Time Mode set is to End Time</BriefDescription>
          <DefaultValue>500</DefaultValue>
        </Int>

        <Int Name="deltaT" Label="Time Step">
          <BriefDescription>The time step in the simulation</BriefDescription>
          <DefaultValue>1</DefaultValue>
        </Int>

        <String Name="writeControl" Label="Write Mode">
          <BriefDescription>Controls the timing of write output to file</BriefDescription>

          <DetailedDescription>
            Controls the timing of write output to file.

            timeStep: Writes data every writeInterval time steps.
            runTime: Writes data every writeInterval seconds of simulated time.
            adjustableRunTime: Writes data every writeInterval seconds of simulated time, adjusting the time steps to coincide with the writeInterval if necessary  used in cases with automatic time step adjustment.
            cpuTime: Writes data every writeInterval seconds of CPU time.
            clockTime: Writes data out every writeInterval seconds of real time.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="timeStep">timeStep</Value>
	    </Structure>
	    <Structure>
              <Value Enum="runTime">runTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="adjustableRunTime">adjustableRunTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="cpuTime">cpuTime</Value>
	    </Structure>
	    <Structure>
              <Value Enum="clockTime">clockTime</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Int Name="writeInterval" Label="Write Interval">
          <BriefDescription>Scalar used in conjunction with writeControl described above.</BriefDescription>
          <DefaultValue>1</DefaultValue>
        </Int>

        <Int Name="purgeWrite" Label="Purge Write">
          <BriefDescription>Scalar used in conjunction with writeControl described above.</BriefDescription>

          <DetailedDescription>
            Integer representing a limit on the number of time
            directories that are stored by overwriting time
            directories on a cyclic basis. For example, if the
            simulations starts at t = 5s and delta t = 1s, then with
            purgeWrite 2;, data is first written into 2 directories, 6
            and 7, then when 8 is written, 6 is deleted, and so on so
            that only 2 new results directories exists at any time. To
            disable the purging, specify purgeWrite 0; (default).
          </DetailedDescription>

          <DefaultValue>0</DefaultValue>
        </Int>

        <String Name="writeFormat" Label="Write Format">
          <BriefDescription>Specifies the format of the data files</BriefDescription>

          <DetailedDescription>
            Specifies the format of the data files.

            ascii (default): ASCII format, written to  writePrecision significant figures.
            binary: binary format.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="ascii">ascii</Value>
	    </Structure>
	    <Structure>
              <Value Enum="binary">binary</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Int Name="writePrecision" Label="Write Precision">
          <BriefDescription>Integer used in conjunction with Write
          Format described above</BriefDescription>

          <DefaultValue>6</DefaultValue>
        </Int>

        <Void Name="writeCompression" Label="Write Compression" IsEnabledByDefault="false">
          <BriefDescription>Switch to specify whether files are
          compressed with gzip when written</BriefDescription>
        </Void>

        <String Name="timeFormat" Label="Time Format">
          <BriefDescription>Choice of format of the naming of the time directories</BriefDescription>

          <DetailedDescription>
            Choice of format of the naming of the time directories.

            fixed:  +/- m.dddddd where the number of  ds is set by timePrecision.
            scientific: +/- m.dddddd e +/- xx where the number of ds is set by timePrecision.
            general (default): Specifies scientific format if the exponent is less
            than -4 or greater than or equal to that specified by Time Precision.
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="2">
	    <Structure>
              <Value Enum="fixed">fixed</Value>
	    </Structure>
	    <Structure>
              <Value Enum="scientific">scientific</Value>
	    </Structure>
	    <Structure>
              <Value Enum="general">general</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Int Name="timePrecision" Label="Time Precision">
          <BriefDescription>Integer used in conjunction with
          Time Format described above</BriefDescription>

          <DefaultValue>6</DefaultValue>
        </Int>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(set main controls)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
