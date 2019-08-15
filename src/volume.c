/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's volume-related routines
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

int mate_compute_section_volumes(section_t *section) {
  
  int i;
  mate_volume_t *fluid_volume;
  mate_volume_t *solid_volume;
  
  for (i = 0; i < section->n_volumes; i++) {
    
    fluid_volume = &section->fluid_volume[i];
    solid_volume = &section->solid_volume[i];
    
    // el fluido circula por un cilindro
    fluid_volume->Di = 0;
    fluid_volume->Do = section->D;
    
    // mientras que el solido es anular
    solid_volume->Di = fluid_volume->Do;
    solid_volume->Do = solid_volume->Di + 2.0 * section->e;
    
    // asociamos los nodos de inicio y final del volumen
    fluid_volume->initial_node = solid_volume->initial_node = &section->node[i];
    fluid_volume->final_node = solid_volume->final_node = &section->node[i+1];
    
    // la seccion asociada al volumen
    fluid_volume->section = section;
    solid_volume->section = section;
    
    wasora_call(mate_set_volume_params(fluid_volume));
    wasora_call(mate_set_volume_params(solid_volume));
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_set_volume_params(mate_volume_t *volume) {
  
  circular_section_t *circular_section;
  helical_section_t *helical_section;
  
  switch(volume->section->type) {
    
    case straight_section_type:
      volume->L = sqrt(pow(volume->final_node->x[0] - volume->initial_node->x[0], 2) + pow(volume->final_node->x[1] - volume->initial_node->x[1], 2) + pow(volume->final_node->x[2] - volume->initial_node->x[2], 2));
      break;
      
    case circular_section_type:
      circular_section = (circular_section_t *) volume->section->section_params;
      
      volume->L = sqrt(pow(volume->final_node->x[0] - volume->initial_node->x[0], 2) + pow(volume->final_node->x[1] - volume->initial_node->x[1], 2) + pow(volume->final_node->x[2] - volume->initial_node->x[2], 2));
      volume->L = 2.0 * circular_section->R * asin(volume->L / 2.0 / circular_section->R);
      break;
      
    case helical_section_type:
      helical_section = (helical_section_t *) volume->section->section_params;
      
      // la longitud entre dos puntos con una determinada coordenada axial y unidos mediante una helice VERTICAL de radio R y pitch p es:
      volume->L = 2.0 * M_PI * (fabs(volume->final_node->x[2] - volume->initial_node->x[2]) / helical_section->p) * sqrt(pow(helical_section->R, 2) + pow(helical_section->p / 2.0 / M_PI , 2));
      break;
  }
  
  volume->Pi = M_PI * volume->Di;
  volume->Po = M_PI * volume->Do;
  
  volume->Af = M_PI * (pow(volume->Do, 2) - pow(volume->Di, 2)) / 4.0;
  volume->Ai = volume->Pi * volume->L;
  volume->Ao = volume->Po * volume->L;
  
  volume->volume = volume->Af * volume->L;
  
  return WASORA_RUNTIME_OK;
}