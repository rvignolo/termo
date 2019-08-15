/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's pipe array-related routines
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

pipe_array_t *mate_define_pipe_array(const char *name, int n_pipes) {
  
  pipe_array_t *pipe_array;
  
  // primero hacemos que wasora checkee sus elementos
  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  // y ahora mate
  if (mate_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  pipe_array = calloc(1, sizeof(pipe_t));
  pipe_array->name = strdup(name);
  
  pipe_array->n_pipes = n_pipes;
  pipe_array->pipe = calloc(n_pipes, sizeof(pipe_t *));
  
  HASH_ADD_KEYPTR(hh, mate.pipe_arrays, pipe_array->name, strlen(pipe_array->name), pipe_array);
  
  return pipe_array;
}

// retorna null si no encuentra
pipe_array_t *mate_get_pipe_array_ptr(const char *name) {
  pipe_array_t *pipe_array;
  HASH_FIND_STR(mate.pipe_arrays, name, pipe_array);
  return pipe_array;
}

int mate_set_pipe_array_indexes(pipe_array_t *pipe_array) {
  
  int i, j;
  int node_section_index;
  int volume_section_index;
  int node_pipe_index;
  int volume_pipe_index;
  int form_loss_pipe_index;
  int node_pipe_array_index;
  int volume_pipe_array_index;
  int form_loss_system_index;
  int point_pipe_index;
  
  component_list_item_t *associated_component;
  form_loss_t *form_loss;
  section_t *section;
  
  pipe_t *pipe;
  
  node_pipe_array_index = volume_pipe_array_index = form_loss_system_index = 0;
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    node_pipe_index = volume_pipe_index = form_loss_pipe_index = point_pipe_index = 0;
    LL_FOREACH(pipe->associated_components, associated_component) {
      
      switch (associated_component->type) {
        
        case form_loss_type:
          
          form_loss = (form_loss_t *) associated_component->component;
          
          form_loss->form_loss_pipe_index = form_loss_pipe_index;
          form_loss_pipe_index++;
          
          form_loss->form_loss_pipe_array_index = form_loss_system_index;
          form_loss_system_index++;
          break;
          
        case section_type:
          
          section = (section_t *) associated_component->component;
          
          node_section_index = 0;
          for (j = 0; j < section->n_nodes; j++) {
            
            section->node[j].node_section_index = node_section_index;
            node_section_index++;
            
            section->node[j].node_pipe_index = node_pipe_index;
            node_pipe_index++;
            
            section->node[j].node_pipe_array_index = node_pipe_array_index;
            node_pipe_array_index++;
          }
          
          volume_section_index = 0;
          for (j = 0; j < section->n_volumes; j++) {
            
            section->fluid_volume[j].volume_section_index = volume_section_index;
            section->solid_volume[j].volume_section_index = volume_section_index;
            volume_section_index++;
            
            section->fluid_volume[j].volume_pipe_index = volume_pipe_index;
            section->solid_volume[j].volume_pipe_index = volume_pipe_index;
            volume_pipe_index++;
            
            section->fluid_volume[j].volume_pipe_array_index = volume_pipe_array_index;
            section->solid_volume[j].volume_pipe_array_index = volume_pipe_array_index;
            volume_pipe_array_index++;
          }
          break;
      }
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_node_pipe_array_index(pipe_array_t *pipe_array, int pipe, int node_pipe_index) {
  
  int i;
  int node_pipe_array_index;
  
  node_pipe_array_index = 0;
  for (i = 0; i < pipe; i++)
    node_pipe_array_index += pipe_array->solver->n_nodes_per_pipe[i];
  
  node_pipe_array_index += node_pipe_index;
  
  return node_pipe_array_index;
}

int mate_compute_form_loss_pipe_array_index(pipe_array_t *pipe_array, int pipe, int form_loss_pipe_index) {
  
  int i;
  int form_loss_pipe_array_index;
  
  form_loss_pipe_array_index = 0;
  for (i = 0; i < pipe; i++)
    form_loss_pipe_array_index += pipe_array->solver->n_form_losses_per_pipe[i];
  
  form_loss_pipe_array_index += form_loss_pipe_index;
  
  return form_loss_pipe_array_index;
}