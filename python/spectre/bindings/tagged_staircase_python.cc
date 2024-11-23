/*
 * Copyright 2024 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(tagged_staircase.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(b3efc11428318a9fc7adc96f0f7d363e)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/spectre/tagged_staircase.h>
// pydoc.h is automatically generated in the build directory
#include <tagged_staircase_pydoc.h>

void bind_tagged_staircase(py::module& m)
{

    using tagged_staircase    = ::gr::spectre::tagged_staircase;


    py::class_<tagged_staircase, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<tagged_staircase>>(m, "tagged_staircase", D(tagged_staircase))

        .def(py::init(&tagged_staircase::make),
           py::arg("min_samples_per_step") = 4000,
           py::arg("max_samples_per_step") = 5000,
           py::arg("freq_step") = 32000,
           py::arg("step_increment") = 200,
           py::arg("samp_rate") = 32000,
           D(tagged_staircase,make)
        )
        



        ;




}








