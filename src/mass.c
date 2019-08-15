/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's mass related routines
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

int mate_compute_lambda_factors(pipe_array_t *pipe_array) {
  
  int i;
  
  double pipe_dp;
  double mean_pipe_array_dp;
  //double dp_g;
  
  pipe_t *pipe;
  
  // computamos la caida de presion media
  wasora_call(mate_compute_pipe_array_mean_dp(&mean_pipe_array_dp, pipe_array));
  
  for ( i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    wasora_call(mate_compute_pipe_dp(&pipe_dp, pipe_array, i));
    //mate_call(mate_compute_pipe_dp_g(&dp_g, pipe_array, i));
    
    // TODO: analizar cual de los siguientes dos esquemas converge mas rapido
    //pipe->lambda = sqrt((mean_dp - dp_g) / (dp - dp_g));
    pipe->lambda = sqrt(mean_pipe_array_dp / pipe_dp);
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_Gamma_factor(pipe_array_t *pipe_array) {
  
  int i;
  
  double dummy_m_dot;
  
  pipe_t *pipe;
  
  dummy_m_dot = 0;
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    dummy_m_dot += pipe->lambda * mate_get_fluid_mass_flow_m(pipe->bc);
  }
  
  // computamos el factor de correccion global Gamma
  pipe_array->Gamma = mate_get_fluid_mass_flow_m(pipe_array->bc) / dummy_m_dot;
  
  return WASORA_RUNTIME_OK;
}

int mate_analyze_factors(pipe_array_t *pipe_array) {
  
  int i;
  
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    // si alguno no cumple la igualdad, retornamos falso
    if (gsl_fcmp(pipe->lambda, 1.0, MAX_RESIDUAL) != 0)
      return 0;
  }
  
  // si llegamos aca es porque todos los lamda equivalen a 1
  // nos queda mirar Gamma
  if (gsl_fcmp(pipe_array->Gamma, 1.0, MAX_RESIDUAL) != 0)
    return 0;
  
  // si llegamos aca, es que estamos en condiciones de retornar true
  return 1;
}

int mate_correct_mass_flows(pipe_array_t *pipe_array) {
  
  int i;
  
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    wasora_call(mate_set_fluid_mass_flow_m(pipe->bc, pipe_array->Gamma * pipe->lambda * mate_get_fluid_mass_flow_m(pipe->bc)));
  }
  
  return WASORA_RUNTIME_OK;
}