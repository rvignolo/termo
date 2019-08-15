/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's fluid-related routines
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

int mate_compute_node_fluid_parameters(mate_node_t *node) {
  
  // seteo de entalpias
  wasora_call(mate_compute_node_fluid_enthalpies(node));
  
  // determinacion del regimen
  wasora_call(mate_compute_node_fluid_regime(node));
  
  // determinacion de titulos
  wasora_call(mate_compute_node_fluid_qualities(node));
  
  // determinacion de caudales masicos
  wasora_call(mate_compute_node_fluid_mass_flows(node));
  
  // determinacion de flujos masicos
  wasora_call(mate_compute_node_fluid_mass_fluxes(node));
  
  // determinacion de densidades
  wasora_call(mate_compute_node_fluid_densities(node));
  
  // determinacion de viscosidades dinamicas
  wasora_call(mate_compute_node_fluid_dynamic_viscocities(node));
  
  // determinacion de capacidades calorificas
  wasora_call(mate_compute_node_fluid_heat_capacities(node));
  
  // determinacion de conductividades termicas
  wasora_call(mate_compute_node_fluid_thermal_conductivities(node));
  
  // determinacion de numeros adimensionales: Re, Pr y Nu
  wasora_call(mate_compute_node_fluid_dimensionless_numbers(node));
  
  // determinacion de la fraccion de vacio
  wasora_call(mate_compute_node_fluid_void_fraction(node));
  
  // determinacion de densidades de la mezcla
  wasora_call(mate_compute_node_fluid_mixture_densities(node));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_enthalpies(mate_node_t *node) {
  
  double p_sat, T_sat;
  double h_f, h_g, h_m_plus;
  
  // en cualquier regimen decimos que hf y hg se computan asumiendo que la presion del nodo es una de saturacion
  p_sat = mate_get_fluid_p(node->fluid_properties);
  T_sat = freesteam_region4_Tsat_p(p_sat);
  
  h_f = freesteam_region4_h_Tx(T_sat, 0.0);
  h_g = freesteam_region4_h_Tx(T_sat, 1.0);
  
  // en todos los regimenes la entalpia dinamica del nodo es la entalpia
  // con la que seteamos el estado termodinamico del nodo (balance de energia)
  h_m_plus = mate_get_fluid_h(node->fluid_properties);
  
  wasora_call(mate_set_fluid_h_f(node->fluid_properties, h_f));
  wasora_call(mate_set_fluid_h_g(node->fluid_properties, h_g));
  
  wasora_call(mate_set_fluid_h_m_plus(node->fluid_properties, h_m_plus));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_regime(mate_node_t *node) {
  
  int region = mate_get_fluid_region(node->fluid_properties);
  
  switch(region) {
    
    case IAPWS_REGION_1:
      
      // si tenemos subcooled boiling, buscamos entre dos nuevas regiones: onb y bd
      if (mate.sub_boiling) {
        
/*
        // si no podemos estimar el ni el flujo de calor ni el Peclet porque es la primera iteracion interna del nodo
        if (mate.solver.inner_iter[node->pipe_id-1] == 0) {
          
          // decimos directamente que es single phase
          node->fluid_properties->regime = single_phase_liquid;
          
          // pero si podemos estimar el flujo de calor y el Peclet
        } else {
          
          // si ya encontramos el node_bd o si este es el primero que verifica
          if (mate.solver.sub_boiling.node_bd[node->pipe_id-1] != NULL || mate_check_for_bubble_departure_regime(node, solid_node)) {
            
            node->fluid_properties->regime = bubble_departure;
            
            // si ya encontramos el node_onb o si este es el primero que verifica
          } else if (mate.solver.sub_boiling.node_onb[node->pipe_id-1] != NULL || mate_check_for_subcooled_boiling_regime(node, solid_node)) {
            
            node->fluid_properties->regime = subcooled_boiling;
            
            // si no encontramos ninguno de los anteriores y no verifico ser node_bd o node_onb
          } else {
            
            node->fluid_properties->regime = single_phase_liquid;
          }
        }
*/
        
        // pero si no tenemos ebullicion subenfriada
      } else {
        node->fluid_properties->regime = single_phase_liquid;
      }
      break;
      
    case IAPWS_REGION_2:
      
      node->fluid_properties->regime = single_phase_vapor;
      break;
      
    case IAPWS_REGION_4:
      
      node->fluid_properties->regime = saturated_boiling;
      
      // si tenemos subcooled boiling
      if (mate.sub_boiling) {
        
/*
        // si es el primer nodo en saturated boiling hacemos la siguiente asignacion
        if (mate.solver.sub_boiling.node_osb[node->pipe_id-1] == NULL)
          mate.solver.sub_boiling.node_osb[node->pipe_id-1] = node;
*/
      }
      break;
      
    default:
      wasora_push_error_message("unspected IAPWS region (%d) in pipe '%s', section '%s' and node with '%d' section index", region, node->section->pipe->name, node->section->name, node->node_section_index);
      return WASORA_RUNTIME_ERROR;
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_qualities(mate_node_t *node) {
  
  double x, xe;
  double hf, hg;
  
  // propiedades de equilibrio
  hf = mate_get_fluid_h_f(node->fluid_properties);
  hg = mate_get_fluid_h_g(node->fluid_properties);
  
  // calculamos titulo termodinamico en todos los regimenes
  xe  = mate_get_fluid_h_m_plus(node->fluid_properties) - hf;
  xe /= (hg - hf);
  
  // calculamos el titulo real segun el regimen
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      // el titulo real (flow quality) es nulo 
      x = 0;
      break;
      
    case attached_bubble:
      
      // suponemos al titulo real (flow quality) como nulo 
      x = 0;
      break;
    
    case bubble_departure:
      
/*
      // aproximamos mediante perfil de levy al titulo real
      wasora_call(mate_compute_levy_quality_aprox(&x, node));
*/
      break;
      
    case saturated_boiling:
      
      // dependiendo si hay ebullicion subenfriada
      if (mate.sub_boiling) {
        
/*
        // si encontro el nodo_bd
        if (mate.solver.sub_boiling.node_bd[node->pipe_id-1] != NULL) {
          
          // aproximamos por levy
          mate_call(mate_compute_levy_quality_aprox(&x, node));
          
        } else {
          
          // decimos que el titulo real es equivalente al termodinamico
          //x = xe;
          x = freesteam_x(node->fluid_properties->S);
        }
*/
        
      } else {
        
        // decimos que el titulo real es equivalente al termodinamico
        //x = xe;
        x = freesteam_x(node->fluid_properties->S);
      }
      break;
      
    case single_phase_vapor:
      
      x = 1.0;
      break;
  }
  
  wasora_call(mate_set_fluid_x(node->fluid_properties, x));
  wasora_call(mate_set_fluid_xe(node->fluid_properties, xe));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_mass_flows(mate_node_t *node) {
  
  double mass_flow_l;
  double mass_flow_v;
  
  mass_flow_l = mate_get_fluid_mass_flow_m(node->fluid_properties) * (1.0 - mate_get_fluid_x(node->fluid_properties));
  mass_flow_v = mate_get_fluid_mass_flow_m(node->fluid_properties) * mate_get_fluid_x(node->fluid_properties);
  
  wasora_call(mate_set_fluid_mass_flow_l(node->fluid_properties, mass_flow_l));
  wasora_call(mate_set_fluid_mass_flow_v(node->fluid_properties, mass_flow_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_mass_fluxes(mate_node_t *node) {
  
  double mass_flux_m;
  double mass_flux_l;
  double mass_flux_v;
  
  mass_flux_m = mate_get_fluid_mass_flow_m(node->fluid_properties) / node->backward_fluid_volume->Af;
  mass_flux_l = mate_get_fluid_mass_flow_l(node->fluid_properties) / node->backward_fluid_volume->Af;
  mass_flux_v = mate_get_fluid_mass_flow_v(node->fluid_properties) / node->backward_fluid_volume->Af;
  
  wasora_call(mate_set_fluid_mass_flux_m(node->fluid_properties, mass_flux_m));
  wasora_call(mate_set_fluid_mass_flux_l(node->fluid_properties, mass_flux_l));
  wasora_call(mate_set_fluid_mass_flux_v(node->fluid_properties, mass_flux_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_densities(mate_node_t *node) {
  
  double p_sat, T_sat;
  double rho_f, rho_g;
  double rho_l, rho_v;
  
  // en cualquier regimen calculamos rho_f y rho_g asumiendo que la presion del nodo es la de saturacion
  p_sat = mate_get_fluid_p(node->fluid_properties);
  T_sat = freesteam_region4_Tsat_p(p_sat);
  
  rho_f = freesteam_region4_rhof_T(T_sat);
  rho_g = freesteam_region4_rhog_T(T_sat);
  
  // ahora determinamos densidades de liquido y vapor presentes en el nodo y segun el regimen
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      rho_l = mate_get_fluid_rho(node->fluid_properties);
      rho_v = 0;
      break;
      
    case attached_bubble:
      
      // el liquido tiene densidad de subenfriado
      rho_l = mate_get_fluid_rho(node->fluid_properties);
      
      // hay burbujas con esta densidad (aunque estimemos x = 0 y alpha = 0)
      rho_v = rho_g;
      break;
      
    case bubble_departure:
      
      // el liquido tiene densidad de subenfriado
      rho_l = mate_get_fluid_rho(node->fluid_properties);
      
      // hay burbujas con esta densidad
      rho_v = rho_g;
      break;
      
    case saturated_boiling:
      
      // cuando esta saturado (esto esta bien cuando no hay equilibrio termodinamico hasta Ze?)
      rho_l = rho_f;
      rho_v = rho_g;
      break;
      
    case single_phase_vapor:
      
      rho_l = 0;
      rho_v = mate_get_fluid_rho(node->fluid_properties);
      break;
  }
  
  wasora_call(mate_set_fluid_rho_f(node->fluid_properties, rho_f));
  wasora_call(mate_set_fluid_rho_g(node->fluid_properties, rho_g));
  
  wasora_call(mate_set_fluid_rho_l(node->fluid_properties, rho_l));
  wasora_call(mate_set_fluid_rho_v(node->fluid_properties, rho_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_dynamic_viscocities(mate_node_t *node) {
  
  double x;
  double p_sat, T_sat;
  double mu_f, mu_g;
  double mu_l, mu_v;
  double mu_tp;
  
  // en cualquier regimen calculamos mu_f y mu_g asumiendo que la presion del nodo es la de saturacion
  p_sat = mate_get_fluid_p(node->fluid_properties);
  T_sat = freesteam_region4_Tsat_p(p_sat);
  
  mu_f = freesteam_mu_rhoT(mate_get_fluid_rho_f(node->fluid_properties), T_sat);
  mu_g = freesteam_mu_rhoT(mate_get_fluid_rho_g(node->fluid_properties), T_sat);
  
  x = mate_get_fluid_x(node->fluid_properties);
  
  // ahora determinamos viscosidades dinamicas de liquido, vapor y bifasica presentes en el nodo
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      mu_l = mate_get_fluid_mu(node->fluid_properties);
      mu_v = 0;
      mu_tp = mu_l;
      break;
      
    case attached_bubble:
      
      // el liquido tiene viscosidad de subenfriado
      mu_l = mate_get_fluid_mu(node->fluid_properties);
      
      // hay burbujas con esta viscosidad (aunque estimemos x = 0 y alpha = 0)
      mu_v = mu_g;
      
      // debido a que estimamos x = 0
      mu_tp = mu_l;
      break;
      
    case bubble_departure:
      
      // el liquido tiene viscosidad de subenfriado
      mu_l = mate_get_fluid_mu(node->fluid_properties);
      
      // hay burbujas con esta densidad
      mu_v = mu_g;
      
      // McAdams aproximation for dynamic viscosity
      mu_tp = x / mu_v + (1.0 - x) / mu_l;
      mu_tp = 1.0 / mu_tp;
      break;
      
    case saturated_boiling:
      
      // cuando esta saturado (esto esta bien cuando no hay equilibrio termodinamico hasta Ze?)
      mu_l = mu_f;
      mu_v = mu_g;
      
      // McAdams aproximation for dynamic viscosity
      mu_tp = x / mu_v + (1.0 - x) / mu_l;
      mu_tp = 1.0 / mu_tp;
      break;
      
    case single_phase_vapor:
      
      mu_l = 0;
      mu_v = mate_get_fluid_mu(node->fluid_properties);
      mu_tp = mu_v;
      break;
  }
  
  wasora_call(mate_set_fluid_mu_tp(node->fluid_properties, mu_tp));
  
  wasora_call(mate_set_fluid_mu_f(node->fluid_properties, mu_f));
  wasora_call(mate_set_fluid_mu_g(node->fluid_properties, mu_g));
  
  wasora_call(mate_set_fluid_mu_l(node->fluid_properties, mu_l));
  wasora_call(mate_set_fluid_mu_v(node->fluid_properties, mu_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_heat_capacities(mate_node_t *node) {
  
  double p_sat, T_sat;
  double cp_f, cp_g;
  double cp_l, cp_v;
  
  // en cualquier regimen calculamos cp_f y cp_g asumiendo que la presion del nodo es la de saturacion
  p_sat = mate_get_fluid_p(node->fluid_properties);
  T_sat = freesteam_region4_Tsat_p(p_sat);
  
  cp_f = freesteam_region4_cp_Tx(T_sat, 0.0);
  cp_g = freesteam_region4_cp_Tx(T_sat, 1.0);
  
  // ahora determinamos los cp de liquido y vapor presentes en el nodo
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      cp_l = mate_get_fluid_cp(node->fluid_properties);
      cp_v = 0;
      break;
      
    case attached_bubble:
      
      // el liquido tiene cp de subenfriado
      cp_l = mate_get_fluid_cp(node->fluid_properties);
      
      // hay burbujas con esta densidad (aunque estimemos x = 0 y alpha = 0)
      cp_v = cp_g;
      break;
      
    case bubble_departure:
      
      // el liquido tiene densidad de subenfriado
      cp_l = mate_get_fluid_cp(node->fluid_properties);
      
      // hay burbujas con esta densidad
      cp_v = cp_g;
      break;
      
    case saturated_boiling:
      
      // cuando esta saturado (esto esta bien cuando no hay equilibrio termodinamico hasta Ze?)
      cp_l = cp_f;
      cp_v = cp_g;
      break;
      
    case single_phase_vapor:
      
      cp_l = 0;
      cp_v = mate_get_fluid_cp(node->fluid_properties);
      break;
  }
  
  wasora_call(mate_set_fluid_cp_f(node->fluid_properties, cp_f));
  wasora_call(mate_set_fluid_cp_g(node->fluid_properties, cp_g));
  
  wasora_call(mate_set_fluid_cp_l(node->fluid_properties, cp_l));
  wasora_call(mate_set_fluid_cp_v(node->fluid_properties, cp_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_thermal_conductivities(mate_node_t *node) {
  
  double p_sat, T_sat;
  double k_f, k_g;
  double k_l, k_v;
  
  // en cualquier regimen calculamos k_f y k_g asumiendo que la presion del nodo es la de saturacion
  p_sat = mate_get_fluid_p(node->fluid_properties);
  T_sat = freesteam_region4_Tsat_p(p_sat);
  
  k_f = freesteam_k_rhoT(freesteam_region4_rhof_T(T_sat), T_sat);
  k_g = freesteam_k_rhoT(freesteam_region4_rhog_T(T_sat), T_sat);
  
  // ahora determinamos los k de liquido y vapor presentes en el nodo
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      k_l = mate_get_fluid_k(node->fluid_properties);
      k_v = 0;
      break;
      
    case attached_bubble:
      
      // el liquido tiene cp de subenfriado
      k_l = mate_get_fluid_k(node->fluid_properties);
      
      // hay burbujas con esta densidad (aunque estimemos x = 0 y alpha = 0)
      k_v = k_g;
      break;
      
    case bubble_departure:
      
      // el liquido tiene densidad de subenfriado
      k_l = mate_get_fluid_k(node->fluid_properties);
      
      // hay burbujas con esta densidad
      k_v = k_g;
      break;
      
    case saturated_boiling:
      
      // cuando esta saturado (esto esta bien cuando no hay equilibrio termodinamico hasta Ze?)
      k_l = k_f;
      k_v = k_g;
      break;
      
    case single_phase_vapor:
      
      k_l = 0;
      k_v = mate_get_fluid_k(node->fluid_properties);
      break;
  }
  
  wasora_call(mate_set_fluid_k_f(node->fluid_properties, k_f));
  wasora_call(mate_set_fluid_k_g(node->fluid_properties, k_g));
  
  wasora_call(mate_set_fluid_k_l(node->fluid_properties, k_l));
  wasora_call(mate_set_fluid_k_v(node->fluid_properties, k_v));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_dimensionless_numbers(mate_node_t *node) {
  
  // el Re y Pr no dependen del tipo de geometria
  wasora_call(mate_compute_node_fluid_Re(node));
  wasora_call(mate_compute_node_fluid_Pr(node));
  
  // pero el Nu claramente si
  switch(node->section->type) {
    
    case straight_section_type:
      wasora_call(mate_compute_straight_section_node_fluid_Nu(node));
      break;
      
    case circular_section_type:
      wasora_call(mate_compute_circular_section_node_fluid_Nu(node));
      break;
      
    case helical_section_type:
      wasora_call(mate_compute_helical_section_node_fluid_Nu(node));
      break;
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_Re(mate_node_t *node) {
  
  double Re_l, Re_v, Re_tp;
  
  // calculamos los Reynolds segun el regimen
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      Re_l  = mate_get_fluid_mass_flux_l(node->fluid_properties) * node->section->D / mate_get_fluid_mu_l(node->fluid_properties);
      // evito division 0/0 porque mu_v es nulo
      Re_v  = 0;
      Re_tp = mate_get_fluid_mass_flux_m(node->fluid_properties) * node->section->D / mate_get_fluid_mu_tp(node->fluid_properties);
      break;
      
    case attached_bubble:
      
      Re_l  = mate_get_fluid_mass_flux_l(node->fluid_properties) * node->section->D / mate_get_fluid_mu_l(node->fluid_properties);
      // el titulo real esta en 0, entonces flux_v = 0 pero mu_v le pusimos un valor y no hay division 0/0
      Re_v  = mate_get_fluid_mass_flux_v(node->fluid_properties) * node->section->D / mate_get_fluid_mu_v(node->fluid_properties);
      Re_tp = mate_get_fluid_mass_flux_m(node->fluid_properties) * node->section->D / mate_get_fluid_mu_tp(node->fluid_properties);
      break;
    
    case bubble_departure:
      
      Re_l  = mate_get_fluid_mass_flux_l(node->fluid_properties) * node->section->D / mate_get_fluid_mu_l(node->fluid_properties);
      Re_v  = mate_get_fluid_mass_flux_v(node->fluid_properties) * node->section->D / mate_get_fluid_mu_v(node->fluid_properties);
      Re_tp = mate_get_fluid_mass_flux_m(node->fluid_properties) * node->section->D / mate_get_fluid_mu_tp(node->fluid_properties);
      break;
      
    case saturated_boiling:
      
      Re_l  = mate_get_fluid_mass_flux_l(node->fluid_properties) * node->section->D / mate_get_fluid_mu_l(node->fluid_properties);
      Re_v  = mate_get_fluid_mass_flux_v(node->fluid_properties) * node->section->D / mate_get_fluid_mu_v(node->fluid_properties);
      Re_tp = mate_get_fluid_mass_flux_m(node->fluid_properties) * node->section->D / mate_get_fluid_mu_tp(node->fluid_properties);
      break;
      
    case single_phase_vapor:
      
      // evito division 0/0 porque mu_l es nulo
      Re_l  = 0;
      Re_v  = mate_get_fluid_mass_flux_v(node->fluid_properties) * node->section->D / mate_get_fluid_mu_v(node->fluid_properties);
      Re_tp = mate_get_fluid_mass_flux_m(node->fluid_properties) * node->section->D / mate_get_fluid_mu_tp(node->fluid_properties);
      break;
  }
  
  wasora_call(mate_set_fluid_Re_l(node->fluid_properties, Re_l));
  wasora_call(mate_set_fluid_Re_v(node->fluid_properties, Re_v));
  wasora_call(mate_set_fluid_Re_tp(node->fluid_properties, Re_tp));
  
  wasora_var(mate.vars.Re_tp) = mate_get_fluid_Re_tp(node->fluid_properties);
  wasora_var(mate.vars.Re_l) = mate_get_fluid_Re_l(node->fluid_properties);
  wasora_var(mate.vars.Re_v) = mate_get_fluid_Re_v(node->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_Pr(mate_node_t *node) {
  
  double Pr;
  
  // calculamos los Prandtl segun el regimen
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      Pr = mate_get_fluid_cp_l(node->fluid_properties) * mate_get_fluid_mu_l(node->fluid_properties) / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case attached_bubble:
      
      Pr = mate_get_fluid_cp_l(node->fluid_properties) * mate_get_fluid_mu_l(node->fluid_properties) / mate_get_fluid_k_l(node->fluid_properties);
      break;
    
    case bubble_departure:
      
      Pr = mate_get_fluid_cp_l(node->fluid_properties) * mate_get_fluid_mu_l(node->fluid_properties) / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case saturated_boiling:
      
      // CORRELATION FOR BOILING HEAT TRANSFER TO SATURATED FLUIDS IN CONVECTIVE FLOW (John C. Chen), pg. 324 
      // se asume que Pr_tp (two-phase) y Pr_l (liquido) son practicamente iguales dado que Pr_l y Pr_v tienen semejante magnitud
      // Por lo tanto, computamos el Pr del liquido en este caso
      Pr = mate_get_fluid_cp_l(node->fluid_properties) * mate_get_fluid_mu_l(node->fluid_properties) / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case single_phase_vapor:
      
      Pr = mate_get_fluid_cp_v(node->fluid_properties) * mate_get_fluid_mu_v(node->fluid_properties) / mate_get_fluid_k_v(node->fluid_properties);
      break;
  }
  
  wasora_call(mate_set_fluid_Pr(node->fluid_properties, Pr));
  wasora_var(mate.vars.Pr) = mate_get_fluid_Pr(node->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_straight_section_node_fluid_Nu(mate_node_t *node) {
  
  double Nu;
  double F, S;
  double h_mac, h_mic, h_tp;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      wasora_call(mate_compute_straight_section_node_fluid_Nu_l(&Nu, node));
      break;
      
    case attached_bubble:
      
      // como x = 0, F sera equivalente a 1 y en consecuencia Re_tp de Chen = Re_l
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      wasora_call(mate_compute_straight_section_node_fluid_Nu_l(&Nu, node));
      
      // multiplico por F aunque este vale 1
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D * F;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
    
    case bubble_departure:
      
      // como x es aprox 0, F sera 1 o aprox 1 (por inv_X_tt) y en consecuencia Re_tp de Chen = Re_l o aprox
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      wasora_call(mate_compute_straight_section_node_fluid_Nu_l(&Nu, node));
      
      // multiplico por F aunque es de esperar que este valga 1 o aproximadamente 1
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D * F;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case saturated_boiling:
      
      // primero se calculan los dos parametros de Chen
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      wasora_call(mate_compute_straight_section_node_fluid_Nu_l(&Nu, node));
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D * F;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case single_phase_vapor:
      
      wasora_call(mate_compute_straight_section_node_fluid_Nu_v(&Nu, node));
      break;
  }
   
/*
  if(mate_get_solid_T_m(node->solid_properties) < mate_get_fluid_T(node->fluid_properties)) {
    
    fprintf(stderr, "error: la transferencia de calor en condensacion aun no se encuentra implementada en el codigo\n");
    return MATE_RUNTIME_ERROR;
    
  } else {
    
    mate_call(mate_compute_Chen_h_mic(&h_mic, node));
    h_mic *= S;
    
  }
*/
  
  wasora_call(mate_set_fluid_Nu(node->fluid_properties, Nu));
  wasora_var(mate.vars.Nu) = mate_get_fluid_Nu(node->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_straight_section_node_fluid_Nu_l(double *Nu, mate_node_t *node) {
  
  double exp;
  
  exp = (mate_get_solid_T_m(node->solid_properties) > mate_get_fluid_T(node->fluid_properties)) ? 0.4 : 0.3;
  
  // TODO: dependencia con el perfil termico (tomar en consideracion temperatura de pared)? agregar eventualmente.
  *Nu = 0.023 * pow(mate_get_fluid_Re_l(node->fluid_properties), 0.8) * pow(mate_get_fluid_Pr(node->fluid_properties), exp);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_straight_section_node_fluid_Nu_v(double *Nu, mate_node_t *node) {
  
  double exp;
  
  exp = (mate_get_solid_T_m(node->solid_properties) > mate_get_fluid_T(node->fluid_properties)) ? 0.4 : 0.3;
  
  // TODO: dependencia con el perfil termico (tomar en consideracion temperatura de pared)? agregar eventualmente.
  *Nu = 0.023 * pow(mate_get_fluid_Re_v(node->fluid_properties), 0.8) * pow(mate_get_fluid_Pr(node->fluid_properties), exp);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_circular_section_node_fluid_Nu(mate_node_t *node) {
  
  // como aproximacion pareciera ser correcta
  wasora_call(mate_compute_straight_section_node_fluid_Nu(node));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_helical_section_node_fluid_Nu(mate_node_t *node) {
  
  double Nu;
  double F, S;
  double h_mac, h_mic, h_tp;
  
  helical_section_t *helical_section;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      wasora_call(mate_compute_helical_section_node_fluid_Nu_l(&Nu, node));
      break;
      
    case attached_bubble:
      
      // como x = 0, F sera equivalente a 1 y en consecuencia Re_tp de Chen = Re_l
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      // como Re_tp de Chen = Re_l, mandar a calcular asi es correcto
      wasora_call(mate_compute_helical_section_node_fluid_Nu_l(&Nu, node));
      
      // F = 1 y por eso no multiplico por el, aunque multiplicar no es buen uso de la teoria, ver bubble_departure o saturated_boiling regime
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case bubble_departure:
      
      // como x es aprox 0, F sera 1 o aprox 1 (por inv_X_tt) y en consecuencia Re_tp de Chen = Re_l o aprox
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      // como no estoy seguro que F sea exactamente la unidad como en subcooled_boiling... hago lo siguiente
      
      // 1era forma: considerando que es como si hubiera usado un dittus boelter, paper del IRIS (esto para mi esta muy mal)
      //wasora_call(mate_compute_helical_section_node_fluid_Nu_l(&Nu, node));
      //h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D * F;
      
      // 2da forma: esta la propone mate haciendo un mejor uso de la teoria
      // no mando a calcular con mate_compute_helical_section_node_fluid_Nu_l ya que necesito Re_tp
      helical_section = (helical_section_t *) node->section->section_params;
      
      // TODO: extender la funcion de compute Nu_l para que reciba el Re a usar como argumento o el F quizas...
      Nu  = 1.0 / 41.0;
      Nu *= pow(mate_get_fluid_Pr(node->fluid_properties), 0.4);
      Nu *= pow(mate_get_fluid_Re_l(node->fluid_properties) * pow(F, 1.25), 5.0 / 6.0);
      Nu *= pow(node->section->D / 2.0 / helical_section->R, 1.0 / 12.0);
      Nu *= (1.0 + 0.061 / (pow(mate_get_fluid_Re_l(node->fluid_properties) * pow(F, 1.25) * pow(node->section->D / 2.0 / helical_section->R, 2.5), 1.0 / 6.0)));
      
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case saturated_boiling:
      
      // primero se calculan los dos parametros de Chen
      wasora_call(mate_compute_Chen_F_and_S(&F, &S, node));
      
      // 1era forma: considerando que es como si hubiera usado un dittus boelter, paper del IRIS (esto para mi esta muy mal)
      //mate_call(mate_compute_helical_section_node_sfluid_Nu_l(&Nu, node));
      //h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D * F;
      
      // 2da forma: esta la propone mate haciendo un mejor uso de la teoria
      // no mando a calcular con mate_compute_helical_section_node_fluid_Nu_l ya que necesito Re_tp
      helical_section = (helical_section_t *) node->section->section_params;
      
      // TODO: extender la funcion de compute Nu_l para que reciba el Re a usar como argumento o el F quizas...
      Nu  = 1.0 / 41.0;
      Nu *= pow(mate_get_fluid_Pr(node->fluid_properties), 0.4);
      Nu *= pow(mate_get_fluid_Re_l(node->fluid_properties) * pow(F, 1.25), 5.0 / 6.0);
      Nu *= pow(node->section->D / 2.0 / helical_section->R, 1.0 / 12.0);
      Nu *= (1.0 + 0.061 / (pow(mate_get_fluid_Re_l(node->fluid_properties) * pow(F, 1.25) * pow(node->section->D / 2.0 / helical_section->R, 2.5), 1.0 / 6.0)));
      
      h_mac = Nu * mate_get_fluid_k_l(node->fluid_properties) / node->section->D;
      
      wasora_call(mate_compute_Chen_h_mic(&h_mic, node));
      h_mic *= S;
      
      // finalmente sumamos ambas contribuciones
      h_tp = h_mic + h_mac;
      
      // y el Nu lo expreso en funcion de la conductividad del liquido
      Nu = h_tp * node->section->D / mate_get_fluid_k_l(node->fluid_properties);
      break;
      
    case single_phase_vapor:
      
      wasora_call(mate_compute_helical_section_node_fluid_Nu_v(&Nu, node));
      break;
  }
  
/*
  if(mate_get_solid_T_m(node->solid_properties) < mate_get_fluid_T(node->fluid_properties)) {
    
    fprintf(stderr, "error: la transferencia de calor en condensacion aun no se encuentra implementada en el codigo\n");
    return MATE_RUNTIME_ERROR;
    
  } else {
    
    mate_call(mate_compute_Chen_h_mic(&h_mic, node));
    h_mic *= S;
    
  }
*/
  
  wasora_call(mate_set_fluid_Nu(node->fluid_properties, Nu));
  wasora_var(mate.vars.Nu) = mate_get_fluid_Nu(node->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_helical_section_node_fluid_Nu_l(double *Nu, mate_node_t *node) {
  
  helical_section_t *helical_section = (helical_section_t *) node->section->section_params;
  
  *Nu  = 1.0 / 41.0;
  // sinceramente tengo una idea de como llega a este termino el paper de IRIS, pero no estoy seguro asi que pongo la posta del paper de Mori y Nakayama.
  //*Nu *= mate_get_fluid_Pr(node->fluid_properties) / (pow(mate_get_fluid_Pr(node->fluid_properties), 0.6) - 0.062);
  *Nu *= pow(mate_get_fluid_Pr(node->fluid_properties), 0.4);
  *Nu *= pow(mate_get_fluid_Re_l(node->fluid_properties), 5.0 / 6.0);
  *Nu *= pow(node->section->D / 2.0 / helical_section->R, 1.0 / 12.0);
  *Nu *= (1.0 + 0.061 / (pow(mate_get_fluid_Re_l(node->fluid_properties) * pow(node->section->D / 2.0 / helical_section->R, 2.5), 1.0 / 6.0)));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_helical_section_node_fluid_Nu_v(double *Nu, mate_node_t *node) {
  
  helical_section_t *helical_section = (helical_section_t *) node->section->section_params;
  
  *Nu  = 1.0 / 26.2;
  *Nu *= mate_get_fluid_Pr(node->fluid_properties) / (pow(mate_get_fluid_Pr(node->fluid_properties), 0.6) - 0.074);
  *Nu *= pow(mate_get_fluid_Re_v(node->fluid_properties), 4.0 / 5.0);
  *Nu *= pow(node->section->D / 2.0 / helical_section->R, 1.0 / 10.0);
  *Nu *= (1.0 + 0.098 / (pow(mate_get_fluid_Re_v(node->fluid_properties) * pow(node->section->D / 2.0 / helical_section->R, 2.0), 1.0 / 5.0)));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_Chen_F_and_S(double *F, double *S, mate_node_t *node) {
  
  double inv_X_tt;
  double Re_tp;
  
  wasora_call(mate_compute_inv_martinelli_parameter(&inv_X_tt, node));
  
  // ratio of the two-phase Re to the liquid Re elevado a la 0.8
  *F = (inv_X_tt > 0.1) ? 2.35 * pow(inv_X_tt + 0.213, 0.736) : 1.0;
  
  // Chen Re two-phase
  Re_tp = mate_get_fluid_Re_l(node->fluid_properties) * pow(*F, 1.25);
  
  // el suppression factor
  *S = 1.0 / (1.0 + 2.53e-6 * pow(Re_tp, 1.17));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_inv_martinelli_parameter(double* inv_X_tt, mate_node_t *node) {
  
  // inverse Martinelli parameter
  *inv_X_tt  = pow(mate_get_fluid_x(node->fluid_properties) / (1.0 - mate_get_fluid_x(node->fluid_properties)), 0.9);
  *inv_X_tt *= pow(mate_get_fluid_rho_l(node->fluid_properties) / mate_get_fluid_rho_v(node->fluid_properties), 0.5);
  *inv_X_tt *= pow(mate_get_fluid_mu_v(node->fluid_properties) / mate_get_fluid_mu_l(node->fluid_properties), 0.1);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_Chen_h_mic(double *h_mic, mate_node_t *node) {
  
  double DT, Dp;
  
  *h_mic  = 0.00122;
  *h_mic *= pow(mate_get_fluid_k_l(node->fluid_properties), 0.79);
  *h_mic *= pow(mate_get_fluid_cp_l(node->fluid_properties), 0.45);
  *h_mic *= pow(mate_get_fluid_rho_l(node->fluid_properties), 0.49);
  *h_mic /= pow(mate_get_fluid_mu_l(node->fluid_properties), 0.29);
  *h_mic /= pow(mate_get_fluid_rho_v(node->fluid_properties), 0.24);
  *h_mic /= pow(mate_get_fluid_h_g(node->fluid_properties) - mate_get_fluid_h_f(node->fluid_properties), 0.24);
  *h_mic /= pow(mate_get_fluid_sigma(node->fluid_properties), 0.50);
  
  DT = mate_get_solid_T_i(node->solid_properties) - mate_get_fluid_T(node->fluid_properties);
  *h_mic *= pow(DT, 0.24);
  
  Dp = freesteam_region4_psat_T(mate_get_solid_T_i(node->solid_properties)) - freesteam_region4_psat_T(mate_get_fluid_T(node->fluid_properties));
  *h_mic *= pow(Dp, 0.75);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_void_fraction(mate_node_t *node) {
  
  double alpha;
  
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      alpha = 0.0;
      break;
      
    case attached_bubble:
      
      // suponemos x = 0
      alpha = 0.0;
      break;
      
    case bubble_departure:
      
      wasora_call(mate_compute_CISE_void_fraction(&alpha, node));
      break;
      
    case saturated_boiling:
      
      wasora_call(mate_compute_CISE_void_fraction(&alpha, node));
      break;
      
    case single_phase_vapor:
      
      alpha = 1.0;
      break;
  }
  
  wasora_call(mate_set_fluid_alpha(node->fluid_properties, alpha));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_CISE_void_fraction(double *alpha, mate_node_t *node) {
  
  double rho_l, rho_v, x;
  double Re, We;
  double E1, E2;
  double beta, y;
  double S;
  
  // por comodidad llamamos una unica vez
  x = mate_get_fluid_x(node->fluid_properties);
  
  rho_l = mate_get_fluid_rho_l(node->fluid_properties);
  rho_v = mate_get_fluid_rho_v(node->fluid_properties);
  
  // en ninguno de los dos modelos (HEM y SEP) hay dependencia con la geometria
  switch(mate.flow_model) {
    
    case HEM:
      
      // Slip Ratio
      S = 1.0;
      break;
      
      // si el modelo es de separate flow, usar correlacion de CISE para Slip Ratio
    case SEP:
      
      // el Re tiene que ser con el mass flux total (liquid + gas)
      Re  = mate_get_fluid_Re_l(node->fluid_properties);
      Re /= (1.0 - x);
      
      // calculamos el numero We
      We = pow(mate_get_fluid_mass_flux_m(node->fluid_properties), 2.0) * node->section->D / mate_get_fluid_sigma(node->fluid_properties) / rho_l;
      
      // calculamos los parametros E1 y E2
      E1 = 1.578 * pow(Re, -0.19) * pow(rho_l / rho_v, 0.22);
      E2 = 0.0273 * We * pow(Re, -0.51) * pow(rho_l / rho_v, -0.08);
      
      // volume fraction
      beta  = rho_l * x;
      beta /= (rho_l * x + rho_v * (1.0 - x));
      
      y = beta / (1.0 - beta);
      
      // Slip Ratio
      S = (y > ((1.0 - E2) / pow(E2, 2.0))) ? 1.0 : 1.0 + E1 * sqrt(y / (1.0 + y * E2) - y * E2);
      break;
  }
  
  // void fraction
  *alpha  = rho_l * x;
  *alpha /= (rho_l * x + rho_v * (1.0 - x) * S);
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_mixture_densities(mate_node_t *node) {
  
  double alpha, x;
  double rho_l, rho_v;
  double rho_m;
  double rho_m_plus;
  
  // titulo real
  x = mate_get_fluid_x(node->fluid_properties);
  
  // fraccion de vacio
  alpha = mate_get_fluid_alpha(node->fluid_properties);
  
  // lo hago mirando regimen por regimen para que sea mas facil entender
  switch(mate_get_fluid_regime(node->fluid_properties)) {
    
    case single_phase_liquid:
      
      rho_m = rho_m_plus = mate_get_fluid_rho_l(node->fluid_properties);
      break;
      
    case attached_bubble:
      
      // como x = 0 y alpha = 0 aunque haya burbujas
      rho_m = rho_m_plus = mate_get_fluid_rho_l(node->fluid_properties);
      break;
      
    case bubble_departure:
      
      rho_l = mate_get_fluid_rho_l(node->fluid_properties);
      rho_v = mate_get_fluid_rho_v(node->fluid_properties);
      
      rho_m = alpha * rho_v + (1.0 - alpha) * rho_l;
      
      rho_m_plus  = pow(1.0 - x, 2.0) / (1.0 - alpha) / rho_l;
      rho_m_plus += pow(x, 2.0) / alpha / rho_v;
      rho_m_plus  = 1.0 / rho_m_plus;
      
      break;
      
    case saturated_boiling:
      
      rho_l = mate_get_fluid_rho_l(node->fluid_properties);
      rho_v = mate_get_fluid_rho_v(node->fluid_properties);
      
      rho_m = alpha * rho_v + (1.0 - alpha) * rho_l;
      
      rho_m_plus  = pow(1.0 - x, 2.0) / (1.0 - alpha) / rho_l;
      rho_m_plus += pow(x, 2.0) / alpha / rho_v;
      rho_m_plus  = 1.0 / rho_m_plus;
      
      break;
      
    case single_phase_vapor:
      
      rho_m = rho_m_plus = mate_get_fluid_rho_v(node->fluid_properties);
      break;
  }
    
  wasora_call(mate_set_fluid_rho_m(node->fluid_properties, rho_m));
  wasora_call(mate_set_fluid_rho_m_plus(node->fluid_properties, rho_m_plus));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_fluid_balances(mate_node_t *node) {
  
  double p, h;
  
  // calculamos la presion del nodo
  wasora_call(mate_compute_node_fluid_momentum_balance(&p, node));
  
  // calculamos la entalpia del nodo
  wasora_call(mate_compute_node_fluid_energy_balance(&h, node));
  
  // una vez obtenidas la presion y entalpia
  wasora_call(mate_set_fluid_properties_ph(node->fluid_properties, p, h));
  
  return WASORA_RUNTIME_OK;
}