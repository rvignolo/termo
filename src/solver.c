/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's solver-related routines
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

int mate_allocate_pipe_array_solver(pipe_array_t *pipe_array) {
  
  int i;
  
  pipe_array->solver = calloc(1, sizeof(pipe_array_solver_t));
  
  // allocamos
  pipe_array->solver->n_nodes_per_pipe  = calloc(pipe_array->n_pipes, sizeof(int));
  pipe_array->solver->n_form_losses_per_pipe  = calloc(pipe_array->n_pipes, sizeof(int));
  pipe_array->solver->n_points_per_pipe = calloc(pipe_array->n_pipes, sizeof(int));
  
  // determinamos la cantidad de nodos, form losses y puntos de calculo totales y en cada cada pipe
  wasora_call(mate_compute_pipe_array_num_calc_points(pipe_array));
  
  pipe_array->solver->point_type = calloc(pipe_array->n_pipes, sizeof(int *));
  for (i = 0; i < pipe_array->n_pipes; i++)
    pipe_array->solver->point_type[i] = calloc(pipe_array->solver->n_points_per_pipe[i], sizeof(int));
  
  pipe_array->solver->node_by_pipe_array_index = calloc(pipe_array->solver->n_total_nodes, sizeof(node_t *));
  pipe_array->solver->form_loss_by_pipe_array_index = calloc(pipe_array->solver->n_total_form_losses, sizeof(form_loss_t *));
  
  // este maneje permite calcular con subcooled boiling y tener el solver paralelizado
  pipe_array->solver->inner_iter = calloc(pipe_array->n_pipes, sizeof(int));
/*  
  // si corresponde, definimos las componentes de subcooled boiling
  
  if (mate.sub_boiling)
    wasora_call(mate_define_sub_boiling_parameters());
*/
  
  return WASORA_RUNTIME_OK;
}

int mate_compute_pipe_array_num_calc_points(pipe_array_t *pipe_array) {
  
  int i, j;
  
  pipe_t *pipe;
  
  component_list_item_t *associated_component;
  section_t *section;
  
  pipe_array->solver->n_total_nodes = 0;
  pipe_array->solver->n_total_form_losses = 0;
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    pipe_array->solver->n_nodes_per_pipe[i] = 0;
    pipe_array->solver->n_form_losses_per_pipe[i] = 0;
    LL_FOREACH(pipe->associated_components, associated_component) {
      
      switch (associated_component->type) {
        
        case form_loss_type:
          pipe_array->solver->n_form_losses_per_pipe[i]++;
          break;
          
        case section_type:
          section = (section_t *) associated_component->component;
          
          for (j = 0; j < section->n_nodes; j++)
            pipe_array->solver->n_nodes_per_pipe[i]++;
          break;
      }
    }
    
    pipe_array->solver->n_total_form_losses += pipe_array->solver->n_form_losses_per_pipe[i];
    pipe_array->solver->n_total_nodes += pipe_array->solver->n_nodes_per_pipe[i];
    
    pipe_array->solver->n_points_per_pipe[i] = pipe_array->solver->n_nodes_per_pipe[i] + pipe_array->solver->n_form_losses_per_pipe[i];
  }
  
  pipe_array->solver->n_total_points = pipe_array->solver->n_total_nodes + pipe_array->solver->n_total_form_losses;
  
  return WASORA_RUNTIME_OK;
}

int mate_point_pipe_array_solver_objects(pipe_array_t *pipe_array) {
  
  int i, j;
  int point_pipe_index;
  
  component_list_item_t *associated_component;
  form_loss_t *form_loss;
  section_t *section;
  
  pipe_t *pipe;
  
  for (i = 0; i < pipe_array->n_pipes; i++) {
    
    pipe = pipe_array->pipe[i];
    
    point_pipe_index = 0;
    LL_FOREACH(pipe->associated_components, associated_component) {
      
      switch (associated_component->type) {
        
        case form_loss_type:
          
          // el punto de calculo es tipo form loss
          pipe_array->solver->point_type[i][point_pipe_index] = FORM_LOSS_CALC_POINT;
          point_pipe_index++;
          
          // apuntamos a la form loss
          form_loss = (form_loss_t *) associated_component->component;
          pipe_array->solver->form_loss_by_pipe_array_index[form_loss->form_loss_pipe_array_index] = form_loss;
          break;
          
        case section_type:
          
          section = (section_t *) associated_component->component;
          
          for (j = 0; j < section->n_nodes; j++) {
            
            // el punto de calculo es tipo node
            pipe_array->solver->point_type[i][point_pipe_index] = NODE_CALC_POINT;
            point_pipe_index++;
            
            // apuntamos al nodo
            pipe_array->solver->node_by_pipe_array_index[section->node[j].node_pipe_array_index] = &section->node[j];
          }
          break;
      }
    }
  }
  
  return WASORA_RUNTIME_OK;
}

int mate_solve_system(system_t *system) {
  
  int i;
  
  pipe_t *pipe;
  pipe_array_t *pipe_array;
  pipe_array_list_item_t *associated_pipe_array;
  
  fluid_properties_t *dummy_fluid = calloc(1, sizeof(fluid_properties_t));
  
  // para el primer pipe array, la condicion de contorno se da por input y se corresponde con la del sistema a resolver
  wasora_call(mate_safe_copy_fluid_properties(dummy_fluid, system->bc->fluid_properties));
  wasora_call(mate_set_fluid_mass_flow_m(dummy_fluid, mate_get_fluid_mass_flow_m(system->bc->fluid_properties)));
  
  LL_FOREACH(system->associated_pipe_arrays, associated_pipe_array) {
    
    pipe_array = associated_pipe_array->pipe_array;
    
    pipe_array->bc = calloc(1, sizeof(fluid_properties_t));
    wasora_call(mate_safe_copy_fluid_properties(pipe_array->bc, dummy_fluid));
    wasora_call(mate_set_fluid_mass_flow_m(pipe_array->bc, mate_get_fluid_mass_flow_m(dummy_fluid)));
    
    for (i = 0; i < pipe_array->n_pipes; i++) {
      
      pipe = pipe_array->pipe[i];
      
      // allocamos memoria para las condiciones de contorno del pipe
      pipe->bc = calloc(1, sizeof(fluid_properties_t));
      
      // copiamos las propiedades del fluido a la entrada del pipe array a la entrada del pipe que estamos mirando
      wasora_call(mate_safe_copy_fluid_properties(pipe->bc, pipe_array->bc));
      
      // estimamos como guess inicial que el caudal masico total del pipe array se reparte uniformemente entre los pipes
      wasora_call(mate_set_fluid_mass_flow_m(pipe->bc, mate_get_fluid_mass_flow_m(pipe_array->bc) / pipe_array->n_pipes));
    }
    
    // ahora si mandamos a resolver el pipe array
    wasora_call(mate_linear_solve_pipe_array(pipe_array));
    
    // y rellenamos las funciones resultado
    wasora_call(mate_fill_result_functions_values(pipe_array));
    
    // computamos las nuevas boundary conditions para el nuevo pipe
    // TODO: fundamental!
    // debemos actualizar las condicones sobre dummy fluid para resolver el siguiente pipe array
    // aca tiene que existir un balance de energia a partir de los ultimos nodos de cada pipe del pipe array que acabamos de resolver!
    // dummy_fluid
  }
  
  free(dummy_fluid);
  
  return WASORA_RUNTIME_OK;
}

int mate_linear_solve_pipe_array(pipe_array_t *pipe_array) {
  
  int i;
  int node_pipe_index;
  int form_loss_pipe_index;
  int point_pipe_index;
  int node_pipe_array_index;
  int form_loss_pipe_array_index;
  
  double dp;
  double T_residual;
  double p_residual;
  double h_residual;
  
  pipe_t *pipe;
  mate_node_t *node;
  form_loss_t *form_loss;
  fluid_properties_t *dummy_fluid = calloc(1, sizeof(fluid_properties_t));
  solid_properties_t *dummy_solid = calloc(1, sizeof(solid_properties_t));
  
  // iteramos con un nuevo guess de mass flows en cada pipe del pipe array
  for (pipe_array->solver->outer_iter = 0; pipe_array->solver->outer_iter < MAX_OUTER_ITERATIONS; pipe_array->solver->outer_iter++) {
    
    for (i = 0; i < pipe_array->n_pipes; i++) {
      
      pipe = pipe_array->pipe[i];
      //printf("%s\n", pipe->name);
      
      // copiamos a una estructura dummy las condiciones de contorno del pipe: presion, temperatura o entalpia y el mass flow
      wasora_call(mate_safe_copy_fluid_properties(dummy_fluid, pipe->bc));
      wasora_call(mate_set_fluid_mass_flow_m(dummy_fluid, mate_get_fluid_mass_flow_m(pipe->bc)));
      
      // como condicion de contorno del solido usamos la temperatura boundary del fluido
      wasora_call(mate_set_solid_T_m(dummy_solid, mate_get_fluid_T(dummy_fluid)));
      wasora_call(mate_set_solid_T_i(dummy_solid, mate_get_fluid_T(dummy_fluid)));
      wasora_call(mate_set_solid_T_o(dummy_solid, mate_get_fluid_T(dummy_fluid)));
      
      
      //// nos aseguramos de resetear los nodos a buscar en cada pipe si hay subcooled boiling
      //if (mate.solver.sub_boiling.on)
      //  mate_parallel_call(mate_reset_important_nodes(pipe->id));
      
      
      // recorremos los puntos de calculo del pipe: nodos + form losses
      for (node_pipe_index = 0, form_loss_pipe_index = 0, point_pipe_index = 0; point_pipe_index < pipe_array->solver->n_points_per_pipe[i]; point_pipe_index++) {
        
        // si el punto de calculo es un nodo
        if (pipe_array->solver->point_type[i][point_pipe_index] == NODE_CALC_POINT) {
          
          // computamos el indice global del nodo
          node_pipe_array_index = mate_compute_node_pipe_array_index(pipe_array, i, node_pipe_index);
          
          // apuntamos al correspondiente nodo
          node = pipe_array->solver->node_by_pipe_array_index[node_pipe_array_index];
          
          // seteamos la condicion inicial del fluido
          wasora_call(mate_safe_copy_fluid_properties(node->fluid_properties, dummy_fluid));
          wasora_call(mate_set_fluid_mass_flow_m(node->fluid_properties, mate_get_fluid_mass_flow_m(dummy_fluid)));
          
          // seteamos la condicion inicial del solido
          wasora_call(mate_copy_solid_properties(node->solid_properties, dummy_solid));
          
          // iteramos hasta resolver el nodo
          for (pipe_array->solver->inner_iter[i] = 0; pipe_array->solver->inner_iter[i] < MAX_INNER_ITERATIONS; pipe_array->solver->inner_iter[i]++) {
            
            //printf("inner iter %d\n", pipe_array->solver->inner_iter[i]);
            
            // antes de calcular algo, hacemos las variables de wasora equivalentes a las variables del nodo
            // FIXME! por ahora esto lo reparti entre las funciones que calculan cada cosa, para no calcular
            // con ceros algun parametro (excepto a la coordenadas del nodo)!
            wasora_call(mate_set_special_vars(node));
            
            // calculamos los parametros del fluido
            wasora_call(mate_compute_node_fluid_parameters(node));
            
            // calculamos los parametros del solido
            wasora_call(mate_compute_node_solid_k(node));
            
            // resolvemos la presion y entalpia del fluido
            if (node->node_section_index != 0)
              wasora_call(mate_compute_node_fluid_balances(node));
            
            // resolvemos la temperatura del solido
            if (node->node_pipe_index == 0 || node->node_section_index != 0)
              wasora_call(mate_compute_node_solid_T(node));
            
            // almacenamos el resultado actual
            pipe_array->solver->fluid_h = mate_get_fluid_h(node->fluid_properties);
            pipe_array->solver->fluid_p = mate_get_fluid_p(node->fluid_properties);
            pipe_array->solver->solid_T = mate_get_solid_T_m(node->solid_properties);
            
            // por lo menos tenemos que iterar dos veces:
            //    i = 0: no tenemos un valor old
            //    i = 1: tenemos tanto un valor old como el actual (podemos confiar el el residuo)
            if (pipe_array->solver->inner_iter[i] > 0) {
              
              // miramos si los valores han convergido
              wasora_call(mate_compute_distribution_residual(&h_residual, &pipe_array->solver->fluid_h, &pipe_array->solver->fluid_h_old, 1));
              wasora_call(mate_compute_distribution_residual(&p_residual, &pipe_array->solver->fluid_p, &pipe_array->solver->fluid_p_old, 1));
              wasora_call(mate_compute_distribution_residual(&T_residual, &pipe_array->solver->solid_T, &pipe_array->solver->solid_T_old, 1));
              
              // si lo hicieron, nos vamos de este nodo
              if (h_residual < MAX_RESIDUAL && p_residual < MAX_RESIDUAL && T_residual < MAX_RESIDUAL) {
                //printf("node: %d\n", node->node_pipe_index);
                break;
              }
            }
            
            // copiamos el valor actual en el valor old
            pipe_array->solver->fluid_h_old = pipe_array->solver->fluid_h;
            pipe_array->solver->fluid_p_old = pipe_array->solver->fluid_p;
            pipe_array->solver->solid_T_old = pipe_array->solver->solid_T;
          }
          
          // usamos como condicion inicial para el siguiente nodo el resultado del nodo actual
          wasora_call(mate_safe_copy_fluid_properties(dummy_fluid, node->fluid_properties));
          wasora_call(mate_copy_solid_properties(dummy_solid, node->solid_properties));
          
          // cuando nos vamos de este nodo, incrementamos el indice nodal
          node_pipe_index++;
          
          // si el punto de calculo es una form loss
        } else {
          
          // computamos el indice global de la junction
          form_loss_pipe_array_index = mate_compute_form_loss_pipe_array_index(pipe_array, i, form_loss_pipe_index);
          
          // apuntamos a la correspondiente junction 
          form_loss = pipe_array->solver->form_loss_by_pipe_array_index[form_loss_pipe_array_index];
          
          // computamos la perdida de carga localizada (form loss se suele llamar en la bibliografia)
          // dp = mate_compute_localized_dp(boundary, junction);
          dp = 0;
          
          // almacenamos en boundary las nuevas propiedades
          // TODO: con esto seguro haga algo como con safe copy, es decir, que se fije en que region esta y setee a partir de ahi con pT o con ph
          //wasora_call(mate_safe_set_fluid_properties_pT(mate_get_fluid_p(&boundary) + dp, mate_get_fluid_h(&boundary, IAPWS_UNSPECIFIED_REGION), mate_get_fluid_T(&boundary)));
          /*
           if (node_pipe_index == 0) {
           wasora_call(mate_copy_fluid_properties_pT(dummy_fluid, mate_get_fluid_p(dummy_fluid) + dp, mate_get_fluid_T(dummy_fluid)));
           } else {
           wasora_call(mate_set_fluid_properties_ph(dummy_fluid, mate_get_fluid_p(dummy_fluid) + dp, mate_get_fluid_h(dummy_fluid)));
           }
           */
          
          // cuando nos vamos de esta junction, incrementamos el indice
          form_loss_pipe_index++;
        }
      }
    }
    
    // computamos los factores lambda
    wasora_call(mate_compute_lambda_factors(pipe_array));
    
    // computamos el factor Gamma con el fin de conservar la masa
    wasora_call(mate_compute_Gamma_factor(pipe_array));
    
    // verificamos si estamos en condiciones de finalizar las iteraciones externas
    if (mate_analyze_factors(pipe_array)) 
      break;
    
    // si no nos fuimos, corregimos
    wasora_call(mate_correct_mass_flows(pipe_array));
  }
  
  free(dummy_fluid);
  free(dummy_solid);
  
  return WASORA_RUNTIME_OK;
}

/*
int mate_parallel_solve_pipe_array(pipe_array_t *pipe_array) {
  
  int i;
  int node_pipe_index;
  int form_loss_pipe_index;
  int point_pipe_index;
  int node_pipe_array_index;
  int form_loss_pipe_array_index;
  
  double dp;
  double T_residual;
  double p_residual;
  double h_residual;
  
  pipe_t *pipe;
  mate_node_t *node;
  form_loss_t *form_loss;
  fluid_properties_t *dummy_fluid = calloc(1, sizeof(fluid_properties_t));
  solid_properties_t *dummy_solid = calloc(1, sizeof(solid_properties_t));
    
  // iteramos con un nuevo guess de mass flows en cada pipe del pipe array
  for (pipe_array->solver->outer_iter = 0; pipe_array->solver->outer_iter < MAX_OUTER_ITERATIONS; pipe_array->solver->outer_iter++) {
    
#pragma omp parallel for private(pipe, node, form_loss, dummy_fluid, dummy_solid, node_pipe_index, form_loss_pipe_index, point_pipe_index, node_pipe_array_index, form_loss_pipe_array_index, dp, T_residual, p_residual, h_residual)
    for (i = 0; i < pipe_array->n_pipes; i++) {
      
      // si el solver ha abortado, el for loop sobre los pipes es dummy y no resolvemos nada
      if (!pipe_array->solver->abort) {
        
        pipe = pipe_array->pipe[i];
        
        // copiamos a una estructura dummy las condiciones de contorno del pipe: presion, temperatura o entalpia y el mass flow
        mate_parallel_call(mate_safe_copy_fluid_properties(dummy_fluid, pipe->bc));
        mate_parallel_call(mate_set_fluid_mass_flow_m(dummy_fluid, mate_get_fluid_mass_flow_m(pipe->bc)));
        
        // como condicion de contorno del solido usamos la temperatura boundary del fluido
        mate_parallel_call(mate_set_solid_T_m(dummy_solid, mate_get_fluid_T(dummy_fluid)));
        mate_parallel_call(mate_set_solid_T_i(dummy_solid, mate_get_fluid_T(dummy_fluid)));
        mate_parallel_call(mate_set_solid_T_o(dummy_solid, mate_get_fluid_T(dummy_fluid)));
        
        
        //// nos aseguramos de resetear los nodos a buscar en cada pipe si hay subcooled boiling
        //if (mate.solver.sub_boiling.on)
        //  mate_parallel_call(mate_reset_important_nodes(pipe->id));
        
        
        // recorremos los puntos de calculo del pipe: nodos + form losses
        for (node_pipe_index = 0, form_loss_pipe_index = 0, point_pipe_index = 0; point_pipe_index < pipe_array->solver->n_points_per_pipe[i]; point_pipe_index++) {
          
          // si el punto de calculo es un nodo
          if (pipe_array->solver->point_type[i][point_pipe_index] == NODE_CALC_POINT) {
            
            // computamos el indice global del nodo
            node_pipe_array_index = mate_compute_node_pipe_array_index(pipe_array, i, node_pipe_index);
            
            // apuntamos al correspondiente nodo
            node = pipe_array->solver->node_by_pipe_array_index[node_pipe_array_index];
            
            // seteamos la condicion inicial del fluido
            mate_parallel_call(mate_safe_copy_fluid_properties(node->fluid_properties, dummy_fluid));
            mate_parallel_call(mate_set_fluid_mass_flow_m(node->fluid_properties, mate_get_fluid_mass_flow_m(dummy_fluid)));
            
            // seteamos la condicion inicial del solido
            mate_parallel_call(mate_copy_solid_properties(node->solid_properties, dummy_solid));
            
            // iteramos hasta resolver el nodo
            for (pipe_array->solver->inner_iter[i] = 0; pipe_array->solver->inner_iter[i] < MAX_INNER_ITERATIONS; pipe_array->solver->inner_iter[i]++) {
              
              // antes de calcular cualquier cosa, hacemos las variables de wasora equivalentes a las variables del nodo
              // OBS: esto creo que no me permite paralelizar bien, porque dos nodos diferentes haran que la misma variable tome dif valores 
              // en un mismo instante de calculo y esto dara fruta!
              mate_parallel_call(mate_set_special_vars(node));
              
              // calculamos los parametros del fluido
              mate_parallel_call(mate_compute_node_fluid_parameters(node));
              
              // calculamos los parametros del solido
              mate_parallel_call(mate_compute_node_solid_k(node));
              
              // resolvemos la presion y entalpia del fluido
              if (node->node_section_index != 0)
                mate_parallel_call(mate_compute_node_fluid_balances(node));
              
              // resolvemos la temperatura del solido
              if (node->node_pipe_index == 0 || node->node_section_index != 0)
                mate_parallel_call(mate_compute_node_solid_T(node));
              
              // almacenamos el resultado actual
              pipe_array->solver->fluid_h = mate_get_fluid_h(node->fluid_properties);
              pipe_array->solver->fluid_p = mate_get_fluid_p(node->fluid_properties);
              pipe_array->solver->solid_T = mate_get_solid_T_m(node->solid_properties);
              
              // por lo menos tenemos que iterar dos veces:
              //    i = 0: no tenemos un valor old
              //    i = 1: tenemos tanto un valor old como el actual (podemos confiar el el residuo)
              if (pipe_array->solver->inner_iter[i] > 0) {
                
                // miramos si los valores han convergido
                mate_parallel_call(mate_compute_distribution_residual(&h_residual, &pipe_array->solver->fluid_h, &pipe_array->solver->fluid_h_old, 1));
                mate_parallel_call(mate_compute_distribution_residual(&p_residual, &pipe_array->solver->fluid_p, &pipe_array->solver->fluid_p_old, 1));
                mate_parallel_call(mate_compute_distribution_residual(&T_residual, &pipe_array->solver->solid_T, &pipe_array->solver->solid_T_old, 1));
                
                // si lo hicieron, nos vamos de este nodo
                if (h_residual < MAX_RESIDUAL && p_residual < MAX_RESIDUAL && T_residual < MAX_RESIDUAL) 
                  break;
              }
              
              // copiamos el valor actual en el valor old
              pipe_array->solver->fluid_h_old = pipe_array->solver->fluid_h;
              pipe_array->solver->fluid_p_old = pipe_array->solver->fluid_p;
              pipe_array->solver->solid_T_old = pipe_array->solver->solid_T;
            }
            
            // usamos como condicion inicial para el siguiente nodo el resultado del nodo actual
            mate_parallel_call(mate_safe_copy_fluid_properties(dummy_fluid, node->fluid_properties));
            mate_parallel_call(mate_copy_solid_properties(dummy_solid, node->solid_properties));
            
            // cuando nos vamos de este nodo, incrementamos el indice nodal
            node_pipe_index++;
            
            // si el punto de calculo es una form loss
          } else {
            
            // computamos el indice global de la junction
            form_loss_pipe_array_index = mate_compute_form_loss_pipe_array_index(pipe_array, i, form_loss_pipe_index);
            
            // apuntamos a la correspondiente junction 
            form_loss = pipe_array->solver->form_loss_by_pipe_array_index[form_loss_pipe_array_index];
            
            // computamos la perdida de carga localizada (form loss se suele llamar en la bibliografia)
            // dp = mate_compute_localized_dp(boundary, junction);
            dp = 0;
            
            // almacenamos en boundary las nuevas propiedades
            // TODO: con esto seguro haga algo como con safe copy, es decir, que se fije en que region esta y setee a partir de ahi con pT o con ph
            //mate_call(mate_safe_set_fluid_properties_pT(mate_get_fluid_p(&boundary) + dp, mate_get_fluid_h(&boundary, IAPWS_UNSPECIFIED_REGION), mate_get_fluid_T(&boundary)));

            //if (node_pipe_index == 0) {
            //  mate_parallel_call(mate_copy_fluid_properties_pT(dummy_fluid, mate_get_fluid_p(dummy_fluid) + dp, mate_get_fluid_T(dummy_fluid)));
            //} else {
            //  mate_parallel_call(mate_set_fluid_properties_ph(dummy_fluid, mate_get_fluid_p(dummy_fluid) + dp, mate_get_fluid_h(dummy_fluid)));
            //}

            
            // cuando nos vamos de esta junction, incrementamos el indice
            form_loss_pipe_index++;
          }
        }
      }
    }
    
    // antes de continuar chekeamos si el parallel loop aborto la corrida
    if (pipe_array->solver->abort) return WASORA_RUNTIME_ERROR;
    
    // computamos los factores lambda
    wasora_call(mate_compute_lambda_factors(pipe_array));
    
    // computamos el factor Gamma con el fin de conservar la masa
    wasora_call(mate_compute_Gamma_factor(pipe_array));
    
    // verificamos si estamos en condiciones de finalizar las iteraciones externas
    if (mate_analyze_factors(pipe_array)) 
      break;
    
    // si no nos fuimos, corregimos
    wasora_call(mate_correct_mass_flows(pipe_array));
  }
  
  free(dummy_fluid);
  free(dummy_solid);
  
  return WASORA_RUNTIME_OK;
}
*/

int mate_compute_distribution_residual(double *residual, double *new_dist, double *old_dist, int size) {
  
  int i;
  
  // computamos el residuo como el desvio cuadratico del error relativo (es la mejor forma de ver la convergencia)
  *residual = 0;
  for (i = 0; i < size; i++) {
    
    if (old_dist[i] > 0)
      *residual += pow((new_dist[i] - old_dist[i]) / old_dist[i], 2);
  }
  
  *residual = sqrt((*residual) / size);
  
  return WASORA_RUNTIME_OK;
}