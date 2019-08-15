/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's solid-related routines
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

int mate_compute_node_solid_k(mate_node_t *node) {
  
  double k;
  
  k = wasora_evaluate_expression(node->section->solid_material->expr_k);
  
  wasora_call(mate_set_solid_k(node->solid_properties, k));
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_solid_T(mate_node_t *node) {
  
  double qr, To, Ti, Tm;
  
  // el heat transfer rate
  wasora_call(mate_compute_qr(&qr, node));
  
  // la temperatura en la cara externa de la pared
  To = mate_compute_node_To(node) - qr * mate_compute_outer_convection_R(node);
  
  // la temperatura en la cara interna de la pared
  Ti = To - qr * mate_compute_solid_conduction_R(node);
  
  // y consideramos la temperatura del solido como la temperatura media en el volumen anular (suponiendo lineal la temperatura entre nodos)
  Tm  = node->backward_solid_volume->Di * (2.0 * Ti + To);
  Tm += node->backward_solid_volume->Do * (2.0 * To + Ti);
  Tm /= (3.0 *(node->backward_solid_volume->Di + node->backward_solid_volume->Do));
  
  wasora_call(mate_set_solid_T_i(node->solid_properties, Ti));
  wasora_call(mate_set_solid_T_o(node->solid_properties, To));
  wasora_call(mate_set_solid_T_m(node->solid_properties, Tm));
  
  wasora_var(mate.vars.Tw) = mate_get_solid_T_m(node->solid_properties);
  
  return WASORA_RUNTIME_OK;
}