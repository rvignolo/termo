/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's get-related routines
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

int mate_get_fluid_region(fluid_properties_t *fluid_properties) {
  
  return freesteam_region(fluid_properties->S);
}

int mate_get_fluid_regime(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->regime;
}

double mate_get_fluid_p(fluid_properties_t *fluid_properties) {
  
  return freesteam_p(fluid_properties->S);
}

double mate_get_node_fluid_p(mate_node_t *node) {
  
  return mate_get_fluid_p(node->fluid_properties);
}

double mate_get_fluid_T(fluid_properties_t *fluid_properties) {
  
  return freesteam_T(fluid_properties->S);
}

double mate_get_node_fluid_T(mate_node_t *node) {
  
  return mate_get_fluid_T(node->fluid_properties);
}

double mate_get_fluid_h(fluid_properties_t *fluid_properties) {
  
  return freesteam_h(fluid_properties->S);
}

double mate_get_node_fluid_h(mate_node_t *node) {
  
  return mate_get_fluid_h(node->fluid_properties);
}

double mate_get_fluid_rho(fluid_properties_t *fluid_properties) {
  
  // esta funcion solo debe ser llamada en las regiones 1 y 2
  // mas alla de que freesteam nos devuelve un valor en region 4.
  // esto es porque mi densidad de mezcla se calcula con alpha.
  return freesteam_rho(fluid_properties->S);
}

double mate_get_fluid_mu(fluid_properties_t *fluid_properties) {
  
  // esta funcion solo debe ser llamada en las regiones 1 y 2
  // aunque freesteam arrojara un warning si se llama en region 4.
  // mate utiliza aprox. de McAdadms para la region 4.
  return freesteam_mu(fluid_properties->S);
}

double mate_get_fluid_cp(fluid_properties_t *fluid_properties) {
  
  // esta funcion solo debe ser llamada en las regiones 1 y 2
  // aunque podria llamarse en la region 4. sin embargo, no 
  // veo necesario este uso en ninguna correlacion.
  return freesteam_cp(fluid_properties->S);
}

double mate_get_fluid_k(fluid_properties_t *fluid_properties) {
  
  // esta funcion solo debe ser llamada en las regiones 1 y 2
  // aunque freesteam arrojara un warning si se llama en region 4.
  // mate no utiliza ninguna aprox. para la region 4, ya que no es
  // requerida para ninguna correlacion
  return freesteam_k(fluid_properties->S);
}

double mate_get_fluid_sigma(fluid_properties_t *fluid_properties) {
  
  // see surftens.c in freesteam
  // calcula la tension superficial entre las fases de vapor y agua
  // a partir de la temperatura
  return freesteam_surftens_T(mate_get_fluid_T(fluid_properties));
}

double mate_get_fluid_x(fluid_properties_t *fluid_properties) {
  
  // x es el titulo masico, que puede o no coincidir con el titulo
  // termodinamico dependiendo si usamos o no ebullicion subenfriada
  return fluid_properties->x;
}

double mate_get_fluid_xe(fluid_properties_t *fluid_properties) {
  
  // xe es el titulo termodinamico y coincide con el titulo entregado
  // por freesteam solo en la region 4, dado que en las regiones 1 y 2
  // es negativo y mayor a 1 respectivamente y freesteam no los calcula.
  // por ese motivo, los calcula mate.
  return fluid_properties->xe;
}

double mate_get_fluid_alpha(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->alpha;
}

double mate_get_fluid_mass_flow_m(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flow_m;
}

double mate_get_fluid_mass_flow_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flow_l;
}

double mate_get_fluid_mass_flow_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flow_v;
}

double mate_get_fluid_mass_flux_m(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flux_m;
}

double mate_get_fluid_mass_flux_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flux_l;
}

double mate_get_fluid_mass_flux_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mass_flux_v;
}

double mate_get_fluid_h_f(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->h_f;
}

double mate_get_fluid_h_g(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->h_g;
}

double mate_get_fluid_h_m_plus(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->h_m_plus;
}

double mate_get_fluid_rho_m(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_m;
}

double mate_get_fluid_rho_f(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_f;
}

double mate_get_fluid_rho_g(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_g;
}

double mate_get_fluid_rho_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_l;
}

double mate_get_fluid_rho_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_v;
}

double mate_get_fluid_rho_m_plus(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->rho_m_plus;
}

double mate_get_fluid_mu_tp(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mu_tp;
}

double mate_get_fluid_mu_f(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mu_f;
}

double mate_get_fluid_mu_g(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mu_g;
}

double mate_get_fluid_mu_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mu_l;
}

double mate_get_fluid_mu_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->mu_v;
}

double mate_get_fluid_cp_f(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->cp_f;
}

double mate_get_fluid_cp_g(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->cp_g;
}

double mate_get_fluid_cp_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->cp_l;
}

double mate_get_fluid_cp_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->cp_v;
}

double mate_get_fluid_k_f(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->k_f;
}

double mate_get_fluid_k_g(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->k_g;
}

double mate_get_fluid_k_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->k_l;
}

double mate_get_fluid_k_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->k_v;
}

double mate_get_fluid_Re_tp(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->Re_tp;
}

double mate_get_fluid_Re_l(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->Re_l;
}

double mate_get_fluid_Re_v(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->Re_v;
}

double mate_get_fluid_Pr(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->Pr;
}

double mate_get_fluid_Nu(fluid_properties_t *fluid_properties) {
  
  return fluid_properties->Nu;
}




/** a partir de aca son cosas geometricas del fluido */
// las get Af no las copie, nada geometrico basicamente




/** a partir de aca listo las cosas del solido, ver mate viejo */

double mate_get_solid_T_i(solid_properties_t *solid_properties) {
  
  return solid_properties->T_i;
}

double mate_get_solid_T_o(solid_properties_t *solid_properties) {
  
  return solid_properties->T_o;
}

double mate_get_solid_T_m(solid_properties_t *solid_properties) {
  
  return solid_properties->T_m;
}

double mate_get_node_solid_T_m(mate_node_t *node) {
  
  return mate_get_solid_T_m(node->solid_properties);
}

double mate_get_solid_k(solid_properties_t *solid_properties) {
  
  return solid_properties->k;
}