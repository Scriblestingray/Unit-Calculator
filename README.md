# unit

A simple C-based configurable unit calculator.

Usage:
- "unit" pulls up a REPL
- "unit [expr]" will evaluate the given expression and return the result

Architecture:
 - Unit relationships are in a graph, so one can work backwards or forwards to convert between any arbitrary combination of units
- Calculator that uses extra precision (BCD-like system) along with rational numbers to convert between units when necessary
 - For unit systems not based on a prefix system, a value that is a fraction of a unit may be described in as many whole units as possible (e.g. 5ft 6in instead of 5.5ft)
 - For prefix-based unit systems, pick the unit that most nicely describes the quantity
 - Automatic calculation of derived units (dividing distance by time results in speed)
 - Usual + - * /, perhaps '^' for power, and "as [unit]" to force a unit conversion (raising an error if quantity does not match). also parens
 - Variables would sure be nice
 - No full-on computer algebra system though


The architecture consists of three parts:
- An interface for defining unit systems
- A config file format; unit loads a config file on startup
- A calculator that works with units


Notes:
- During runtime, convert the graph of unit relationships into a flat one that describes all relationships in terms of the base unit. Should be easier than traversing that graph during each calculation, and will also make it faster.

