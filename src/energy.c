/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's energy balance related routines
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

int mate_compute_node_fluid_energy_balance(double *h, mate_node_t *node) {
  
  double qr;
  
  mate_node_t *previous_node = node->backward_fluid_volume->initial_node;
  
  wasora_call(mate_compute_qr(&qr, node));
  
  *h = mate_get_fluid_h(previous_node->fluid_properties) + qr / mate_get_fluid_mass_flow_m(node->fluid_properties);
  
  // sin despreciar
  //*h  = mate_get_fluid_node_h_m(previous_node);
  //*h += mate_compute_qr(fluid_node, solid_node) / mate_get_fluid_node_mass_flow_m(fluid_node);
  //*h -= GRAVITY * (mate_get_node_pos_z(fluid_node) - mate_get_node_pos_z(previous_node));
  // falta el ultimo termino
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_qr(double *qr, mate_node_t *node) {
  
  double UA;
  
  wasora_call(mate_compute_global_UA(&UA, node));
  
  *qr = UA * (mate_compute_node_To(node) - mate_get_fluid_T(node->fluid_properties));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_global_UA(double *UA, mate_node_t *node) {
  
  *UA  = mate_compute_inner_convection_R(node);
  *UA += mate_compute_solid_conduction_R(node);
  *UA += mate_compute_outer_convection_R(node);
  *UA  = 1.0 / (*UA);
  
  return WASORA_RUNTIME_OK;
}

double mate_compute_inner_convection_R(mate_node_t *node) {
  
  return 1.0 / mate_compute_node_hi(node) / node->backward_fluid_volume->Ao;
}

double mate_compute_node_hi(mate_node_t *node) {
  
  double hi = mate_get_fluid_Nu(node->fluid_properties);
  
  // si es single_phase_vapor el h se calcula asi, pero en todos los otros casos se calcula asa
  switch(mate_get_fluid_regime(node->fluid_properties)) {
      
    case single_phase_vapor:
      
      hi *= mate_get_fluid_k_v(node->fluid_properties);
      break;
      
    default:
      hi *= mate_get_fluid_k_l(node->fluid_properties);
      break;
  }
  
  return hi / node->section->D;
}

double mate_compute_solid_conduction_R(mate_node_t *node) {
  
  return log(node->backward_solid_volume->Do / node->backward_solid_volume->Di) / 2.0 / M_PI / mate_get_solid_k(node->solid_properties) / node->backward_solid_volume->L;
}

double mate_compute_outer_convection_R(mate_node_t *node) {
  
  return 1.0 / mate_compute_node_ho(node) / node->backward_solid_volume->Ao;
}

double mate_compute_node_To(mate_node_t *node) {
  
  return wasora_evaluate_expression(node->section->pipe->boundary_expr.expr_To);
}

double mate_compute_node_ho(mate_node_t *node) {
  
  return wasora_evaluate_expression(node->section->pipe->boundary_expr.expr_ho);
}