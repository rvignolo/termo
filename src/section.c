/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's section-related routines
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

section_t *mate_define_section(const char *name, int type) {
  
  section_t *section;
  
  // primero hacemos que wasora checkee sus elementos
  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  // y ahora mate
  if (mate_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  section = calloc(1, sizeof(section_t));
  section->name = strdup(name);
  section->type = type;
  
  switch(section->type) {
    case straight_section_type:
      section->section_params = calloc(1, sizeof(straight_section_t));
      break;
      
    case circular_section_type:
      section->section_params = calloc(1, sizeof(circular_section_t));
      break;
      
    case helical_section_type:
      section->section_params = calloc(1, sizeof(helical_section_t));
      break;
  }
  
  section->solid_material = calloc(1, sizeof(solid_material_t));
  
  HASH_ADD_KEYPTR(hh, mate.sections, section->name, strlen(section->name), section);

  return section;
}

// retorna null si no encuentra
section_t *mate_get_section_ptr(const char *name) {
  section_t *section;
  HASH_FIND_STR(mate.sections, name, section);
  return section;
}

int mate_instruction_section(void *arg) {
  
  section_t *section = (section_t *) arg;
  
  if (section->initialized) {
    return WASORA_RUNTIME_OK;
  }
  
  wasora_call(mate_evaluate_section_expressions(section));
  
  wasora_call(mate_allocate_section_objects(section));

  // si la malla es special, tenemos que leer los nodos
  if (section->special_mesh) {
    wasora_call(mate_read_section_nodes_from_file(section));
  } else {
    wasora_call(mate_create_section_structured_nodes(section));
  }
  
  wasora_call(mate_compute_section_volumes(section));
  
  // asociamos los backward volumes a cada nodo
  wasora_call(mate_assign_nodes_backward_volume(section));
  
  // decimos que la seccion esta inicializada ya para siempre
  section->initialized = 1;
  
  return WASORA_RUNTIME_OK;
}

int mate_evaluate_section_expressions(section_t *section) {
  
  straight_section_t *straight_section;
  circular_section_t *circular_section;
  helical_section_t *helical_section;
  
  // tenemos que resolver las expresiones
  section->D = wasora_evaluate_expression(section->expr_D);
  section->e = wasora_evaluate_expression(section->expr_e);
  
  // check
  if (!(section->D > 0)) {
    wasora_push_error_message("diameter should be greater than zero instead of '%e'", section->D);
    return WASORA_RUNTIME_ERROR;
  }
  
  if (!(section->e > 0)) {
    wasora_push_error_message("thickness should be greater than zero instead of '%e'", section->e);
    return WASORA_RUNTIME_ERROR;
  }
  
  if (section->solid_material->expr_epsilon == NULL) {
    section->solid_material->epsilon = 0.0;
  } else {
    section->solid_material->epsilon = wasora_evaluate_expression(section->solid_material->expr_epsilon);
  }
  
  if (section->solid_material->epsilon < 0) {
    wasora_push_error_message("roughness should not be negative '%e'", section->solid_material->epsilon);
    return WASORA_RUNTIME_ERROR;
  }
  
  // si no se dio el file, la malla no es special
  if (!section->special_mesh) {
    
    straight_section = (straight_section_t *) section->section_params;
    
    // se tuvieron que proveer las siguientes expresiones
    section->L = wasora_evaluate_expression(section->expr_L);
    section->n_nodes = wasora_evaluate_expression(section->expr_n_nodes);
    straight_section->theta = wasora_evaluate_expression(straight_section->expr_theta);
    
    // y checkeamos que todo este manzana
    if (!(section->L > 0))  {
      wasora_push_error_message("section lenght should be greater than zero instead of '%e'", section->L);
      return WASORA_RUNTIME_ERROR;
    }
    
    if (!(section->n_nodes > 1))  {
      wasora_push_error_message("number of nodes should be greater than one instead of '%d'", section->n_nodes);
      return WASORA_RUNTIME_ERROR;
    }
    
    if (straight_section->theta < -90 || straight_section->theta > 90)  {
      wasora_push_error_message("section inclination angle should be in between -90 and 90 degrees instead of '%e'", straight_section->theta);
      return WASORA_RUNTIME_ERROR;
    }
    
  } else {
    
    // pero si se dio el file, se tuvieron que proveer las siguientes expresiones
    section->first_node_id = wasora_evaluate_expression(section->expr_first_node_id);
    section->last_node_id = wasora_evaluate_expression(section->expr_last_node_id);
    
    // check
    if (section->last_node_id <= section->first_node_id) {
      wasora_push_error_message("first node id should be smaller than last node id");
      return WASORA_RUNTIME_ERROR;
    }
    
    // si se dio el file y ademas la seccion es de tipo circular
    if (section->type == circular_section_type) {
       circular_section = (circular_section_t *) section->section_params;
       
       circular_section->R = wasora_evaluate_expression(circular_section->expr_R);
       
       // check
       if (!(circular_section->R > 0))  {
         wasora_push_error_message("radius should be greater than zero instead of '%e'", circular_section->R);
         return WASORA_RUNTIME_ERROR;
       }
       
       // si se dio el file y ademas la seccion es de tipo helicoidal
    } else if (section->type == helical_section_type) {
      
      helical_section = (helical_section_t *) section->section_params;
      
      helical_section->R = wasora_evaluate_expression(helical_section->expr_R);
      helical_section->p = wasora_evaluate_expression(helical_section->expr_p);
      
      // check
      if (!(helical_section->R > 0))  {
        wasora_push_error_message("radius should be greater than zero instead of '%e'", helical_section->R);
        return WASORA_RUNTIME_ERROR;
      }
      
      if (!(helical_section->p > 0))  {
        wasora_push_error_message("pitch should be greater than zero instead of '%e'", helical_section->p);
        return WASORA_RUNTIME_ERROR;
      }
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_allocate_section_objects(section_t *section) {
  
  // si la malla es special, hay que contar los nodos
  if (section->special_mesh) {
    
    section->n_volumes = section->last_node_id - section->first_node_id;
    section->n_nodes = section->n_volumes + 1; 
    
  } else {
    
    // pero si no es special, solo hay que determinar el numero de volumenes
    // ya que los nodos se proveen por input
    section->n_volumes = section->n_nodes - 1;
  }
  
  // allocamos la memoria necesaria para cargar los nodos
  section->node = calloc(section->n_nodes, sizeof(mate_node_t));
  
  // allocamos la memoria necesaria para los volumenes
  section->fluid_volume = calloc(section->n_volumes, sizeof(mate_volume_t));
  section->solid_volume = calloc(section->n_volumes, sizeof(mate_volume_t));
  
  return WASORA_RUNTIME_OK;
}
