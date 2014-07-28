!-------------------------------------------------------------------------------

! This file is part of Code_Saturne, a general-purpose CFD tool.
!
! Copyright (C) 1998-2014 EDF S.A.
!
! This program is free software; you can redistribute it and/or modify it under
! the terms of the GNU General Public License as published by the Free Software
! Foundation; either version 2 of the License, or (at your option) any later
! version.
!
! This program is distributed in the hope that it will be useful, but WITHOUT
! ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
! FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
! details.
!
! You should have received a copy of the GNU General Public License along with
! this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
! Street, Fifth Floor, Boston, MA 02110-1301, USA.

!-------------------------------------------------------------------------------

subroutine cfxtcl &
!================

 ( nvar   ,                                                       &
   icodcl , itypfb ,                                              &
   dt     ,                                                       &
   rcodcl )

!===============================================================================
! FONCTION :
! --------

!    CONDITIONS AUX LIMITES AUTOMATIQUES

!           COMPRESSIBLE SANS CHOC


!-------------------------------------------------------------------------------
! Arguments
!__________________.____._____.________________________________________________.
! name             !type!mode ! role                                           !
!__________________!____!_____!________________________________________________!
! nvar             ! i  ! <-- ! total number of variables                      !
! icodcl           ! te ! --> ! code de condition limites aux faces            !
!  (nfabor,nvar    !    !     !  de bord                                       !
!                  !    !     ! = 1   -> dirichlet                             !
!                  !    !     ! = 3   -> densite de flux                       !
!                  !    !     ! = 4   -> glissemt et u.n=0 (vitesse)           !
!                  !    !     ! = 5   -> frottemt et u.n=0 (vitesse)           !
!                  !    !     ! = 6   -> rugosite et u.n=0 (vitesse)           !
!                  !    !     ! = 9   -> entree/sortie libre (vitesse          !
! itypfb           ! ia ! <-- ! boundary face types                            !
! dt(ncelet)       ! ra ! <-- ! time step (per cell)                           !
! rcodcl           ! tr ! --> ! valeur des conditions aux limites              !
!  (nfabor,nvar    !    !     !  aux faces de bord                             !
!                  !    !     ! rcodcl(1) = valeur du dirichlet                !
!                  !    !     ! rcodcl(2) = valeur du coef. d'echange          !
!                  !    !     !  ext. (infinie si pas d'echange)               !
!                  !    !     ! rcodcl(3) = valeur de la densite de            !
!                  !    !     !  flux (negatif si gain) w/m2 ou                !
!                  !    !     !  hauteur de rugosite (m) si icodcl=6           !
!                  !    !     ! pour les vitesses (vistl+visct)*gradu          !
!                  !    !     ! pour la pression             dt*gradp          !
!                  !    !     ! pour les scalaires                             !
!                  !    !     !        cp*(viscls+visct/sigmas)*gradt          !
!__________________!____!_____!________________________________________________!

!     TYPE : E (ENTIER), R (REEL), A (ALPHANUMERIQUE), T (TABLEAU)
!            L (LOGIQUE)   .. ET TYPES COMPOSES (EX : TR TABLEAU REEL)
!     MODE : <-- donnee, --> resultat, <-> Donnee modifiee
!            --- tableau de travail
!===============================================================================

!===============================================================================
! Module files
!===============================================================================

! Arguments

use paramx
use numvar
use optcal
use cstphy
use cstnum
use entsor
use parall
use ppppar
use ppthch
use ppincl
use cfpoin
use mesh
use field

!===============================================================================

implicit none

! Arguments

integer          nvar

integer          icodcl(nfabor,nvarcl)
integer          itypfb(nfabor)

double precision dt(ncelet)
double precision rcodcl(nfabor,nvarcl,3)

! Local variables

integer          ivar  , ifac  , iel, l_size
integer          ii    , iii   , iccfth
integer          icalep, icalgm
integer          iflmab
integer          ien   , itk
integer          nvarcf

integer          nvcfmx
parameter       (nvcfmx=6)
integer          ivarcf(nvcfmx)

double precision hint  , gammag

double precision, allocatable, dimension(:) :: w1, w2
double precision, allocatable, dimension(:) :: w4, w5, w6
double precision, allocatable, dimension(:) :: w7
double precision, allocatable, dimension(:) :: wbfb
double precision, allocatable, dimension(:,:) :: bval

double precision, dimension(:), pointer :: bmasfl
double precision, dimension(:), pointer :: coefbp
double precision, dimension(:), pointer :: crom, brom, cpro_cv, cvar_en
double precision, dimension(:,:), pointer :: vel

!===============================================================================

! Map field arrays
call field_get_val_v(ivarfl(iu), vel)

!===============================================================================
! 1.  INITIALISATIONS
!===============================================================================

! Allocate work arrays
allocate(w1(ncelet), w2(ncelet))
allocate(w4(ncelet), w5(ncelet), w6(ncelet))

allocate(w7(nfabor), wbfb(nfabor))
allocate(bval(nfabor,nvar))

ien = isca(ienerg)
itk = isca(itempk)

call field_get_key_int(ivarfl(ien), kbmasf, iflmab)
call field_get_val_s(iflmab, bmasfl)

call field_get_val_s(icrom, crom)
call field_get_val_s(ibrom, brom)

call field_get_val_s(ivarfl(ien), cvar_en)

if (icv.gt.0) call field_get_val_s(iprpfl(icv), cpro_cv)

!     Liste des variables compressible :
ivarcf(1) = ipr
ivarcf(2) = iu
ivarcf(3) = iv
ivarcf(4) = iw
ivarcf(5) = ien
ivarcf(6) = itk
nvarcf    = 6

call field_get_coefb_s(ivarfl(ipr), coefbp)
do ifac = 1, nfabor
  wbfb(ifac) = coefbp(ifac)
enddo

!     Calcul de epsilon_sup = e - CvT
!     On en a besoin si on a des parois a temperature imposee.
!     Il est calcul� aux cellules W5 et aux faces de bord COEFU.
!     On n'en a besoin ici qu'aux cellules de bord : s'il est
!     n�cessaire de gagner de la m�moire, on pourra modifier
!     cf_thermo_eps_sup

icalep = 0
do ifac = 1, nfabor
  if(icodcl(ifac,itk).eq.5) then
    icalep = 1
  endif
enddo
if(icalep.ne.0) then
  ! At cell centers
  call cf_thermo_eps_sup(w5, ncel)
  !===================

  ! At boundary faces centers
  call cf_thermo_eps_sup(w7, nfabor)
  !===================
endif


!     Calcul de gamma (constant ou variable ; pour le moment : cst)
!       On en a besoin pour les entrees sorties avec rusanov

icalgm = 0
do ifac = 1, nfabor
  if ( ( itypfb(ifac).eq.iesicf ) .or.                    &
       ( itypfb(ifac).eq.ieqhcf ) ) then
    icalgm = 1
  endif
enddo
if(icalgm.ne.0) then
  if(ieos.eq.1) then
    call cf_thermo_gamma(gammag)
  else
    ! TODO
    ! Gamma is used in cfrusb. If non uniform (ieos different from 1),
    ! the cell values have to be passed and used as gammag(ifabor(ifac))
    ! in the Rusanov flux computation.
    ! For now, we stop here and an error message is printed out.
    write(nfecra,7000)
    call csexit (1)
  endif

endif



!     Boucle sur les faces

do ifac = 1, nfabor
  iel = ifabor(ifac)

!===============================================================================
! 2.  REMPLISSAGE DU TABLEAU DES CONDITIONS LIMITES
!       ON BOUCLE SUR TOUTES LES FACES DE PAROI
!===============================================================================

  if ( itypfb(ifac).eq.iparoi) then

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       En paroi, on traite toutes les variables.
    do ivar = 1, nvar
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     Le flux de masse est nul

    bmasfl(ifac) = 0.d0

!     Pression :

!       Si la gravite est predominante : pression hydrostatique
!         (approximatif et surtout explicite en rho)

    if(icfgrp.eq.1) then

      icodcl(ifac,ipr) = 3
      hint = dt(iel)/distb(ifac)
      rcodcl(ifac,ipr,3) = -hint                                  &
           * ( gx*(cdgfbo(1,ifac)-xyzcen(1,iel))                  &
           + gy*(cdgfbo(2,ifac)-xyzcen(2,iel))                    &
           + gz*(cdgfbo(3,ifac)-xyzcen(3,iel)) )                  &
           * crom(iel)

    else

!       En g�n�ral : proportionnelle a la valeur interne
!         (Pbord = COEFB*Pi)
!       Si on d�tend trop : Dirichlet homogene

      call cf_thermo_wall_bc(wbfb, ifac)
      !===================

!       En outre, il faut appliquer une pre-correction pour compenser
!        le traitement fait dans condli... Si on pouvait remplir COEFA
!        et COEFB directement, on gagnerait en simplicite, mais cela
!        demanderait un test sur IPPMOD dans condli : � voir)

!FIXME with the new cofaf
      icodcl(ifac,ipr) = 1
      if(wbfb(ifac).lt.rinfin*0.5d0.and.                  &
         wbfb(ifac).gt.0.d0  ) then
        hint = dt(iel)/distb(ifac)
        rcodcl(ifac,ipr,1) = 0.d0
        rcodcl(ifac,ipr,2) = hint*(1.d0/wbfb(ifac)-1.d0)
      else
        rcodcl(ifac,ipr,1) = 0.d0
      endif

    endif


!       La vitesse et la turbulence sont trait�es de mani�re standard,
!         dans condli.

!       Pour la thermique, on doit effectuer ici un pr�traitement,
!         la variable r�solue �tant l'energie
!         (energie interne+epsilon sup+energie cin�tique). En particulier
!         lorsque la paroi est � temp�rature impos�e, on pr�pare le
!         travail de clptur. Hormis l'�nergie r�solue, toutes les
!         variables rho et s prendront arbitrairement une condition de
!         flux nul (leurs conditions aux limites ne servent qu'� la
!         reconstruction des gradients et il parait d�licat d'imposer
!         autre chose qu'un flux nul sans risque de cr�er des valeurs
!         aberrantes au voisinage de la couche limite)

!       Par d�faut : adiabatique
    if(  icodcl(ifac,itk).eq.0.and.                          &
         icodcl(ifac,ien).eq.0) then
      icodcl(ifac,itk) = 3
      rcodcl(ifac,itk,3) = 0.d0
    endif

!       Temperature imposee
    if(icodcl(ifac,itk).eq.5) then

!           On impose la valeur de l'energie qui conduit au bon flux.
!             On notera cependant qu'il s'agit de la condition � la
!               limite pour le flux diffusif. Pour la reconstruction
!               des gradients, il faudra utiliser autre chose.
!               Par exemple un flux nul ou encore toute autre
!               condition respectant un profil : on se calquera sur
!               ce qui sera fait pour la temp�rature si c'est possible,
!               sachant que l'energie contient l'energie cinetique,
!               ce qui rend le choix du profil d�licat.

      icodcl(ifac,ien) = 5
      if(icv.eq.0) then
        rcodcl(ifac,ien,1) = cv0*rcodcl(ifac,itk,1)
      else
        rcodcl(ifac,ien,1) = cpro_cv(iel)*rcodcl(ifac,itk,1)
      endif
      rcodcl(ifac,ien,1) = rcodcl(ifac,ien,1)             &
           + 0.5d0*(vel(1,iel)**2+vel(2,iel)**2+vel(3,iel)**2)          &
           + w5(iel)
!                   ^epsilon sup (cf USCFTH)

!           Les flux en grad epsilon sup et �nergie cin�tique doivent
!             �tre nuls puisque tout est pris par le terme de
!             diffusion d'energie.
      ifbet(ifac) = 1

!           Flux nul pour la reconstruction �ventuelle de temp�rature
      icodcl(ifac,itk) = 3
      rcodcl(ifac,itk,3) = 0.d0

!       Flux impose
    elseif(icodcl(ifac,itk).eq.3) then

!           On impose le flux sur l'energie
      icodcl(ifac,ien) = 3
      rcodcl(ifac,ien,3) = rcodcl(ifac,itk,3)

!           Les flux en grad epsilon sup et �nergie cin�tique doivent
!             �tre nuls puisque tout est pris par le terme de
!             diffusion d'energie.
      ifbet(ifac) = 1

!           Flux nul pour la reconstruction �ventuelle de temp�rature
      icodcl(ifac,itk) = 3
      rcodcl(ifac,itk,3) = 0.d0

    endif


!     Scalaires : flux nul (par defaut dans typecl pour iparoi)


!===============================================================================
! 3.  REMPLISSAGE DU TABLEAU DES CONDITIONS LIMITES
!       ON BOUCLE SUR TOUTES LES FACES DE SYMETRIE
!===============================================================================

  elseif ( itypfb(ifac).eq.isymet ) then

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       En symetrie, on traite toutes les variables.
    do ivar = 1, nvar
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     Le flux de masse est nul

    bmasfl(ifac) = 0.d0

! Pressure condition:
! homogeneous Neumann condition, nothing to be done.

!     Pression :
!       En g�n�ral : proportionnelle a la valeur interne
!         (Pbord = COEFB*Pi)
!       Si on d�tend trop : Dirichlet homogene

!       En outre, il faut appliquer une pre-correction pour compenser le
!        traitement fait dans condli... Si on pouvait remplir COEFA
!        et COEFB directement, on gagnerait en simplicite, mais cela
!        demanderait un test sur IPPMOD dans condli : � voir)

    icodcl(ifac,ipr) = 3
    rcodcl(ifac,ipr,1) = 0.d0
    rcodcl(ifac,ipr,2) = rinfin
    rcodcl(ifac,ipr,3) = 0.d0

!       Toutes les autres variables prennent un flux nul (sauf la vitesse
!         normale, qui est nulle) : par defaut dans typecl pour isymet.

!===============================================================================
! 4.  REMPLISSAGE DU TABLEAU DES CONDITIONS LIMITES
!       ON BOUCLE SUR TOUTES LES FACES D'ENTREE/SORTIE
!       ETAPE DE THERMO
!===============================================================================


!===============================================================================
!     4.1 Entree/sortie impos�e (par exemple : entree supersonique)
!===============================================================================

  elseif ( itypfb(ifac).eq.iesicf ) then

!     On a
!       - la vitesse,
!       - 2 variables parmi P, rho, T, E (mais pas (T,E)),
!       - la turbulence
!       - les scalaires

!     On recherche la variable a initialiser
!       (si on a donne une valeur nulle, c'est pas adapte : on supposera
!        qu'on n'a pas initialise et on sort en erreur)
    iccfth = 10000
    if(rcodcl(ifac,ipr,1).gt.0.d0) iccfth = 2*iccfth
    if(brom(ifac).gt.0.d0)         iccfth = 3*iccfth
    if(rcodcl(ifac,itk,1).gt.0.d0) iccfth = 5*iccfth
    if(rcodcl(ifac,ien,1).gt.0.d0) iccfth = 7*iccfth
    if((iccfth.le.70000.and.iccfth.ne.60000).or.                &
         (iccfth.eq.350000)) then
      write(nfecra,1000)iccfth
      call csexit (1)
    endif
    iccfth = iccfth + 900

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite d'abord les variables autres que la turbulence et les
!       scalaires passifs : celles-ci sont traitees plus bas.
    do iii = 1, nvarcf
      ivar = ivarcf(iii)
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     On calcule les variables manquantes parmi P,rho,T,E
!     COEFA sert de tableau de transfert dans USCFTH

    do ivar = 1, nvar
      bval(ifac,ivar) = rcodcl(ifac,ivar,1)
    enddo

    call cfther                                                   &
    !==========
 ( nvar   ,                                                       &
   iccfth , ifac   ,                                              &
   w1     , w2     , bval )


!     Rusanov, flux de masse et type de conditions aux limites :
!       voir plus bas


!===============================================================================
!     4.2 Sortie supersonique
!===============================================================================

  elseif ( itypfb(ifac).eq.isspcf ) then

!     On impose un Dirichlet �gal � la valeur interne pour rho u E
!       (on impose des Dirichlet d�duit pour les autres variables).
!       Il est inutile de passer dans Rusanov.
!     Il serait n�cessaire de reconstruire ces valeurs en utilisant
!       leur gradient dans la cellule de bord : dans un premier temps,
!       on utilise des valeurs non reconstruites (non consistant mais
!       potentiellement plus stable).
!     On pourrait imposer des flux nuls (a tester), ce qui �viterait
!       la n�cessit� de reconstruire les valeurs.

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite d'abord les variables autres que la turbulence et les
!       scalaires passifs : celles-ci sont traitees plus bas.
    do iii = 1, nvarcf
      ivar = ivarcf(iii)
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     Valeurs de rho u E
    brom(ifac) = crom(iel)
    rcodcl(ifac,iu ,1) = vel(1,iel)
    rcodcl(ifac,iv ,1) = vel(2,iel)
    rcodcl(ifac,iw ,1) = vel(3,iel)
    rcodcl(ifac,ien,1) = cvar_en(iel)

    do ivar = 1, nvar
      bval(ifac,ivar) = rcodcl(ifac,ivar,1)
    enddo

    l_size = 1
    call cf_thermo_pt_from_de_ni(brom(ifac:ifac), bval(ifac,ien), bval(ifac,ipr),  &
                                 bval(ifac,itk), bval(ifac,iu), bval(ifac,iv),     &
                                 bval(ifac,iw), l_size)

!               flux de masse et type de conditions aux limites :
!       voir plus bas


!===============================================================================
!     4.3 Sortie a pression imposee
!===============================================================================

  elseif ( itypfb(ifac).eq.isopcf ) then

!       Sortie subsonique a priori (si c'est supersonique dans le
!         domaine, ce n'est pas pour autant que c'est supersonique
!         � la sortie, selon la pression que l'on a impos�e)

!     On utilise un scenario dans lequel on a une 1-d�tente et un
!       2-contact entrant dans le domaine. On d�termine les conditions
!       sur l'interface selon la thermo et on passe dans Rusanov
!       ensuite pour lisser.

!     Si P n'est pas donn�, erreur ; on sort aussi en erreur si P
!       n�gatif, m�me si c'est possible, dans la plupart des cas ce
!       sera une erreur
    if(rcodcl(ifac,ipr,1).lt.-rinfin*0.5d0) then
      write(nfecra,1100)
      call csexit (1)
    endif

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite d'abord les variables autres que la turbulence et les
!       scalaires passifs : celles-ci sont traitees plus bas.
    do iii = 1, nvarcf
      ivar = ivarcf(iii)
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     Valeurs de rho, u, E, s
    do ivar = 1, nvar
      bval(ifac,ivar) = rcodcl(ifac,ivar,1)
    enddo

    call cf_thermo_subsonic_outlet_bc(bval, ifac)
    !==============================

!     Rusanov, flux de masse et type de conditions aux limites :
!       voir plus bas

!===============================================================================
!     4.4 Entree � P et H imposees
!===============================================================================

  elseif ( itypfb(ifac).eq.iephcf ) then

!       Entree subsonique a priori (si c'est supersonique dans le
!         domaine, ce n'est pas pour autant que c'est supersonique
!         � l'entree, selon les valeurs que l'on a impos�es)

!     On utilise un scenario d�tente ou choc.
!       On d�termine les conditions sur l'interface
!       selon la thermo.

!     Si P et H ne sont pas donn�s, erreur

! rcodcl(ifac,isca(ienerg),1) holds the boundary total enthalpy values prescribed by the user
    if(rcodcl(ifac,ipr ,1).lt.-rinfin*0.5d0.or.               &
         rcodcl(ifac,isca(ienerg) ,1).lt.-rinfin*0.5d0) then
      write(nfecra,1200)
      call csexit (1)
    endif

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite d'abord les variables autres que la turbulence et les
!       scalaires passifs : celles-ci sont traitees plus bas.
    do iii = 1, nvarcf
      ivar = ivarcf(iii)
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
        rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

    do ivar = 1, nvar
      bval(ifac,ivar) = rcodcl(ifac,ivar,1)
    enddo

    call cf_thermo_ph_inlet_bc(bval, ifac)
    !=======================

!     flux de masse et type de conditions aux limites :
!     voir plus bas


!===============================================================================
!     4.5 Entree � rho*U et rho*U*H imposes
!===============================================================================

  elseif ( itypfb(ifac).eq.ieqhcf ) then

!       Entree subsonique a priori (si c'est supersonique dans le
!         domaine, ce n'est pas pour autant que c'est supersonique
!         � l'entree, selon les valeurs que l'on a impos�es)

!     On utilise un scenario dans lequel on a un 2-contact et une
!       3-d�tente entrant dans le domaine. On d�termine les conditions
!       sur l'interface selon la thermo et on passe dans Rusanov
!       ensuite pour lisser.

!     Si rho et u ne sont pas donn�s, erreur
! TODO to be implemented
    if(rcodcl(ifac,irunh,1).lt.-rinfin*0.5d0) then
      write(nfecra,1300)
      call csexit (1)
    endif

!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite d'abord les variables autres que la turbulence et les
!       scalaires passifs : celles-ci sont traitees plus bas.
    do iii = 1, nvarcf
      ivar = ivarcf(iii)
      if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
          rcodcl(ifac,ivar,1) = 0.d0
      endif
    enddo

!     A coder

!     IRUNH = ISCA(IENER)
!     (aliases pour simplifier uscfcl)

    write(nfecra,1301)
    call csexit (1)

!===============================================================================
! 5. CONDITION NON PREVUE
!===============================================================================
!     Stop
  else

    write(nfecra,1400)
    call csexit (1)

! --- Fin de test sur les types de faces
  endif


!===============================================================================
! 6. FIN DU TRAITEMENT DES ENTREE/SORTIES
!     CALCUL DU FLUX DE MASSE,
!     CALCUL DES FLUX DE BORD AVEC RUSANOV (SI BESOIN)
!     TYPE DE C    .L. (DIRICHLET NEUMANN)
!===============================================================================

  if ( ( itypfb(ifac).eq.iesicf ) .or.                    &
       ( itypfb(ifac).eq.isspcf ) .or.                    &
       ( itypfb(ifac).eq.iephcf ) .or.                    &
       ( itypfb(ifac).eq.isopcf ) .or.                    &
       ( itypfb(ifac).eq.ieqhcf ) ) then

!===============================================================================
!     6.1 Flux de bord Rusanov ou simplement flux de masse
!         Attention a bien avoir calcule gamma pour Rusanov
!===============================================================================

!     Sortie supersonique :
    if ( itypfb(ifac).eq.isspcf ) then

!     Seul le flux de masse est calcule (on n'appelle pas Rusanov)
!       (toutes les variables sont connues)

      bmasfl(ifac) = brom(ifac) *                                              &
                     ( bval(ifac,iu)*surfbo(1,ifac)                            &
                     + bval(ifac,iv)*surfbo(2,ifac)                            &
                     + bval(ifac,iw)*surfbo(3,ifac) )

!     Autres entrees/sorties :
    else

!     On calcule des flux par Rusanov
!       (en particulier, le flux de masse est complete)
!       pour la condition d'entree supersonique seulement

      if ( itypfb(ifac).eq.iesicf ) then

        call cfrusb(nvar, ifac, gammag, bval)
        !==========

!    Pour les autres types (sortie subsonique, entree QH, entree PH),
!    On calcule des flux analytiques

      else

        call cffana                                                   &
        !==========
      ( nvar   ,  ifac   , bval )

      endif

    endif

!===============================================================================
!     6.2 Recuperation de COEFA
!===============================================================================

!     On r�tablit COEFA dans RCODCL
    do ivar = 1, nvar
      rcodcl(ifac,ivar,1) = bval(ifac,ivar)
    enddo

!===============================================================================
!     6.3 Types de C.L.
!===============================================================================

!     P               : Dirichlet sauf IESICF : Neumann (choix arbitraire)
!     rho, U, E, T    : Dirichlet
!     k, R, eps, scal : Dirichlet/Neumann selon flux de masse

!     Pour P, le Neumann est cens� etre moins genant pour les
!       reconstructions de gradient si la valeur de P fournie par
!       l'utilisateur est tres differente de la valeur interne.
!       Le choix est cependant arbitraire.

!     On suppose que par defaut,
!            RCODCL(IFAC,X,1) = utilisateur ou calcule ci-dessus
!            RCODCL(IFAC,X,2) = RINFIN
!            RCODCL(IFAC,X,3) = 0.D0
!       et si ICODCL(IFAC,X) = 3, seul RCODCL(IFAC,X,3) est utilis�


!-------------------------------------------------------------------------------
!     Pression : Dirichlet ou Neumann homogene
!-------------------------------------------------------------------------------

      icodcl(ifac,ipr)   = 13

!-------------------------------------------------------------------------------
!     rho U E T : Dirichlet
!-------------------------------------------------------------------------------

!     Vitesse
    icodcl(ifac,iu)    = 1
    icodcl(ifac,iv)    = 1
    icodcl(ifac,iw)    = 1
!     Energie totale
    icodcl(ifac,ien)   = 1
!     Temperature
    icodcl(ifac,itk)   = 1

!-------------------------------------------------------------------------------
!     turbulence et scalaires passifs : Dirichlet/Neumann selon flux
!-------------------------------------------------------------------------------

!       Dirichlet ou Neumann homog�ne
!       On choisit un Dirichlet si le flux de masse est entrant et
!       que l'utilisateur a donn� une valeur dans RCODCL

    if (bmasfl(ifac).ge.0.d0) then
      if(itytur.eq.2) then
        icodcl(ifac,ik ) = 3
        icodcl(ifac,iep) = 3
      elseif(itytur.eq.3) then
        icodcl(ifac,ir11) = 3
        icodcl(ifac,ir22) = 3
        icodcl(ifac,ir33) = 3
        icodcl(ifac,ir12) = 3
        icodcl(ifac,ir13) = 3
        icodcl(ifac,ir23) = 3
        icodcl(ifac,iep ) = 3
      elseif(iturb.eq.50) then
        icodcl(ifac,ik  ) = 3
        icodcl(ifac,iep ) = 3
        icodcl(ifac,iphi) = 3
        icodcl(ifac,ifb ) = 3
      elseif(iturb.eq.60) then
        icodcl(ifac,ik  ) = 3
        icodcl(ifac,iomg) = 3
      elseif(iturb.eq.70) then
        icodcl(ifac,inusa) = 3
      endif
      if(nscaus.gt.0) then
        do ii = 1, nscaus
          icodcl(ifac,isca(ii)) = 3
        enddo
      endif
    else
      if(itytur.eq.2) then
        if(rcodcl(ifac,ik ,1).gt.0.d0.and.               &
             rcodcl(ifac,iep,1).gt.0.d0) then
          icodcl(ifac,ik ) = 1
          icodcl(ifac,iep) = 1
        else
          icodcl(ifac,ik ) = 3
          icodcl(ifac,iep) = 3
        endif
      elseif(itytur.eq.3) then
        if(rcodcl(ifac,ir11,1).gt.0.d0.and.              &
             rcodcl(ifac,ir22,1).gt.0.d0.and.              &
             rcodcl(ifac,ir33,1).gt.0.d0.and.              &
             rcodcl(ifac,ir12,1).gt.-rinfin*0.5d0.and.     &
             rcodcl(ifac,ir13,1).gt.-rinfin*0.5d0.and.     &
             rcodcl(ifac,ir23,1).gt.-rinfin*0.5d0.and.     &
             rcodcl(ifac,iep ,1).gt.0.d0) then
          icodcl(ifac,ir11) = 1
          icodcl(ifac,ir22) = 1
          icodcl(ifac,ir33) = 1
          icodcl(ifac,ir12) = 1
          icodcl(ifac,ir13) = 1
          icodcl(ifac,ir23) = 1
          icodcl(ifac,iep ) = 1
        else
          icodcl(ifac,ir11) = 3
          icodcl(ifac,ir22) = 3
          icodcl(ifac,ir33) = 3
          icodcl(ifac,ir12) = 3
          icodcl(ifac,ir13) = 3
          icodcl(ifac,ir23) = 3
          icodcl(ifac,iep ) = 3
        endif
      elseif(iturb.eq.50) then
        if(rcodcl(ifac,ik  ,1).gt.0.d0.and.              &
             rcodcl(ifac,iep ,1).gt.0.d0.and.              &
             rcodcl(ifac,iphi,1).gt.0.d0.and.              &
             rcodcl(ifac,ifb ,1).gt.-rinfin*0.5d0 ) then
          icodcl(ifac,ik  ) = 1
          icodcl(ifac,iep ) = 1
          icodcl(ifac,iphi) = 1
          icodcl(ifac,ifb ) = 1
        else
          icodcl(ifac,ik  ) = 3
          icodcl(ifac,iep ) = 3
          icodcl(ifac,iphi) = 3
          icodcl(ifac,ifb ) = 3
        endif
      elseif(iturb.eq.60) then
         if(rcodcl(ifac,ik  ,1).gt.0.d0.and.               &
              rcodcl(ifac,iomg,1).gt.0.d0 ) then
           icodcl(ifac,ik  ) = 1
           icodcl(ifac,iomg) = 1
         else
           icodcl(ifac,ik  ) = 3
           icodcl(ifac,iomg) = 3
         endif
       elseif(iturb.eq.70) then
         if(rcodcl(ifac,inusa,1).gt.0.d0) then
           icodcl(ifac,inusa) = 1
         else
           icodcl(ifac,inusa) = 3
         endif
       endif
       if(nscaus.gt.0) then
         do ii = 1, nscaus
           if(rcodcl(ifac,isca(ii),1).gt.-rinfin*0.5d0) then
             icodcl(ifac,isca(ii)) = 1
           else
             icodcl(ifac,isca(ii)) = 3
           endif
         enddo
       endif
     endif


!     Les RCODCL ont ete initialises a -RINFIN pour permettre de
!       verifier ceux que l'utilisateur a modifies. On les remet a zero
!       si l'utilisateur ne les a pas modifies.
!       On traite la turbulence et les scalaires passifs (pour
!       simplifier la boucle, on traite toutes les variables : les
!       variables du compressible sont donc vues deux fois, mais ce
!       n'est pas grave).
     do ivar = 1, nvar
       if(rcodcl(ifac,ivar,1).le.-rinfin*0.5d0) then
         rcodcl(ifac,ivar,1) = 0.d0
       endif
     enddo


! --- Fin de test sur les faces d'entree sortie
   endif

! --- Fin de boucle sur les faces de bord
 enddo

! Free memory
deallocate(w1, w2)
deallocate(w4, w5, w6)
deallocate(w7)
deallocate(bval)

!----
! FORMATS
!----

 1000 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    Deux variables independantes et deux seulement parmi    ',/,&
'@    P, rho, T et E doivent etre imposees aux bords de type  ',/,&
'@    IESICF dans uscfcl (ICCFTH = ',I10,').                  ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier les conditions aux limites dans uscfcl.          ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 1100 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    La pression n''a pas ete fournie en sortie a pression   ',/,&
'@    impos�e.                                                ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier les conditions aux limites dans uscfcl.          ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 1200 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    La masse volumique ou la vitesse n''a pas �t� fournie   ',/,&
'@    en entree a masse volumique et vitesse imposee.         ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier les conditions aux limites dans uscfcl.          ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 1300 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    Le debit massique ou le debit enthalpique n''a pas �t�  ',/,&
'@    fourni en entree a debit massique et enthalpique impos�.',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier les conditions aux limites dans uscfcl.          ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 1301 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    Entree � debit massique et debit enthalpique non prevue ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Contacter l''equipe de developpement pour uscfcl.         ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 1400 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    Une condition a la limite ne fait pas partie des        ',/,&
'@      conditions aux limites predefinies en compressible.   ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier les conditions aux limites dans uscfcl.          ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
 7000 format(                                                           &
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/,&
'@ @@ ATTENTION : ARRET A L''EXECUTION                        ',/,&
'@    =========                                               ',/,&
'@    cfxtcl doit etre modifie pour prendre en compte une loi ',/,&
'@      d''etat a gamma variable. Seul est pris en compte le  ',/,&
'@      cas IEOS = 1                                          ',/,&
'@                                                            ',/,&
'@  Le calcul ne sera pas execute.                            ',/,&
'@                                                            ',/,&
'@  Verifier IEOS dans cfther.f90.                            ',/,&
'@                                                            ',/,&
'@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',/,&
'@                                                            ',/)
!----
! FIN
!----

return
end subroutine
