/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's parsing routines
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

int plugin_parse_line(char *line) {
  
  char *token;

  if ((token = wasora_get_next_token(line)) != NULL) {

// ---------------------------------------------------------------------
///kw+MATE_PROBLEM+usage MATE_PROBLEM
///kw+MATE_PROBLEM+desc Defines the the flow model, either homogeneous or separate flow.
///kw+MATE_PROBLEM+desc It also selects if subcooled regime has to be taken into account
///kw+MATE_PROBLEM+desc and the thermohydraulic system of the problem.
    if (strcasecmp(token, "MATE_PROBLEM") == 0) {
      
      system_t *system;
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+MATE_PROBLEM+usage [ FLOW_MODEL { hem | sep } ]
        if (strcasecmp(token, "FLOW_MODEL") == 0) {
          char *keywords[] = {"hem", "sep", ""};
          int values[] = {HEM, SEP, 0};
          wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&mate.flow_model));
          
///kw+MATE_PROBLEM+usage [ SUBCOOLED_BOILING ]
        } else if (strcasecmp(token, "SUBCOOLED_BOILING") == 0) {
          mate.sub_boiling = 1;
          
///kw+MATE_PROBLEM+usage SYSTEM <system_name>
        } else if (strcasecmp(token, "SYSTEM") == 0) {
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected system name");
            return WASORA_PARSER_ERROR;
          }
          
          if ((system = mate_get_system_ptr(token)) != NULL) {
            mate.system = system;
          } else {
            wasora_push_error_message("undefined system '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
        } else {
          wasora_push_error_message("undefined keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      // si no pusieron un sistema en MATE_PROBLEM, ponemos el principal
      // y si no se definio ningun sistema en el input, fallamos
      if (mate.system == NULL && (mate.system = mate.main_system) == NULL) {
        wasora_push_error_message("unknown system for MATE_PROBLEM (no SYSTEM keyword)");
        return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+MATE_STEP+usage MATE_STEP
///kw+MATE_STEP+desc Solves the non-linear problem.
    } else if (strcasecmp(token, "MATE_STEP") == 0) {
      
      mate_step_t *mate_step = calloc(1, sizeof(mate_step_t));
      
      mate_step->do_not_solve = 0;
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+MATE_STEP+usage [ DO_NOT_SOLVE ]
        if (strcasecmp(token, "DO_NOT_SOLVE") == 0) {
          mate_step->do_not_solve = 1;
        
///kw+MATE_STEP+usage [ DO_NOT_PARALLELIZE ]
        } else if (strcasecmp(token, "DO_NOT_PARALLELIZE") == 0) {
          
          wasora_push_error_message("keyword '%s' not yet implemented", token);
          return WASORA_PARSER_ERROR;
          
///kw+MATE_STEP+usage [ N_CORES ]
        } else if (strcasecmp(token, "N_CORES") == 0) {
          
          wasora_push_error_message("keyword '%s' not yet implemented", token);
          return WASORA_PARSER_ERROR;
          
///kw+MATE_STEP+usage [ VERBOSE ]
        } else if (strcasecmp(token, "VERBOSE") == 0) {
          
          wasora_push_error_message("keyword '%s' not yet implemented", token);
          return WASORA_PARSER_ERROR;
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      wasora_define_instruction(mate_instruction_step, mate_step);
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+BOUNDARY_CONDITION+usage BOUNDARY_CONDITION
///kw+BOUNDARY_CONDITION+desc Defines the boundary conditions for the mate problem.
    } else if (strcasecmp(token, "BOUNDARY_CONDITION") == 0 || strcasecmp(token, "BC") == 0) {
      
      system_t *system;
      mate_bc_t *bc = calloc(1, sizeof(mate_bc_t));
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+BOUNDARY_CONDITION+usage TYPE { ph | ps | pu | pv | pT | Ts | Tx }
        if (strcasecmp(token, "TYPE") == 0) {
          char *keywords[] = {"ph", "ps", "pu", "pv", "pT", "Ts", "Tx", ""};
          int values[] = {ph, ps, pu, pv, pT, Ts, Tx, 0};
          wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&bc->type));
          
///kw+BOUNDARY_CONDITION+usage SYSTEM <system_name>
        } else if (strcasecmp(token, "SYSTEM") == 0) {
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected system name");
            return WASORA_PARSER_ERROR;
          }
          
          if ((system = mate_get_system_ptr(token)) == NULL) {
            wasora_push_error_message("undefined system '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          // los asociamos
          system->bc = bc;
          
///kw+BOUNDARY_CONDITION+usage [ { TEMPERATURE | TEMP } <expr> ]
        } else if (strcasecmp(token, "TEMPERATURE") == 0 || strcasecmp(token, "TEMP") == 0 || strcasecmp(token, "T") == 0) {
          bc->expr_T = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_T));
          
///kw+BOUNDARY_CONDITION+usage [ { PRESSURE | PRESS } <expr> ]
        } else if (strcasecmp(token, "PRESSURE") == 0 || strcasecmp(token, "PRESS") == 0 || strcasecmp(token, "P") == 0) {
          bc->expr_p = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_p));
          
///kw+BOUNDARY_CONDITION+usage [ { SPECIFIC_ENTROPY | ENTROPY } <expr> ]
        } else if (strcasecmp(token, "SPECIFIC_ENTROPY") == 0 || strcasecmp(token, "ENTROPY") == 0 || strcasecmp(token, "S") == 0) {
          bc->expr_s = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_s));
          
///kw+BOUNDARY_CONDITION+usage [ { SPECIFIC_ENTHALPY | ENTHALPY } <expr> ]
        } else if (strcasecmp(token, "SPECIFIC_ENTHALPY") == 0 || strcasecmp(token, "ENTHALPY") == 0 || strcasecmp(token, "H") == 0) {
          bc->expr_h = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_h));
          
///kw+BOUNDARY_CONDITION+usage [ { SPECIFIC_VOLUME | VOLUME } <expr> ]
        } else if (strcasecmp(token, "SPECIFIC_VOLUME") == 0 || strcasecmp(token, "VOLUME") == 0 || strcasecmp(token, "V") == 0) {
          bc->expr_v = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_v));
          
///kw+BOUNDARY_CONDITION+usage [ { SPECIFIC_INTERNAL_ENERGY | INTERNAL_ENERGY } <expr> ]
        } else if (strcasecmp(token, "SPECIFIC_INTERNAL_ENERGY") == 0 || strcasecmp(token, "INTERNAL_ENERGY") == 0 || strcasecmp(token, "U") == 0) {
          bc->expr_u = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_u));
          
///kw+BOUNDARY_CONDITION+usage [ QUALITY <expr> ]
        } else if (strcasecmp(token, "QUALITY") == 0 || strcasecmp(token, "X") == 0) {
          bc->expr_x = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_x));
          
///kw+BOUNDARY_CONDITION+usage [ { MASS_FLOW | M_DOT } <expr> ]
        } else if (strcasecmp(token, "MASS_FLOW") == 0 || strcasecmp(token, "M_DOT") == 0 || strcasecmp(token, "M") == 0) {
          bc->expr_m_dot = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(bc->expr_m_dot));
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+SECTION+usage SECTION
///kw+SECTION+desc Defines a section given its type and properties
    } else if (strcasecmp(token, "SECTION") == 0) {
      
      int i;
      int type;
      char *name;
      section_t *section;
      straight_section_t *straight_section;
      circular_section_t *circular_section;
      helical_section_t *helical_section;
      
///kw+SECTION+usage <name>
      if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
      
///kw+SECTION+usage TYPE { straight | circular | helical }
      if ((token = wasora_get_next_token(NULL)) != NULL && strcasecmp(token, "TYPE") == 0) {
        char *keywords[] = {"straight", "circular", "helical", ""};
        int values[] = {straight_section_type, circular_section_type, helical_section_type, 0};
        wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&type));
      } else {
        wasora_push_error_message("TYPE keyword expected");
        return WASORA_PARSER_ERROR;
      }
      
      if ((section = mate_define_section(name, type)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      // por default ponemos las columnas 1 2 3 4 (node_id node_x_coord node_y_coord node_z_coord)
      for (i = 0; i < 4; i++) {
        section->column[i] = i+1;
      }
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {

///kw+SECTION+usage DIAMETER <expr>
        if (strcasecmp(token, "DIAMETER") == 0) {
          section->expr_D = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->expr_D));
          
///kw+SECTION+usage THICKNESS <expr>
        } else if (strcasecmp(token, "THICKNESS") == 0) {
          section->expr_e = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->expr_e));
          
///kw+SECTION+usage MATERIAL <name>
        } else if (strcasecmp(token, "MATERIAL") == 0) {
/*
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected material name");
            return WASORA_PARSER_ERROR;
          }
          
          if ((section->solid_material = wasora_get_material_ptr(token)) == NULL) {
            wasora_push_error_message("undefined material '%s'", token);
            return WASORA_PARSER_ERROR;
          }
*/
          
          wasora_push_error_message("keyword '%s' not yet implemented", token);
          return WASORA_PARSER_ERROR;
          
///kw+SECTION+usage CONDUCTIVITY <expr>
        } else if (strcasecmp(token, "CONDUCTIVITY") == 0) {
          section->solid_material->expr_k = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->solid_material->expr_k));
          
///kw+SECTION+usage [ ROUGHNESS <expr> ]
        } else if (strcasecmp(token, "ROUGHNESS") == 0) {
          section->solid_material->expr_epsilon = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->solid_material->expr_epsilon));
          
///kw+SECTION+usage [ FILE <file_id> ]
        } else if (strcasecmp(token, "FILE") == 0) {
          wasora_call(wasora_parser_file(&section->file));
          section->special_mesh = 1;
          
///kw+SECTION+usage [ COLUMNS <num_expr_1> <num_expr_2> <num_expr_3> <num_expr_4> ]
        } else if (strcasecmp(token, "COLUMNS") == 0) {
          for (i = 0; i < 4; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected 4 columns specifications");
              return WASORA_PARSER_ERROR;
            }
            section->column[i] = (int)(wasora_evaluate_expression_in_string(token));
          }
          
///kw+SECTION+usage [ FIRST_NODE_ID <expr> ]
        } else if (strcasecmp(token, "FIRST_NODE_ID") == 0) {
          section->expr_first_node_id = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->expr_first_node_id));
          
///kw+SECTION+usage [ LAST_NODE_ID <expr> ]
        } else if (strcasecmp(token, "LAST_NODE_ID") == 0) {
          section->expr_last_node_id = malloc(sizeof(expr_t));
          wasora_call(wasora_parser_expression(section->expr_last_node_id));
          
///kw+SECTION+usage [ RADIUS <expr> ]
        } else if (strcasecmp(token, "RADIUS") == 0) {
          
          switch (section->type) {
            case straight_section_type:
              wasora_push_error_message("RADIUS is an invalid parameter for '%s' straight nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case circular_section_type:
              circular_section = (circular_section_t *) section->section_params;
              
              circular_section->expr_R = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(circular_section->expr_R));
              break;
              
            case helical_section_type:
              helical_section = (helical_section_t *) section->section_params;
              
              helical_section->expr_R = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(helical_section->expr_R));
              break;
          }
          
///kw+SECTION+usage [ PITCH <expr> ]
        } else if (strcasecmp(token, "PITCH") == 0) {
          
          switch (section->type) {
            case straight_section_type:
              wasora_push_error_message("PITCH is an invalid parameter for '%s' straight nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case circular_section_type:
              wasora_push_error_message("PITCH is an invalid parameter for '%s' circular nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case helical_section_type:
              helical_section = (helical_section_t *) section->section_params;
              
              helical_section->expr_p = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(helical_section->expr_p));
              break;
          }
          
///kw+SECTION+usage [ LENGTH <expr> ]
        } else if (strcasecmp(token, "LENGTH") == 0) {
          
          switch (section->type) {
            case straight_section_type:
              section->expr_L = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(section->expr_L));
              break;
              
            case circular_section_type:
              wasora_push_error_message("LENGTH is an invalid parameter for '%s' circular nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case helical_section_type:
              wasora_push_error_message("LENGTH is an invalid parameter for '%s' helical nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
          }
          
///kw+SECTION+usage [ N_NODES <expr> ]
        } else if (strcasecmp(token, "N_NODES") == 0) {
          
          switch (section->type) {
            case straight_section_type:
              section->expr_n_nodes = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(section->expr_n_nodes));
              break;
              
            case circular_section_type:
              wasora_push_error_message("N_NODES is an invalid parameter for '%s' circular section", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case helical_section_type:
              wasora_push_error_message("N_NODES is an invalid parameter for '%s' helical nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
          }
          
///kw+SECTION+usage [ ANGLE <expr> ]
        } else if (strcasecmp(token, "ANGLE") == 0) {
          
          switch (section->type) {
            case straight_section_type:
              straight_section = (straight_section_t *) section->section_params;
              
              straight_section->expr_theta = malloc(sizeof(expr_t));
              wasora_call(wasora_parser_expression(straight_section->expr_theta));
              break;
              
            case circular_section_type:
              wasora_push_error_message("ANGLE is an invalid parameter for '%s' circular nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
              
            case helical_section_type:
              wasora_push_error_message("ANGLE is an invalid parameter for '%s' helical nodalization", section->name);
              return WASORA_PARSER_ERROR;
              break;
          }
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      // si no se proveyo el diametro
      if (section->expr_D == NULL) {
        wasora_push_error_message("expected diameter (DIAM keyword) for '%s' nodalization", section->name);
        return WASORA_PARSER_ERROR;
      }
      
      // si no es una malla special
      if (!section->special_mesh) {
        
        // el type de la seccion tiene que ser straight
        if (section->type != straight_section_type) {
          wasora_push_error_message("straight section type expected because no file with nodalization was given");
          return WASORA_PARSER_ERROR;
        }
        
        // no tiene sentido dar los ids
        if (section->expr_first_node_id != NULL || section->expr_last_node_id != NULL) {
          wasora_push_error_message("first and last nodes ids should not be given if the '%s' nodalization does not come from a file", section->name);
          return WASORA_PARSER_ERROR;
        }
        
        // pero hay que dar la longitud
        if (section->expr_L == NULL) {
          wasora_push_error_message("section lenght expected (LENGTH keyword) for '%s' nodalization", section->name);
          return WASORA_PARSER_ERROR;
        }
        
        // pero hay que dar el numero de nodos
        if (section->expr_n_nodes == NULL) {
          wasora_push_error_message("section number of nodes expected (N_NODES keyword) for '%s' nodalization", section->name);
          return WASORA_PARSER_ERROR;
        }
        
        // pero hay que dar el angulo
        // ya checkee que el tipo sea straight
        straight_section = (straight_section_t *) section->section_params;
        if (straight_section->expr_theta == NULL) {
          wasora_push_error_message("polar angle expected (ANGLE keyword) for '%s' nodalization", section->name);
          return WASORA_PARSER_ERROR;
        }
      
        // pero si la malla es special
      } else if (section->special_mesh) {
        
        // hay que verificar que se hayan provisto los ids
        if (section->expr_first_node_id == NULL || section->expr_last_node_id == NULL) {
          wasora_push_error_message("first and last nodes ids should be given if the '%s' nodalization comes from a file", section->name);
          return WASORA_PARSER_ERROR;
        }
        
        // y si ademas son nodos circulares
        if (section->type == circular_section_type) {
          
          circular_section = (circular_section_t *) section->section_params;
          
          if (circular_section->expr_R == NULL) {
            wasora_push_error_message("RADIUS keyword expected for '%s' circular nodalization", section->name);
            return WASORA_PARSER_ERROR;
          }
          
          // o nodos helicoidales
        } else if (section->type == helical_section_type) {
          
          helical_section = (helical_section_t *) section->section_params;
          
          if (helical_section->expr_R == NULL) {
            wasora_push_error_message("RADIUS keyword expected for '%s' helical nodalization", section->name);
            return WASORA_PARSER_ERROR;
          }
          
          if (helical_section->expr_p == NULL) {
            wasora_push_error_message("PITCH keyword expected for '%s' helical nodalization", section->name);
            return WASORA_PARSER_ERROR;
          }
        }
      }
      
      // si no se da la conductividad, fallamos
      if (section->solid_material->expr_k == NULL) {
        wasora_push_error_message("CONDUCTIVITY keyword expected in section '%s' description", section->name);
        return WASORA_PARSER_ERROR;
      }
      
/*
      // si no se da la rugosidad, no pasa nada porque metemos cero mas adelante
      if (section->solid_material->expr_epsilon == NULL) {
        // TODO: permitir que la rugosisdad no sea provista por input
        wasora_push_error_message("ROUGHNESS keyword expected in section '%s' description", section->name);
        return WASORA_PARSER_ERROR;
      }
*/
      
      // ahora si podemos definir la instruccion
      if (wasora_define_instruction(mate_instruction_section, section) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+FORM_LOSS+usage FORM_LOSS
///kw+FORM_LOSS+desc Defines a form loss given its type and properties
    } else if (strcasecmp(token, "FORM_LOSS") == 0) {
      
      wasora_push_error_message("keyword '%s' not yet implemented", token);
      return WASORA_PARSER_ERROR;
      
// ---------------------------------------------------------------------
///kw+PIPE+usage PIPE
///kw+PIPE+desc Defines a pipe given its description
    } else if (strcasecmp(token, "PIPE") == 0) {
      
      char *name;
      section_t *section;
      form_loss_t *form_loss;
      pipe_t *pipe;
      
///kw+PIPE+usage <name>
      if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
      
      if ((pipe = mate_define_pipe(name)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      while ((token = wasora_get_next_token(NULL)) != NULL && strcasecmp(token, "DATA") != 0) {
        
        if (strcasecmp(token, "INNER_H") == 0) {
          
          // copiamos los strings porque en la definicion de un pipe pueden aparecer funciones aun no definidas 
          // como funciones de wasora (i.e. temperatura externa de un pipe puede ser la interna de otro)
          // y wasora_parser_expression fallaria dado que no las encontraria.
          // entonces, copiamos el string y parseamos la expresion en el init_after_parser, y si ahi falla es porque
          // efectivamente estas funciones no existen (no se han definido esos pipes)
          wasora_parser_string(&pipe->boundary_expr.hi_string);
          
        } else if (strcasecmp(token, "OUTER_H") == 0) {
          
          // idem comentario INNER_H
          wasora_parser_string(&pipe->boundary_expr.ho_string);
          
        } else if (strcasecmp(token, "OUTER_T") == 0) {
          
          // idem comentario INNER_H
          wasora_parser_string(&pipe->boundary_expr.To_string);
          
        } else if (strcasecmp(token, "FRIC_FACTOR") == 0) {
          
          // idem comentario INNER_H
          wasora_parser_string(&pipe->boundary_expr.fi_string);
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
///kw+PIPE+usage DATA <section_1 | form_loss_1> <section_2 | form_loss_2> ... <section_m | form_loss_n>
      if (strcasecmp(token, "DATA") == 0) {
        
        // recorremos los nombres de sections o form losses
        while ((token = wasora_get_next_token(NULL)) != NULL) {
          
          // buscamos si hay una form loss o section que se llame asi.
          // como mucho hay una ya que no pueden llamarse iguales las form losses y las sections
          if ((form_loss = mate_get_form_loss_ptr(token)) != NULL) {
            
            // no permitimos que una form loss pertenezca a diferentes pipes
            if (form_loss->pipe != NULL) {
              wasora_push_error_message("cannot associate form loss '%s' to pipe '%s' because it is already associated to pipe '%s'", form_loss->name, pipe->name, form_loss->pipe->pipe_array);
              return WASORA_RUNTIME_ERROR;
            }
            // como lo anterior no se cumple, podemos agregar la form loss al pipe
            mate_append_component_to_pipe(&pipe->associated_components, form_loss_type, form_loss);
            // aprovechamos para asignarle este pipe a la form loss
            form_loss->pipe = pipe;
            
          } else if ((section = mate_get_section_ptr(token)) != NULL) {
            
            // no permitimos que una section pertenezca a diferentes pipes
            if (section->pipe != NULL) {
              wasora_push_error_message("cannot associate section '%s' to pipe '%s' because it is already associated to pipe '%s'", section->name, pipe->name, section->pipe->name);
              return WASORA_RUNTIME_ERROR;
            }
            // como lo anterior no se cumple, podemos agregar la section al pipe
            mate_append_component_to_pipe(&pipe->associated_components, section_type, section);
            // aprovechamos para asignarle este pipe a la section
            section->pipe = pipe;
            
          } else if (section == NULL && form_loss == NULL) {
            wasora_push_error_message("neither section nor form loss found with name '%s'", token);
            return WASORA_PARSER_ERROR;
          }
        }
        
      } else {
        wasora_push_error_message("last keyword in PIPE '%s' description should be DATA", name);
        return WASORA_PARSER_ERROR;
      }
      
      // si no se proveyo de la expresion para temperatura externa fallamos
      if (pipe->boundary_expr.To_string == NULL) {
        wasora_push_error_message("outside temperature in PIPE '%s' should be given (OUTER_T keyword)", name);
        return WASORA_PARSER_ERROR;
      } 
      
      // si no se proveyo el coeficiente de transferencia de calor externo fallamos
      if (pipe->boundary_expr.ho_string == NULL) {
        wasora_push_error_message("outside heat transfer coefficient in PIPE '%s' should be given (OUTER_H keyword)", name);
        return WASORA_PARSER_ERROR;
      }
      
      // ahora si podemos definir la instruccion
      if (wasora_define_instruction(mate_instruction_pipe, pipe) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+PIPE_ARRAY+usage PIPE_ARRAY
///kw+PIPE_ARRAY+desc Defines a pipe array given its description
    } else if (strcasecmp(token, "PIPE_ARRAY") == 0) {
      
      int i;
      int n_pipes;
      char *name;
      char *dummy;
      char *backup1, *backup2;
      pipe_t *pipe;
      pipe_array_t *pipe_array;
      
///kw+PIPE_ARRAY+usage <name>
      if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
      
///kw+PIPE_ARRAY+usage DATA <pipe_1> <pipe_2> ... <pipe_n>
      if ((token = wasora_get_next_token(NULL)) != NULL && strcasecmp(token, "DATA") == 0) {
        
        // primero contamos la cantidad de pipes que forman al pipe array
        dummy = token + strlen(token)+1;
        
        backup1 = malloc(strlen(dummy)+8);
        sprintf(backup1, "dummy %s", dummy);
        backup2 = malloc(strlen(dummy)+8);
        sprintf(backup2, "dummy %s", dummy);
        
        // contamos cuanta informacion hay
        n_pipes = 0;
        if ((token = wasora_get_next_token(backup1)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        while ((token = wasora_get_next_token(NULL)) != NULL) {
          n_pipes++;
        }
        
        if ((pipe_array = mate_define_pipe_array(name, n_pipes)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        
        // leemos la informacion
        i = 0;
        if ((token = wasora_get_next_token(backup2)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        while ((token = wasora_get_next_token(NULL)) != NULL) { 
          
          if ((pipe = mate_get_pipe_ptr(token)) == NULL) {
            wasora_push_error_message("undefined pipe '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          // no permitimos que un pipe pertenezca a diferentes pipe arrays
          if (pipe->pipe_array != NULL) {
            wasora_push_error_message("cannot associate pipe '%s' to pipe array '%s' because it is already associated to pipe array '%s'", pipe->name, pipe_array->name, pipe->pipe_array->name);
            return WASORA_RUNTIME_ERROR;
          }
          // como lo anterior no se cumple, podemos agregar el pipe al pipe array
          pipe_array->pipe[i] = pipe;
          // aprovechamos para asignarle este pipe array al pipe
          pipe_array->pipe[i]->pipe_array = pipe_array;
          
          i++;
        }
        
      } else {
        wasora_push_error_message("unknown keyword '%s'", token);
        return WASORA_PARSER_ERROR;
        //wasora_push_error_message("last keyword in PIPE_ARRAY '%s' description should be DATA", name);
        //return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+BRANCH+usage BRANCH
///kw+BRANCH+desc Defines a branch given its description
    } else if (strcasecmp(token, "BRANCH") == 0) {
      
      wasora_push_error_message("keyword '%s' not yet implemented", token);
      return WASORA_PARSER_ERROR;
      
// ---------------------------------------------------------------------
///kw+JUNCTION+usage JUNCTION
///kw+JUNCTION+desc Defines a junction given its description
    } else if (strcasecmp(token, "JUNCTION") == 0) {
      
      wasora_push_error_message("keyword '%s' not yet implemented", token);
      return WASORA_PARSER_ERROR;
      
// ---------------------------------------------------------------------
///kw+SYSTEM+usage SYSTEM
///kw+SYSTEM+desc Defines a system given its description
    } else if (strcasecmp(token, "SYSTEM") == 0) {
      
      char *name;
      char *dummy_name;
      system_t *system;
      pipe_t *pipe;
      pipe_array_t *pipe_array;
      
///kw+SYSTEM+usage <name>
      if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
      
      if ((system = mate_define_system(name)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
///kw+SYSTEM+usage DATA <pipe_1 | pipe_array_1> <pipe_1 | pipe_array_2> ... <pipe_n | pipe_array_m>
      if ((token = wasora_get_next_token(NULL)) != NULL && strcasecmp(token, "DATA") == 0) {
        
        // recorremos los nombres de pipes o pipe arrays que forman al sistema
        while ((token = wasora_get_next_token(NULL)) != NULL) {
          
          if ((pipe = mate_get_pipe_ptr(token)) != NULL) {
            
            // si se dio un pipe, definimos un pipe array de tamaño 1
            // con un nombre dummy
            dummy_name = malloc(strlen(pipe->name)+256);
            sprintf(dummy_name, "dummy_pipe_array_from_pipe_%s", pipe->name);
            
            // pero previamente se pudo haber intentado definir un sistem con un pipe
            // y haberse generado un pipe array dummy, por eso lo buscamos
            if ((pipe_array = mate_get_pipe_array_ptr(dummy_name)) != NULL) {
              wasora_push_error_message("cannot associate pipe '%s' to system '%s' because it is already associated to system '%s'", pipe->name, system->name, pipe_array->system->name);
              return WASORA_RUNTIME_ERROR;
              
              // si no, definimos efectivamente al pipe array
            } else if ((pipe_array = mate_define_pipe_array(dummy_name, 1)) == NULL) {
              return WASORA_RUNTIME_ERROR;
            }
            // y rellenamos el pipe array
            pipe_array->pipe[0] = pipe;
            
            // como llegamos hasta aqui, podemos agregar el pipe array al system
            mate_append_pipe_array_to_system(&system->associated_pipe_arrays, pipe_array);
            // aprovechamos para asignarle este system al pipe array
            pipe_array->system = system;
            
            free(dummy_name);
            
          } else if ((pipe_array = mate_get_pipe_array_ptr(token)) != NULL) {
            
            // no permitimos que un pipe array pertenezca a diferentes systems
            if (pipe_array->system != NULL) {
              wasora_push_error_message("cannot associate pipe array '%s' to system '%s' because it is already associated to system '%s'", pipe_array->name, system->name, pipe_array->system->name);
              return WASORA_RUNTIME_ERROR;
            }
            // como lo anterior no se cumple, podemos agregar el pipe array al system
            mate_append_pipe_array_to_system(&system->associated_pipe_arrays, pipe_array);
            // aprovechamos para asignarle este system al pipe array
            pipe_array->system = system;
            
          } else {
            wasora_push_error_message("undefined pipe or pipe array '%s' for SYSTEM '%s' DATA", token, system->name);
            return WASORA_PARSER_ERROR;
          }
        }
        
      } else {
        wasora_push_error_message("last keyword in SYSTEM '%s' description should be DATA", name);
        return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;
    }
  }
  
  return WASORA_PARSER_UNHANDLED;
}
