Session: Discrete
-----------------

SMTK has a session type named *discrete* that bridges a VTK-based discrete modeling kernel.
This kernel provides access to 2-D and 3-D polygonal and polyhedral models,
with operators that allow model edges and faces to be split or merged.

Models need not include a full topology (e.g., only volumes and faces may be represented,
with edges implied; or geometric entities may be modeled but not all of their oriented use-records).
However, several operations such as "*create edges"* exist to generate a full topology from
a self-consistent but incomplete model.
