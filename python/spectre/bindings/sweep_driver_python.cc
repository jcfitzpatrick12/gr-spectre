/*
 * Copyright 2025 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(sweep_driver.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(f8deac244aefb0f65690db71d9619b6c)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/spectre/sweep_driver.h>
// pydoc.h is automatically generated in the build directory
#include <sweep_driver_pydoc.h>

void bind_sweep_driver(py::module& m)
{

    using sweep_driver    = ::gr::spectre::sweep_driver;


    py::class_<sweep_driver, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<sweep_driver>>(m, "sweep_driver", D(sweep_driver))

        .def(py::init(&sweep_driver::make),
           py::arg("min_freq") = 80000000,
           py::arg("max_freq") = 120000000,
           py::arg("freq_step") = 6000000,
           py::arg("samp_rate") = 6000000,
           py::arg("samples_per_step") = 350000,
           py::arg("retune_cmd_name") = "freq",
           D(sweep_driver,make)
        )
        



        ;




}








