/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate plugin for wasora
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
 *  along with wasora.  If not, see <http://www.gnu.org/licenses/>.
 *------------------- ------------  ----    --------  --     -       -         -
 */

#include "mate.h"


int mate_instruction_step(void *arg) {
  
  mate_step_t *mate_step = (mate_step_t *) arg;
  
  // en realidad aca dentro vamos a llamar a todos los systems que mate tiene que resolver 
  //  para ir inicialiandolos y luego mandaremos a resolver a todos ellos
  if (!mate.initialized) {
    
    // no se me ocurre cuando podria pasar esto, pero igual lo checkeo
    if (mate.system == NULL) {
      wasora_push_error_message("no system found");
      return WASORA_RUNTIME_ERROR;
    }
    
    wasora_call(mate_init_system(mate.system));
    
    //wasora_call(mate_post_system(mate.system));
    
    wasora_call(mate_set_system_boundary_conditions(mate.system));
    
    mate.initialized = 1;
  }
  
  // si el solver esta paralelizado, mandamos a paralized solver, sino no.
  // ver pcex como hace eso, las rutinas de fortran
  if (mate_step->do_not_solve == 0) {
    
    wasora_call(mate_solve_system(mate.system));
  }
  
  return WASORA_RUNTIME_OK;
}
