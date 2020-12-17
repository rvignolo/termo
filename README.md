Termo is a computational tool designed to model steady-state and, eventually, transients of thermo-hydraulic systems that involve steam and water mixtures or two-phase flow. It has been implemented as a [wasora](https://seamplex.com/wasora/) plugin, which means that it inherits all its properties.

Many approaches can be applied to describe two-phase flow. Termo considers two different mixture models, namely:

  1. The homogeneous equilibrium mixture model (*HEM*). Here, the two phases move with the same velocity and also exist at the same temperature (i.e., they are at the saturation temperature for the prevailing pressure). The mixture can then be treated as a single fluid. In other words, this model suppose both thermodynamic and mechanic equilibrium.

  2. A thermal equilibrium mixture model (*SEP*) which considers an algebraic relation between the velocities (or a slip ratio) of the two phases. In this case, the model only assumes a thermodynamic equilibrium.