/*
 * libiio - Library for interfacing industrial I/O (IIO) devices
 *
 * Copyright (C) 2014 Analog Devices, Inc.
 * Author: Paul Cercueil <paul.cercueil@analog.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * */

#include "iio-private.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || (defined(__USE_XOPEN2K8) && \
		(!defined(__UCLIBC__) || defined(__UCLIBC_HAS_LOCALE__)))
#define LOCALE_SUPPORT
#endif

#ifdef LOCALE_SUPPORT
#ifdef _WIN32
static int read_double_locale(const char *str, double *val)
{
	char *end;
	double value;
	_locale_t locale = _create_locale(LC_NUMERIC, "POSIX");

	value = _strtod_l(str, &end, locale);
	_free_locale(locale);

	if (end == str)
		return -EINVAL;

	*val = value;
	return 0;
}

static void write_double_locale(char *buf, size_t len, double val)
{
	_locale_t locale = _create_locale(LC_NUMERIC, "POSIX");

	_snprintf_l(buf, len, "%lf", locale, val);
	_free_locale(locale);
}
#else
static int read_double_locale(const char *str, double *val)
{
	char *end;
	double value;
	locale_t old_locale, new_locale;

	new_locale = newlocale(LC_NUMERIC_MASK, "POSIX", (locale_t) 0);
	old_locale = uselocale(new_locale);

	value = strtod(str, &end);
	uselocale(old_locale);
	freelocale(new_locale);

	if (end == str)
		return -EINVAL;

	*val = value;
	return 0;
}

static void write_double_locale(char *buf, size_t len, double val)
{
	locale_t old_locale, new_locale;

	new_locale = newlocale(LC_NUMERIC_MASK, "POSIX", (locale_t) 0);
	old_locale = uselocale(new_locale);

	snprintf(buf, len, "%lf", val);

	uselocale(old_locale);
	freelocale(new_locale);
}
#endif
#endif

int read_double(const char *str, double *val)
{
#ifdef LOCALE_SUPPORT
	return read_double_locale(str, val);
#else
	char *end;
	double value = strtod(str, &end);

	if (end == str)
		return -EINVAL;

	*val = value;
	return 0;
#endif
}

void write_double(char *buf, size_t len, double val)
{
#ifdef LOCALE_SUPPORT
	write_double_locale(buf, len, val);
#else
	snprintf(buf, len, "%lf", val);
#endif
}

void iio_library_get_version(unsigned int *major,
		unsigned int *minor, char git_tag[8])
{
	if (major)
		*major = LIBIIO_VERSION_MAJOR;
	if (minor)
		*minor = LIBIIO_VERSION_MINOR;
	if (git_tag) {
		strncpy(git_tag, LIBIIO_VERSION_GIT, 8);
		git_tag[7] = '\0';
	}
}
