## Process this file with automake to produce Makefile.in

#-------------------------------------------------------------------------------

# This file is part of Code_Saturne, a general-purpose CFD tool.
#
# Copyright (C) 1998-2018 EDF S.A.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
# Street, Fifth Floor, Boston, MA 02110-1301, USA.

#-------------------------------------------------------------------------------

include $(top_srcdir)/config/salome_module.mk

MODULE_NAME = FSI

AM_CFLAGS = $(SALOME_INCLUDES) -fexceptions

salomelib_LTLIBRARIES = libFSI_MILIEUEngine.la
salomeinclude_HEADERS = FSI_MILIEU.hxx

libFSI_MILIEUEngine_la_SOURCES = FSI_MILIEU.cxx 
nodist_libFSI_MILIEUEngine_la_SOURCES =
libFSI_MILIEUEngine_la_CXXFLAGS = -I$(top_builddir)/salome/fsi_coupling/idl  $(SALOME_INCLUDES) -I$(top_srcdir)/salome/fsi_coupling/libmilieu
libFSI_MILIEUEngine_la_LIBADD  = $(top_builddir)/salome/fsi_coupling/libmilieu/libmilieu.la $(top_builddir)/salome/fsi_coupling/idl/libSalomeIDLFSI.la $(SALOME_LIBS)
libFSI_MILIEUEngine_la_LDFLAGS = 

