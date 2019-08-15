/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's set-related routines
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

// las funciones de copy en definitiva usan sets, por eso las agrego aqui
int mate_safe_copy_fluid_properties(fluid_properties_t *fluid_prop_1, fluid_properties_t *fluid_prop_2) {
  
  int region;
  
  region = mate_get_fluid_region(fluid_prop_2);
  
  if (region == IAPWS_REGION_1 || region == IAPWS_REGION_2) {
    
    wasora_call(mate_copy_fluid_properties_pT(fluid_prop_1, fluid_prop_2));
    
  } else if (region == IAPWS_REGION_4) {
    
    wasora_call(mate_copy_fluid_properties_ph(fluid_prop_1, fluid_prop_2));
    
  } else {
    wasora_push_error_message("IAPWS region '%d' not expected when copying fluid properties", region);
    return WASORA_RUNTIME_ERROR;
  }
  
  return WASORA_RUNTIME_OK;
}

// las funciones de copy en definitiva usan sets, por eso las agrego aqui
int mate_copy_fluid_properties_pT(fluid_properties_t *fluid_prop_1, fluid_properties_t *fluid_prop_2) {
  
  wasora_call(mate_set_fluid_properties_pT(fluid_prop_1, mate_get_fluid_p(fluid_prop_2), mate_get_fluid_T(fluid_prop_2)));
  
  return WASORA_RUNTIME_OK;
}

// las funciones de copy en definitiva usan sets, por eso las agrego aqui
int mate_copy_fluid_properties_ph(fluid_properties_t *fluid_prop_1, fluid_properties_t *fluid_prop_2) {
  
  wasora_call(mate_set_fluid_properties_ph(fluid_prop_1, mate_get_fluid_p(fluid_prop_2), mate_get_fluid_h(fluid_prop_2)));
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_properties_pT(fluid_properties_t *fluid_properties, double p, double T) {
  
  fluid_properties->S = freesteam_set_pT(p, T);
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_properties_ph(fluid_properties_t *fluid_properties, double p, double h) {
  
  fluid_properties->S = freesteam_set_ph(p, h);
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_x(fluid_properties_t *fluid_properties, double x) {
  
  fluid_properties->x =  x;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_xe(fluid_properties_t *fluid_properties, double xe) {
  
  fluid_properties->xe =  xe;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_alpha(fluid_properties_t *fluid_properties, double alpha) {
  
  fluid_properties->alpha =  alpha;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flow_m(fluid_properties_t *fluid_properties, double mass_flow_m) {
  
  fluid_properties->mass_flow_m =  mass_flow_m;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flow_l(fluid_properties_t *fluid_properties, double mass_flow_l) {
  
  fluid_properties->mass_flow_l =  mass_flow_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flow_v(fluid_properties_t *fluid_properties, double mass_flow_v) {
  
  fluid_properties->mass_flow_v =  mass_flow_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flux_m(fluid_properties_t *fluid_properties, double mass_flux_m) {
  
  fluid_properties->mass_flux_m =  mass_flux_m;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flux_l(fluid_properties_t *fluid_properties, double mass_flux_l) {
  
  fluid_properties->mass_flux_l =  mass_flux_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mass_flux_v(fluid_properties_t *fluid_properties, double mass_flux_v) {
  
  fluid_properties->mass_flux_v =  mass_flux_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_h_f(fluid_properties_t *fluid_properties, double h_f) {
  
  fluid_properties->h_f =  h_f;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_h_g(fluid_properties_t *fluid_properties, double h_g) {
  
  fluid_properties->h_g =  h_g;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_h_m_plus(fluid_properties_t *fluid_properties, double h_m_plus) {
  
  fluid_properties->h_m_plus =  h_m_plus;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_m(fluid_properties_t *fluid_properties, double rho_m) {
  
  fluid_properties->rho_m =  rho_m;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_f(fluid_properties_t *fluid_properties, double rho_f) {
  
  fluid_properties->rho_f =  rho_f;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_g(fluid_properties_t *fluid_properties, double rho_g) {
  
  fluid_properties->rho_g =  rho_g;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_l(fluid_properties_t *fluid_properties, double rho_l) {
  
  fluid_properties->rho_l =  rho_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_v(fluid_properties_t *fluid_properties, double rho_v) {
  
  fluid_properties->rho_v =  rho_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_rho_m_plus(fluid_properties_t *fluid_properties, double rho_m_plus) {
  
  fluid_properties->rho_m_plus =  rho_m_plus;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mu_tp(fluid_properties_t *fluid_properties, double mu_tp) {
  
  fluid_properties->mu_tp =  mu_tp;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mu_f(fluid_properties_t *fluid_properties, double mu_f) {
  
  fluid_properties->mu_f =  mu_f;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mu_g(fluid_properties_t *fluid_properties, double mu_g) {
  
  fluid_properties->mu_g =  mu_g;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mu_l(fluid_properties_t *fluid_properties, double mu_l) {
  
  fluid_properties->mu_l =  mu_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_mu_v(fluid_properties_t *fluid_properties, double mu_v) {
  
  fluid_properties->mu_v =  mu_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_cp_f(fluid_properties_t *fluid_properties, double cp_f) {
  
  fluid_properties->cp_f =  cp_f;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_cp_g(fluid_properties_t *fluid_properties, double cp_g) {
  
  fluid_properties->cp_g =  cp_g;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_cp_l(fluid_properties_t *fluid_properties, double cp_l) {
  
  fluid_properties->cp_l =  cp_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_cp_v(fluid_properties_t *fluid_properties, double cp_v) {
  
  fluid_properties->cp_v =  cp_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_k_f(fluid_properties_t *fluid_properties, double k_f) {
  
  fluid_properties->k_f =  k_f;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_k_g(fluid_properties_t *fluid_properties, double k_g) {
  
  fluid_properties->k_g =  k_g;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_k_l(fluid_properties_t *fluid_properties, double k_l) {
  
  fluid_properties->k_l =  k_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_k_v(fluid_properties_t *fluid_properties, double k_v) {
  
  fluid_properties->k_v =  k_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_Re_tp(fluid_properties_t *fluid_properties, double Re_tp) {
  
  fluid_properties->Re_tp =  Re_tp;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_Re_l(fluid_properties_t *fluid_properties, double Re_l) {
  
  fluid_properties->Re_l =  Re_l;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_Re_v(fluid_properties_t *fluid_properties, double Re_v) {
  
  fluid_properties->Re_v =  Re_v;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_Pr(fluid_properties_t *fluid_properties, double Pr) {
  
  fluid_properties->Pr =  Pr;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_fluid_Nu(fluid_properties_t *fluid_properties, double Nu) {
  
  fluid_properties->Nu =  Nu;
  
  return WASORA_RUNTIME_OK;
}







/** a partir de aca listo las cosas del solido, ver mate viejo */

int mate_copy_solid_properties(solid_properties_t *solid_prop_1, solid_properties_t *solid_prop_2) {
  
  wasora_call(mate_set_solid_T_m(solid_prop_1, mate_get_solid_T_m(solid_prop_2)));
  wasora_call(mate_set_solid_T_i(solid_prop_1, mate_get_solid_T_i(solid_prop_2)));
  wasora_call(mate_set_solid_T_o(solid_prop_1, mate_get_solid_T_o(solid_prop_2)));
  
  return WASORA_RUNTIME_OK;
}

int mate_set_solid_T_i(solid_properties_t *solid_properties, double T_i) {
  
  solid_properties->T_i = T_i;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_solid_T_o(solid_properties_t *solid_properties, double T_o) {
  
  solid_properties->T_o = T_o;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_solid_T_m(solid_properties_t *solid_properties, double T_m) {
  
  solid_properties->T_m = T_m;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_solid_k(solid_properties_t *solid_properties, double k) {
  
  solid_properties->k = k;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_special_vars(mate_node_t *node) {
  
  wasora_var(wasora_mesh.vars.x) = node->x[0];
  wasora_var(wasora_mesh.vars.y) = node->x[1];
  wasora_var(wasora_mesh.vars.z) = node->x[2];
  
  wasora_var(mate.vars.s) = node->s;
  
  wasora_var(mate.vars.Tw) = mate_get_solid_T_m(node->solid_properties);
  
  wasora_var(mate.vars.Re_tp) = mate_get_fluid_Re_tp(node->fluid_properties);
  wasora_var(mate.vars.Re_l) = mate_get_fluid_Re_l(node->fluid_properties);
  wasora_var(mate.vars.Re_v) = mate_get_fluid_Re_v(node->fluid_properties);
  
  wasora_var(mate.vars.Pr) = mate_get_fluid_Pr(node->fluid_properties);
  
  wasora_var(mate.vars.Nu) = mate_get_fluid_Nu(node->fluid_properties);
  
  return WASORA_RUNTIME_OK;
}