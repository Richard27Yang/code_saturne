/*============================================================================
 * Manage the (generic) evaluation of extended definitions
 *============================================================================*/

/*
  This file is part of Code_Saturne, a general-purpose CFD tool.

  Copyright (C) 1998-2018 EDF S.A.

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
  Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*----------------------------------------------------------------------------*/

#include "cs_defs.h"

/*----------------------------------------------------------------------------
 * Standard C library headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*----------------------------------------------------------------------------
 * Local headers
 *----------------------------------------------------------------------------*/

#include <bft_mem.h>

#include "cs_defs.h"
#include "cs_mesh_location.h"
#include "cs_reco.h"

/*----------------------------------------------------------------------------
 * Header for the current file
 *----------------------------------------------------------------------------*/

#include "cs_xdef_eval.h"

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

/*=============================================================================
 * Local Macro definitions and structure definitions
 *============================================================================*/

/* Redefined the name of functions from cs_math to get shorter names */
#define _dp3  cs_math_3_dot_product

static const char _err_empty_array[] =
  " %s: Array storing the evaluation should be allocated before the call"
  " to this function.";

/*============================================================================
 * Private function prototypes
 *============================================================================*/

/*============================================================================
 * Public function prototypes
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a scalar-valued quantity for a list of elements
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_scalar_by_val(cs_lnum_t                    n_elts,
                           const cs_lnum_t             *elt_ids,
                           bool                         compact,
                           const cs_mesh_t             *mesh,
                           const cs_cdo_connect_t      *connect,
                           const cs_cdo_quantities_t   *quant,
                           const cs_time_step_t        *ts,
                           void                        *input,
                           cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(quant);
  CS_UNUSED(connect);
  CS_UNUSED(ts);
  assert(eval != NULL);

  const cs_real_t  *constant_val = (cs_real_t *)input;

  if (elt_ids != NULL && !compact) {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++)
      eval[elt_ids[i]] = constant_val[0];

  }
  else {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++)
      eval[i] = constant_val[0];

  }
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a scalar-valued quantity by a cellwise process
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_scalar_by_val(const cs_cell_mesh_t     *cm,
                              const cs_time_step_t     *ts,
                              void                     *input,
                              cs_real_t                *eval)
{
  CS_UNUSED(cm);
  CS_UNUSED(ts);

  cs_real_t  *constant_val = (cs_real_t *)input;
  *eval = constant_val[0];
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a vector-valued quantity for a list of elements
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_vector_by_val(cs_lnum_t                    n_elts,
                           const cs_lnum_t             *elt_ids,
                           bool                         compact,
                           const cs_mesh_t             *mesh,
                           const cs_cdo_connect_t      *connect,
                           const cs_cdo_quantities_t   *quant,
                           const cs_time_step_t        *ts,
                           void                        *input,
                           cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(quant);
  CS_UNUSED(connect);
  CS_UNUSED(ts);
  assert(eval != NULL);

  const cs_real_t  *constant_val = (cs_real_t *)input;

  if (elt_ids != NULL && !compact) {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++) {
      const cs_lnum_t  id = elt_ids[i];
      eval[3*id  ] = constant_val[0];
      eval[3*id+1] = constant_val[1];
      eval[3*id+2] = constant_val[2];
    }

  }
  else {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++) {
      eval[3*i  ] = constant_val[0];
      eval[3*i+1] = constant_val[1];
      eval[3*i+2] = constant_val[2];
    }

  }
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a vector-valued quantity by a cellwise process
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_vector_by_val(const cs_cell_mesh_t     *cm,
                              const cs_time_step_t     *ts,
                              void                     *input,
                              cs_real_t                *eval)
{
  CS_UNUSED(cm);
  CS_UNUSED(ts);

  const cs_real_t  *constant_val = (cs_real_t *)input;

  eval[0] = constant_val[0];
  eval[1] = constant_val[1];
  eval[2] = constant_val[2];
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a tensor-valued quantity for a list of elements
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_tensor_by_val(cs_lnum_t                    n_elts,
                           const cs_lnum_t             *elt_ids,
                           bool                         compact,
                           const cs_mesh_t             *mesh,
                           const cs_cdo_connect_t      *connect,
                           const cs_cdo_quantities_t   *quant,
                           const cs_time_step_t        *ts,
                           void                        *input,
                           cs_real_t                   *eval)
{
  CS_UNUSED(quant);
  CS_UNUSED(mesh);
  CS_UNUSED(connect);
  CS_UNUSED(ts);
  assert(eval != NULL);

  const cs_real_3_t  *constant_val = (const cs_real_3_t *)input;

  if (elt_ids != NULL && !compact) {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++) {

      const cs_lnum_t  id = elt_ids[i];
      cs_real_t  *shift_eval = eval + 9*id;
      for (int ki = 0; ki < 3; ki++)
        for (int kj = 0; kj < 3; kj++)
          shift_eval[3*ki+kj] = constant_val[ki][kj];

    }

  }
  else {

#   pragma omp parallel for if (n_elts > CS_THR_MIN)
    for (cs_lnum_t i = 0; i < n_elts; i++) {

      cs_real_t  *shift_eval = eval + 9*i;
      for (int ki = 0; ki < 3; ki++)
        for (int kj = 0; kj < 3; kj++)
          shift_eval[3*ki+kj] = constant_val[ki][kj];

    }

  }
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a tensor-valued quantity by a cellwise process
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_tensor_by_val(const cs_cell_mesh_t     *cm,
                              const cs_time_step_t     *ts,
                              void                     *input,
                              cs_real_t                *eval)
{
  CS_UNUSED(cm);
  CS_UNUSED(ts);

  const cs_real_3_t  *constant_val = (const cs_real_3_t *)input;
  for (int ki = 0; ki < 3; ki++)
    for (int kj = 0; kj < 3; kj++)
      eval[3*ki+kj] = constant_val[ki][kj];
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined at cells using an analytic function
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_at_cells_by_analytic(cs_lnum_t                    n_elts,
                                  const cs_lnum_t             *elt_ids,
                                  bool                         compact,
                                  const cs_mesh_t             *mesh,
                                  const cs_cdo_connect_t      *connect,
                                  const cs_cdo_quantities_t   *quant,
                                  const cs_time_step_t        *ts,
                                  void                        *input,
                                  cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(connect);

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  /* Evaluate the function for this time at the cell center */
  anai->func(ts->t_cur,
             n_elts, elt_ids, quant->cell_centers,
             compact, /* Is output compacted ? */
             anai->input,
             eval);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined at border faces using an analytic
 *         function
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_at_b_faces_by_analytic(cs_lnum_t                    n_elts,
                                    const cs_lnum_t             *elt_ids,
                                    bool                         compact,
                                    const cs_mesh_t             *mesh,
                                    const cs_cdo_connect_t      *connect,
                                    const cs_cdo_quantities_t   *quant,
                                    const cs_time_step_t        *ts,
                                    void                        *input,
                                    cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(connect);

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  /* Evaluate the function for this time at the border face center */
  anai->func(ts->t_cur,
             n_elts, elt_ids, quant->b_face_center,
             compact,  // compacted output ?
             anai->input,
             eval);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined at border faces using an analytic
 *         function
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[in]  qtype     quadrature type
 * \param[in]  dim       dimension of the analytic function return
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_avg_at_b_faces_by_analytic(cs_lnum_t                    n_elts,
                                        const cs_lnum_t             *elt_ids,
                                        bool                         compact,
                                        const cs_mesh_t             *mesh,
                                        const cs_cdo_connect_t      *connect,
                                        const cs_cdo_quantities_t   *quant,
                                        const cs_time_step_t        *ts,
                                        void                        *input,
                                        cs_quadrature_type_t         qtype,
                                        const short int              dim,
                                        cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(compact);

  cs_quadrature_tria_integral_t  *qfunc = NULL;
  cs_xdef_analytic_input_t *anai = (cs_xdef_analytic_input_t *)input;

  switch (dim) {

  case 1: /* Scalar-valued case */
    {
      switch (qtype) {
        /* Barycenter of the tetrahedral subdiv. */
      case CS_QUADRATURE_BARY:
      case CS_QUADRATURE_BARY_SUBDIV:
        qfunc = cs_quadrature_tria_1pt_scal;
        break;
        /* Quadrature with a unique weight */
      case CS_QUADRATURE_HIGHER:
        qfunc = cs_quadrature_tria_3pts_scal;
        break;
        /* Most accurate quadrature available */
      case CS_QUADRATURE_HIGHEST:
        qfunc = cs_quadrature_tria_4pts_scal;
        break;

      default:
        bft_error(__FILE__, __LINE__, 0,
                  _("Invalid quadrature type.\n"));

      } /* Which type of quadrature to use */
    }
    break;

  case 3: /* Vector-valued case */
    {
      switch (qtype) {
        /* Barycenter of the tetrahedral subdiv. */
      case CS_QUADRATURE_BARY:
      case CS_QUADRATURE_BARY_SUBDIV:
        qfunc = cs_quadrature_tria_1pt_vect;
        break;
        /* Quadrature with a unique weight */
      case CS_QUADRATURE_HIGHER:
        qfunc = cs_quadrature_tria_3pts_vect;
        break;
        /* Most accurate quadrature available */
      case CS_QUADRATURE_HIGHEST:
        qfunc = cs_quadrature_tria_4pts_vect;
        break;

      default:
        bft_error(__FILE__, __LINE__, 0,
                  _("Invalid quadrature type.\n"));

      } /* Which type of quadrature to use */
    }
    break;

  default:
    bft_error(__FILE__, __LINE__, 0,
              _(" Invalid dimension of the analytical fucntion.\n"));

  } /* Switch on space dimension */

  const double  tcur = ts->t_cur;
  const cs_adjacency_t  *f2e = connect->f2e;
  const cs_adjacency_t  *e2v = connect->e2v;
  const cs_real_t  *xv = quant->vtx_coord;

  if (elt_ids == NULL) {
#   pragma omp parallel for if (quant->n_faces > CS_THR_MIN)
    for (cs_lnum_t f_id = 0; f_id < quant->n_faces; f_id++) {

      const cs_quant_t pfq = cs_quant_set_face(f_id, quant);
      double *val_i = eval + dim*f_id;
      const cs_lnum_t  start_idx = f2e->idx[f_id],
                       end_idx   = f2e->idx[f_id+1];

      switch (end_idx - start_idx) {

      case CS_TRIANGLE_CASE:
        {
          cs_lnum_t v1, v2, v3;
          cs_connect_get_next_3_vertices(f2e->ids, e2v->ids, start_idx,
                                         &v1, &v2, &v3);
          qfunc(tcur, xv + 3*v1, xv + 3*v2, xv + 3*v3, pfq.meas,
                anai->func, anai->input, val_i);
        }
        break;

      default:
        for (cs_lnum_t j = start_idx; j < end_idx; j++) {

          const cs_lnum_t  _2e = 2*f2e->ids[j];
          const cs_lnum_t  v1 = e2v->ids[_2e];
          const cs_lnum_t  v2 = e2v->ids[_2e+1];

          qfunc(tcur, xv + 3*v1, xv + 3*v2, pfq.center,
                cs_math_surftri(xv + 3*v1, xv + 3*v2, pfq.center),
                anai->func, anai->input, val_i);

        } /* Loop on edges */

      } /* Switch on the type of face. Special case for triangles */

      /* Compute the average */
      const double _os = 1./pfq.meas;
      for (short int xyz = 0; xyz < dim; xyz++)
        val_i[xyz] *= _os;

    } /* Loop on faces */

  }
  else { /* There is an indirection list */

    for (cs_lnum_t i = 0; i < n_elts; i++) { // Loop on selected faces

      cs_lnum_t  f_id = elt_ids[i];
      const cs_quant_t pfq = cs_quant_set_face(f_id, quant);
      double *val_i = eval + dim*f_id;
      const cs_lnum_t  start_idx = f2e->idx[f_id],
                       end_idx   = f2e->idx[f_id+1];

      switch (end_idx - start_idx) {

      case CS_TRIANGLE_CASE:
        {
          /* If triangle, one-shot computation */
          cs_lnum_t v1, v2, v3;
          cs_connect_get_next_3_vertices(f2e->ids, e2v->ids, start_idx,
                                         &v1, &v2, &v3);
          qfunc(tcur, xv + 3*v1, xv + 3*v2, xv + 3*v3,
                pfq.meas, anai->func, anai->input, val_i);
        }
        break;

      default:
        for (cs_lnum_t k = start_idx; k < end_idx; k++) {

          const cs_lnum_t  _2e = 2*f2e->ids[k];
          const cs_lnum_t  v1 = e2v->ids[_2e];
          const cs_lnum_t  v2 = e2v->ids[_2e+1];

          qfunc(tcur, xv + 3*v1, xv + 3*v2, pfq.center,
                cs_math_surftri(xv+3*v1, xv+3*v2, pfq.center),
                anai->func, anai->input, val_i);

        } /* Loop on edges */

      } /* Switch on the type of face. Special case for triangles */

      /* Compute the average */
      const double _os = 1./pfq.meas;
      for (short int xyz = 0; xyz < dim; xyz++)
        val_i[xyz] *= _os;

    } // Loop on selected faces

  }
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined at vertices using an analytic function
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_at_vertices_by_analytic(cs_lnum_t                    n_elts,
                                     const cs_lnum_t             *elt_ids,
                                     bool                         compact,
                                     const cs_mesh_t             *mesh,
                                     const cs_cdo_connect_t      *connect,
                                     const cs_cdo_quantities_t   *quant,
                                     const cs_time_step_t        *ts,
                                     void                        *input,
                                     cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(connect);

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  /* Evaluate the function for this time at the cell center */
  anai->func(ts->t_cur,
             n_elts, elt_ids, quant->vtx_coord,
             compact,  // compacted output ?
             anai->input,
             eval);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined using an analytic function by a
 *         cellwise process (usage of a cs_cell_mesh_t structure)
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_cell_by_analytic(const cs_cell_mesh_t       *cm,
                                 const cs_time_step_t       *ts,
                                 void                       *input,
                                 cs_real_t                  *eval)
{
  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  /* Evaluate the function for this time at the cell center */
  anai->func(ts->t_cur,
             1, NULL, cm->xc,
             true, // compacted output ?
             anai->input,
             eval);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a scalar-valued quantity at cells defined by an array.
 *         Array is assumed to be interlaced.
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_scalar_at_cells_by_array(cs_lnum_t                    n_elts,
                                      const cs_lnum_t             *elt_ids,
                                      bool                         compact,
                                      const cs_mesh_t             *mesh,
                                      const cs_cdo_connect_t      *connect,
                                      const cs_cdo_quantities_t   *quant,
                                      const cs_time_step_t        *ts,
                                      void                        *input,
                                      cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(ts);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  assert(array_input->stride == 1);

  if ((array_input->loc & cs_flag_primal_cell) == cs_flag_primal_cell) {

    if (elt_ids != NULL && !compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        eval[c_id] = array_input->values[c_id];
      }

    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++)
        eval[i] = array_input->values[elt_ids[i]];

    }
    else {

      assert(elt_ids == NULL);
      memcpy(eval, array_input->values, n_elts * sizeof(cs_real_t));

    }

  }
  else if ((array_input->loc & cs_flag_primal_vtx) == cs_flag_primal_vtx) {

    if (elt_ids != NULL && !compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        cs_reco_pv_at_cell_center(c_id,
                                  connect->c2v,
                                  quant,
                                  array_input->values,
                                  eval + c_id);
      }

    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++)
        cs_reco_pv_at_cell_center(elt_ids[i],
                                  connect->c2v,
                                  quant,
                                  array_input->values,
                                  eval + i);

    }
    else {

      assert(elt_ids == NULL);
      for (cs_lnum_t i = 0; i < n_elts; i++)
        cs_reco_pv_at_cell_center(i,
                                  connect->c2v,
                                  quant,
                                  array_input->values,
                                  eval + i);

    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a nd-valued quantity at cells defined by an array.
 *         Array is assumed to be interlaced.
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_nd_at_cells_by_array(cs_lnum_t                    n_elts,
                                  const cs_lnum_t             *elt_ids,
                                  bool                         compact,
                                  const cs_mesh_t             *mesh,
                                  const cs_cdo_connect_t      *connect,
                                  const cs_cdo_quantities_t   *quant,
                                  const cs_time_step_t        *ts,
                                  void                        *input,
                                  cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(ts);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  const int  stride = array_input->stride;
  assert(stride > 1);

  if (cs_flag_test(array_input->loc, cs_flag_primal_cell)) {

    if (elt_ids != NULL && !compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        for (int k = 0; k < stride; k++)
          eval[stride*c_id + k] = array_input->values[stride*c_id + k];
      }

    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        for (int k = 0; k < stride; k++)
          eval[stride*i + k] = array_input->values[stride*c_id + k];
      }

    }
    else {

      assert(elt_ids == NULL);
      memcpy(eval, array_input->values, stride*n_elts * sizeof(cs_real_t));

    }

  }
  else if (cs_flag_test(array_input->loc, cs_flag_dual_face_byc)) {

    assert(stride == 3);
    assert(array_input->index == connect->c2e->idx);

    if (elt_ids != NULL && !compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        cs_reco_dfbyc_at_cell_center(c_id,
                                     connect->c2e,
                                     quant,
                                     array_input->values,
                                     eval + c_id*stride);
      }

    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++)
        cs_reco_dfbyc_at_cell_center(elt_ids[i],
                                     connect->c2e,
                                     quant,
                                     array_input->values,
                                     eval + i*stride);

    }
    else {

      for (cs_lnum_t i = 0; i < n_elts; i++)
        cs_reco_dfbyc_at_cell_center(i,
                                     connect->c2e,
                                     quant,
                                     array_input->values,
                                     eval + i*stride);

    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid case for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity defined at vertices using an array
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_at_vertices_by_array(cs_lnum_t                    n_elts,
                                  const cs_lnum_t             *elt_ids,
                                  bool                         compact,
                                  const cs_mesh_t             *mesh,
                                  const cs_cdo_connect_t      *connect,
                                  const cs_cdo_quantities_t   *quant,
                                  const cs_time_step_t        *ts,
                                  void                        *input,
                                  cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(connect);
  CS_UNUSED(quant);
  CS_UNUSED(ts);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  const int  stride = array_input->stride;

  if (cs_flag_test(array_input->loc, cs_flag_primal_vtx)) {

    if (elt_ids != NULL && !compact) {

      switch (stride) {

      case 1: /* Scalar-valued */
        for (cs_lnum_t i = 0; i < n_elts; i++) {
          const cs_lnum_t  v_id = elt_ids[i];
          eval[v_id] = array_input->values[v_id];
        }
        break;

      default:
        for (cs_lnum_t i = 0; i < n_elts; i++) {
          const cs_lnum_t  v_id = elt_ids[i];
          for (int j = 0; j < stride; j++)
            eval[stride*v_id + j] = array_input->values[stride*v_id+j];
        }
        break;

      } /* End of switch */

    }
    else if (elt_ids != NULL && compact) {

      switch (stride) {

      case 1: /* Scalar-valued */
        for (cs_lnum_t i = 0; i < n_elts; i++)
          eval[i] = array_input->values[elt_ids[i]];
        break;

      default:
        for (cs_lnum_t i = 0; i < n_elts; i++) {
          for (int j = 0; j < stride; j++)
            eval[stride*i + j] = array_input->values[stride*elt_ids[i] + j];
        }
        break;

      } /* End of switch */

    }
    else {

      assert(elt_ids == NULL);
      memcpy(eval, array_input->values, n_elts*stride * sizeof(cs_real_t));

    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a vector-valued quantity at all vertices defined by an
 *         array.
 *         Array is assumed to be interlaced.
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_3_at_all_vertices_by_array(cs_lnum_t                   n_elts,
                                        const cs_lnum_t            *elt_ids,
                                        bool                        compact,
                                        const cs_mesh_t            *mesh,
                                        const cs_cdo_connect_t     *connect,
                                        const cs_cdo_quantities_t  *quant,
                                        const cs_time_step_t       *ts,
                                        void                       *input,
                                        cs_real_t                  *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(ts);
  CS_UNUSED(compact);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  const int  stride = array_input->stride;

  if (elt_ids != NULL || n_elts < quant->n_vertices)
    bft_error(__FILE__, __LINE__, 0, " %s: Invalid case\n", __func__);

  double  *dc_vol = NULL;
  BFT_MALLOC(dc_vol, quant->n_vertices, double);

# pragma omp parallel for if (quant->n_vertices > CS_THR_MIN)
  for (cs_lnum_t i = 0; i < quant->n_vertices; i++)
    dc_vol[i] = 0;

  if (cs_flag_test(array_input->loc, cs_flag_primal_cell)) {

    assert(stride == 3);
    for (cs_lnum_t c_id = 0; c_id < quant->n_cells; c_id++) {

      /* Retrieve the cell vector */
      cs_real_3_t  cell_vector;
      for (int k = 0; k < stride; k++)
        cell_vector[k] = array_input->values[stride*c_id + k];

      /* Interpolate with a weighting related to the vertex volume in each
         cell */
      const cs_lnum_t  *c2v_idx = connect->c2v->idx + c_id;
      const cs_lnum_t  *c2v_ids = connect->c2v->ids + c2v_idx[0];
      const double  *vol_vc = quant->dcell_vol + c2v_idx[0];

      for (short int v = 0; v < c2v_idx[1]-c2v_idx[0]; v++) {

        const cs_lnum_t  v_id = c2v_ids[v];

        dc_vol[v_id] += vol_vc[v];
        cs_real_t  *v_val = eval + 3*v_id;
        for (int k = 0; k < 3; k++) v_val[k] += vol_vc[v] * cell_vector[k];

      } // Loop on cell vertices

    } // Loop on cells

#   pragma omp parallel for if (quant->n_vertices > CS_THR_MIN)
    for (cs_lnum_t v_id = 0; v_id < quant->n_vertices; v_id++) {

      const double  inv_dcvol = 1/dc_vol[v_id];
      cs_real_t *v_val = eval + 3*v_id;
      for (int k = 0; k < 3; k++) v_val[k] *= inv_dcvol;

    } // Loop on vertices

  }
  else if (cs_flag_test(array_input->loc, cs_flag_dual_face_byc)) {

    for (cs_lnum_t c_id = 0; c_id < quant->n_cells; c_id++) {

      /* Compute a estimated cell vector */
      cs_real_3_t  cell_vector;
      cs_reco_dfbyc_at_cell_center(c_id,
                                   connect->c2e,
                                   quant,
                                   array_input->values,
                                   cell_vector);

      /* Interpolate with a weighting related to the vertex volume in each
         cell */
      const cs_lnum_t  *c2v_idx = connect->c2v->idx + c_id;
      const cs_lnum_t  *c2v_ids = connect->c2v->ids + c2v_idx[0];
      const double  *vol_vc = quant->dcell_vol + c2v_idx[0];

      for (short int v = 0; v < c2v_idx[1]-c2v_idx[0]; v++) {

        const cs_lnum_t  v_id = c2v_ids[v];

        dc_vol[v_id] += vol_vc[v];
        cs_real_t  *v_val = eval + 3*v_id;
        for (int k = 0; k < 3; k++) v_val[k] += vol_vc[v] * cell_vector[k];

      } // Loop on cell vertices

    } // Loop on cells

#   pragma omp parallel for if (quant->n_vertices > CS_THR_MIN)
    for (cs_lnum_t v_id = 0; v_id < quant->n_vertices; v_id++) {

      const double  inv_dcvol = 1/dc_vol[v_id];
      cs_real_t *v_val = eval + 3*v_id;
      for (int k = 0; k < 3; k++) v_val[k] *= inv_dcvol;

    } // Loop on vertices

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid case for the input array", __func__);

  /* Free temporary buffer */
  BFT_FREE(dc_vol);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity at cells defined by an array.
 *         Array is assumed to be interlaced.
 *         Variation using a cs_cell_mesh_t structure
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_cell_by_array(const cs_cell_mesh_t      *cm,
                              const cs_time_step_t      *ts,
                              void                      *input,
                              cs_real_t                 *eval)
{
  CS_UNUSED(ts);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  const int  stride = array_input->stride;

  /* array is assumed to be interlaced */
  if (cs_flag_test(array_input->loc, cs_flag_primal_cell)) {

    for (int k = 0; k < stride; k++)
      eval[k] = array_input->values[stride*cm->c_id + k];

  }
  else if (cs_flag_test(array_input->loc, cs_flag_primal_vtx)) {

    /* Sanity checks */
    assert(cs_flag_test(cm->flag, CS_CDO_LOCAL_PVQ));

    /* Reconstruct (or interpolate) value at the current cell center */
    for (short int v = 0; v < cm->n_vc; v++) {
      for (int k = 0; k < stride; k++)
        eval[k] += cm->wvc[v] * array_input->values[stride*cm->v_ids[v] + k];
    }

  }
  else if (cs_flag_test(array_input->loc, cs_flag_dual_face_byc)) {

    assert(array_input->index != NULL);

    /* Reconstruct (or interpolate) value at the current cell center */
    cs_reco_dfbyc_in_cell(cm,
                          array_input->values + array_input->index[cm->c_id],
                          eval);

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity inside a cell defined using a field
 *
 * \param[in]  n_elts    number of elements to consider
 * \param[in]  elt_ids   list of element ids
 * \param[in]  compact   true:no indirection, false:indirection for output
 * \param[in]  mesh      pointer to a cs_mesh_t structure
 * \param[in]  connect   pointer to a cs_cdo_connect_t structure
 * \param[in]  quant     pointer to a cs_cdo_quantities_t structure
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cell_by_field(cs_lnum_t                    n_elts,
                           const cs_lnum_t             *elt_ids,
                           bool                         compact,
                           const cs_mesh_t             *mesh,
                           const cs_cdo_connect_t      *connect,
                           const cs_cdo_quantities_t   *quant,
                           const cs_time_step_t        *ts,
                           void                        *input,
                           cs_real_t                   *eval)
{
  CS_UNUSED(mesh);
  CS_UNUSED(ts);

  cs_field_t  *field = (cs_field_t *)input;
  assert(field != NULL);
  cs_real_t  *values = field->val;
  int  c_ml_id = cs_mesh_location_get_id_by_name(N_("cells"));
  int  v_ml_id = cs_mesh_location_get_id_by_name(N_("vertices"));

  if (field->location_id == c_ml_id) {

    if (elt_ids != NULL && !compact) {
      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        for (int k = 0; k < field->dim; k++)
          eval[field->dim*c_id + k] = values[field->dim*c_id + k];
      }
    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {
        const cs_lnum_t  c_id = elt_ids[i];
        for (int k = 0; k < field->dim; k++)
          eval[field->dim*i + k] = values[field->dim*c_id + k];
      }

    }
    else {

      assert(elt_ids == NULL);
      memcpy(eval, values, field->dim*n_elts * sizeof(cs_real_t));

    }

  }
  else if (field->location_id == v_ml_id) {

    assert(field->dim == 1);
    if (elt_ids != NULL && !compact) {
      for (cs_lnum_t i = 0; i < n_elts; i++) {

        const cs_lnum_t  c_id = elt_ids[i];
        cs_reco_pv_at_cell_center(c_id,
                                  connect->c2v,
                                  quant,
                                  values,
                                  eval + c_id);

      }
    }
    else if (elt_ids != NULL && compact) {

      for (cs_lnum_t i = 0; i < n_elts; i++) {

        const cs_lnum_t  c_id = elt_ids[i];
        cs_reco_pv_at_cell_center(c_id,
                                  connect->c2v,
                                  quant,
                                  values,
                                  eval + i);

      }

    }
    else {

      assert(elt_ids == NULL);
      for (cs_lnum_t i = 0; i < n_elts; i++) {
        cs_reco_pv_at_cell_center(i,
                                  connect->c2v,
                                  quant,
                                  values,
                                  eval + i);

      }

    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid case for the input array", __func__);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Evaluate a quantity inside a cell defined using a field
 *         Variation using a cs_cell_mesh_t structure
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     value of the property at the cell center
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_cell_by_field(const cs_cell_mesh_t        *cm,
                              const cs_time_step_t        *ts,
                              void                        *input,
                              cs_real_t                   *eval)
{
  CS_UNUSED(ts);

  cs_field_t  *field = (cs_field_t *)input;
  assert(field != NULL);
  cs_real_t  *values = field->val;
  int  c_ml_id = cs_mesh_location_get_id_by_name(N_("cells"));
  int  v_ml_id = cs_mesh_location_get_id_by_name(N_("vertices"));

  if (field->location_id == c_ml_id) {

    for (int k = 0; k < field->dim; k++)
      eval[k] = values[field->dim*cm->c_id + k];

  }
  else if (field->location_id == v_ml_id) {

    /* Sanity checks */
    assert(field->dim == 1);
    assert(cs_flag_test(cm->flag, CS_CDO_LOCAL_PVQ));

    /* Reconstruct (or interpolate) value at the current cell center */
    for (short int v = 0; v < cm->n_vc; v++)
      eval[0] += cm->wvc[v] * values[cm->v_ids[v]];


  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined by analytic
 *         function at a precise location inside a cell
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]  cm        pointer to a cs_cell_mesh_t structure
 * \param[in]  n_points  number of points where to compute the evaluation
 * \param[in]  xyz       where to compute the evaluation
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_at_xyz_by_analytic(const cs_cell_mesh_t       *cm,
                                   cs_lnum_t                   n_points,
                                   const cs_real_t            *xyz,
                                   const cs_time_step_t       *ts,
                                   void                       *input,
                                   cs_real_t                  *eval)
{
  CS_UNUSED(cm);

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  /* Evaluate the function for this time at the given coordinates */
  anai->func(ts->t_cur,
             n_points, NULL, xyz,
             true, // compacted output ?
             anai->input,
             eval);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined by analytic
 *         function at a precise location inside a cell
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]  cm        pointer to a cs_cell_mesh_t structure
 * \param[in]  n_points  number of points where to compute the evaluation
 * \param[in]  xyz       where to compute the evaluation
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_vector_at_xyz_by_val(const cs_cell_mesh_t       *cm,
                                     cs_lnum_t                   n_points,
                                     const cs_real_t            *xyz,
                                     const cs_time_step_t       *ts,
                                     void                       *input,
                                     cs_real_t                  *eval)
{
  CS_UNUSED(cm);
  CS_UNUSED(xyz);
  CS_UNUSED(ts);

  const cs_real_t  *constant_val = (cs_real_t *)input;

  for (int i = 0; i < n_points; i++) {
    eval[3*i    ] = constant_val[0];
    eval[3*i + 1] = constant_val[1];
    eval[2*i + 2] = constant_val[2];
  }
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined by analytic
 *         function at a precise location inside a cell
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]  cm        pointer to a cs_cell_mesh_t structure
 * \param[in]  n_points  number of points where to compute the evaluation
 * \param[in]  xyz       where to compute the evaluation
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_3_at_xyz_by_array(const cs_cell_mesh_t       *cm,
                                  cs_lnum_t                   n_points,
                                  const cs_real_t            *xyz,
                                  const cs_time_step_t       *ts,
                                  void                       *input,
                                  cs_real_t                  *eval)
{
  CS_UNUSED(xyz);
  CS_UNUSED(ts);

  cs_xdef_array_input_t  *array_input = (cs_xdef_array_input_t *)input;

  const int  stride = array_input->stride;

  /* array is assumed to be interlaced */
  if (cs_flag_test(array_input->loc, cs_flag_primal_cell)) {

    assert(stride == 3);
    cs_real_3_t  cell_vector;
    for (int k = 0; k < stride; k++)
      cell_vector[k] = array_input->values[stride*cm->c_id + k];
    for (int i = 0; i < n_points; i++) {
      eval[3*i    ] = cell_vector[0];
      eval[3*i + 1] = cell_vector[1];
      eval[2*i + 2] = cell_vector[2];
    }

  }
  else if (cs_flag_test(array_input->loc, cs_flag_primal_vtx)) {

    /* Sanity checks */
    assert(cs_flag_test(cm->flag, CS_CDO_LOCAL_PVQ));
    assert(stride == 3);

    /* Reconstruct (or interpolate) value at the current cell center */
    for (int k = 0; k < stride; k++) {

      for (short int v = 0; v < cm->n_vc; v++) {
        eval[k] += cm->wvc[v] * array_input->values[stride*cm->v_ids[v] + k];

      }

    }

  }
  else if (cs_flag_test(array_input->loc, cs_flag_dual_face_byc)) {

    assert(array_input->index != NULL);

    /* Reconstruct (or interpolate) value at the current cell center */
    cs_real_3_t  cell_vector;
    cs_reco_dfbyc_in_cell(cm,
                          array_input->values + array_input->index[cm->c_id],
                          cell_vector);

    for (int i = 0; i < n_points; i++) {
      eval[3*i    ] = cell_vector[0];
      eval[3*i + 1] = cell_vector[1];
      eval[3*i + 2] = cell_vector[2];
    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined by a field
 *         at a precise location inside a cell
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]  cm        pointer to a cs_cell_mesh_t structure
 * \param[in]  n_points  number of points where to compute the evaluation
 * \param[in]  xyz       where to compute the evaluation
 * \param[in]  ts        pointer to a cs_time_step_t structure
 * \param[in]  input     pointer to an input structure
 * \param[out] eval      result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_3_at_xyz_by_field(const cs_cell_mesh_t       *cm,
                                  cs_lnum_t                   n_points,
                                  const cs_real_t            *xyz,
                                  const cs_time_step_t       *ts,
                                  void                       *input,
                                  cs_real_t                  *eval)
{
  CS_UNUSED(xyz);
  CS_UNUSED(ts);

  cs_field_t  *field = (cs_field_t *)input;
  const cs_real_t  *values = field->val;

  assert(field != NULL);
  assert(field->dim == 3);

  const int  c_ml_id = cs_mesh_location_get_id_by_name(N_("cells"));
  const int  v_ml_id = cs_mesh_location_get_id_by_name(N_("vertices"));

  /* array is assumed to be interlaced */
  if (field->location_id == c_ml_id) {

    cs_real_3_t  cell_vector;
    for (int k = 0; k < 3; k++)
      cell_vector[k] = values[3*cm->c_id + k];
    for (int i = 0; i < n_points; i++) { // No interpolation
      eval[3*i    ] = cell_vector[0];
      eval[3*i + 1] = cell_vector[1];
      eval[3*i + 2] = cell_vector[2];
    }

  }
  else if (field->location_id == v_ml_id) {

    /* Sanity check */
    assert(cs_flag_test(cm->flag, CS_CDO_LOCAL_PVQ));

    /* Reconstruct (or interpolate) value at the current cell center */
    for (int k = 0; k < 3; k++) {

      for (short int v = 0; v < cm->n_vc; v++) {
        eval[k] += cm->wvc[v] * values[3*cm->v_ids[v] + k];
      }

    }

  }
  else
    bft_error(__FILE__, __LINE__, 0,
              " %s: Invalid support for the input array", __func__);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by values. The normal flux is then added to each portion of
 *         face related to a vertex.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      input   pointer to an input structure
 * \param[in, out] eval    result of the evaluation (updated inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_at_vtx_flux_by_val(const cs_cell_mesh_t     *cm,
                                   short int                 f,
                                   void                     *input,
                                   cs_real_t                *eval)
{
  assert(cs_flag_test(cm->flag, CS_CDO_LOCAL_EV | CS_CDO_LOCAL_FE));

  const cs_real_t  *flux = (cs_real_t *)input;

  if (cs_flag_test(cm->flag, CS_CDO_LOCAL_FEQ)) {

    /* Loop on face edges */
    for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

      const short int  e = cm->f2e_ids[i];
      const double  _flx = 0.5 * cm->tef[i] * _dp3(flux, cm->face[f].unitv);

      eval[cm->e2v_ids[2*e  ]] += _flx;
      eval[cm->e2v_ids[2*e+1]] += _flx;

    }

  }
  else {

    /* Loop on face edges */
    for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

      const short int  e = cm->f2e_ids[i];
      const double  tef = cs_compute_area_from_quant(cm->edge[e],
                                                     cm->face[f].center);
      const double  _flx = 0.5 * tef * _dp3(flux, cm->face[f].unitv);

      eval[cm->e2v_ids[2*e  ]] += _flx;
      eval[cm->e2v_ids[2*e+1]] += _flx;

    }

  }

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by analytic function. The normal flux is then added to each
 *         portion of face related to a vertex.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      ts      pointer to a cs_time_step_t structure
 * \param[in]      input   pointer to an input structure
 * \param[in]      qtype   level of quadrature to use
 * \param[in, out] eval    result of the evaluation (updated inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_at_vtx_flux_by_analytic(const cs_cell_mesh_t      *cm,
                                        short int                  f,
                                        const cs_time_step_t      *ts,
                                        void                      *input,
                                        cs_quadrature_type_t       qtype,
                                        cs_real_t                 *eval)
{
  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  switch (qtype) {

  case CS_QUADRATURE_NONE:
  case CS_QUADRATURE_BARY:
    {
      cs_real_3_t  flux_xc = {0, 0, 0};

      /* Evaluate the function for this time at the given coordinates */
      anai->func(ts->t_cur, 1, NULL, cm->xc, true, // compacted output ?
                 anai->input,
                 flux_xc);

      /* Plug into the evaluation by value now */
      cs_xdef_eval_cw_at_vtx_flux_by_val(cm, f, flux_xc, eval);
    }
    break;

  case CS_QUADRATURE_BARY_SUBDIV:
    {
      const cs_quant_t  fq = cm->face[f];

      cs_real_3_t  _val[2], _xyz[2];

      if (cs_flag_test(cm->flag, CS_CDO_LOCAL_FEQ)) {

        /* Loop on face edges */
        for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

          const short int  e = cm->f2e_ids[i];
          const short int v1 = cm->e2v_ids[2*e];
          const short int v2 = cm->e2v_ids[2*e+1];

          for (int k = 0; k < 3; k++) {
            const double xef = cm->edge[e].center[k] + fq.center[k];
            _xyz[0][k] = cs_math_onethird * (xef + cm->xv[3*v1+k]);
            _xyz[1][k] = cs_math_onethird * (xef + cm->xv[3*v2+k]);
          }

          /* Evaluate the function for this time at the given coordinates */
          anai->func(ts->t_cur, 2, NULL,
                     (const cs_real_t *)_xyz, true, // compacted output ?
                     anai->input,
                     (cs_real_t *)_val);

          eval[v1] += 0.5*cm->tef[i] * _dp3(_val[0], fq.unitv);
          eval[v2] += 0.5*cm->tef[i] * _dp3(_val[1], fq.unitv);

        }

      }
      else {

        /* Loop on face edges */
        for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

          const short int  e = cm->f2e_ids[i];
          const short int v1 = cm->e2v_ids[2*e];
          const short int v2 = cm->e2v_ids[2*e+1];

          for (int k = 0; k < 3; k++) {
            const double xef = cm->edge[e].center[k] + fq.center[k];
            _xyz[0][k] = cs_math_onethird * (xef + cm->xv[3*v1+k]);
            _xyz[1][k] = cs_math_onethird * (xef + cm->xv[3*v2+k]);
          }

          /* Evaluate the function for this time at the given coordinates */
          anai->func(ts->t_cur, 2, NULL,
                     (const cs_real_t *)_xyz, true, // compacted output ?
                     anai->input,
                     (cs_real_t *)_val);

          const double tef = cs_compute_area_from_quant(cm->edge[e], fq.center);

          eval[v1] += 0.5 * tef * _dp3(_val[0], fq.unitv);
          eval[v2] += 0.5 * tef * _dp3(_val[1], fq.unitv);

        }

      }

    }
    break; /* BARY_SUBDIV */

  case CS_QUADRATURE_HIGHER:
    {
      cs_real_t  w[2];
      cs_real_3_t  gpts[6], _val[6];

      const cs_quant_t  fq = cm->face[f];

      if (cs_flag_test(cm->flag, CS_CDO_LOCAL_FEQ)) {

        /* Loop on face edges */
        for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

          const short int  e = cm->f2e_ids[i];
          const short int v1 = cm->e2v_ids[2*e];
          const short int v2 = cm->e2v_ids[2*e+1];
          const cs_real_t  svef = 0.5 * cm->tef[i];

          /* Two triangles composing the portion of face related to a vertex
             Evaluate the field at the three quadrature points */
          cs_quadrature_tria_3pts(cm->edge[e].center, fq.center, cm->xv + 3*v1,
                                  svef,
                                  gpts, w);

          cs_quadrature_tria_3pts(cm->edge[e].center, fq.center, cm->xv + 3*v2,
                                  svef,
                                  gpts + 3, w + 1);

          /* Evaluate the function for this time at the given coordinates */
          anai->func(ts->t_cur, 6, NULL,
                     (const cs_real_t *)gpts, true, // compacted output ?
                     anai->input,
                     (cs_real_t *)_val);

          cs_real_t  add0 = 0, add1 = 0;
          for (int p = 0; p < 3; p++) add0 += _dp3(_val[p], fq.unitv);
          add0 *= w[0];
          for (int p = 0; p < 3; p++) add1 += _dp3(_val[p+3], fq.unitv);
          add1 *= w[1];

          eval[v1] += add0;
          eval[v2] += add1;

        }

      }
      else {

        /* Loop on face edges */
        for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

          const short int  e = cm->f2e_ids[i];
          const short int v1 = cm->e2v_ids[2*e];
          const short int v2 = cm->e2v_ids[2*e+1];
          const double svef = 0.5 * cs_compute_area_from_quant(cm->edge[e],
                                                               fq.center);

          /* Two triangles composing the portion of face related to a vertex
             Evaluate the field at the three quadrature points */
          cs_quadrature_tria_3pts(cm->edge[e].center, fq.center, cm->xv + 3*v1,
                                  svef,
                                  gpts, w);

          cs_quadrature_tria_3pts(cm->edge[e].center, fq.center, cm->xv + 3*v2,
                                  svef,
                                  gpts + 3, w + 1);

          /* Evaluate the function for this time at the given coordinates */
          anai->func(ts->t_cur, 6, NULL,
                     (const cs_real_t *)gpts, true, // compacted output ?
                     anai->input,
                     (cs_real_t *)_val);

          cs_real_t  add0 = 0, add1 = 0;
          for (int p = 0; p < 3; p++) add0 += _dp3(_val[p], fq.unitv);
          add0 *= w[0];
          for (int p = 0; p < 3; p++) add1 += _dp3(_val[p+3], fq.unitv);
          add1 *= w[1];

          eval[v1] += add0;
          eval[v2] += add1;

        }

      } /* tef is already computed */

    }
    break;

  case CS_QUADRATURE_HIGHEST: // Not yet implemented
  default:
    bft_error(__FILE__, __LINE__, 0, " Invalid type of quadrature.");
    break;

  } // switch type of quadrature

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by values.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      input   pointer to an input structure
 * \param[in, out] eval    result of the evaluation (set inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_flux_by_val(const cs_cell_mesh_t     *cm,
                            short int                 f,
                            void                     *input,
                            cs_real_t                *eval)
{
  const cs_real_t  *flux = (cs_real_t *)input;
  const cs_quant_t  fq = cm->face[f];

  eval[f] = fq.meas * _dp3(fq.unitv, flux);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by values.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      input   pointer to an input structure
 * \param[in, out] eval    result of the evaluation (set inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_tensor_flux_by_val(const cs_cell_mesh_t     *cm,
                                   short int                 f,
                                   void                     *input,
                                   cs_real_t                *eval)
{
  const cs_real_t  *flux = (cs_real_t *)input;
  const cs_quant_t  fq = cm->face[f];

  cs_math_33_3_product((const cs_real_t (*)[3])flux, fq.unitv, eval);
  for (int k = 0; k < 3; k++)
    eval[3*f+k] *= fq.meas;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by analytic function.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      ts      pointer to a cs_time_step_t structure
 * \param[in]      input   pointer to an input structure
 * \param[in]      qtype   level of quadrature to use
 * \param[in, out] eval    result of the evaluation (set inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_flux_by_analytic(const cs_cell_mesh_t      *cm,
                                 short int                  f,
                                 const cs_time_step_t      *ts,
                                 void                      *input,
                                 cs_quadrature_type_t       qtype,
                                 cs_real_t                 *eval)
{
  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  switch (qtype) {

  case CS_QUADRATURE_NONE:
  case CS_QUADRATURE_BARY:
    {
      cs_real_3_t  flux_xc = {0, 0, 0};

      /* Evaluate the function for this time at the given coordinates */
      anai->func(ts->t_cur, 1, NULL, cm->xc, true, // compacted output ?
                 anai->input,
                 flux_xc);

      /* Plug into the evaluation by value now */
      cs_xdef_eval_cw_flux_by_val(cm, f, flux_xc, eval);
    }
    break;

  case CS_QUADRATURE_BARY_SUBDIV:
    {
      assert(cs_flag_test(cm->flag,
                          CS_CDO_LOCAL_EV|CS_CDO_LOCAL_FE|CS_CDO_LOCAL_FEQ));

      const cs_quant_t  fq = cm->face[f];

      cs_real_3_t  _val, _xyz;

      /* Loop on face edges */
      for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

        const short int  e = cm->f2e_ids[i];
        const short int v1 = cm->e2v_ids[2*e];
        const short int v2 = cm->e2v_ids[2*e+1];

        for (int k = 0; k < 3; k++)
          _xyz[k] = cs_math_onethird *
            (fq.center[k] + cm->xv[3*v1+k] + cm->xv[3*v2+k]);

        /* Evaluate the function for this time at the given coordinates */
        anai->func(ts->t_cur, 1, NULL,
                   (const cs_real_t *)_xyz, true, // compacted output ?
                   anai->input,
                   (cs_real_t *)_val);

        eval[f] += cm->tef[i] * _dp3(_val, fq.unitv);

      }

    }
    break; /* BARY_SUBDIV */

  case CS_QUADRATURE_HIGHER:
    {
      cs_real_t  w;
      cs_real_3_t  gpts[3], _val[3];

      const cs_quant_t  fq = cm->face[f];

      eval[f] = 0.; /* Reset value */

      /* Loop on face edges */
      for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

        const short int  e = cm->f2e_ids[i];
        const short int v1 = cm->e2v_ids[2*e];
        const short int v2 = cm->e2v_ids[2*e+1];

        /* Evaluate the field at the three quadrature points */
        cs_quadrature_tria_3pts(fq.center, cm->xv + 3*v1, cm->xv + 3*v2,
                                cm->tef[e],
                                gpts, &w);

        /* Evaluate the function for this time at the given coordinates */
        anai->func(ts->t_cur, 3, NULL,
                   (const cs_real_t *)gpts, true, // compacted output ?
                   anai->input,
                   (cs_real_t *)_val);

        cs_real_t  add = 0;
        for (int p = 0; p < 3; p++) add += _dp3(_val[p], fq.unitv);

        eval[f] += w * add;

      }

    }
    break;

  case CS_QUADRATURE_HIGHEST: // Not yet implemented
  default:
    bft_error(__FILE__, __LINE__, 0, " Invalid type of quadrature.");
    break;

  } // switch type of quadrature

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the normal flux of a quantity
 *         defined by analytic function.
 *         Case of vector-valued quantities.
 *         Use of a cs_cell_mesh_t structure.
 *
 * \param[in]      cm      pointer to a cs_cell_mesh_t structure
 * \param[in]      f       local face id
 * \param[in]      ts      pointer to a cs_time_step_t structure
 * \param[in]      input   pointer to an input structure
 * \param[in]      qtype   level of quadrature to use
 * \param[in, out] eval    result of the evaluation (set inside)
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_tensor_flux_by_analytic(const cs_cell_mesh_t      *cm,
                                        short int                  f,
                                        const cs_time_step_t      *ts,
                                        void                      *input,
                                        cs_quadrature_type_t       qtype,
                                        cs_real_t                 *eval)
{
  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  switch (qtype) {

  case CS_QUADRATURE_NONE:
  case CS_QUADRATURE_BARY:
    {
      cs_real_33_t  flux_xc = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

      /* Evaluate the function for this time at the given coordinates */
      anai->func(ts->t_cur, 1, NULL, cm->xc, true, // compacted output ?
                 anai->input,
                 (cs_real_t *)flux_xc);

      /* Plug into the evaluation by value now */
      cs_xdef_eval_cw_tensor_flux_by_val(cm, f, flux_xc, eval);
    }
    break;

  case CS_QUADRATURE_BARY_SUBDIV:
    {
      assert(cs_flag_test(cm->flag,
                          CS_CDO_LOCAL_EV|CS_CDO_LOCAL_FE|CS_CDO_LOCAL_FEQ));

      const cs_quant_t  fq = cm->face[f];

      cs_real_3_t  _xyz, _val;
      cs_real_33_t  _eval;

      /* Loop on face edges */
      for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

        const short int  _2e = cm->f2e_ids[i];
        const short int v1 = cm->e2v_ids[_2e];
        const short int v2 = cm->e2v_ids[_2e+1];

        for (int k = 0; k < 3; k++)
          _xyz[k] = cs_math_onethird *
            (fq.center[k] + cm->xv[3*v1+k] + cm->xv[3*v2+k]);

        /* Evaluate the function for this time at the given coordinates */
        anai->func(ts->t_cur, 1, NULL,
                   (const cs_real_t *)_xyz, true, // compacted output ?
                   anai->input,
                   (cs_real_t *)_eval);

        cs_math_33_3_product((const cs_real_t (*)[3])_eval, fq.unitv, _val);
        for (int k = 0; k < 3; k++)
          eval[3*f+k] += cm->tef[i] * _val[k];

      }

    }
    break; /* BARY_SUBDIV */

  case CS_QUADRATURE_HIGHER:
    {
      cs_real_t  w;
      cs_real_3_t  gpts[3], _val;
      cs_real_33_t  _eval[3];

      const cs_quant_t  fq = cm->face[f];

      eval[f] = 0.; /* Reset value */

      /* Loop on face edges */
      for (int i = cm->f2e_idx[f]; i < cm->f2e_idx[f+1]; i++) {

        const short int  e = cm->f2e_ids[i];
        const short int v1 = cm->e2v_ids[2*e];
        const short int v2 = cm->e2v_ids[2*e+1];

        /* Evaluate the field at the three quadrature points */
        cs_quadrature_tria_3pts(fq.center, cm->xv + 3*v1, cm->xv + 3*v2,
                                cm->tef[e],
                                gpts, &w);

        /* Evaluate the function for this time at the given coordinates */
        anai->func(ts->t_cur, 3, NULL,
                   (const cs_real_t *)gpts, true, // compacted output ?
                   anai->input,
                   (cs_real_t *)_eval);

        const cs_real_t coef = w * cm->tef[i];
        for (int p = 0; p < 3; p++) {
          cs_math_33_3_product((const cs_real_t (*)[3])_eval[p], fq.unitv,
                               _val);
          for (int k = 0; k < 3; k++)
            eval[3*f+k] += coef * _val[k];
        }

      }

    }
    break;

  case CS_QUADRATURE_HIGHEST: // Not yet implemented
  default:
    bft_error(__FILE__, __LINE__, 0, " Invalid type of quadrature.");
    break;

  } // switch type of quadrature

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the average on a face of a scalar
 *         function defined through a descriptor (cs_xdef_t structure) by a
 *         cellwise process (usage of a cs_cell_mesh_t structure)
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  f        local face id
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[in]  qtype    level of quadrature to use
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_face_avg_scalar_by_analytic(const cs_cell_mesh_t       *cm,
                                            short int                   f,
                                            const cs_time_step_t       *ts,
                                            void                       *input,
                                            cs_quadrature_type_t        qtype,
                                            cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tria_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tria_1pt_scal;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tria_3pts_scal;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tria_4pts_scal;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_face(cm, ts->t_cur, f,
                           anai->func, anai->input, qfunc, eval);

  /* Average */
  eval[0] /= cm->face[f].meas;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the average on a face of a scalar
 *         function defined through a descriptor (cs_xdef_t structure) by a
 *         cellwise process (usage of a cs_cell_mesh_t structure)
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  f        local face id
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[in]  qtype    level of quadrature to use
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_face_avg_vector_by_analytic(const cs_cell_mesh_t       *cm,
                                            short int                   f,
                                            const cs_time_step_t       *ts,
                                            void                       *input,
                                            cs_quadrature_type_t        qtype,
                                            cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tria_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tria_1pt_vect;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tria_3pts_vect;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tria_4pts_vect;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_face(cm, ts->t_cur, f,
                           anai->func, anai->input, qfunc, eval);

  /* Average */
  const double _oversurf = 1./cm->face[f].meas;
  for (short int xyz = 0; xyz < 3; xyz++)
    eval[xyz] *= _oversurf;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating the average on a face of a scalar
 *         function defined through a descriptor (cs_xdef_t structure) by a
 *         cellwise process (usage of a cs_cell_mesh_t structure)
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  f        local face id
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  input    pointer to an input structure
 * \param[in]  qtype    level of quadrature to use
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_face_avg_tensor_by_analytic(const cs_cell_mesh_t       *cm,
                                            short int                   f,
                                            const cs_time_step_t       *ts,
                                            void                       *input,
                                            cs_quadrature_type_t        qtype,
                                            cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tria_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tria_1pt_tens;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tria_3pts_tens;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tria_4pts_tens;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_face(cm, ts->t_cur, f,
                           anai->func, anai->input, qfunc, eval);

  /* Average */
  const double _oversurf = 1./cm->face[f].meas;
  for (short int i = 0; i < 9; i++)
    eval[i] *= _oversurf;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Routine to integrate an analytic function over a cell and its faces
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  ana      analytic function to integrate
 * \param[in]  input    pointer to an input structure
 * \param[in]  dim      dimension of the function
 * \param[in]  q_tet    quadrature function to use on tetrahedra
 * \param[in]  q_tri    quadrature function to use on triangles
 * \param[out] c_int    result of the evaluation on the cell
 * \param[out] f_int    result of the evaluation on the faces
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_int_on_cell_faces(const cs_cell_mesh_t          *cm,
                               double                         t_cur,
                               cs_analytic_func_t            *ana,
                               void                          *input,
                               const short int                dim,
                               cs_quadrature_tetra_integral_t *q_tet,
                               cs_quadrature_tria_integral_t  *q_tri,
                               cs_real_t                      *c_int,
                               cs_real_t                      *f_int)
{
  const short int nf = cm->n_fc;

  /* Switching on cell-type: optimised version for tetra */
  switch (cm->type) {

  case FVM_CELL_TETRA:
    {
      assert(cm->n_fc == 4 && cm->n_vc == 4);
      q_tet(t_cur, cm->xv, cm->xv+3, cm->xv+6, cm->xv+9, cm->vol_c,
            ana, input, c_int);
      short int  v0, v1, v2;
      for (short int f = 0; f < nf; ++f) {
        const cs_quant_t  pfq = cm->face[f];
        const short int  *f2e_ids = cm->f2e_ids + cm->f2e_idx[f];
        cs_cell_mesh_get_next_3_vertices(f2e_ids, cm->e2v_ids, &v0, &v1, &v2);
        q_tri(t_cur, cm->xv+3*v0, cm->xv+3*v1, cm->xv+3*v2, pfq.meas,
              ana, input, f_int + dim*f);
      }
    }
    break;

  case FVM_CELL_PYRAM:
  case FVM_CELL_PRISM:
  case FVM_CELL_HEXA:
  case FVM_CELL_POLY:
  {
    for (short int f = 0; f < nf; ++f) {

      const cs_quant_t  pfq = cm->face[f];
      const double  hf_coef = cs_math_onethird * cm->hfc[f];
      const int  start = cm->f2e_idx[f];
      const int  end = cm->f2e_idx[f+1];
      const short int n_vf = end - start; // #vertices (=#edges)
      const short int *f2e_ids = cm->f2e_ids + start;

      assert(n_vf > 2);
      switch(n_vf){

      case CS_TRIANGLE_CASE: /* triangle (optimized version, no subdivision) */
        {
          short int  v0, v1, v2;
          cs_cell_mesh_get_next_3_vertices(f2e_ids, cm->e2v_ids, &v0, &v1, &v2);

          const double  *xv0 = cm->xv + 3*v0;
          const double  *xv1 = cm->xv + 3*v1;
          const double  *xv2 = cm->xv + 3*v2;
          q_tet(t_cur, xv0, xv1, xv2, cm->xc,  hf_coef * pfq.meas,
                ana, input, c_int);
          q_tri(t_cur, cm->xv+3*v0, cm->xv+3*v1, cm->xv+3*v2, pfq.meas,
                ana, input, f_int + dim*f);
        }
        break;

      default:
        {
          const double  *tef = cm->tef + start;

          for (short int e = 0; e < n_vf; e++) { /* Loop on face edges */

            // Edge-related variables
            const short int e0  = f2e_ids[e];
            const double  *xv0 = cm->xv + 3*cm->e2v_ids[2*e0];
            const double  *xv1 = cm->xv + 3*cm->e2v_ids[2*e0+1];

            q_tet(t_cur, xv0, xv1, pfq.center, cm->xc, hf_coef*tef[e],
                  ana, input, c_int);

            q_tri(t_cur, xv0, xv1, pfq.center, tef[e],
                  ana, input, f_int + dim*f);

          }
        }
        break;

      } /* End of switch */

    }   /* End of loop on faces */

  }
  break;

  default:
    bft_error(__FILE__, __LINE__, 0,  _(" Unknown cell-type.\n"));
    break;

  } /* End of switch on the cell-type */
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined through a
 *         descriptor (cs_xdef_t structure) by a cellwise process (usage of a
 *         cs_cell_mesh_t structure) which is hinged on integrals
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  qtype    quadrature type
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_avg_scalar_by_analytic(const cs_cell_mesh_t       *cm,
                                       const cs_time_step_t       *ts,
                                       void                       *input,
                                       cs_quadrature_type_t        qtype,
                                       cs_real_t                  *eval)

{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tetra_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tet_1pt_scal;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tet_4pts_scal;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tet_5pts_scal;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_cell(cm, ts->t_cur, anai->func, anai->input, qfunc, eval);

  /* Average */
  eval[0] /= cm->vol_c;

}


/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined through a
 *         descriptor (cs_xdef_t structure) by a cellwise process (usage of a
 *         cs_cell_mesh_t structure) which is hinged on integrals
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  qtype    quadrature type
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_avg_vector_by_analytic(const cs_cell_mesh_t       *cm,
                                       const cs_time_step_t       *ts,
                                       void                       *input,
                                       cs_quadrature_type_t        qtype,
                                       cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tetra_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tet_1pt_vect;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tet_4pts_vect;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tet_5pts_vect;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_cell(cm, ts->t_cur, anai->func, anai->input, qfunc, eval);

  /* Average */
  const double _overvol = 1./cm->vol_c;
  for (short int xyz = 0; xyz < 3; xyz++)
    eval[xyz] *= _overvol;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a quantity defined through a
 *         descriptor (cs_xdef_t structure) by a cellwise process (usage of a
 *         cs_cell_mesh_t structure) which is hinged on integrals
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  qtype    quadrature type
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_avg_tensor_by_analytic(const cs_cell_mesh_t       *cm,
                                       const cs_time_step_t       *ts,
                                       void                       *input,
                                       cs_quadrature_type_t        qtype,
                                       cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tetra_integral_t  *qfunc = NULL;
  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    qfunc = cs_quadrature_tet_1pt_tens;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    qfunc = cs_quadrature_tet_4pts_tens;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    qfunc = cs_quadrature_tet_5pts_tens;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  cs_xdef_eval_int_on_cell(cm, ts->t_cur, anai->func, anai->input, qfunc, eval);

  /* Average */
  const double _overvol = 1./cm->vol_c;
  for (short int xyz = 0; xyz < 9; xyz++)
    eval[xyz] *= _overvol;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Function pointer for evaluating a the reduction by averages of a
 *         analytic function by a cellwise process (usage of a
 *         cs_cell_mesh_t structure) which is hinged on integrals
 *         (faces first, then cell DoFs)
 *
 * \param[in]  cm       pointer to a cs_cell_mesh_t structure
 * \param[in]  ts       pointer to a cs_time_step_t structure
 * \param[in]  qtype    quadrature type
 * \param[in]  input    pointer to an input structure
 * \param[out] eval     result of the evaluation
 */
/*----------------------------------------------------------------------------*/

void
cs_xdef_eval_cw_avg_reduction_by_analytic(const cs_cell_mesh_t       *cm,
                                          const cs_time_step_t       *ts,
                                          void                       *input,
                                          cs_quadrature_type_t        qtype,
                                          cs_real_t                  *eval)
{
  if (eval == NULL)
    bft_error(__FILE__, __LINE__, 0, _err_empty_array, __func__);
  assert(input != NULL);
  assert(cs_flag_test(cm->flag,
                      CS_CDO_LOCAL_PEQ | CS_CDO_LOCAL_PFQ | CS_CDO_LOCAL_FE |
                      CS_CDO_LOCAL_FEQ | CS_CDO_LOCAL_EV));

  cs_quadrature_tetra_integral_t  *q_tet = NULL;
  cs_quadrature_tria_integral_t   *q_tri = NULL;

  switch (qtype) {

  case CS_QUADRATURE_BARY: /* Barycenter of the tetrahedral subdiv. */
  case CS_QUADRATURE_BARY_SUBDIV:
    q_tet = cs_quadrature_tet_1pt_vect;
    q_tri = cs_quadrature_tria_1pt_vect;
    break;

  case CS_QUADRATURE_HIGHER: /* Quadrature with a unique weight */
    q_tet = cs_quadrature_tet_4pts_vect;
    q_tri = cs_quadrature_tria_3pts_vect;
    break;

  case CS_QUADRATURE_HIGHEST: /* Most accurate quadrature available */
    q_tet = cs_quadrature_tet_5pts_vect;
    q_tri = cs_quadrature_tria_4pts_vect;
    break;

  default:
    bft_error(__FILE__, __LINE__, 0, _("Invalid quadrature type.\n"));

  } /* Which type of quadrature to use */

  cs_xdef_analytic_input_t  *anai = (cs_xdef_analytic_input_t *)input;

  const short int nf = cm->n_fc;
  cs_real_t *c_eval = eval + 3*nf;
  cs_xdef_eval_int_on_cell_faces(cm, ts->t_cur,
                                 anai->func, anai->input,
                                 3, //dimension
                                 q_tet, q_tri,
                                 c_eval, eval);

  /* Compute the averages */
  for (short int f = 0; f < nf; f++) {
    const cs_real_t _os = 1. / cm->face[f].meas;
    cs_real_t *f_eval = eval + 3*f;
    f_eval[0] *= _os, f_eval[1] *= _os, f_eval[2] *= _os;
  }

  const cs_real_t _ov = 1. / cm->vol_c;
  c_eval[0] *= _ov, c_eval[1] *= _ov, c_eval[2] *= _ov;
}
/*----------------------------------------------------------------------------*/

#undef _dp3

END_C_DECLS
