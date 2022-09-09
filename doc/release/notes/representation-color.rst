Resource Representation Color Fix
---------------------------------

An index out-of-range crash could occur when a component's "color" property
is set, but has fewer than 4 values.
Now the following is done:

+ 0 values is treated as the default color (white).
+ 1 value is treated as an opaque greyscale value.
+ 2 values is treated as a greyscale value plus opacity.
+ 3 values is treated as an opaque (red, green, blue) color.
+ 4 values is treated as before: (red, gree, blue, opacity).
