#ifndef __CS_LAGR_LEC_H__
#define __CS_LAGR_LEC_H__

/*============================================================================
 * Functions and types for lagrangian specific prints
 *============================================================================*/

/*
  This file is part of Code_Saturne, a general-purpose CFD tool.

  Copyright (C) 1998-2016 EDF S.A.

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

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS
/*=============================================================================
 * Macro definitions
 *============================================================================*/

/*============================================================================
 * Type definitions
 *============================================================================*/

/*=============================================================================
 * Global variables
 *============================================================================*/

/*=============================================================================
 * Public function prototypes
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Lecture des fichiers suite Lagrangien "lagamo" et "lasamo"
 *    contenant les informations sur les particule, les statistiques
 *    volumiques et aux frontieres, ainsi que les termes sources
 *    de couplage retour.
 *     Tous les tableaux sont initialise a zero avant d'être remplis
 *    dans le cas d'une suite (sinon ils restent a zero).
 *    On realise donc ici l'initialisation des tableaux ouverts
 *    dans MEMLA1, ce qui termine l'etape d'initialisation debutee
 *    dans LAGOPT.
 *
 * Parameters:
 * \param[in] nsalto
 * \param[in] nsalpp
 * \param[in] iprpfl
 */
/*----------------------------------------------------------------------------*/

void
CS_PROCF(laglec, LAGLEC)(cs_lnum_t *nsalto,
                         cs_lnum_t *nsalpp,
                         cs_lnum_t  iprpfl[]);

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Lecture des fichiers suite Lagrangien "lagamo" et "lasamo"
 *    contenant les informations sur les particule, les statistiques
 *    volumiques et aux frontieres, ainsi que les termes sources
 *    de couplage retour.
 *    Tous les tableaux sont initialise a zero avant d'être remplis
 *    dans le cas d'une suite (sinon ils restent a zero).
 *    On realise donc ici l'initialisation des tableaux ouverts
 *    dans MEMLA1, ce qui termine l'etape d'initialisation debutee
 *    dans LAGOPT.
 */
/*----------------------------------------------------------------------------*/

void
cs_lagr_restart_read_p(void);

/*----------------------------------------------------------------------------*/
/*!
 * \brief Restart files writings
 *
 * 1. Ecriture du fichier suite 'lagava' :
 *    - variables sur les particules (ETTP)
 *    - informations sur les particules (ITEPA, TEPA)
 * 2. Ecriture du fichier suite statistiques et termes sources
 *    'lasava' :
 *    - statistiques volumiques (STATIS)
 *    - statistiques aux frontieres (PARBOR)
 *    - termes sources de couplage retour (TSLAGR)
 * 3. Finalisation des sorties graphiques
 *
 * Parameters:
 * \param[in] nsalpp
 * \param[in] nsalto
 * \param[in] iprpfl
 */
/*----------------------------------------------------------------------------*/

void
CS_PROCF (lagout, LAGOUT)(cs_lnum_t *nsalpp,
                          cs_lnum_t *nsalto,
                          cs_lnum_t  iprpfl[]);

/*----------------------------------------------------------------------------*/

END_C_DECLS

#endif /* __CS_LAGR_LEC_H__ */