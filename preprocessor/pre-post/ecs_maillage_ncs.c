/*============================================================================
 *  Définitions des fonctions
 *   associées à la structure `ecs_maillage_t' décrivant un maillage
 *   et réalisant les sorties pour l'interfacage avec le Noyau
 *============================================================================*/

/*
  This file is part of the Code_Saturne Preprocessor, element of the
  Code_Saturne CFD tool.

  Copyright (C) 1999-2009 EDF S.A., France

  contact: saturne-support@edf.fr

  The Code_Saturne Preprocessor is free software; you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  The Code_Saturne Preprocessor is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the Code_Saturne Preprocessor; if not, write to the
  Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA
*/


/*============================================================================
 *                                 Visibilité
 *============================================================================*/

#include "cs_config.h"


/*----------------------------------------------------------------------------
 *  Fichiers `include' librairie standard C
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>


/*----------------------------------------------------------------------------
 *  Fichiers `include' visibles du  paquetage global "Utilitaire"
 *----------------------------------------------------------------------------*/

#include "ecs_def.h"
#include "ecs_comm.h"
#include "ecs_mem.h"
#include "ecs_tab.h"


/*----------------------------------------------------------------------------
 *  Fichiers `include' visibles des paquetages visibles
 *----------------------------------------------------------------------------*/

#include "ecs_table_att.h"
#include "ecs_table_def.h"
#include "ecs_table_comm.h"
#include "ecs_famille_chaine.h"


/*----------------------------------------------------------------------------
 *  Fichiers `include' visibles du  paquetage courant
 *----------------------------------------------------------------------------*/

#include "ecs_maillage.h"

/*----------------------------------------------------------------------------
 *  Fichier  `include' du  paquetage courant associe au fichier courant
 *----------------------------------------------------------------------------*/

#include "ecs_maillage_ncs.h"


/*----------------------------------------------------------------------------
 *  Fichiers `include' prives   du  paquetage courant
 *----------------------------------------------------------------------------*/

#include "ecs_maillage_priv.h"


/*============================================================================
 *                       Prototypes de fonctions privees
 *============================================================================*/


/*============================================================================
 *                              Fonctions privees
 *============================================================================*/

/*----------------------------------------------------------------------------
 *  Fonction qui construit la liste des de bord.
 *
 *  Un tableau indiquant le type associé à chaque face (0 pour face isolée,
 *  1 ou 2 pour face de bord, 3 pour face interne, et 4 pour tous les autres
 *  cas (faces voyant au moins deux cellules sur un même côté, d'ou erreur
 *  de connectivité) doit être fourni en entrée.
 *----------------------------------------------------------------------------*/

static void
_maillage_ncs__liste_fac_bord(const ecs_tab_int_t  *typ_fac,
                              ecs_tab_int_t        *liste_fac_de_bord)
{
  size_t  ifac;
  size_t  cpt_fac_de_bord = 0;

  /*xxxxxxxxxxxxxxxxxxxxxxxxxxx Instructions xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/

  assert(typ_fac != NULL);
  assert(liste_fac_de_bord != NULL);

  /* Initialisations */

  cpt_fac_de_bord = 0;

  /* Première boucle sur les faces : comptage */
  /*------------------------------------------*/

  for (ifac = 0; ifac < typ_fac->nbr; ifac++) {
    if (typ_fac->val[ifac] == 1 || typ_fac->val[ifac] == 2)
      cpt_fac_de_bord += 1;
  }

  /* Initialisation de la liste */

  liste_fac_de_bord->nbr = cpt_fac_de_bord;
  ECS_MALLOC(liste_fac_de_bord->val, liste_fac_de_bord->nbr, ecs_int_t);

  /* Seconde boucle sur les faces : remplissage de la liste */
  /*--------------------------------------------------------*/

  cpt_fac_de_bord = 0;

  for (ifac = 0; ifac < typ_fac->nbr; ifac++) {
    if (typ_fac->val[ifac] == 1 || typ_fac->val[ifac] == 2)
      liste_fac_de_bord->val[cpt_fac_de_bord++] = ifac;
  }
}

/*----------------------------------------------------------------------------
 *  Fonction qui (re)numérote les groupes et construit un tableau contenant
 *   les noms ordonnés de ces groupes. Le tableau de chaînes de caractères
 *   noms_groupes doit initialement être vide.
 *----------------------------------------------------------------------------*/

static void
_maillage_ncs__renum_groupes(const ecs_maillage_t  *maillage,
                             const ecs_int_t        nbr_fam_ent[],
                             ecs_tab_char_t        *tab_propr_nom_fam_ent[],
                             ecs_tab_char_t         *noms_groupes)
{
  int             ient;
  int             ifam;
  size_t          iloc;
  size_t          ind;

  size_t          cpt_nom_tot;

  ecs_tab_int_t   tab_renum;

  ecs_tab_char_t  tab_nom_cat;
  ecs_tab_char_t  tab_nom_trie;
  ecs_tab_char_t  tab_nom_cpct;

  /*xxxxxxxxxxxxxxxxxxxxxxxxxxx Instructions xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/

  noms_groupes->nbr = 0;
  noms_groupes->val = NULL;

  /* Comptage */

  cpt_nom_tot = 0;

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

    if (maillage->famille[ient] != NULL) {
      for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++)
        cpt_nom_tot += tab_propr_nom_fam_ent[ient][ifam].nbr;
    }
  }

  tab_nom_cat.nbr = cpt_nom_tot;
  ECS_MALLOC(tab_nom_cat.val, tab_nom_cat.nbr, char *);

  /* Préparation */

  cpt_nom_tot = 0;

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

    if (maillage->famille[ient] != NULL) {

      for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++) {

        for (iloc = 0; iloc < tab_propr_nom_fam_ent[ient][ifam].nbr; iloc++) {

          tab_nom_cat.val[cpt_nom_tot]
            = tab_propr_nom_fam_ent[ient][ifam].val[iloc];

          cpt_nom_tot++;

        }

      }
    }
  }

  /* Tri et renumérotation */

  tab_renum.nbr = tab_nom_cat.nbr;
  ECS_MALLOC(tab_renum.val, tab_renum.nbr,  ecs_int_t);

  tab_nom_trie = ecs_tab_char__trie_et_renvoie(tab_nom_cat, tab_renum);

  ECS_FREE(tab_nom_cat.val);
  ECS_FREE(tab_renum.val);

  tab_nom_cpct = ecs_tab_char__compacte(tab_nom_trie);

  ECS_FREE(tab_nom_trie.val);

  /* Recopie dans le tableau des noms de groupes; */

  noms_groupes->nbr = tab_nom_cpct.nbr;
  ECS_MALLOC(noms_groupes->val, noms_groupes->nbr, char *);

  for (ind = 0; ind < tab_nom_cpct.nbr; ind++) {

    ECS_MALLOC(noms_groupes->val[ind], strlen(tab_nom_cpct.val[ind]) + 1, char);

    strcpy(noms_groupes->val[ind], tab_nom_cpct.val[ind]);

  }

  ECS_FREE(tab_nom_cpct.val);
}

/*----------------------------------------------------------------------------
 *  Fonction qui affiche le nombre de cellules, faces internes, et faces
 *  de bord fournies en argument.
 *----------------------------------------------------------------------------*/

static void
_maillage_ncs__aff_nbr_ent(ecs_int_t  nbr_elt_cel,
                           ecs_int_t  nbr_elt_fac_interne,
                           ecs_int_t  nbr_elt_fac_de_bord)
{
  /*xxxxxxxxxxxxxxxxxxxxxxxxxxx Instructions xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/

  size_t lng_var_nbr;

  lng_var_nbr = strlen(_("Number of cells"));
  lng_var_nbr = ECS_MAX(lng_var_nbr, strlen(_("Number of internal faces")));
  lng_var_nbr = ECS_MAX(lng_var_nbr, strlen(_("Number of boundary faces")));

  if (nbr_elt_cel > 0) {
    printf("  ");
    ecs_print_padded_str(_("Number of cells"), lng_var_nbr);
    printf(" : %*ld\n", ECS_LNG_AFF_ENT, (long)nbr_elt_cel);
  }

  if (nbr_elt_fac_interne > 0) {
    printf("  ");
    ecs_print_padded_str(_("Number of internal faces"), lng_var_nbr);
    printf(" : %*ld\n", ECS_LNG_AFF_ENT, (long)nbr_elt_fac_interne);
  }

  if (nbr_elt_fac_de_bord > 0) {
    printf("  ");
    ecs_print_padded_str(_("Number of boundary faces"), lng_var_nbr);
    printf(" : %*ld\n", ECS_LNG_AFF_ENT, (long)nbr_elt_fac_de_bord);
  }
}

/*----------------------------------------------------------------------------
 *  Fonction qui renvoie la liste des tables "famille"
 *   pour toutes les entités de maillage
 *  et qui détermine :
 *  - le nombre   de familles à transférer au Noyau
 *  - le nombre maximal de  propriétés des familles
 *  - les numéros de famille  des cellules
 *  - les numéros de famille  des faces
 *  - les numéros à affecter aux groupes (et inversement, les noms par numéro)
 *  - les propriétés des familles
 *----------------------------------------------------------------------------*/

static void
_maillage_ncs__cree_fam(const ecs_maillage_t    *maillage,
                        size_t                   nbr_cel,
                        size_t                   nbr_fac,
                        ecs_tab_int_t           *liste_fac_de_bord,
                        int                     *nbr_fam,
                        int                     *nbr_max_propr,
                        ecs_tab_int_t           *tab_fam_cel,
                        ecs_tab_int_t           *tab_fam_fac,
                        ecs_tab_int_t           *tab_propr_fam,
                        ecs_tab_char_t          *noms_groupes)
{
  bool           bool_cree_fam_par_defaut;

  int            ient;
  int            cpt_fam;
  size_t         cpt_prop;
  size_t         decal_fam_ent;
  size_t         icel;
  size_t         ifac;
  ecs_int_t      ifam;
  size_t         ipropr;
  size_t         nbr_cel_avec_fam_defaut;
  size_t         nbr_fac_avec_fam_defaut;
  size_t         nbr_fbr_avec_fam_defaut;
  ecs_int_t      nbr_fam_tot;
  ecs_int_t      nbr_loc_propr;
  ecs_int_t      num_fam_defaut;

  ecs_int_t     *cpt_elt_ent_fam;
  ecs_int_t      nbr_fam_ent[ECS_N_ENTMAIL];

  ecs_int_t      left;
  ecs_int_t      right;
  ecs_int_t      mid;
  ecs_int_t      num_grp_loc;
  char          *nom_grp_loc;

  ecs_tab_int_t    tab_nbr_cel_fam;
  ecs_tab_int_t    tab_nbr_fac_fam;
  ecs_tab_int_t    tab_nbr_fbr_fam;
  ecs_tab_char_t  *tab_propr_nom_fam_ent[ECS_N_ENTMAIL];

  /*xxxxxxxxxxxxxxxxxxxxxxxxxxx Instructions xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/

  assert(maillage != NULL);

  noms_groupes->nbr = 0;
  noms_groupes->val = NULL;

  nbr_fam_tot = 0;

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

    if (maillage->famille[ient] != NULL) {

      nbr_fam_ent[ient] = ecs_famille_chaine__ret_nbr(maillage->famille[ient]);
      nbr_fam_tot += nbr_fam_ent[ient];

    }
    else {

      assert(maillage->famille[ient] == NULL);

      nbr_fam_ent[ient] = 0;

    }

  } /* Fin : boucle sur les entités de maillage */

  tab_nbr_cel_fam.nbr = nbr_fam_tot;
  tab_nbr_fac_fam.nbr = nbr_fam_tot;
  tab_nbr_fbr_fam.nbr = nbr_fam_tot;

  if (nbr_fam_tot != 0) {
    ECS_MALLOC(tab_nbr_cel_fam.val,
               tab_nbr_cel_fam.nbr, ecs_int_t);
    ECS_MALLOC(tab_nbr_fac_fam.val,
               tab_nbr_fac_fam.nbr, ecs_int_t);
    ECS_MALLOC(tab_nbr_fbr_fam.val,
               tab_nbr_fbr_fam.nbr, ecs_int_t);
  }
  else {
    tab_nbr_cel_fam.val = NULL;
    tab_nbr_fac_fam.val = NULL;
    tab_nbr_fbr_fam.val = NULL;
  }

  num_fam_defaut = nbr_fam_tot + 1;

  /* Numéros des familles des cellules */
  /*-----------------------------------*/

  if (maillage->elt_fam[ECS_ENTMAIL_CEL] != NULL && nbr_cel != 0) {

    *tab_fam_cel = ecs_table_att__fam_elt(nbr_cel,
                                          maillage->elt_fam[ECS_ENTMAIL_CEL],
                                          &tab_nbr_cel_fam);

  }
  else {

    ECS_MALLOC(tab_fam_cel->val, nbr_cel, ecs_int_t);
    tab_fam_cel->nbr = nbr_cel;

    for (icel = 0; icel < nbr_cel; icel++)
      tab_fam_cel->val[icel] = 0;

    for (ifam = 0; ifam < nbr_fam_tot; ifam++)
      tab_nbr_cel_fam.val[ifam] = 0;

  }

  nbr_cel_avec_fam_defaut = 0;

  for (icel = 0; icel < nbr_cel; icel++) {
    if (tab_fam_cel->val[icel] == 0) {
      tab_fam_cel->val[icel] = num_fam_defaut;
      nbr_cel_avec_fam_defaut++;
    }
  }

  /* Numéros des familles des faces */
  /*--------------------------------*/

  if (maillage->elt_fam[ECS_ENTMAIL_FAC] != NULL) {

    *tab_fam_fac = ecs_table_att__fam_elt(nbr_fac,
                                          maillage->elt_fam[ECS_ENTMAIL_FAC],
                                          &tab_nbr_fac_fam);

  }
  else {

    ECS_MALLOC(tab_fam_fac->val, nbr_fac, ecs_int_t);
    tab_fam_fac->nbr = nbr_fac;

    for (ifac = 0; ifac < nbr_fac; ifac++)
      tab_fam_fac->val[ifac] = 0;

    for (ifam = 0; ifam < nbr_fam_tot; ifam++)
      tab_nbr_fac_fam.val[ifam] = 0;

  }

  nbr_fac_avec_fam_defaut = 0;
  nbr_fbr_avec_fam_defaut = 0;

  for (ifac = 0; ifac < nbr_fac; ifac++) {
    if (tab_fam_fac->val[ifac] == 0) {
      tab_fam_fac->val[ifac] = num_fam_defaut;
      nbr_fac_avec_fam_defaut++;
    }
  }

  for (ifam = 0; ifam < (ecs_int_t)(tab_nbr_fbr_fam.nbr); ifam++)
    tab_nbr_fbr_fam.val[ifam] = 0;

  if (nbr_fam_tot > 0) {
    for (ifac = 0; ifac < liste_fac_de_bord->nbr; ifac++) {
      if (tab_fam_fac->val[liste_fac_de_bord->val[ifac]] == num_fam_defaut)
        nbr_fbr_avec_fam_defaut++;
      else
        tab_nbr_fbr_fam.val
          [tab_fam_fac->val[liste_fac_de_bord->val[ifac]] - 1] += 1;
    }
  }
  else
    nbr_fbr_avec_fam_defaut += liste_fac_de_bord->nbr;

  /*-------------------------------------------------------*/
  /* Détermination du  nombre  de  propriétés des familles */
  /* Détermination             des propriétés des familles */
  /*-------------------------------------------------------*/

  *nbr_max_propr = 1;

  for (ient =  (ecs_int_t)ECS_ENTMAIL_CEL;
       ient >= (ecs_int_t)ECS_ENTMAIL_FAC; ient--) {

    if (maillage->famille[ient] != NULL) {

      tab_propr_nom_fam_ent[ient]
        = ecs_famille_chaine__ret_nom(maillage->famille[ient]);

      /* Calcul du nombre maximal de propriétés */
      for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++) {
        nbr_loc_propr = tab_propr_nom_fam_ent[ient][ifam].nbr;
        *nbr_max_propr = ECS_MAX(*nbr_max_propr, nbr_loc_propr);
      }

    }

  }

  /* Mise à jour du tableau des numéros locaux des groupes */

  _maillage_ncs__renum_groupes(maillage,
                               nbr_fam_ent,
                               tab_propr_nom_fam_ent,
                               noms_groupes);

  if (nbr_fam_tot != 0) {

    for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

      cpt_elt_ent_fam = NULL;

      if (maillage->elt_fam[ient] != NULL) {

        /* Nombre d'éléments par entité et par famille */
        cpt_elt_ent_fam
          = ecs_table_att__ret_nbr_elt_fam
              (ecs_table__ret_elt_nbr(maillage->table_def[ient]),
               maillage->elt_fam[ient],
               nbr_fam_tot);

      }
      else {

        ECS_MALLOC(cpt_elt_ent_fam, nbr_fam_tot, ecs_int_t);
        for (ifam = 0; ifam < nbr_fam_tot; ifam++)
          cpt_elt_ent_fam[ifam] = 0;

      }

      if (cpt_elt_ent_fam != NULL)
        ECS_FREE(cpt_elt_ent_fam);

    }
  }

  /* Affichage des définitions des familles */
  /*----------------------------------------*/

  printf(_("\n\n"
           "Definition of face and cell families\n"
           "------------------------------------\n\n"));

  if (nbr_fac_avec_fam_defaut > 0) {

    /* On a du attribuer une famille par défaut à certaines faces */

    if (nbr_fbr_avec_fam_defaut > 0)
      ecs_warn();

    printf
      (_("%d faces from a total of %d do not belong to a group...\n"
         "A default family is assigned to those faces.\n\n"),
       (int)nbr_fac_avec_fam_defaut, (int)nbr_fac);

  }

  if (nbr_cel_avec_fam_defaut > 0) {

    /* On a du attribuer une famille par défaut à certaines cellules */

    ecs_warn();
    printf
      (_("%d cells from a total of %d do not belong to a group...\n"
         "A default family is assigned to those cells.\n\n"),
       (int)nbr_cel_avec_fam_defaut, (int)nbr_cel);

  }

  cpt_fam = 0;

  if (nbr_fac_avec_fam_defaut > 0 || nbr_cel_avec_fam_defaut > 0)
    bool_cree_fam_par_defaut = true;
  else
    bool_cree_fam_par_defaut = false;

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

    for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++) {

      ecs_famille_chaine__affiche(ifam + 1, maillage->famille[ient]);

      _maillage_ncs__aff_nbr_ent
        (tab_nbr_cel_fam.val[cpt_fam],
         (tab_nbr_fac_fam.val[cpt_fam] - tab_nbr_fbr_fam.val[cpt_fam]),
         tab_nbr_fbr_fam.val[cpt_fam]);

      cpt_fam++;

    }
  }

  if (nbr_fam_tot != 0) {

    if (tab_nbr_cel_fam.nbr != 0)
      ECS_FREE(tab_nbr_cel_fam.val);

    if (tab_nbr_fac_fam.nbr != 0)
      ECS_FREE(tab_nbr_fac_fam.val);

    if (tab_nbr_fbr_fam.nbr != 0)
      ECS_FREE(tab_nbr_fbr_fam.val);

  }

  /* Affichage famille par défaut si nécessaire */

  if (bool_cree_fam_par_defaut == true) {

    printf("  %s %d\n", _("Family"),
           cpt_fam + 1);

    printf("  %*s%s\n", (int)(strlen(_("Family")) + 1), "",
           _("Default family"));
    printf("  %*s%s\n", (int)(strlen(_("Family")) + 1), "",
           _("(no group)"));

    /* Comme on ne traite pas les faces internes, inutile d'affecter la */
    /* famille par défaut à celles n'appartenant pas à une famille > 0  */

    _maillage_ncs__aff_nbr_ent
      (nbr_cel_avec_fam_defaut,
       (nbr_fac_avec_fam_defaut - nbr_fbr_avec_fam_defaut),
       nbr_fbr_avec_fam_defaut);

  } /* Fin : si affichage de la famille par défaut */

  *nbr_fam = cpt_fam;
  if (bool_cree_fam_par_defaut == true)
    *nbr_fam += 1;

  /* Concaténation des propriétés des familles */
  /*-------------------------------------------*/

  /* Pas de propriétés pour la famille 0 */

  tab_propr_fam->nbr = (*nbr_fam) * (*nbr_max_propr);
  ECS_MALLOC(tab_propr_fam->val, tab_propr_fam->nbr, ecs_int_t);

  /* Liste des propriétés des familles  */

  /* Attention : pour le noyau on n'écrit pas les propriétés par famille */
  /*             mais on écrit les i-emes propriétés de toutes           */

  decal_fam_ent = 0;

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {

    for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++) {

      cpt_prop = 0;

      /* Groupes */

      for (ipropr = 0;
           ipropr < tab_propr_nom_fam_ent[ient][ifam].nbr;
           ipropr++) {

        /* Recherche par dichotomie dans le tableau des groupes */

        nom_grp_loc = tab_propr_nom_fam_ent[ient][ifam].val[ipropr];

        left  = 0;
        right = noms_groupes->nbr - 1;

        while ((right - left) > 1) {

          mid = (right + left) / 2;  /* Division entière */
          if (strcmp(nom_grp_loc, noms_groupes->val[mid]) <= 0)
            right = mid;
          else
            left  = mid;

        }

        if (strcmp(nom_grp_loc, noms_groupes->val[right]) == 0)
          num_grp_loc = -(right + 1);
        else if (strcmp(nom_grp_loc, noms_groupes->val[left]) == 0)
          num_grp_loc = -(left + 1);
        else {
          assert(0);
          num_grp_loc = 0;
        }

        tab_propr_fam->val[decal_fam_ent + cpt_prop * (*nbr_fam) + ifam]
          = num_grp_loc;

        cpt_prop++;

      }

      /* On complète par des '0' */

      for (ipropr = cpt_prop;
           ipropr < (size_t)(*nbr_max_propr); ipropr++)
        tab_propr_fam->val[decal_fam_ent + ipropr * (*nbr_fam) + ifam] = 0;

    }

    decal_fam_ent += nbr_fam_ent[ient];
  }

  if (bool_cree_fam_par_defaut == true) {

    /* On remplit la famille par défaut avec des '0' */

    for (ipropr = 0; ipropr < (size_t)(*nbr_max_propr); ipropr++)
      tab_propr_fam->val[decal_fam_ent + ipropr * (*nbr_fam)] = 0;

  }

  for (ient = ECS_ENTMAIL_CEL; ient >= ECS_ENTMAIL_FAC; ient--) {
    for (ifam = 0; ifam < nbr_fam_ent[ient]; ifam++)
      ECS_FREE(tab_propr_nom_fam_ent[ient][ifam].val);
    if (nbr_fam_ent[ient] != 0)
      ECS_FREE(tab_propr_nom_fam_ent[ient]);
  }
}

/*============================================================================
 *                             Fonctions publiques
 *============================================================================*/

/*----------------------------------------------------------------------------
 *  Fonction qui écrit les données dans le fichier d'interface pour le noyau
 *----------------------------------------------------------------------------*/

void
ecs_maillage_ncs__ecr(const char      *output,
                      ecs_maillage_t  *maillage)
{
  size_t          n_cells, n_faces, n_vertices, face_vertices_size;

  int             nbr_fam;
  int             nbr_max_propr;

  size_t          ind_grp;
  ecs_int_t      *pos_nom_grp;
  char           *val_nom_grp;

  ecs_tab_int_t   typ_fac_cel;
  ecs_tab_int_t   connect_fac_cel;
  ecs_tab_int_t   liste_fac_de_bord;

  ecs_tab_int_t   tab_fam_cel;
  ecs_tab_int_t   tab_fam_fac;
  ecs_tab_int_t   tab_propr_fam;

  ecs_tab_char_t  tab_noms_groupes;

  ecs_comm_t     *comm;

  /*xxxxxxxxxxxxxxxxxxxxxxxxxxx Instructions xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/

  assert(maillage != NULL);
  assert(maillage->vertex_coords != NULL);
  assert(maillage->table_def[ECS_ENTMAIL_FAC] != NULL);
  assert(maillage->table_def[ECS_ENTMAIL_CEL] != NULL);

  n_cells = ecs_table__ret_elt_nbr(maillage->table_def[ECS_ENTMAIL_CEL]);
  n_faces = ecs_table__ret_elt_nbr(maillage->table_def[ECS_ENTMAIL_FAC]);
  n_vertices = maillage->n_vertices;

  face_vertices_size
    = ecs_table__ret_val_nbr(maillage->table_def[ECS_ENTMAIL_FAC]);

  /*--------------------------------------------------*/
  /* Détermination :                                  */
  /* - du nombre   de familles à transférer au Noyau  */
  /* - des numéros de famille  des faces de bord      */
  /* - du nombre maximal de  propriétés des familles  */
  /* - des propriétés des familles                    */
  /*--------------------------------------------------*/

  typ_fac_cel
    = ecs_table_def__typ_fac_cel(maillage->table_def[ECS_ENTMAIL_CEL],
                                 maillage->table_def[ECS_ENTMAIL_FAC]);

  _maillage_ncs__liste_fac_bord(&typ_fac_cel,
                                &liste_fac_de_bord);

  typ_fac_cel.nbr = 0;
  ECS_FREE(typ_fac_cel.val);

  _maillage_ncs__cree_fam(maillage,
                          n_cells,
                          n_faces,
                          &liste_fac_de_bord,
                          &nbr_fam,
                          &nbr_max_propr,
                          &tab_fam_cel,
                          &tab_fam_fac,
                          &tab_propr_fam,
                          &tab_noms_groupes);

  liste_fac_de_bord.nbr = 0;
  ECS_FREE(liste_fac_de_bord.val);

  /* En cas de simulation seule, libération des tableaux temporaires
     et sortie directe */

  if (output == NULL) {

    if (tab_propr_fam.nbr != 0)
      ECS_FREE(tab_propr_fam.val);
    if (tab_fam_cel.nbr != 0)
      ECS_FREE(tab_fam_cel.val);
    if (tab_fam_fac.nbr != 0)
      ECS_FREE(tab_fam_fac.val);

    if (tab_noms_groupes.nbr > 0) {
      for (ind_grp = 0; ind_grp < tab_noms_groupes.nbr; ind_grp++)
        ECS_FREE(tab_noms_groupes.val[ind_grp]);
      ECS_FREE(tab_noms_groupes.val);
      tab_noms_groupes.nbr = 0;
    }

    return;
  }

  /* Initialisation de la communication vers le Noyau */
  /*--------------------------------------------------*/

  printf(_("\n\nWrite output for Kernel\n"
           "-----------------------\n"));

  comm = ecs_comm_initialize(output);

  /*------------------------------------------------------------------------
   * Écriture des dimensions :
   *------------------------------------------------------------------------
   * - nombre de cellules
   * - nombre de faces
   * - nombre de sommets
   * - dimensionnement table de connectivité "faces -> sommets"
   * - nombre de familles
   * - nombre de propriétés max. des familles
   * - propriétés  des familles
   *------------------------------------------------------------------------*/

  ecs_comm_write_section("start_block:dimensions",
                         0,
                         0, 0, 0, true,
                         NULL,
                         ECS_TYPE_void,
                         comm);

  /* Écriture du nombre de cellules, faces, et sommets */

  ecs_comm_write_section("n_cells",
                         1,
                         1, 0, 0, true,
                         &n_cells,
                         ECS_TYPE_size_t,
                         comm);

  ecs_comm_write_section("n_faces",
                         1,
                         2, 0, 0, true,
                         &n_faces,
                         ECS_TYPE_size_t,
                         comm);

  ecs_comm_write_section("n_vertices",
                         1,
                         3, 0, 0, true,
                         &n_vertices,
                         ECS_TYPE_size_t,
                         comm);

  ecs_comm_write_section("face_vertices_size",
                         1,
                         0, 0, 1, true,
                         &face_vertices_size,
                         ECS_TYPE_size_t,
                         comm);

  /* Écriture du nombre de familles (on ne compte pas la famille `0') */

  ecs_comm_write_section("n_group_classes",
                         1,
                         0, 0, 1, true,
                         &nbr_fam,
                         ECS_TYPE_ecs_int_t,
                         comm);

  /* Écriture du nombre de propriétés max. des familles */

  ecs_comm_write_section("n_group_class_props_max",
                         1,
                         0, 0, 1, true,
                         &nbr_max_propr,
                         ECS_TYPE_ecs_int_t,
                         comm);

  /* Écriture des noms de groupes si lieu */

  if (tab_noms_groupes.nbr > 0) {

    ecs_comm_write_section("n_groups",
                           1,
                           0, 0, 1, true,
                           &(tab_noms_groupes.nbr),
                           ECS_TYPE_size_t,
                           comm);

    ECS_MALLOC(pos_nom_grp, tab_noms_groupes.nbr + 1, ecs_int_t);

    pos_nom_grp[0] = 1;
    for (ind_grp = 0; ind_grp < tab_noms_groupes.nbr; ind_grp++)
      pos_nom_grp[ind_grp + 1]
        = pos_nom_grp[ind_grp] + strlen(tab_noms_groupes.val[ind_grp]) + 1;

    ecs_comm_write_section("group_name_index",
                           tab_noms_groupes.nbr + 1,
                           0, 1, 1, true,
                           pos_nom_grp,
                           ECS_TYPE_ecs_int_t,
                           comm);

    ECS_MALLOC(val_nom_grp, pos_nom_grp[tab_noms_groupes.nbr] - 1, char);

    for (ind_grp = 0; ind_grp < tab_noms_groupes.nbr; ind_grp++)
      strcpy(val_nom_grp + (pos_nom_grp[ind_grp] - 1),
             tab_noms_groupes.val[ind_grp]);

    ecs_comm_write_section("group_name",
                           pos_nom_grp[tab_noms_groupes.nbr] - 1,
                           0, 1, 1, true,
                           val_nom_grp,
                           ECS_TYPE_char,
                           comm);

    ECS_FREE(pos_nom_grp);
    ECS_FREE(val_nom_grp);

    for (ind_grp = 0; ind_grp < tab_noms_groupes.nbr; ind_grp++)
      ECS_FREE(tab_noms_groupes.val[ind_grp]);
    ECS_FREE(tab_noms_groupes.val);
    tab_noms_groupes.nbr = 0;

  }

  /* Écriture des propriétés des familles   */
  /* - propriétés des familles des cellules */
  /* - propriétés des familles des faces    */
  /* Pas de propriété pour la famille `0'   */

  ecs_comm_write_section("group_class_properties",
                         nbr_fam * nbr_max_propr,
                         0, 0, nbr_max_propr, true,
                         tab_propr_fam.val,
                         ECS_TYPE_ecs_int_t,
                         comm);

  if (tab_propr_fam.nbr != 0)
    ECS_FREE(tab_propr_fam.val);

  /* Passage du bloc des dimensions au bloc des données */
  /*----------------------------------------------------*/

  ecs_comm_write_section("end_block:dimensions",
                         0,
                         0, 0, 0, true,
                         NULL,
                         ECS_TYPE_void,
                         comm);

  ecs_comm_write_section("start_block:data",
                         0,
                         0, 0, 0, true,
                         NULL,
                         ECS_TYPE_void,
                         comm);

  /*---------------------------------------------------------------
   * Écriture des données :
   *---------------------------------------------------------------
   * - connectivité faces -> cellules voisines
   * - numeros     de  famille
   * - positions   des sommets des faces
   * - connectivité faces -> sommets
   * - coordonnées des sommets
   *---------------------------------------------------------------*/


  /* Écriture de la connectivité faces -> cellules voisines */

  connect_fac_cel
    = ecs_table_def__fac_cel(maillage->table_def[ECS_ENTMAIL_CEL],
                             maillage->table_def[ECS_ENTMAIL_FAC]);

  ecs_comm_write_section("face_cells",
                         n_faces * 2,
                         2, 0, 2, false,
                         connect_fac_cel.val,
                         ECS_TYPE_ecs_int_t,
                         comm);

  connect_fac_cel.nbr = 0;
  ECS_FREE(connect_fac_cel.val);

  /* Écriture des numéros de famille des cellules */

  ecs_comm_write_section("cell_group_class_id",
                         tab_fam_cel.nbr,
                         1, 0, 1, false,
                         tab_fam_cel.val,
                         ECS_TYPE_ecs_int_t,
                         comm);

  /* Écriture des numéros de famille des faces */

  ecs_comm_write_section("face_group_class_id",
                         tab_fam_fac.nbr,
                         2, 0, 1, false,
                         tab_fam_fac.val,
                         ECS_TYPE_ecs_int_t,
                         comm);

  /* Écriture des positions des sommets des faces */

  ecs_table_comm__ecr_pos(maillage->table_def[ECS_ENTMAIL_FAC],
                          "face_vertices_index",
                          2, 1,
                          comm);

  /* Écriture de la connectivité "faces -> sommets" */

  ecs_table_comm__ecr(maillage->table_def[ECS_ENTMAIL_FAC],
                      "face_vertices",
                      2, 1, 1,
                      comm);

  /* Écriture des coordonnées des sommets */

  ecs_comm_write_section("vertex_coords",
                         maillage->n_vertices*3,
                         3, 0, 3, false,
                         maillage->vertex_coords,
                         ECS_TYPE_ecs_coord_t,
                         comm);

  /* Ecriture de la rubrique de fin  du bloc sur les données */
  /*---------------------------------------------------------*/

  ecs_comm_write_section("end_block:data",
                         0,
                         0, 0, 0, true,
                         NULL,
                         ECS_TYPE_void,
                         comm);

  /* Ecriture de la rubrique de fin de fichier et fermeture */

  ecs_comm_write_section("EOF",
                         0,
                         0, 0, 0, true,
                         NULL,
                         ECS_TYPE_void,
                         comm);

  /* Fermeture du fichier de communication */
  /*---------------------------------------*/

  ecs_comm_finalize(&comm);

  /* Libération de la mémoire */
  /*--------------------------*/

  if (tab_fam_cel.nbr != 0)
    ECS_FREE(tab_fam_cel.val);

  if (tab_fam_fac.nbr != 0)
    ECS_FREE(tab_fam_fac.val);
}

/*----------------------------------------------------------------------------*/

