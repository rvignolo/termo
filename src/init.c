/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's initialization routines
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

int plugin_init_before_parser(void) {

///va+s+name s
///va+s+desc Holder variable: curvilinear coordinate within a pipe.
///va+s+desc s takes the value of the s coordinate of each mate node so functions
///va+s+desc with s as the independent variable could be correctly interpreted.
  mate.vars.s = wasora_define_variable("s");
  //mate.vars.s = wasora_define_variable("node_s");
  
///va+Tw+name Tw
///va+Tw+desc Holder variable: Wall Temperature.
///va+Tw+desc Tw takes the value of the wall temperature of each mate node so functions
///va+Tw+desc with Tw as the independent variable could be correctly interpreted.
  mate.vars.Tw = wasora_define_variable("Tw");
  //mate.vars.Tw = wasora_define_variable("node_Tw");
  
///va+Re_l+name Re_l
///va+Re_l+desc Holder variable: Reynolds liquid number.
///va+Re_l+desc Re_l takes the value of the Reynolds liquid number of each mate node so functions
///va+Re_l+desc with Re_l as the independent variable could be correctly interpreted.
  mate.vars.Re_l = wasora_define_variable("Re_l");
  //mate.vars.Re_l = wasora_define_variable("node_Re_l");
  
///va+Re_v+name Re_v
///va+Re_v+desc Holder variable: Reynolds vapour number.
///va+Re_v+desc Re_v takes the value of the Reynolds vapour number of each mate node so functions
///va+Re_v+desc with Re_v as the independent variable could be correctly interpreted.
  mate.vars.Re_v = wasora_define_variable("Re_v");
  //mate.vars.Re_v = wasora_define_variable("node_Re_v");
  
///va+Re_tp+name Re_tp
///va+Re_tp+desc Holder variable: Reynolds two-phase number.
///va+Re_tp+desc Re_tp takes the value of the Reynolds two-phase number of each mate node so functions
///va+Re_tp+desc with Re_tp as the independent variable could be correctly interpreted.
  mate.vars.Re_tp = wasora_define_variable("Re_tp");
  //mate.vars.Re_tp = wasora_define_variable("node_Re_tp");
  
///va+Re_tp+name Pr
///va+Re_tp+desc Holder variable: Prandtl number.
///va+Re_tp+desc Pr takes the value of the Prandtl number of each mate node so functions
///va+Re_tp+desc with Pr as the independent variable could be correctly interpreted.
  mate.vars.Pr = wasora_define_variable("Pr");
  //mate.vars.Re_tp = wasora_define_variable("node_Pr");
  
///va+Re_tp+name Nu
///va+Re_tp+desc Holder variable: Nusselt number.
///va+Re_tp+desc Nu takes the value of the Nusselt number of each mate node so functions
///va+Re_tp+desc with Nu as the independent variable could be correctly interpreted.
  mate.vars.Nu = wasora_define_variable("Nu");
  //mate.vars.Re_tp = wasora_define_variable("node_Pr");
  
  return WASORA_RUNTIME_OK;
}

// verificamos la consistencia del input
int plugin_init_after_parser(void) {
  
  pipe_t *pipe;
  
  // ahora si verificamos que las expresiones dadas en las descripciones de los pipes sean coherentes
  // esto es asi porque antes pudimos usar, en las expresiones, funciones aun no definidas
  for (pipe = mate.pipes; pipe != NULL; pipe = pipe->hh.next) {
    
    // esta desigualdad se cumple ya que si no mate fallo previamente en el parser
    if (pipe->boundary_expr.ho_string != NULL) {
      pipe->boundary_expr.expr_ho = malloc(sizeof(expr_t));
      
      if (wasora_parse_expression(pipe->boundary_expr.ho_string, pipe->boundary_expr.expr_ho) != WASORA_PARSER_OK) {
        return WASORA_RUNTIME_ERROR;
      }
    }
    
    // esta desigualdad se cumple ya que si no mate fallo previamente en el parser
    if (pipe->boundary_expr.To_string != NULL) {
      pipe->boundary_expr.expr_To = malloc(sizeof(expr_t));
      
      if (wasora_parse_expression(pipe->boundary_expr.To_string, pipe->boundary_expr.expr_To) != WASORA_PARSER_OK) {
        return WASORA_RUNTIME_ERROR;
      }
    }
    
    // esta desigualdad no necesariamente se cumple ya que mate puede usar sus modelos internos
    if (pipe->boundary_expr.hi_string != NULL) {
      pipe->boundary_expr.expr_hi = malloc(sizeof(expr_t));
      
      if (wasora_parse_expression(pipe->boundary_expr.hi_string, pipe->boundary_expr.expr_hi) != WASORA_PARSER_OK) {
        return WASORA_RUNTIME_ERROR;
      }
    }
    
    // esta desigualdad no necesariamente se cumple ya que mate puede usar sus modelos internos
    if (pipe->boundary_expr.fi_string != NULL) {
      pipe->boundary_expr.expr_fi = malloc(sizeof(expr_t));
      
      if (wasora_parse_expression(pipe->boundary_expr.fi_string, pipe->boundary_expr.expr_fi) != WASORA_PARSER_OK) {
        return WASORA_RUNTIME_ERROR;
      }
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int plugin_init_before_run(void) {
  
  mate.initialized = 0;

  return WASORA_RUNTIME_OK;
}


int plugin_finalize(void) {

  return WASORA_RUNTIME_OK;
}
