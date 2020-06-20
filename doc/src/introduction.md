
<!-- mass flow -->
\newcommand{\mdotm}{\dot{m}_{\text{m}}}
\newcommand{\mdotl}{\dot{m}_{\ell}}
\newcommand{\mdotv}{\dot{m}_{\text{v}}}

<!-- volumetric flow -->
\newcommand{\Qm}{Q_{\text{m}}}
\newcommand{\Ql}{Q_{\ell}}
\newcommand{\Qv}{Q_{\text{v}}}

<!-- mass flux -->
\newcommand{\Gm}{G_{\text{m}}}
\newcommand{\Gl}{G_{\ell}}
\newcommand{\Gv}{G_{\text{v}}}

<!-- density -->
\newcommand{\rhom}{\rho_{\text{m}}}
\newcommand{\rhol}{\rho_{\ell}}
\newcommand{\rhov}{\rho_{\text{v}}}
\newcommand{\rhomplus}{\rho_{\text{m}}^{+}}

<!-- cross sectional areas -->
\newcommand{\As}{A_{\text{s}}}
\newcommand{\Al}{A_{\ell}}
\newcommand{\Av}{A_{\text{v}}}

<!-- velocity -->
\newcommand{\vl}{v_{\ell}}
\newcommand{\vv}{v_{\text{v}}}

<!-- entalphy -->
\newcommand{\hm}{h_{\text{m}}}
\newcommand{\hmplus}{h_{\text{m}}^{+}}
\newcommand{\hl}{h_{\ell}}
\newcommand{\hv}{h_{\text{v}}}
\newcommand{\hf}{h_{\text{f}}}
\newcommand{\hg}{h_{\text{g}}}

\newcommand{\xe}{x_{\text{e}}}

# Introduction

Termo is a computational tool designed to model steady-state and, eventually, transients of thermo-hydraulic systems that involve steam and water mixtures or two-phase flow. It has been implemented as a wasora plugin, which means that it inherits all its properties, such as algebraic expressions, functions... (see wasora site).

Many approaches can be applied to describe two-phase flow. Termo considers two different mixture models, namely:

  1. The homogeneous equilibrium mixture model (**HEM**). Here, the two phases move with the same velocity and also exist at the same temperature (i.e., they are at the saturation temperature for the prevailing pressure). The mixture can then be treated as a single fluid. In other words, this model considers both thermodynamic and mechanic equilibrium.

  2. A thermal equilibrium mixture model (**SEP**) with an algebraic relation between the velocities (or a slip ratio) of the two phases. In this case, the model only considers a thermodynamic equilibrium.

Termo allows us to model single-phase flow because it is simply a particular case of two-phase flow dynamics.

The numerical solution is obtained by solving the mass, momentum, and energy differential equations using an implicit finite difference scheme.

## Flow models

As we have already highlighted, there exist many flow models, which mainly differentiate because they either consider a unique pseudo fluid (mixture models) or two intercorrelated fluids (multifluid models). Termo has implemented two different models, the **HEM** and the **SEP** *mixture* models, i.e. they lie in the former case.

## Definitions and Nomenclature

The mixture or total mass flow is given by the summation of the liquid and vapor flows:

$$\mdotm = \mdotl + \mdotv$$

Similarly, the volumetric flow rate is given by:

$$\Qm = \Ql + \Qv.$$

The mass flow rate and volumetric flow rate for the phase $k$ are related by the density:

$$Q_k = \frac{\dot{m}_{k}}{\rho_k}.$$

The mixture or total mass flux is defined as the average mass flow rate per unit flow (cross-sectional) area:

$$\Gm = \frac{\mdotm}{\As} = \frac{1}{\As} \left( \mdotl + \mdotv \right) = \Gl + \Gv.$$

The vapor mass flow fraction of the total flow is called the flow quality:

$$x = \frac{\mdotv}{\mdotm} = \frac{\mdotv}{\mdotl + \mdotv}.$$

The flow quality allows to define the following useful relationships:

\begin{align*}
  \mdotl &= \mdotm \cdot (1 - x),  &  \mdotv &= \mdotm \cdot x, \\
  \Gl &= \Gm \cdot (1 - x),        &  \Gv &= \Gm \cdot x.
\end{align*}


$$\beta = \frac{\Qv}{\Qm} = \frac{\Qv}{\Ql + \Qv} = \frac{\rhol \cdot x}{\rhol \cdot x + \rhov \cdot (1 - x)}$$

$$\alpha_k = \frac{A_k}{\As} = \frac{A_k}{\Al + \Av}$$

$$S = \frac{\vv}{\vl}$$

$$v_k = \frac{Q_k}{A_k} = \frac{Q_k}{\As \cdot \alpha_k}$$

$$S = \frac{\rhol \cdot x \cdot (1 - \alpha)}{\rhov \cdot (1 - x) \cdot \alpha}$$

$$\alpha = \frac{\rhol \cdot x}{\rhol \cdot x + \rhov \cdot (1 - x) \cdot S}$$

$$\rhom = \sum_k \alpha_k \cdot \rho_k = \alpha \cdot \rhov + (1 - \alpha) \cdot \rhol$$

$$\rhomplus = \frac{\Gm^2}{\rhov \cdot \vv^2 \cdot \alpha + \rhol \cdot \vl^2 \cdot (1 - \alpha)}$$

$$\hm = \frac{1}{\rhom} \cdot \left( \rhov \cdot \alpha \cdot \hv + \rhol \cdot (1 - \alpha) \cdot \hl \right)$$

$$ \hmplus = \frac{1}{\Gm} \cdot \left( \rhov \cdot \alpha \cdot \hv \cdot \vv + \rhol \cdot (1 - \alpha) \cdot \hl \cdot \vl \right) $$

$$\xe = \frac{\hmplus - \hf}{\hg - \hf} = \frac{x \cdot \hv + (1 - x) \cdot \hl - \hf}{\hg - \hf}$$