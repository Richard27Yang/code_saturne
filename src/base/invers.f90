!-------------------------------------------------------------------------------

!     This file is part of the Code_Saturne Kernel, element of the
!     Code_Saturne CFD tool.

!     Copyright (C) 1998-2011 EDF S.A., France

!     contact: saturne-support@edf.fr

!     The Code_Saturne Kernel is free software; you can redistribute it
!     and/or modify it under the terms of the GNU General Public License
!     as published by the Free Software Foundation; either version 2 of
!     the License, or (at your option) any later version.

!     The Code_Saturne Kernel is distributed in the hope that it will be
!     useful, but WITHOUT ANY WARRANTY; without even the implied warranty
!     of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!     GNU General Public License for more details.

!     You should have received a copy of the GNU General Public License
!     along with the Code_Saturne Kernel; if not, write to the
!     Free Software Foundation, Inc.,
!     51 Franklin St, Fifth Floor,
!     Boston, MA  02110-1301  USA

!-------------------------------------------------------------------------------

subroutine invers &
!================

 ( cnom   , idbia0 , idbra0 ,                                     &
   isym   , ipol   , ireslp , nitmap , imgrp  ,                   &
   ncymxp , nitmfp ,                                              &
   iwarnp , nfecra , niterf , icycle , iinvpe ,                   &
   epsilp , rnorm  , residu ,                                     &
   ia     ,                                                       &
   dam    , xam    , smbrp  , vx     ,                            &
   w1     , w2     , w3     , w4     , w5     , w6     ,          &
   ra     )

!===============================================================================
! Purpose:
! -------

! Call linear system resolution:
! - multigrid + -conjugate gradient or Jacobi or Bi-CGstab)
! - Jacobi
! - Bi-CGstab
! we assume vx in initialized on input.

!-------------------------------------------------------------------------------
! Arguments
!__________________.____._____.________________________________________________.
! name             !type!mode ! role                                           !
!__________________!____!_____!________________________________________________!
! cnom             ! a  ! <-- ! variable name                                  !
! idbia0           ! i  ! <-- ! number of first free position in ia            !
! idbra0           ! i  ! <-- ! number of first free position in ra            !
! isym             ! e  ! <-- ! flag = 1: symmetric matrix                     !
!                  !    !     !        2: non-symmetric matrix                 !
! ipol             ! e  ! <-- ! polynomial degree for preconditioning          !
!                  !    !     !   (0 <-- diagonal)                             !
! ireslp           ! e  ! <-- ! solver type: 0 conjugate gradient              !
!                  !    !     !              1 Jacobi                          !
!                  !    !     !              2 CG-stab                         !
! nitmap           ! e  ! <-- ! max number of iterations for soultion          !
! imgrp            ! e  ! <-- ! 1 for multigrid, 0 otherwise                   !
! ncymxp           ! e  ! <-- ! max. number of multigrid cycles                !
! nitmfp           ! e  ! <-- ! number of equivalent iterations on fine mesh   !
! iwarnp           ! i  ! <-- ! verbosity                                      !
! nfecra           ! e  ! <-- ! standart output unit                           !
! niterf           ! e  ! --> ! number of iterations done (non-multigrid)      !
! icycle           ! e  ! --> ! number of multigrid cycles done                !
! iinvpe           ! e  ! <-- ! flag to cancel increments in rotational        !
!                  !    !     ! periodicity (=2) or to exchange them normally  !
!                  !    !     ! in a scalar fashion (=1)                       !
! epsilp           ! r  ! <-- ! precision for iterative resolution             !
! rnorm            ! r  ! <-- ! residue normalization                          !
! residu           ! r  ! --> ! final non-normalized residue                   !
! ia(*)            ! ia ! --- ! main integer work array                        !
! dam(ncelet       ! tr ! <-- ! diagonal (fine mesh if multigrid)              !
! xam(nfac,isym    ! tr ! <-- ! extradiagonal (fine mesh if multigrid)         !
! smbrp(ncelet     ! tr ! <-- ! right hand side (fine mesh if multigrid)       !
! vx(ncelet)       ! tr ! <-- ! system solution                                !
! w1,2,3,4,5,6     ! tr ! --- ! auxiliary arrays                               !
!   (ncelet)       !    !     !                                                !
! ra(*)            ! ra ! --- ! main real work array                           !
!__________________!____!_____!________________________________________________!

!     Type: i (integer), r (real), s (string), a (array), l (logical),
!           and composite types (ex: ra real array)
!     mode: <-- input, --> output, <-> modifies data, --- work array
!===============================================================================

!===============================================================================
! Module files
!===============================================================================

use paramx
use optcal
use mesh

!===============================================================================

implicit none

! Arguments

character*16     cnom
integer          idbia0 , idbra0
integer          isym   , ipol   , ireslp , nitmap
integer          imgrp  , ncymxp , nitmfp
integer          iwarnp , nfecra
integer          niterf , icycle , iinvpe
double precision epsilp , rnorm  , residu

integer          ia(*)

double precision dam(ncelet), xam(nfac ,2)
double precision smbrp(ncelet)
double precision vx(ncelet)
double precision w1(ncelet),w2(ncelet),w3(ncelet),w4(ncelet)
double precision w5(ncelet),w6(ncelet)
double precision ra(*)


! Local variables

integer          idebia, idebra
integer          lnom
integer          iresds, iresas, nitmds, nitmas

!===============================================================================

! INITIALISATIONS

lnom = len(cnom)

icycle = 0
niterf = 0

idebia = idbia0
idebra = idbra0

! RESOLUTION

if( imgrp.eq.1 ) then

  iresds = ireslp
  iresas = ireslp

  nitmds = nitmfp
  nitmas = nitmfp

  call resmgr                                                     &
  !==========
 ( cnom   , lnom   , ncelet , ncel   , nfac   ,                   &
   isym   , iresds , iresas , ireslp , ipol   ,                   &
   ncymxp , nitmds , nitmas , nitmap , iinvpe ,                   &
   iwarnp , icycle , niterf , epsilp , rnorm  , residu ,          &
   ifacel , smbrp  , vx     )

elseif(imgrp.eq.0) then

  if (ireslp.ge.0 .and. ireslp.le. 2) then

    call reslin                                                   &
    !==========
 ( cnom   , lnom   , ncelet , ncel   , nfac   ,                   &
   isym   , ireslp , ipol   , nitmap , iinvpe ,                   &
   iwarnp , niterf , epsilp , rnorm  , residu ,                   &
   !        ------                     ------
   ifacel , dam    , xam    , smbrp  , vx     )
    !                         -----

  else
    write(nfecra,1000) cnom, ireslp
    call csexit (1)
  endif

endif


#if defined(_CS_LANG_FR)

 1000 format('INVERS APPELE POUR ',A8,' AVEC IRESOL = ', I10)

#else

 1000 format('INVERS CALLED FOR ',A8,' WITH IRESOL = ', I10)

#endif

!----
! End
!----

return

end subroutine
