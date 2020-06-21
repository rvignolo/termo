
<!-- TODO: definir subindices y usarlos para las otras definiciones -->
\newcommand{\acc}{\text{acc}}
\newcommand{\fric}{\text{fric}}
\newcommand{\gra}{\text{gra}}
\newcommand{\TP}{\text{TP}}
\newcommand{\lo}{\ell\text{o}}}


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
\newcommand{\Gmp}{G_{\text{m}p}}
\newcommand{\Gl}{G_{\ell}}
\newcommand{\Gv}{G_{\text{v}}}

<!-- density -->
\newcommand{\rhom}{\rho_{\text{m}}}
\newcommand{\rhol}{\rho_{\ell}}
\newcommand{\rhov}{\rho_{\text{v}}}
\newcommand{\rhomplus}{\rho_{\text{m}}^{+}}
\newcommand{\rhomplusi}[1]{\rho_{\text{m},#1}^{+}}

<!-- position -->
\newcommand{\si}[1]{s_{#1}}

<!-- cross sectional areas -->
\newcommand{\As}{A_{\text{s}}}
\newcommand{\Asp}{A_{\text{s}p}}
\newcommand{\Al}{A_{\ell}}
\newcommand{\Av}{A_{\text{v}}}

<!-- perimeter -->
\newcommand{\Ps}{P_{\text{s}}}

<!-- diameters -->
\newcommand{\De}{D_{\text{e}}}

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

<!-- heat flux -->
\newcommand{\qpp}{q''}

<!-- name? -->
\newcommand{\xe}{x_{\text{e}}}

<!-- darcy friction factor -->
\newcommand{\ftp}{f_{\TP}}
\newcommand{\flo}{f_{\lo}}
\newcommand{\ftpi}[1]{f_{\TP,#1}}

<!-- phi liquid only -->
\newcommand{\philo}{\phi_{\lo}}
\newcommand{\philoi}[1]{\phi_{\lo,#1}}

<!-- pressure -->
\newcommand{\pacc}{p_{\acc}}
\newcommand{\pfric}{p_{\fric}}
\newcommand{\pgra}{p_{\gra}}

<!-- pressure gradient -->
\newcommand{\dpds}{\left( \frac{dp}{ds} \right)}

<!-- dynamic viscosity -->
\newcommand{\mul}{\mu_{\ell}}
\newcommand{\muv}{\mu_{\text{v}}}
\newcommand{\mutp}{\mu_{\TP}}

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

<!-- CHECK -->
\begin{align}
  \frac{d\Gm}{ds} &= 0\\
  -\frac{dp}{ds} &= -\frac{d}{ds} \left( \frac{\Gm^2}{\rhomplus} \right) +  \frac{\ftp}{\De} \cdot \frac{\Gm^2}{2 \rhomplus} + \rhom \cdot g \cdot \cos \theta\\
  \Gm \cdot \frac{d\hmplus}{ds} &= \frac{\qpp \cdot \Ps}{\As}
\end{align}

<!-- CHECK -->
$$\sum_{p=1}^{P} \Gmp \cdot \Asp = \mdotm$$

$$\Gmp^{n+1} = \omega_p^n \cdot \Gmp^n$$

$$\omega_p = \Gamma \cdot \lambda_p$$

<!-- COMPLETE -->
$$\lambda_p = \sqrt{\frac{\Delta}{\Delta}}$$

$$\Gamma = \frac{\mdotm}{\sum_{p=1}^{P} \lambda_p \cdot \Gmp \cdot \Asp}$$

$$-\frac{dp}{ds} = \dpds_{\acc} + \dpds_{\fric} + \dpds_{\gra}$$

\begin{align}
  \dpds_{\acc} &= \frac{d}{ds} \left( \frac{\Gm^2}{\rhomplus} \right)\\
  \dpds_{\fric} &= \frac{\ftp}{\De} \cdot \frac{\Gm^2}{2 \rhomplus}\\
  \dpds_{\gra} &= \rhom \cdot g \cdot \cos \theta
\end{align}

\begin{align}
  \dpds_{\fric}^{\TP} &= \philo^2 \cdot \dpds_{\fric}^{\lo}\\
  \dpds_{\fric}^{\lo} &= \frac{\flo}{\De} \cdot \frac{\Gm^2}{2\rhol} 
\end{align}

$$\philo^2 = \frac{\rhol}{\rhomplus} \cdot \frac{\ftp}{\flo}$$

$$\frac{1}{\mutp} = \frac{x}{\muv} + \frac{1 - x}{\mul}$$

$$\Delta \pacc = \Gm^2 \left( \frac{1}{\rhomplusi{i+1}} - \frac{1}{\rhomplusi{i}} \right)$$

$$\Delta \pfric = \ftpi{i+1} \cdot \frac{\left( \si{i+1} - \si{i} \right)}{\De} \cdot \frac{\Gm^2}{2\rhomplusi{i+1}} = \philoi{i+1}^2$$