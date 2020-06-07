# Introduction

Termo is a computational tool designed to model steady-state and, eventually, transients of thermo-hydraulic systems that involve steam and water mixtures or two-phase flow. It has been implemented as a wasora plugin, which means that it inherits all its properties, such as algebraic expressions, functions... (see wasora site).

Many approaches can be applied to describe two-phase flow. Termo considers two different mixture models, namely:

  1. The homogeneous equilibrium mixture model (HEM). Here, the two phases move with the same velocity and also exist at the same temperature (i.e., they are at the saturation temperature for the prevailing pressure). The mixture can then be treated as a single fluid. In other words, this model considers both thermodynamic and mechanic equilibrium.

  2. A thermal equilibrium mixture model (SEP) with an algebraic relation between the velocities (or a slip ratio) of the two phases. In this case, the model only considers a thermodynamic equilibrium.

Termo allows us to model single-phase flow because it is simply a particular case of two-phase flow dynamics.

The numerical solution is obtained by solving the mass, momentum, and energy differential equations using an implicit finite difference scheme.

## Flow models

As we have already highlighted, there exist many flow models, which mainly differentiate because they either consider a unique pseudo fluid (mixture models) or two intercorrelated fluids (multifluid models). Termo has implemented two different models, the **HEM** and the **SEP** *mixture* models, i.e. they lie in the former case.

## Definitions and Nomenclature

The mixture or total mass flow is given by the summation of the liquid and vapor flows:

$$\dot{m}_{\text{m}} = \dot{m}_{\ell} + \dot{m}_{\text{v}}$$

Similarly, the volumetric flow rate is given by:

$$Q_{\text{m}} = Q_{\ell} + Q_{\text{v}}.$$

The mass flow rate and volumetric flow rate for the phase $$k$$ are related by:

$$Q_k = \frac{\dot{m}_{k}}{\rho_k}.$$

The mixture or total mass flux is defined as the average mass flow rate per unit flow (cross-sectional) area:

$$G_{\text{m}} = \frac{\dot{m}_{\text{m}}}{A_\text{s}} = \frac{1}{A_\text{s}} \left( \dot{m}_{\ell} + \dot{m}_{\text{v}} \right) = G_{\ell} + G_{\text{v}}.$$

The vapor mass flow fraction of the total flow is called the flow quality:

$$x = \frac{\dot{m}_{\text{v}}}{\dot{m}_{\text{m}}} = \frac{\dot{m}_{\text{v}}}{\dot{m}_{\ell} + \dot{m}_{\text{v}}}.$$

The flow quality allows to define the following useful relationships:

$$ $$
