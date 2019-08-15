/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's system-related routines
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

system_t *mate_define_system(const char *name) {
  
  system_t *system;
  
  // primero hacemos que wasora checkee sus elementos
  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  // y ahora mate
  if (mate_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }
  
  system = calloc(1, sizeof(system_t));
  system->name = strdup(name);
  
  HASH_ADD_KEYPTR(hh, mate.systems, system->name, strlen(system->name), system);
  
  // decimos que el ultimo sistema definido en un input es el principal de mate
  mate.main_system = system;
  
  return system;
}

// retorna null si no encuentra
system_t *mate_get_system_ptr(const char *name) {
  system_t *system;
  HASH_FIND_STR(mate.systems, name, system);
  return system;
}

int mate_append_pipe_array_to_system(pipe_array_list_item_t **list, pipe_array_t *pipe_array) {
  
  pipe_array_list_item_t *item = calloc(1, sizeof(pipe_array_list_item_t));
  
  item->pipe_array = pipe_array;
  
  LL_APPEND(*list, item);
  
  return WASORA_PARSER_OK;
}

int mate_init_system(system_t *system) {
  
  pipe_array_t *pipe_array;
  pipe_array_list_item_t *associated_pipe_array;
  
  if(system->initialized) {
    return WASORA_RUNTIME_OK;
  }
  
  // para cada pipe array del sistema, mandamos a:
  LL_FOREACH(system->associated_pipe_arrays, associated_pipe_array) {
    
    pipe_array = associated_pipe_array->pipe_array;
    
    // allocar la memoria necesaria en el solver de cada pipe array
    wasora_call(mate_allocate_pipe_array_solver(pipe_array));
    
    // rellenar los indices sobre cada nodo, volumen y, eventualmente, form loss en cada pipe array
    wasora_call(mate_set_pipe_array_indexes(pipe_array));
    
    // apuntar los objetos del solver correctamente
    wasora_call(mate_point_pipe_array_solver_objects(pipe_array));
    
    // allocar las propiedades sobre los nodos
    wasora_call(mate_allocate_pipe_array_node_properties(pipe_array));
    
    // rellenamos datos administrativos de las funciones resultado
    wasora_call(mate_fill_result_functions_args(pipe_array));
    
    pipe_array->initialized = 1;
  }
  
  system->initialized = 1;
  
  return WASORA_RUNTIME_OK;
}

int mate_set_system_boundary_conditions(system_t *system) {
  
  if (system->bc == NULL) {
    wasora_push_error_message("undefined boundary conditions for system '%s'", system->name);
    return WASORA_RUNTIME_ERROR;
  }
  
  system->bc->fluid_properties = calloc(1, sizeof(fluid_properties_t));
  
  switch (system->bc->type) {
    
    case ph:
      
      if (system->bc->expr_p != NULL && system->bc->expr_h != NULL) {
        
        system->bc->p = wasora_evaluate_expression(system->bc->expr_p);
        system->bc->h = wasora_evaluate_expression(system->bc->expr_h);
        
        if (system->bc->p > IAPWS97_PMAX) {
          wasora_push_error_message("pressure in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_PMAX, system->bc->p);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (!(system->bc->h > 0)) {
          wasora_push_error_message("specific entalphy in system '%s' boundary condition should be positive", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_ph(system->bc->p, system->bc->h);
        
      } else {
        wasora_push_error_message("both pressure and specific entalphy should be given for 'ph' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case ps:
      
      if (system->bc->expr_p != NULL && system->bc->expr_s != NULL) {
        
        system->bc->p = wasora_evaluate_expression(system->bc->expr_p);
        system->bc->s = wasora_evaluate_expression(system->bc->expr_s);
        
        if (system->bc->p > IAPWS97_PMAX) {
          wasora_push_error_message("pressure in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_PMAX, system->bc->p);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (!(system->bc->s > 0)) {
          wasora_push_error_message("specific entropy in system '%s' boundary condition should be positive", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_ps(system->bc->p, system->bc->s);
        
      } else {
        wasora_push_error_message("both pressure and specific entropy should be given for 'ps' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case pu:
      
      if (system->bc->expr_p != NULL && system->bc->expr_u != NULL) {
        
        system->bc->p = wasora_evaluate_expression(system->bc->expr_p);
        system->bc->u = wasora_evaluate_expression(system->bc->expr_u);
        
        if (system->bc->p > IAPWS97_PMAX) {
          wasora_push_error_message("pressure in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_PMAX, system->bc->p);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (!(system->bc->u > 0)) {
          wasora_push_error_message("specific internal energy in system '%s' boundary condition should be positive", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_pu(system->bc->p, system->bc->u);
        
      } else {
        wasora_push_error_message("both pressure and specific internal energy should be given for 'pu' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case pv:
      
      if (system->bc->expr_p != NULL && system->bc->expr_v != NULL) {
        
        system->bc->p = wasora_evaluate_expression(system->bc->expr_p);
        system->bc->v = wasora_evaluate_expression(system->bc->expr_v);
        
        if (system->bc->p > IAPWS97_PMAX) {
          wasora_push_error_message("pressure in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_PMAX, system->bc->p);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (!(system->bc->v > 0)) {
          wasora_push_error_message("specific volume in system '%s' boundary condition should be positive", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_pu(system->bc->p, system->bc->v);
        
      } else {
        wasora_push_error_message("both pressure and specific volume should be given for 'pv' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case pT:
      
      if (system->bc->expr_p != NULL && system->bc->expr_T != NULL) {
        
        system->bc->p = wasora_evaluate_expression(system->bc->expr_p);
        system->bc->T = wasora_evaluate_expression(system->bc->expr_T);
        
        if (system->bc->p > IAPWS97_PMAX) {
          wasora_push_error_message("pressure in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_PMAX, system->bc->p);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (system->bc->T < IAPWS97_TMIN) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at least '%e' instead of '%e'", system->name, IAPWS97_TMIN, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        } else if (system->bc->T > IAPWS97_TMAX) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_TMAX, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_pT(system->bc->p, system->bc->T);
        
      } else {
        wasora_push_error_message("both pressure and temperature should be given for 'pv' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case Ts:
      
      if (system->bc->expr_T != NULL && system->bc->expr_s != NULL) {
        
        system->bc->T = wasora_evaluate_expression(system->bc->expr_T);
        system->bc->s = wasora_evaluate_expression(system->bc->expr_s);
        
        if (system->bc->T < IAPWS97_TMIN) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at least '%e' instead of '%e'", system->name, IAPWS97_TMIN, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        } else if (system->bc->T > IAPWS97_TMAX) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_TMAX, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (!(system->bc->s > 0)) {
          wasora_push_error_message("specific entropy in system '%s' boundary condition should be positive", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_Ts(system->bc->T, system->bc->s);
        
      } else {
        wasora_push_error_message("both temperature and entropy should be given for 'Ts' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
      
    case Tx:
      
      if (system->bc->expr_T != NULL && system->bc->expr_s != NULL) {
        
        system->bc->T = wasora_evaluate_expression(system->bc->expr_T);
        system->bc->x = wasora_evaluate_expression(system->bc->expr_x);
        
        if (system->bc->T < IAPWS97_TMIN) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at least '%e' instead of '%e'", system->name, IAPWS97_TMIN, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        } else if (system->bc->T > IAPWS97_TMAX) {
          wasora_push_error_message("temperature in system '%s' boundary condition should be at most '%e' instead of '%e'", system->name, IAPWS97_TMAX, system->bc->T);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (system->bc->x < 0) {
          wasora_push_error_message("flow quality in system '%s' boundary condition cannot be negative", system->name);
          return WASORA_RUNTIME_ERROR;
        } else if (system->bc->x > 1) {
          wasora_push_error_message("flow quality in system '%s' boundary condition cannot be greater than unity", system->name);
          return WASORA_RUNTIME_ERROR;
        }
        
        system->bc->fluid_properties->S = freesteam_set_Tx(system->bc->T, system->bc->x);
        
      } else {
        wasora_push_error_message("both temperature and flow quality should be given for 'Tx' boundary type in system '%s'", system->name);
        return WASORA_RUNTIME_ERROR;
      }
      break;
  }
  
  if (system->bc->expr_m_dot != NULL) {
    
    wasora_call(mate_set_fluid_mass_flow_m(mate.system->bc->fluid_properties, wasora_evaluate_expression(system->bc->expr_m_dot)));
    
    if (!(mate_get_fluid_mass_flow_m(system->bc->fluid_properties) > 0)) {
      wasora_push_error_message("mass flow in system '%s' boundary condition should be positive", system->name);
      return WASORA_RUNTIME_ERROR;
    }
    
  } else {
    wasora_push_error_message("mass flow should be given in system '%s' boundary condition", system->name);
    return WASORA_RUNTIME_ERROR;
  }
  
  system->bc->initialized = 1;
  
  return WASORA_RUNTIME_OK;
}



// esta func no se si va a quedar
int mate_post_system(system_t *system) {
  
  pipe_array_t *pipe_array;
  pipe_array_list_item_t *associated_pipe_array;
  
  char filename[64];
  FILE *output_file;
  
  // abrimos un file
  sprintf(filename, "system.dat");
  output_file = fopen(filename, "w+");
  
  fprintf(output_file, "# Thermo-hydraulic system '%s' description\n", system->name);
  fprintf(output_file, "\n");
  
  LL_FOREACH(system->associated_pipe_arrays, associated_pipe_array) {
    pipe_array = associated_pipe_array->pipe_array;
    wasora_call(mate_post_pipe_array(pipe_array, output_file));
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_post_pipe_array(pipe_array_t *pipe_array, FILE *file) {
  
  int i;
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    pipe = pipe_array->pipe[i];
    wasora_call(mate_post_pipe(pipe, file));
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_post_pipe(pipe_t *pipe, FILE* file) {
  
  int i;
  component_list_item_t *associated_component;
  section_t *section;
  circular_section_t *circular_section;
  helical_section_t *helical_section;
  mate_node_t *node;
  mate_volume_t *volume;
  
  LL_FOREACH(pipe->associated_components, associated_component) {
    
    switch (associated_component->type) {
      
      case form_loss_type:
        
        
        break;
        
      case section_type:
        section = (section_t *) associated_component->component;
        
        fprintf(file, "# Section '%s' parameters\n", section->name);
        switch(section->type) {
          case straight_section_type:
            fprintf(file, "#   type: straight section\n");
            break;
          case circular_section_type:
            fprintf(file, "#   type: circular section\n");
            break;
          case helical_section_type:
            fprintf(file, "#   type: helical section\n");
            break;
        }
        fprintf(file, "#   diameter: %e\n", section->D);
        fprintf(file, "#   thickness: %e\n", section->e);
        switch(section->type) {
          case straight_section_type:
            break;
          case circular_section_type:
            circular_section = (circular_section_t *) section->section_params;
            fprintf(file, "#   curvature radius: %e\n", circular_section->R);
            break;
          case helical_section_type:
            helical_section = (helical_section_t *) section->section_params;
            fprintf(file, "#   helix radius: %e\n", helical_section->R);
            fprintf(file, "#   pitch: %e\n", helical_section->p);
            break;
        }
        
        fprintf(file, "#   nodes description:\n");
        fprintf(file, "#     section number of nodes:%d\n", section->n_nodes);
        fprintf(file, "#     node_section_index\tx\ty\tz\ts\n");
        for (i = 0; i < section->n_nodes; i++) {
          node = &section->node[i];
          fprintf(file, "      %.04d\t", node->node_section_index + 1);
          fprintf(file, "      %e\t", node->x[0]);
          fprintf(file, "      %e\t", node->x[1]);
          fprintf(file, "      %e\t", node->x[2]);
          fprintf(file, "      %e\n", node->s);
        }
        
        fprintf(file, "#   fluid volumes description:\n");
        fprintf(file, "#     volume_section_index\tL\tDi\tDo\tPi\tPo\tAf\tAi\tAo\tvolume\n");
        for (i = 0; i < section->n_volumes; i++) {
          volume = &section->fluid_volume[i];
          fprintf(file, "      %.04d\t", volume->volume_section_index + 1);
          fprintf(file, "      %e\t", volume->L);
          fprintf(file, "      %e\t", volume->Di);
          fprintf(file, "      %e\t", volume->Do);
          fprintf(file, "      %e\t", volume->Pi);
          fprintf(file, "      %e\t", volume->Po);
          fprintf(file, "      %e\t", volume->Af);
          fprintf(file, "      %e\t", volume->Ai);
          fprintf(file, "      %e\t", volume->Ao);
          fprintf(file, "      %e\n", volume->volume);
        }
        
        fprintf(file, "#   solid volumes description:\n");
        fprintf(file, "#     volume_section_index\tL\tDi\tDo\tPi\tPo\tAf\tAi\tAo\tvolume\n");
        for (i = 0; i < section->n_volumes; i++) {
          volume = &section->solid_volume[i];
          fprintf(file, "      %.04d\t", volume->volume_section_index + 1);
          fprintf(file, "      %e\t", volume->L);
          fprintf(file, "      %e\t", volume->Di);
          fprintf(file, "      %e\t", volume->Do);
          fprintf(file, "      %e\t", volume->Pi);
          fprintf(file, "      %e\t", volume->Po);
          fprintf(file, "      %e\t", volume->Af);
          fprintf(file, "      %e\t", volume->Ai);
          fprintf(file, "      %e\t", volume->Ao);
          fprintf(file, "      %e\n", volume->volume);
        }
        break;
    }
    
  }
  
  return WASORA_RUNTIME_OK;
}