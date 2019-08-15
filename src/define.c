/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's defines routines
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

// vemos si no existe ya una seccion, form loss, pipe, pipe arrays o system que se llame name
int mate_check_name(const char *name) {
  
  section_t *section;
  form_loss_t *form_loss;
  pipe_t *pipe;
  pipe_array_t *pipe_array;
  //system_t *system;
  
  HASH_FIND_STR(mate.sections, name, section);
  if (section != NULL) {
    wasora_push_error_message("there already exists a section named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(mate.form_losses, name, form_loss);
  if (form_loss != NULL) {
    wasora_push_error_message("there already exists a form loss named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(mate.pipes, name, pipe);
  if (pipe != NULL) {
    wasora_push_error_message("there already exists a pipe named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(mate.pipe_arrays, name, pipe_array);
  if (pipe_array != NULL) {
    wasora_push_error_message("there already exists a pipe array named '%s'", name);
    return WASORA_PARSER_ERROR;
  }
  
/*
  HASH_FIND_STR(mate.systems, name, system);
  if (system != NULL) {
    wasora_push_error_message("there already exists a system named '%s'", name);
    return WASORA_PARSER_ERROR;
  }
*/

  return WASORA_PARSER_OK;
}
