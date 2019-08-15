/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's pipe-related routines
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

pipe_t *mate_define_pipe(const char *name) {
  
  pipe_t *pipe;
  char *function_name;
  
  // primero hacemos que wasora checkee sus elementos
  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  // y ahora mate
  if (mate_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  pipe = calloc(1, sizeof(pipe_t));
  pipe->name = strdup(name);
  
  // tambien definimos las funciones resultado asociadas al pipe (en funcion de coord curvilinea seran)
  // ya que en el input pueden pedirse para imprimir o algo asi y deben existir como wasora functions
  function_name = malloc(strlen(name)+32);
  
  sprintf(function_name, "pi_%s", name);
  if ((pipe->result_functions.pi = wasora_define_function(function_name, 1)) == NULL) {
    return NULL;
  }
  
  sprintf(function_name, "hi_%s", name);
  if ((pipe->result_functions.hi = wasora_define_function(function_name, 1)) == NULL) {
    return NULL;
  }
  
  sprintf(function_name, "Ti_%s", name);
  if ((pipe->result_functions.Ti = wasora_define_function(function_name, 1)) == NULL) {
    return NULL;
  }
  
  sprintf(function_name, "Tw_%s", name);
  if ((pipe->result_functions.Tw = wasora_define_function(function_name, 1)) == NULL) {
    return NULL;
  }
  
  free(function_name);
  
  HASH_ADD_KEYPTR(hh, mate.pipes, pipe->name, strlen(pipe->name), pipe);
  
  return pipe;
}

// retorna null si no encuentra
pipe_t *mate_get_pipe_ptr(const char *name) {
  pipe_t *pipe;
  HASH_FIND_STR(mate.pipes, name, pipe);
  return pipe;
}

int mate_append_component_to_pipe(component_list_item_t **list, int component_type, void *component) {
  
  component_list_item_t *item = calloc(1, sizeof(component_list_item_t));
  
  item->type = component_type;
  item->component = component;
  
  LL_APPEND(*list, item);
  
  return WASORA_PARSER_OK;
}

int mate_instruction_pipe(void *arg) {
  
  pipe_t *pipe = (pipe_t *) arg;
  
  if (pipe->initialized) {
    return WASORA_RUNTIME_OK;
  }
  
  // calculamos la coordenada curvilinea de cada nodo sobre cada pipe
  wasora_call(mate_compute_nodes_s_coordinate(pipe));
  
  pipe->initialized = 1;
  
  return WASORA_RUNTIME_OK;
}