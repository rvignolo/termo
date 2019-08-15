/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate main header
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
#ifndef _MATE_H_
#define _MATE_H_

#include <wasora.h>

#include <freesteam/steam.h>
#include <freesteam/steam_ph.h>
#include <freesteam/steam_ps.h>
#include <freesteam/steam_pu.h>
#include <freesteam/steam_pv.h>
#include <freesteam/steam_pT.h>
#include <freesteam/steam_Ts.h>
#include <freesteam/steam_Tx.h>
#include <freesteam/region1.h>
#include <freesteam/region2.h>
#include <freesteam/region4.h>
#include <freesteam/thcond.h>
#include <freesteam/surftens.h>
#include <freesteam/viscosity.h>

#define mate_parallel_call(function) if (!(pipe_array->solver->abort)) if ((function) != WASORA_RUNTIME_OK) pipe_array->solver->abort = 1

#define FORM_LOSS_CALC_POINT       0
#define NODE_CALC_POINT            1

#define MAX_INNER_ITERATIONS    1000
#define MAX_OUTER_ITERATIONS     100

#define MAX_RESIDUAL            1e-7

#define IAPWS_UNSPECIFIED_REGION   0
#define IAPWS_REGION_1             1
#define IAPWS_REGION_2             2
#define IAPWS_REGION_4             4

#define UNSPECIFIED_CASE           0
#define LIQUID_ONLY_CASE           1
#define VAPOUR_ONLY_CASE           2
#define TWO_PHASE_CASE             3

#define GRAVITY              9.80665

typedef struct mate_node_t mate_node_t;
typedef struct mate_volume_t mate_volume_t;

typedef struct section_t section_t;
typedef struct straight_section_t straight_section_t;
typedef struct circular_section_t circular_section_t; 
typedef struct helical_section_t helical_section_t;

typedef struct form_loss_t form_loss_t;
typedef struct bend_form_loss_t bend_form_loss_t;
typedef struct elbow_form_loss_t elbow_form_loss_t;

typedef struct pipe_t pipe_t;
typedef struct component_list_item_t component_list_item_t;

typedef struct pipe_array_t pipe_array_t;
typedef struct pipe_array_solver_t pipe_array_solver_t;

typedef struct system_t system_t;
typedef struct pipe_array_list_item_t pipe_array_list_item_t;

typedef struct fluid_properties_t fluid_properties_t;
typedef struct solid_properties_t solid_properties_t;

typedef struct mate_bc_t mate_bc_t;

typedef struct mate_step_t mate_step_t;

typedef struct solid_material_t solid_material_t;

struct mate_node_t {
  int node_section_index;        // indice del nodo respecto a la seccion que pertenece
  int node_pipe_index;           // indice del nodo respecto al pipe que pertenece
  int node_pipe_array_index;     // indice del nodo respecto al sistema que pertenece (recordar que mate actualmente resuelve UN sistema por lo que esto es como un indice global)
  
  double s;                      // coordenada curvilinea sobre el pipe (no sobre la seccion)
  double x[3];                   // coordenadas espaciales del nodo
  
  fluid_properties_t *fluid_properties;    // propiedades del fluido en el nodo
  solid_properties_t *solid_properties;    // propiedades de la pared en el nodo
  
  section_t *section;                      // seccion a la que pertenece el nodo
  
  mate_volume_t *backward_fluid_volume;    // backward fluid volume asociado al nodo (los nodos con local_section_id = 1 tienen un forward volumen asociado)
  mate_volume_t *backward_solid_volume;    // backward solid volume asociado al nodo (los nodos con local_section_id = 1 tienen un forward volumen asociado)
};

struct mate_volume_t {           // un volumen de mate hace referencia a una geometria anular
  int volume_section_index;      // indice del volumen respecto a la seccion que pertenece
  int volume_pipe_index;         // indice del volumen respecto al pipe que pertenece
  int volume_pipe_array_index;   // indice del volumen respecto al sistema que pertenece (recordar que mate actualmente resuelve UN sistema por lo que esto es como un indice global)
  
  double L;                      // longitud del volumen medida sobre el tipo de seccion
  
  double Di;                     // diametro interno del volumen
  double Do;                     // diametro externo del volumen
  
  double Pi;                     // perimetro interno
  double Po;                     // perimetro externo
  
  double Af;                     // area transversal del volumen
  double Ai;                     // area lateral interna del volumen
  double Ao;                     // area lateral externa del volumen
  
  double volume;                 // volumen del volumen
  
  section_t *section;            // seccion a la que pertenece el volumen
  
  mate_node_t *initial_node;     // nodo inicial asociado al volumen
  mate_node_t *final_node;       // nodo final asociado al volumen
};


struct section_t {
  char *name;                    // nombre de la seccion
  
  int initialized;               // flag de inicializacion
  
  enum {
    straight_section_type,
    circular_section_type,
    helical_section_type
  } type;
  
  void *section_params;          // apuntador a una estructura con parametros geometricos que solo dependen del tipo de seccion
  
  int special_mesh;              // cuando definimos una malla a partir de nodos en un file, decimos que es special (puede contener nodos circulares o helicoidales)
  file_t *file;                  // archivo con los datos de los nodos (pseudo malla)
  
  int column[4];                 // columnas donde hay que ir a buscar los datos (id, x_cord, y_cord, z_cord)
  
  expr_t * expr_first_node_id;
  int first_node_id;             // id del nodo inicial a leer en data_file
  expr_t * expr_last_node_id;
  int last_node_id;              // id del nodo final a leer en data_file
  
  expr_t * expr_n_nodes;
  int n_nodes;                   // numero de nodos que conforman la seccion
  int n_volumes;                 // numero de volumenes que conforman la seccion
  
  solid_material_t *solid_material;    // por si la seccion tiene asociada una pared
  
  expr_t * expr_L;
  double L;                      // longitud total de la seccion
  
  expr_t * expr_D;
  double D;                      // diametro hidraulico, termico y geometrico de la seccion: mate acepta solo volumenes hidrodinamicos cilindricos
  
  expr_t *expr_e;
  double e;                      // espesor de la pared de la seccion
  
  mate_node_t *node;             // apuntador a nodos de la seccion
  
  mate_volume_t *fluid_volume;   // apuntador a los volumenes fluid de la seccion
  mate_volume_t *solid_volume;   // apuntador a los volumenes solid de la seccion
  
  pipe_t *pipe;                  // pipe al que pertenece la seccion
  
  UT_hash_handle hh;
};

struct straight_section_t {
  expr_t *expr_theta;
  double theta;                  // angulo polar o de inclinacion de la seccion $-\frac{\pi}{2} \leq \theta \leq \frac{\pi}{2}$
};

struct circular_section_t {
  expr_t *expr_R;
  double R;                      // radio de la seccion circular
};

struct helical_section_t {
  expr_t *expr_R;
  double R;                      // radio de la seccion helicoidal
  expr_t *expr_p;
  double p;                      // pitch de la seccion helicoidal
};


struct form_loss_t {
  char *name;                    // nombre de la perdida de carga localizada
  
  int initialized;               // flag de inicializacion
  
  int form_loss_pipe_index;       // indice de la form loss respecto al pipe que pertenece
  int form_loss_pipe_array_index; // indice de la form loss respecto al pipe array que pertenece
  
  enum {
    //contraction_form_loss_type,
    //expansion_form_loss_type,
    bend_form_loss_type,
    elbow_form_loss_type,
    constant_form_loss_type
  } type;
  
  void *loss;                    // apuntador a un elemento del tipo de form loss que contiene los parametros geometricos de la misma
  
  expr_t *expr_K;
  double K;                      // coeficiente de perdida de carga localizado
  
  pipe_t *pipe;                  // pipe al que pertenece la form loss
  
  UT_hash_handle hh;
};

struct bend_form_loss_t {
  expr_t *expr_Ro_Dh;
  double Ro_Dh;
  expr_t *expr_phi;
  double phi;
};

struct elbow_form_loss_t {
  expr_t *expr_Ro_Dh;
  double Ro_Dh;
  expr_t *expr_phi;
  double phi;
};


struct pipe_t {
  char *name;                    // nombre del pipe
  
  int initialized;               // flag de inicializacion
  
  double lambda;                 // factor de correccion local para el mass_flow del pipe
  
  fluid_properties_t *bc;        // las boundary conditions del pipe
  
  struct {
    char *ho_string;
    expr_t *expr_ho;             // coeficiente de transferencia de calor externo del  pipe
    
    char *To_string;
    expr_t *expr_To;             // temperatura externa en cada pipe
    
    char *hi_string;
    expr_t *expr_hi;             // coeficiente de transferencia de calor interno del pipe (si es null no se reemplaza formulacion propia de mate)
    
    char *fi_string;
    expr_t *expr_fi;             // factor de friccion interno del pipe (si es null no se reemplaza formulacion propia de mate)
  } boundary_expr;
  
  mate_node_t *node_onb;         // nodo donde comienza el nucleate boiling del pipe (null si no hay subcooled boiling)
  mate_node_t *node_obd;         // nodo donde comienza el bubble departure del pipe (null si no hay subcooled boiling)
  mate_node_t *node_osb;         // nodo donde comienza el saturaded boiling del pipe
  
  struct {                       // cada funcion es una solucion sobre la coordenada curvilinea del nodo sobre el pipe
    function_t *pi;              // presion interna del pipe
    function_t *hi;              // coeficiente de transferencia de calor interno del pipe
    function_t *Ti;              // temperatura interna del pipe
    function_t *Tw;              // temperatura de pared del pipe
  } result_functions;
  
  pipe_array_t *pipe_array;      // pipe array al que pertenece el pipe
  
  component_list_item_t *associated_components;  // linked list de componentes que forman al pipe (secciones o form losses)
  
  UT_hash_handle hh;
};

struct component_list_item_t {
  enum {
    form_loss_type,
    section_type
  } type;
  
  void *component;               // apuntador a la seccion o a la form loss
  
  component_list_item_t *next;   // siguiente componente de la lista (puede ser una nueva seccion o una form loss)
};


struct pipe_array_t {
  char *name;                    // nombre del pipe array
  
  int initialized;               // flag de inicializacion
  
  int n_pipes;                   // numero de pipes pertenecientes al pipe array
  
  pipe_t **pipe;                 // arreglo de pipes pertenecientes al pipe array
  
  double Gamma;                  // factor de correccion masico global para cada pipe del pipe array
  
  fluid_properties_t *bc;        // las boundary conditions del pipe array
  
  pipe_array_solver_t *solver;   // solver del pipe array
  
  system_t *system;              // system al que pertenece el pipe array
  
  UT_hash_handle hh;
};

struct pipe_array_solver_t {
  int abort;                   // flag que indica si el solver debe abortar
  
  int outer_iter;              // number of outer iterations
  int *inner_iter;             // es un maneje para poder paralelizar el solver con subcooled boiling on
  
  int *n_nodes_per_pipe;       // arreglo de cantidades de nodos por pipe
  int *n_form_losses_per_pipe; // arreglo de cantidades de form losses por pipe
  int *n_points_per_pipe;      // arreglo de cantidades de puntos de calculo por pipe (suma de los dos anteriores para cada pipe)
  
  int n_total_nodes;           // numero total de nodos del pipe array
  int n_total_form_losses;     // numero total de form losses del pipe array
  int n_total_points;          // numero total de puntos de calculo del pipe array (suma de los dos anteriores)
  
  int **point_type;            // indica si un punto de calculo del problema es tipo nodo o tipo form loss para un pipe i y punto de calculo j
  
  double fluid_h;              // entalpia del fluido actual en el nodo
  double fluid_h_old;          // entalpia del fluido previa en el nodo
  double fluid_p;              // presion del fluido actual en el nodo
  double fluid_p_old;          // presion del fluido previa en el nodo
  double solid_T;              // temperatura del solido actual en el nodo
  double solid_T_old;          // temperatura del solido previa en el nodo
  
  mate_node_t **node_by_pipe_array_index;      // arreglo que permite apuntar a un nodo a partir de un indice i sobre el pipe array (node_by_pipe_array_index[i] = mate_node_t *)
  
  form_loss_t **form_loss_by_pipe_array_index; // arreglo que permite apuntar a una form loss a partir de un indice i sobre el pipe array (form_loss_by_pipe_array_index[i] = form_loss_t *)
};


struct system_t {
  char *name;                  // nombre del pipe array
  
  int initialized;             // flag de inicializacion
  
  mate_bc_t *bc;               // las boundary conditions del system
  
  pipe_array_list_item_t *associated_pipe_arrays;  // linked list de pipe arrays que forman al system
  
  UT_hash_handle hh;
};

struct pipe_array_list_item_t {
  pipe_array_t *pipe_array;      // apuntador al pipe array
  pipe_array_list_item_t *next;  // siguiente elemento de la lista de pipe arrays que forman al sistema
};


struct mate_bc_t {
  int initialized;             // flag de inicializacion
  
  enum {
    ph,
    ps,
    pu,
    pv,
    pT,
    Ts,
    Tx
  } type;
  
  expr_t *expr_p;
  double p;
  expr_t *expr_h;
  double h;
  expr_t *expr_s;
  double s;
  expr_t *expr_u;
  double u;
  expr_t *expr_v;
  double v;
  expr_t *expr_T;
  double T;
  expr_t *expr_x;
  double x;
  
  fluid_properties_t *fluid_properties;
  
  expr_t *expr_m_dot;
};


struct fluid_properties_t {
  SteamState S;           // freesteam thermodynamic state
  
  enum  {
    single_phase_liquid,  // fluid node regime is single phase liquid
    attached_bubble,      // fluid node regime is attached_bubble (esta region comienza con onset of nucleate boiling)
    bubble_departure,     // fluid node regime is bubble departure
    saturated_boiling,    // fluid node regime is saturated nucleate boiling
    single_phase_vapor    // fluid node regime is single phase vapor
  } regime;
  
  double x;               // flow quality (real quality)
  double xe;              // equilibrium or thermodynamic quality
  
  double alpha;           // void fraction
  
  double mass_flow_m;     // mixture mass flow [kg/s]
  double mass_flow_l;     // liquid mass flow [kg/s]
  double mass_flow_v;     // vapour mass flow [kg/s]
  
  double mass_flux_m;     // mixture mass flux [kg/s/m2]
  double mass_flux_l;     // liquid mass flux [kg/s/m2]
  double mass_flux_v;     // vapour mass flux [kg/s/m2]
  
  //double h_m;           // static mixture enthalpy (legacy for steady state)
  double h_f;             // saturated liquid entalphy assuming p_node = p_sat
  double h_g;             // saturated vapour entalphy assuming p_node = p_sat
  double h_m_plus;        // dynamic mixture enthalpy; = h_m for one phase or HEM model
  
  double rho_m;           // static mixture density [kg/m3]
  double rho_f;           // saturated liquid density assuming p_node = p_sat [kg/m3]
  double rho_g;           // saturated vapour density assuming p_node = p_sat [kg/m3]
  double rho_l;           // liquid density if present; = rho_f in saturated boiling regime [kg/m3]
  double rho_v;           // vapour density if present; = rho_g in saturated boiling regime [kg/m3]
  double rho_m_plus;      // dynamic mixture density; = rho_m for one phase or HEM model [kg/m3]
  
  double mu_tp;           // McAdams approximation for mixture dynamic viscosity
  double mu_f;            // saturated liquid dynamic viscosity assuming p_node = p_sat
  double mu_g;            // saturated vapour dynamic viscosity assuming p_node = p_sat
  double mu_l;            // liquid dynamic viscosity if present; = mu_f in saturated boiling regime
  double mu_v;            // vapour dynamic viscosity if present; = mu_g in saturated boiling regime
  
  double cp_f;            // saturated liquid heat capacity assuming p_node = p_sat
  double cp_g;            // saturated vapour heat capacity assuming p_node = p_sat
  double cp_l;            // liquid heat capacity if present; = cp_f when saturated
  double cp_v;            // vapour heat capacity if present; = cp_g when saturated
  
  double k_f;             // saturated liquid thermal conductivity assuming p_node = p_sat
  double k_g;             // saturated vapour thermal conductivity assuming p_node = p_sat
  double k_l;             // liquid thermal conductivity if present; = k_f when saturated
  double k_v;             // vapour thermal conductivity if present; = k_g when saturated
  
  double Re_tp;           // Reynolds two phase number (McAdams dynamic viscosity)
  double Re_l;            // Reynolds liquid number
  double Re_v;            // Reynolds vapour number
  
  double Pr;              // Prandtl number
  
  double Nu;              // Nusselt number
};

struct solid_properties_t {
  double k;               // conductividad termica del solido
  
  double T_i;             // inner wall temperature
  double T_o;             // outer wall temperature
  double T_m;             // mean wall temperature
};

struct solid_material_t {
  expr_t *expr_k;                // conductividad termica
  expr_t *expr_epsilon;          // rugosidad
  double epsilon;
};


struct {
  int initialized;               // flag de inicializacion
  
  enum {
    HEM,                         // Homogeneous Equilibrium Mixture flow model
    SEP                          // Separate flow model (in this case, only thermodynamic equilibrium conditions are assumed)
  } flow_model;
  
  section_t *sections;           // secciones definidas en el input
  
  form_loss_t *form_losses;      // form losses definidas en el input
  
  pipe_t *pipes;                 // pipes definidios en el input
  
  pipe_array_t *pipe_arrays;     // pipe arrays definidios en el input
  
  //branch_t *branches;          // to be done
  
  //junction_t *junctions;       // to be done
  
  system_t *systems;             // systems definidos en el input
  
  system_t *main_system;         // es el ultimo system definido en un input
  
  system_t *system;              // apuntador al sistema a resolver por mate (se da en mate problem)
  
  int sub_boiling;               // flag que indica si se debe tener en cuenta la ebullicion subenfriada
  
  struct {
    var_t *s;                    // variable que toma el valor de la coordenada curvilinea en cada nodo del problema
    
    var_t *Tw;                   // variable que toma el valor de la temperatura de pared en cada nodo del problema
    
    var_t *Re_l;                 // variable que toma el valor del Reynolds liquido en cada nodo del problema
    var_t *Re_v;                 // variable que toma el valor del Reynolds vapor en cada nodo del problema
    var_t *Re_tp;                // variable que toma el valor del Reynolds two phase en cada nodo del problema
    
    var_t *Pr;                   // variable que toma el valor del Prandtl en cada nodo del problema
    
    var_t *Nu;                   // variable que toma el valor del Nusselt en cada nodo del problema
  } vars;
} mate;

struct mate_step_t {
  int do_not_solve;
  int do_not_parallelize;
  int n_cores;
};

// mate.c
extern int mate_instruction_step(void *);

// init.c
extern int plugin_init_before_parser(void);
extern int plugin_init_after_parser(void);
extern int plugin_init_before_run(void);
extern int plugin_finalize(void);

// parser.c
extern int plugin_parse_line(char *);

// define.c
extern int mate_check_name(const char *);

// node.c
extern int mate_read_section_nodes_from_file(section_t *);
extern int mate_create_section_structured_nodes(section_t *);
extern int mate_assign_nodes_backward_volume(section_t *);
extern int mate_compute_nodes_s_coordinate(pipe_t *);
extern int mate_allocate_pipe_array_node_properties(pipe_array_t *);

// volume.c
extern int mate_compute_section_volumes(section_t *);
extern int mate_set_volume_params(mate_volume_t *);

// section.c
extern section_t *mate_define_section(const char *, int);
extern section_t *mate_get_section_ptr(const char *);
extern int mate_instruction_section(void *);
extern int mate_evaluate_section_expressions(section_t *);
extern int mate_allocate_section_objects(section_t *);

// form_loss.c
extern form_loss_t *mate_define_form_loss(const char *, int);
extern form_loss_t *mate_get_form_loss_ptr(const char *);

// pipe.c
extern pipe_t *mate_define_pipe(const char *);
extern pipe_t *mate_get_pipe_ptr(const char *);
extern int mate_append_component_to_pipe(component_list_item_t **, int, void *);
extern int mate_instruction_pipe(void *);

// pipe_array.c
extern pipe_array_t *mate_define_pipe_array(const char *, int);
extern pipe_array_t *mate_get_pipe_array_ptr(const char *);
extern int mate_set_pipe_array_indexes(pipe_array_t *);
extern int mate_compute_node_pipe_array_index(pipe_array_t *, int, int);
extern int mate_compute_form_loss_pipe_array_index(pipe_array_t *, int, int);

// system.c
extern system_t *mate_define_system(const char *);
extern system_t *mate_get_system_ptr(const char *);
extern int mate_append_pipe_array_to_system(pipe_array_list_item_t **, pipe_array_t *);
extern int mate_init_system(system_t *);
extern int mate_set_system_boundary_conditions(system_t *);
extern int mate_post_system(system_t *);
extern int mate_post_pipe_array(pipe_array_t *, FILE *);
extern int mate_post_pipe(pipe_t *, FILE *);

// solver.c
extern int mate_allocate_pipe_array_solver(pipe_array_t *);
extern int mate_compute_pipe_array_num_calc_points(pipe_array_t *);
extern int mate_point_pipe_array_solver_objects(pipe_array_t *);
extern int mate_solve_system(system_t *);
extern int mate_linear_solve_pipe_array(pipe_array_t *);
extern int mate_parallel_solve_pipe_array(pipe_array_t *);
extern int mate_compute_distribution_residual(double *, double *, double *, int);

// fluid.c
extern int mate_compute_node_fluid_parameters(mate_node_t *);
extern int mate_compute_node_fluid_enthalpies(mate_node_t *);
extern int mate_compute_node_fluid_regime(mate_node_t *);
extern int mate_compute_node_fluid_qualities(mate_node_t *);
extern int mate_compute_node_fluid_mass_flows(mate_node_t *);
extern int mate_compute_node_fluid_mass_fluxes(mate_node_t *);
extern int mate_compute_node_fluid_densities(mate_node_t *);
extern int mate_compute_node_fluid_dynamic_viscocities(mate_node_t *);
extern int mate_compute_node_fluid_heat_capacities(mate_node_t *);
extern int mate_compute_node_fluid_thermal_conductivities(mate_node_t *);
extern int mate_compute_node_fluid_dimensionless_numbers(mate_node_t *);
extern int mate_compute_node_fluid_Re(mate_node_t *);
extern int mate_compute_node_fluid_Pr(mate_node_t *);
extern int mate_compute_straight_section_node_fluid_Nu(mate_node_t *);
extern int mate_compute_straight_section_node_fluid_Nu_l(double *, mate_node_t *);
extern int mate_compute_straight_section_node_fluid_Nu_v(double *, mate_node_t *);
extern int mate_compute_circular_section_node_fluid_Nu(mate_node_t *);
extern int mate_compute_helical_section_node_fluid_Nu(mate_node_t *);
extern int mate_compute_helical_section_node_fluid_Nu_l(double *, mate_node_t *);
extern int mate_compute_helical_section_node_fluid_Nu_v(double *, mate_node_t *);
extern int mate_compute_Chen_F_and_S(double *, double *, mate_node_t *);
extern int mate_compute_inv_martinelli_parameter(double*, mate_node_t *);
extern int mate_compute_Chen_h_mic(double *, mate_node_t *);
extern int mate_compute_node_fluid_void_fraction(mate_node_t *);
extern int mate_compute_CISE_void_fraction(double *, mate_node_t *);
extern int mate_compute_node_fluid_mixture_densities(mate_node_t *);
extern int mate_compute_node_fluid_balances(mate_node_t *);

// solid.c
extern int mate_compute_node_solid_k(mate_node_t *);
extern int mate_compute_node_solid_T(mate_node_t *);

// mass.c
extern int mate_compute_lambda_factors(pipe_array_t *);
extern int mate_compute_Gamma_factor(pipe_array_t *);
extern int mate_analyze_factors(pipe_array_t *);
extern int mate_correct_mass_flows(pipe_array_t *);

// momentum.c
extern int mate_compute_node_fluid_momentum_balance(double *, mate_node_t *);
extern int mate_compute_gravity_dp(double *, mate_node_t *, mate_node_t *);
extern int mate_compute_acceleration_dp(double *, mate_node_t *, mate_node_t *);
extern int mate_compute_friction_dp(double *, mate_node_t *);
extern int mate_compute_node_fluid_f(double *, mate_node_t *, int);
extern int mate_compute_straight_section_node_fluid_f(double *, mate_node_t *, int);
extern int mate_compute_circular_section_node_fluid_f(double *, mate_node_t *, int);
extern int mate_compute_helical_section_node_fluid_f(double *, mate_node_t *, int);
extern int mate_compute_node_fluid_phi_lo(double *, mate_node_t *);
extern int mate_compute_node_fluid_HEM_phi_lo(double *, mate_node_t *);
extern int mate_compute_straight_section_node_fluid_SEP_phi_lo(double *, mate_node_t *);
extern int mate_compute_helical_section_node_fluid_SEP_phi_lo(double *, mate_node_t *);
extern int mate_compute_pipe_array_mean_dp(double *, pipe_array_t *);
extern int mate_compute_pipe_dp(double *, pipe_array_t *, int);

// energy.c
extern int mate_compute_node_fluid_energy_balance(double *, mate_node_t *);
extern int mate_compute_qr(double *, mate_node_t *);
extern int mate_compute_global_UA(double *, mate_node_t *);
extern double mate_compute_inner_convection_R(mate_node_t *);
extern double mate_compute_node_hi(mate_node_t *);
extern double mate_compute_solid_conduction_R(mate_node_t *);
extern double mate_compute_outer_convection_R(mate_node_t *);
extern double mate_compute_node_To(mate_node_t *);
extern double mate_compute_node_ho(mate_node_t *);

// set.c
extern int mate_safe_copy_fluid_properties(fluid_properties_t *, fluid_properties_t *);
extern int mate_copy_fluid_properties_pT(fluid_properties_t *, fluid_properties_t *);
extern int mate_copy_fluid_properties_ph(fluid_properties_t *, fluid_properties_t *);
extern int mate_set_fluid_properties_pT(fluid_properties_t *, double, double);
extern int mate_set_fluid_properties_ph(fluid_properties_t *, double, double);
extern int mate_set_fluid_x(fluid_properties_t *, double);
extern int mate_set_fluid_xe(fluid_properties_t *, double);
extern int mate_set_fluid_alpha(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flow_m(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flow_l(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flow_v(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flux_m(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flux_l(fluid_properties_t *, double);
extern int mate_set_fluid_mass_flux_v(fluid_properties_t *, double);
extern int mate_set_fluid_h_f(fluid_properties_t *, double);
extern int mate_set_fluid_h_g(fluid_properties_t *, double);
extern int mate_set_fluid_h_m_plus(fluid_properties_t *, double);
extern int mate_set_fluid_rho_m(fluid_properties_t *, double);
extern int mate_set_fluid_rho_f(fluid_properties_t *, double);
extern int mate_set_fluid_rho_g(fluid_properties_t *, double);
extern int mate_set_fluid_rho_l(fluid_properties_t *, double);
extern int mate_set_fluid_rho_v(fluid_properties_t *, double);
extern int mate_set_fluid_rho_m_plus(fluid_properties_t *, double);
extern int mate_set_fluid_mu_tp(fluid_properties_t *, double);
extern int mate_set_fluid_mu_f(fluid_properties_t *, double);
extern int mate_set_fluid_mu_g(fluid_properties_t *, double);
extern int mate_set_fluid_mu_l(fluid_properties_t *, double);
extern int mate_set_fluid_mu_v(fluid_properties_t *, double);
extern int mate_set_fluid_cp_f(fluid_properties_t *, double);
extern int mate_set_fluid_cp_g(fluid_properties_t *, double);
extern int mate_set_fluid_cp_l(fluid_properties_t *, double);
extern int mate_set_fluid_cp_v(fluid_properties_t *, double);
extern int mate_set_fluid_k_f(fluid_properties_t *, double);
extern int mate_set_fluid_k_g(fluid_properties_t *, double);
extern int mate_set_fluid_k_l(fluid_properties_t *, double);
extern int mate_set_fluid_k_v(fluid_properties_t *, double);
extern int mate_set_fluid_Re_tp(fluid_properties_t *, double);
extern int mate_set_fluid_Re_l(fluid_properties_t *, double);
extern int mate_set_fluid_Re_v(fluid_properties_t *, double);
extern int mate_set_fluid_Pr(fluid_properties_t *, double);
extern int mate_set_fluid_Nu(fluid_properties_t *, double);

extern int mate_copy_solid_properties(solid_properties_t *, solid_properties_t *);
extern int mate_set_solid_T_i(solid_properties_t *, double);
extern int mate_set_solid_T_o(solid_properties_t *, double);
extern int mate_set_solid_T_m(solid_properties_t *, double);
extern int mate_set_solid_k(solid_properties_t *, double);

extern int mate_set_special_vars(mate_node_t *);

// get.c
extern int mate_get_fluid_region(fluid_properties_t *);
extern int mate_get_fluid_regime(fluid_properties_t *);
extern double mate_get_fluid_p(fluid_properties_t *);
extern double mate_get_node_fluid_p(mate_node_t *);
extern double mate_get_fluid_T(fluid_properties_t *);
extern double mate_get_node_fluid_T(mate_node_t *);
extern double mate_get_fluid_h(fluid_properties_t *);
extern double mate_get_node_fluid_h(mate_node_t *);
extern double mate_get_fluid_rho(fluid_properties_t *);
extern double mate_get_fluid_mu(fluid_properties_t *);
extern double mate_get_fluid_cp(fluid_properties_t *);
extern double mate_get_fluid_k(fluid_properties_t *);
extern double mate_get_fluid_sigma(fluid_properties_t *);
extern double mate_get_fluid_x(fluid_properties_t *);
extern double mate_get_fluid_xe(fluid_properties_t *);
extern double mate_get_fluid_alpha(fluid_properties_t *);
extern double mate_get_fluid_mass_flow_m(fluid_properties_t *);
extern double mate_get_fluid_mass_flow_l(fluid_properties_t *);
extern double mate_get_fluid_mass_flow_v(fluid_properties_t *);
extern double mate_get_fluid_mass_flux_m(fluid_properties_t *);
extern double mate_get_fluid_mass_flux_l(fluid_properties_t *);
extern double mate_get_fluid_mass_flux_v(fluid_properties_t *);
extern double mate_get_fluid_h_f(fluid_properties_t *);
extern double mate_get_fluid_h_g(fluid_properties_t *);
extern double mate_get_fluid_h_m_plus(fluid_properties_t *);
extern double mate_get_fluid_rho_m(fluid_properties_t *);
extern double mate_get_fluid_rho_f(fluid_properties_t *);
extern double mate_get_fluid_rho_g(fluid_properties_t *);
extern double mate_get_fluid_rho_l(fluid_properties_t *);
extern double mate_get_fluid_rho_v(fluid_properties_t *);
extern double mate_get_fluid_rho_m_plus(fluid_properties_t *);
extern double mate_get_fluid_mu_tp(fluid_properties_t *);
extern double mate_get_fluid_mu_f(fluid_properties_t *);
extern double mate_get_fluid_mu_g(fluid_properties_t *);
extern double mate_get_fluid_mu_l(fluid_properties_t *);
extern double mate_get_fluid_mu_v(fluid_properties_t *);
extern double mate_get_fluid_cp_f(fluid_properties_t *);
extern double mate_get_fluid_cp_g(fluid_properties_t *);
extern double mate_get_fluid_cp_l(fluid_properties_t *);
extern double mate_get_fluid_cp_v(fluid_properties_t *);
extern double mate_get_fluid_k_f(fluid_properties_t *);
extern double mate_get_fluid_k_g(fluid_properties_t *);
extern double mate_get_fluid_k_l(fluid_properties_t *);
extern double mate_get_fluid_k_v(fluid_properties_t *);
extern double mate_get_fluid_Re_tp(fluid_properties_t *);
extern double mate_get_fluid_Re_l(fluid_properties_t *);
extern double mate_get_fluid_Re_v(fluid_properties_t *);
extern double mate_get_fluid_Pr(fluid_properties_t *);
extern double mate_get_fluid_Nu(fluid_properties_t *);

extern double mate_get_solid_T_i(solid_properties_t *);
extern double mate_get_solid_T_o(solid_properties_t *);
extern double mate_get_solid_T_m(solid_properties_t *);
extern double mate_get_node_solid_T_m(mate_node_t *);
extern double mate_get_solid_k(solid_properties_t *);

// result.c
extern int mate_fill_result_functions_args(pipe_array_t *);
extern int mate_fill_result_function_args(pipe_array_t *, int, function_t *);
extern int mate_fill_result_functions_values(pipe_array_t *);
extern int mate_fill_result_function_values(pipe_array_t *, int, function_t *, double (mate_node_t *));

// version.c
extern void mate_usage(char *);
extern void mate_version(FILE *, int, int);
extern void mate_license(FILE *);

extern const char *plugin_name(void);
extern const char *plugin_version(void);
extern const char *plugin_description(void);
extern const char *plugin_longversion(void);
extern const char *plugin_copyright(void);

#endif  /* _MATE_H_ */