/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's node-related routines
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

int mate_read_section_nodes_from_file(section_t *section) {
  
  int i, line, id;
  int node_section_index;
  char *token;
  char data_line[BUFFER_SIZE*BUFFER_SIZE];
  mate_node_t *node;
  
  if (section->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(section->file));
  }
  
  // ahora leemos los datos
  line = 1;
  node_section_index = 0;
  while (wasora_read_data_line(section->file->pointer, data_line) != 0) {
    
    if (data_line[0] != '\0') {
      
      // tomamos el id del nodo
      if ((token = wasora_get_nth_token(data_line, section->column[0])) == NULL) {
        wasora_push_error_message("wrong-formatted file '%s' at line %d", section->file->name, line);
        return WASORA_RUNTIME_ERROR;
      }
      
      sscanf(token, "%d", &id);
      free(token);
      
      // nos fijamos si es un nodo que nos interesa
      if (!(id < section->first_node_id || id > section->last_node_id)) {
        
        node = &section->node[node_section_index];
        
        for (i = 0; i < 3; i++) {
          if ((token = wasora_get_nth_token(data_line, section->column[i+1])) == NULL) {
            wasora_push_error_message("wrong-formatted file '%s' at line %d", section->file->name, line);
            return WASORA_RUNTIME_ERROR;
          }
          
          // rellenamos las coordenadas espaciales del nodo que estamos leyendo del file
          sscanf(token, "%lf", &node->x[i]);
          free(token);
        }
        
        // la seccion asociada al nodo
        node->section = section;
        
        // nos preparamos para el proximo nodo
        node_section_index++;
      }
      
      line++;
    }
  }
  
  // otra seccion puede depender del mismo file, asi que lo rebobinamos
  rewind(section->file->pointer);
  
  return WASORA_RUNTIME_OK;
}

int mate_create_section_structured_nodes(section_t *section) {
  
  int i, n;
  double ds;
  double delta[3];
  
  mate_node_t *node;
  straight_section_t *straight_section;
  
  straight_section = (straight_section_t *) section->section_params;
  
  // a partir de la longitud y numero de nodos, definimos uniform delta
  ds = section->L / section->n_volumes;
  
  // luego los uniform dx, dy y dz
  delta[0] = ds * cos(straight_section->theta);
  delta[1] = 0;
  delta[2] = ds * sin(straight_section->theta);
  
  for (n = 0; n < section->n_nodes; n++) {
    
    node = &section->node[n];
    
    for (i = 0; i < 3; i++)
      node->x[i] = n * delta[i];
    
    // la seccion asociada al nodo
    node->section = section;
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_assign_nodes_backward_volume(section_t *section) {
  
  int i;
  
  mate_node_t *node;
  
  for (i = 0; i < section->n_nodes; i++) {
    
    node = &section->node[i];
    
    if (i == 0) {
    //if (node->node_section_index == 0) {  // este indice se rellena mas adelante recien
      
      // al primer nodo de cada seccion le asignamos un forward volume
      node->backward_fluid_volume = &section->fluid_volume[i];
      node->backward_solid_volume = &section->solid_volume[i];
      
    } else {
      
      // al resto le asignamos el backward volume
      node->backward_fluid_volume = &section->fluid_volume[i-1];
      node->backward_solid_volume = &section->solid_volume[i-1];
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_nodes_s_coordinate(pipe_t *pipe) {
  
  int i;
  double s;
  component_list_item_t *associated_component;
  //form_loss_t *form_loss;
  section_t *section;
  mate_node_t *node;
  
  s = 0;
  LL_FOREACH(pipe->associated_components, associated_component) {
    
    switch (associated_component->type) {
      
      case form_loss_type:
        
        //form_loss = (form_loss_t *) associated_component->component;
        // una form loss no contribuye a la coordenada curvilinea
        // por lo que no hacemos nada
        break;
        
      case section_type:
        
        section = (section_t *) associated_component->component;
        
        for (i = 0; i < section->n_nodes; i++) {
          
          node = &section->node[i];
          
          // el primer nodo de cada seccion no contribuye a la sumatoria
          if (i != 0)
            s += node->backward_fluid_volume->L;
          
          node->s = s;
        }
        break;
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_allocate_pipe_array_node_properties(pipe_array_t *pipe_array) {
  
  int i;
  
  mate_node_t *node;
  
  // recorremos todos los nodos del problema
  for (i = 0; i < pipe_array->solver->n_total_nodes; i++) {
    
    // apuntamos al correspondiente nodo
    node = pipe_array->solver->node_by_pipe_array_index[i];
    
    if (i != node->node_pipe_array_index) {
      return WASORA_RUNTIME_ERROR;
    }
    
    // allocamos memoria para las propiedades del fluid node
      node->fluid_properties = calloc(1, sizeof(fluid_properties_t));
      
      // allocamos memoria para las propiedades del solid node
      node->solid_properties = calloc(1, sizeof(solid_properties_t));
  }
  
  return WASORA_RUNTIME_OK;
}