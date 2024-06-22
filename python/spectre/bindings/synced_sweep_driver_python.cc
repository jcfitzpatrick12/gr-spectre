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
/* BINDTOOL_HEADER_FILE(synced_sweep_driver.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(361f626f8a14f49bc2412aca4fc86ce2)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/spectre/synced_sweep_driver.h>
// pydoc.h is automatically generated in the build directory
#include <synced_sweep_driver_pydoc.h>

void bind_synced_sweep_driver(py::module& m)
{

    using synced_sweep_driver    = gr::spectre::synced_sweep_driver;


    py::class_<synced_sweep_driver, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<synced_sweep_driver>>(m, "synced_sweep_driver", D(synced_sweep_driver))

        .def(py::init(&synced_sweep_driver::make),
           D(synced_sweep_driver,make)
        )
        



        ;




}







