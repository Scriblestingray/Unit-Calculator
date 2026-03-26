# unit

A simple C-based configurable unit calculator.

Usage:
- "unit" pulls up a REPL
- "unit [expr]" will evaluate the given expression and return the result

Architecture:
 - Unit relationships are in a graph, so one can work backwards or forwards to convert between any arbitrary combination of units
- Calculator that uses extra precision
 - Pick the unit that most nicely describes the quantity
 - Automatic calculation of derived units
 - Usual + - * /, and "as [unit]" to force a unit conversion (raising an error if quantity does not match). also parens

The architecture consists of three parts:
- An interface for defining unit systems
- A config file format; unit loads a config file on startup
- A calculator that works with units