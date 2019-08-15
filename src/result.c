/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's results related routines
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

int mate_fill_result_functions_args(pipe_array_t *pipe_array) {
  
  int i;
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    wasora_call(mate_fill_result_function_args(pipe_array, i, pipe->result_functions.pi));
    wasora_call(mate_fill_result_function_args(pipe_array, i, pipe->result_functions.hi));
    wasora_call(mate_fill_result_function_args(pipe_array, i, pipe->result_functions.Ti));
    wasora_call(mate_fill_result_function_args(pipe_array, i, pipe->result_functions.Tw));
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_fill_result_function_args(pipe_array_t *pipe_array, int pipe, function_t *function) {
  
  int i;
  int node_pipe_array_index;
  mate_node_t *node;
  
  function->data_size = pipe_array->solver->n_nodes_per_pipe[pipe];
  
  // las funciones resultado son funcion de la coordenada curvilinea del pipe
  function->n_arguments = 1;
  function->data_argument = calloc(function->n_arguments, sizeof(double *));
  function->data_argument[0] = calloc(function->data_size, sizeof(double));
  function->data_value = calloc(function->data_size, sizeof(double));
  
  // TODO: puede que mas adelante estas funciones las defina con data_value apuntando a
  // vectores de wasora para que cuando estos se actualicen por shm, la funcion cambie 
  // y podamos tener acoplado con otra instancia de mate.
  
  for (i = 0; i < function->data_size; i++) {
    
    node_pipe_array_index = mate_compute_node_pipe_array_index(pipe_array, pipe, i);
    node = pipe_array->solver->node_by_pipe_array_index[node_pipe_array_index];
    
/*
    if (node->node_pipe_index == 0 || node->node_section_index != 0) {
      function->data_argument[0][i] = node->s;
    } else {
      function->data_argument[0][i] = 1.0e-7 * node->s;
    }
*/
    //ver como hacer para que no se solapen los puntos!
    function->data_argument[0][i] = (node->node_pipe_index == 0 || node->node_section_index != 0) ? node->s : node->s + 1.0e-12 * node->s;
  }
  
  function->type = type_pointwise_data;
  function->multidim_threshold = DEFAULT_MULTIDIM_INTERPOLATION_THRESHOLD;
  
  return WASORA_RUNTIME_OK;
}

int mate_fill_result_functions_values(pipe_array_t *pipe_array) {
  
  int i;
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    wasora_call(mate_fill_result_function_values(pipe_array, i, pipe->result_functions.pi, mate_get_node_fluid_p));
    //wasora_call(mate_fill_result_function_values(pipe_array, i, pipe->result_functions.hi, mate_compute_node_hi));
    wasora_call(mate_fill_result_function_values(pipe_array, i, pipe->result_functions.hi, mate_get_node_fluid_h));
    wasora_call(mate_fill_result_function_values(pipe_array, i, pipe->result_functions.Ti, mate_get_node_fluid_T));
    wasora_call(mate_fill_result_function_values(pipe_array, i, pipe->result_functions.Tw, mate_get_node_solid_T_m));
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_fill_result_function_values(pipe_array_t *pipe_array, int pipe, function_t *function, double (*func)(mate_node_t *)) {
  
  int i;
  int node_pipe_array_index;
  mate_node_t *node;
  
  for (i = 0; i < function->data_size; i++) {
    
    node_pipe_array_index = mate_compute_node_pipe_array_index(pipe_array, pipe, i);
    node = pipe_array->solver->node_by_pipe_array_index[node_pipe_array_index];
    
    function->data_value[i] = func(node);
  }
  
  return WASORA_RUNTIME_OK;
}