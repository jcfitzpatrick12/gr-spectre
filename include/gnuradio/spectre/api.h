/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_API_H
#define INCLUDED_SPECTRE_API_H

#include <gnuradio/attributes.h>

#ifdef gnuradio_spectre_EXPORTS
#define SPECTRE_API __GR_ATTR_EXPORT
#else
#define SPECTRE_API __GR_ATTR_IMPORT
#endif

#endif /* INCLUDED_SPECTRE_API_H */
