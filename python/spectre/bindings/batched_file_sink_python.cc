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
/* BINDTOOL_HEADER_FILE(batched_file_sink.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(94132f2471725579b421d4697a5d38e3)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/spectre/batched_file_sink.h>
// pydoc.h is automatically generated in the build directory
#include <batched_file_sink_pydoc.h>

void bind_batched_file_sink(py::module& m)
{

    using batched_file_sink    = ::gr::spectre::batched_file_sink;


    py::class_<batched_file_sink, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<batched_file_sink>>(m, "batched_file_sink", D(batched_file_sink))

        .def(py::init(&batched_file_sink::make),
           py::arg("parent_dir_path") = "./",
           py::arg("tag") = "my-tag",
           py::arg("batch_size") = 3.,
           py::arg("samp_rate") = 32000,
           py::arg("is_sweeping") = false,
           py::arg("frequency_tag_key") = "freq",
           py::arg("initial_center_frequency") = 0,
           D(batched_file_sink,make)
        )
        



        ;




}








