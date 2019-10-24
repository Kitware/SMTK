## Changes to the model subsystem

### Instance editing

In addition to creating instances, it is now possible to
+ divide an instance by creating a point or cell selection in a
  render window and running the "divide instance" operation;
+ change an instance's prototype;
+ merge multiple instances with the same prototype into a
  single tabular instance.

### Instance placement

When snapping to entities with mesh tessellations, support has been
added to snap to the nearest point on the entity's surface (rather
than the nearest point explicitly defined in the tessellation).
