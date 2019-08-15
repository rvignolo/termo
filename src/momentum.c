/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's momentum balance related routines
 *
 *  Copyright (C) 2017 ramiro vignolo
 *
 *  This file is part of mate.
 *
 *  mate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  mate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with mate.  If not, see <http://www.gnu.org/licenses/>.
 *------------------- ------------  ----    --------  --     -       -         -
 */
#include "mate.h"

int mate_compute_node_fluid_momentum_balance(double *p, mate_node_t *node) {
  
  double dp_g, dp_acc, dp_fric;
  
  // vamos a necesitar las condiciones del nodo previo
  mate_node_t *previous_node = node->backward_fluid_volume->initial_node;
  
  // la caida de presion por gravedad
  wasora_call(mate_compute_gravity_dp(&dp_g, previous_node, node));
  
  // la caida de presion por aceleracion
  wasora_call(mate_compute_acceleration_dp(&dp_acc, previous_node, node));
  
  // la caida de presion por friccion
  wasora_call(mate_compute_friction_dp(&dp_fric, node));
  
  // sumamos las contribuciones al anterior nodo
  *p  = mate_get_fluid_p(previous_node->fluid_properties) - (dp_g + dp_fric + dp_acc);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_gravity_dp(double *dp_g, mate_node_t *previous_node, mate_node_t *node) {
  
  *dp_g = mate_get_fluid_rho_m(node->fluid_properties) * GRAVITY * (node->x[2] - previous_node->x[2]);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_acceleration_dp(double *dp_acc, mate_node_t *previous_node, mate_node_t *node) {
  
  *dp_acc  = 1.0 / mate_get_fluid_rho_m_plus(node->fluid_properties) - 1.0 / mate_get_fluid_rho_m_plus(previous_node->fluid_properties);
  *dp_acc *= pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_friction_dp(double *dp_fric, mate_node_t *node) {
  
  double phi_lo;
  double f;
  double rho;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      // no hace falta especificar el caso
      wasora_call(mate_compute_node_fluid_f(&f, node, UNSPECIFIED_CASE));
      
      // tomamos la densidad del liquido
      rho = mate_get_fluid_rho_l(node->fluid_properties);
      
      // la perdida de carga por friccion
      *dp_fric = f * node->backward_fluid_volume->L / node->section->D * pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / 2.0 / rho;
      
      break;
      
    case attached_bubble:
      
      // no hace falta especificar el caso
      wasora_call(mate_compute_node_fluid_f(&f, node, UNSPECIFIED_CASE));
      
      // tomamos la densidad del liquido
      rho = mate_get_fluid_rho_l(node->fluid_properties);
      
      // la perdida de carga por friccion
      *dp_fric = f * node->backward_fluid_volume->L / node->section->D * pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / 2.0 / rho;
      
      break;
      
    case bubble_departure:
      
      // computamos el factor de friccion considerando todo liquido
      wasora_call(mate_compute_node_fluid_f(&f, node, LIQUID_ONLY_CASE));
      
      // tomamos la densidad del liquido ya que es como two phase
      rho = mate_get_fluid_rho_l(node->fluid_properties);
      
      // computamos el multiplicador de dos fases
      wasora_call(mate_compute_node_fluid_phi_lo(&phi_lo, node));
      
      // la perdida de carga por friccion
      *dp_fric = phi_lo * f * node->backward_fluid_volume->L / node->section->D * pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / 2.0 / rho;
      
      break;
      
    case saturated_boiling:
      
      // computamos el factor de friccion considerando todo liquido
      wasora_call(mate_compute_node_fluid_f(&f, node, LIQUID_ONLY_CASE));
      
      // tomamos la densidad del liquido ya que es two phase
      rho = mate_get_fluid_rho_l(node->fluid_properties);
      
      // computamos el multiplicador de dos fases
      wasora_call(mate_compute_node_fluid_phi_lo(&phi_lo, node));
      
      // la perdida de carga por friccion
      *dp_fric = phi_lo * f * node->backward_fluid_volume->L / node->section->D * pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / 2.0 / rho;
      
      break;
      
    case single_phase_vapor:
      
      // no hace falta especificar el caso
      wasora_call(mate_compute_node_fluid_f(&f, node, UNSPECIFIED_CASE));
      
      // tomamos la densidad del vapor
      rho = mate_get_fluid_rho_v(node->fluid_properties);
      
      // la perdida de carga por friccion
      *dp_fric = f * node->backward_fluid_volume->L / node->section->D * pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / 2.0 / rho;
      
      break;
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_f(double *f, mate_node_t *node, int option) {
  
  switch(node->section->type) {
    
    case straight_section_type:
      
      wasora_call(mate_compute_straight_section_node_fluid_f(f, node, option));
      break;
      
    case circular_section_type:
      
      wasora_call(mate_compute_circular_section_node_fluid_f(f, node, option));
      break;
      
    case helical_section_type:
      
      wasora_call(mate_compute_helical_section_node_fluid_f(f, node, option));
      break;
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_straight_section_node_fluid_f(double *f, mate_node_t *node, int option) {
  
  double Re;
  double df;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      Re = mate_get_fluid_Re_l(node->fluid_properties);
      break;
      
    case attached_bubble:
      
      // como x = 0
      Re = mate_get_fluid_Re_l(node->fluid_properties);
      break;
      
    case bubble_departure:
      
      if (option == LIQUID_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_l(node->fluid_properties) / (1.0 - mate_get_fluid_x(node->fluid_properties));
        
      } else if (option == VAPOUR_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_v(node->fluid_properties) / mate_get_fluid_x(node->fluid_properties);
        
      } else if (option == TWO_PHASE_CASE) {
        
        Re = mate_get_fluid_Re_tp(node->fluid_properties);
      }
      break;
      
    case saturated_boiling:
      
      if (option == LIQUID_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_l(node->fluid_properties) / (1.0 - mate_get_fluid_x(node->fluid_properties));
        
      } else if (option == VAPOUR_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_v(node->fluid_properties) / mate_get_fluid_x(node->fluid_properties);
        
      } else if (option == TWO_PHASE_CASE) {
        
        Re = mate_get_fluid_Re_tp(node->fluid_properties);
      }
      break;
      
    case single_phase_vapor:
      
      Re = mate_get_fluid_Re_v(node->fluid_properties);
      break;
  }
  
  if (Re < 2300) {
    
    // darcy
    *f = 64.0 / Re;
    
  } else if (Re < 4000) {
    // zona de transicion
    wasora_push_error_message("cannot compute Darcy friction factor in pipe '%s', section '%s' and node with '%d' section index because it lies in the transition zone (Re = %e)", node->section->pipe->name, node->section->name, node->node_section_index, Re);
    return WASORA_RUNTIME_ERROR;
    
  } else {
    
    // semilla inicial (aprox. de Haaland)
    *f = -1.8 * log10(6.9 / Re + pow(node->section->solid_material->epsilon / node->section->D / 3.7, 1.11));
    *f = pow(1.0 / *f, 2.0);
    
    do {
      df = *f;
      
      // Colebrook
      *f = -2.0 * log10(node->section->solid_material->epsilon / node->section->D / 3.7 + 2.51 / Re / sqrt(*f));
      *f = pow(1.0 / *f, 2.0);
      
      df = fabs(df - *f);
    } while (df > MAX_RESIDUAL);
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_circular_section_node_fluid_f(double *f, mate_node_t *node, int option) {
  
  // TODO: creo que es correcto que por ahora sean equivalentes (cuando termine de ver el Idelchik lo chequeo)
  // creo que lo que faltaria es la otra parte no debida a friccion, aunque probablemente esta contribucion va en form loss keyword
  wasora_call(mate_compute_straight_section_node_fluid_f(f, node, option));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_helical_section_node_fluid_f(double *f, mate_node_t *node, int option) {
  
  double Re, Re_crit;
  
  helical_section_t *helical_section = (helical_section_t *) node->section->section_params;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      Re = mate_get_fluid_Re_l(node->fluid_properties);
      break;
      
    case attached_bubble:
      
      // como x = 0
      Re = mate_get_fluid_Re_l(node->fluid_properties);
      break;
      
    case bubble_departure:
      
      if (option == LIQUID_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_l(node->fluid_properties) / (1.0 - mate_get_fluid_x(node->fluid_properties));
        
      } else if (option == VAPOUR_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_v(node->fluid_properties) / mate_get_fluid_x(node->fluid_properties);
        
      } else if (option == TWO_PHASE_CASE) {
        
        Re = mate_get_fluid_Re_tp(node->fluid_properties);
      }
      break;
      
    case saturated_boiling:
      
      if (option == LIQUID_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_l(node->fluid_properties) / (1.0 - mate_get_fluid_x(node->fluid_properties));
        
      } else if (option == VAPOUR_ONLY_CASE) {
        
        Re = mate_get_fluid_Re_v(node->fluid_properties) / mate_get_fluid_x(node->fluid_properties);
        
      } else if (option == TWO_PHASE_CASE) {
        
        Re = mate_get_fluid_Re_tp(node->fluid_properties);
      }
      break;
      
    case single_phase_vapor:
      
      Re = mate_get_fluid_Re_v(node->fluid_properties);
      break;
  }
  
  Re_crit = 2.0e4 * pow(node->section->D / 2.0 / helical_section->R, 0.32);
  
  if (Re < Re_crit) {
    
    // fanning
    *f  = 344.0 * pow(2.0 * helical_section->R / node->section->D, -0.5);
    *f /= pow(1.56 + log10(Re * pow(2.0 * helical_section->R / node->section->D, -0.5)), 5.73);
    
  } else {
    
    // fanning
    *f = 0.076 * pow(Re, -0.25) + 0.00725 * pow(2.0 * helical_section->R / node->section->D, -0.5);
  }
  
  // darcy
  *f *= 4.0;
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_phi_lo(double *phi_lo, mate_node_t *node) {
  
  switch(mate.flow_model) {
    
    case HEM:
      
      // funciona para un nodo perteneciente a cualquier geometria
      wasora_call(mate_compute_node_fluid_HEM_phi_lo(phi_lo, node));
      break;
      
    case SEP:
      
      // debemos mirar el tipo de geometria al que pertenece el nodo
      switch(node->section->type) {
        
        case straight_section_type:
          
          wasora_call(mate_compute_straight_section_node_fluid_SEP_phi_lo(phi_lo, node));
          break;
          
        case circular_section_type:
          
          // aproximamos de la siguiente manera
          wasora_call(mate_compute_straight_section_node_fluid_SEP_phi_lo(phi_lo, node));
          break;
          
        case helical_section_type:
          
          wasora_call(mate_compute_helical_section_node_fluid_SEP_phi_lo(phi_lo, node));
          break;
      }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_HEM_phi_lo(double *phi_lo, mate_node_t *node) {
  
  double rho_l, rho_m;
  double f_fo, f_tp;
  
  // estamos en HEM por lo que rho_m_plus = rho_m
  rho_l = mate_get_fluid_rho_l(node->fluid_properties);
  rho_m = mate_get_fluid_rho_m(node->fluid_properties);
  
  // computamos los factores de friccion: considerando que es todo liquido y mezcla hom.
  wasora_call(mate_compute_node_fluid_f(&f_fo, node, LIQUID_ONLY_CASE));
  wasora_call(mate_compute_node_fluid_f(&f_tp, node, TWO_PHASE_CASE));
  
  // finalmente
  *phi_lo = rho_l / rho_m * f_tp / f_fo;
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_straight_section_node_fluid_SEP_phi_lo(double *phi_lo, mate_node_t *node) {
  
  double x;
  double rho_m;
  double rho_l, rho_v;
  double mu_f, mu_g;
  
  double f_fo, f_go;
  double E, F, H;
  double Fr, We;
  
  x = mate_get_fluid_x(node->fluid_properties);
  
  rho_l = mate_get_fluid_rho_l(node->fluid_properties);
  rho_v = mate_get_fluid_rho_v(node->fluid_properties);
  
  // Friedel usa la aproximacion homogenea de la densisdad de la mezcla
  // como estamos en SEP, tengo que calcularla aca
  rho_m = x / rho_v + (1.0 - x) / rho_l;
  rho_m = 1.0 / rho_m;
  
  mu_f = mate_get_fluid_mu_l(node->fluid_properties);
  mu_g = mate_get_fluid_mu_v(node->fluid_properties);
  
  wasora_call(mate_compute_node_fluid_f(&f_fo, node, LIQUID_ONLY_CASE));
  wasora_call(mate_compute_node_fluid_f(&f_go, node, VAPOUR_ONLY_CASE));
  
  E = pow(1.0 - x, 2.0) + pow(x, 2.0) * rho_l / rho_v * f_go / f_fo;
  F = pow(x, 0.78) * pow(1.0 - x, 0.224);
  H = pow(rho_l / rho_v, 0.91) * pow(mu_g / mu_f, 0.19) * pow(1.0 - mu_g / mu_f, 0.7);
  
  Fr = pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) / GRAVITY / node->section->D / pow(rho_m, 2.0);
  We = pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) * node->section->D / mate_get_fluid_sigma(node->fluid_properties) / rho_m;
  
  *phi_lo  = E + 3.24 * F * H;
  *phi_lo /= pow(Fr, 0.045);
  *phi_lo /= pow(We, 0.035);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_helical_section_node_fluid_SEP_phi_lo(double *phi_lo, mate_node_t *node) {
  
  double x, alpha;
  double rho_l, rho_v;
  double mu_f, mu_g;
  double Re_tp;
  
  helical_section_t *helical_section = (helical_section_t *) node->section->section_params;
  
  x = mate_get_fluid_x(node->fluid_properties);
  
  alpha = mate_get_fluid_alpha(node->fluid_properties);
  
  rho_l = mate_get_fluid_rho_l(node->fluid_properties);
  rho_v = mate_get_fluid_rho_v(node->fluid_properties);
  
  mu_f = mate_get_fluid_mu_l(node->fluid_properties);
  mu_g = mate_get_fluid_mu_v(node->fluid_properties);
  
  Re_tp = mate_get_fluid_Re_tp(node->fluid_properties);
  
  *phi_lo  = 2.06 * pow(node->section->D / 2.0 / helical_section->R, 0.05) * pow(Re_tp, -0.025);
  *phi_lo *= pow(1.0 + alpha * (rho_v / rho_l - 1.0), 0.8);
  *phi_lo *= pow(1.0 + alpha * (mu_g / mu_f - 1.0), 0.2);
  *phi_lo *= pow(1.0 + x * (rho_l / rho_v - 1.0), 1.8);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_pipe_array_mean_dp(double *mean_dp, pipe_array_t *pipe_array) {
  
  int i;
  double dp;
  
  *mean_dp = 0;
  for (i = 0; i < pipe_array->n_pipes; i++) {
    wasora_call(mate_compute_pipe_dp(&dp, pipe_array, i));
    *mean_dp += dp;
  }
  
  *mean_dp /= pipe_array->n_pipes;
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_pipe_dp(double *dp, pipe_array_t *pipe_array, int pipe) {
  
  int n_nodes;
  int index_i, index_o;
  mate_node_t *node_i;
  mate_node_t *node_o;
  
  n_nodes = pipe_array->solver->n_nodes_per_pipe[pipe];
  
  index_i = mate_compute_node_pipe_array_index(pipe_array, pipe, 0);
  index_o = mate_compute_node_pipe_array_index(pipe_array, pipe, n_nodes-1);
  
  node_i = pipe_array->solver->node_by_pipe_array_index[index_i];
  node_o = pipe_array->solver->node_by_pipe_array_index[index_o];
  
  // hago presion a la entrada - presion a la salida
  *dp = mate_get_fluid_p(node_i->fluid_properties) - mate_get_fluid_p(node_o->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}