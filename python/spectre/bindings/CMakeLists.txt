# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
)
include(GrPybind)

########################################################################
# Python Bindings
########################################################################
list(APPEND spectre_python_files
    batched_file_sink_python.cc
    sweep_driver_python.cc
    tagged_staircase_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(spectre
   ../../..
   gr::spectre
   "${spectre_python_files}")

install(TARGETS spectre_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/spectre COMPONENT pythonapi)
