/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */
#ifndef __UKGCOV_H__
#define __UKGCOV_H__

static const size_t INITIAL_BUFFER_SIZE = 100 * (1 << 10); // 100 KB

/* Dump the gcov information to whatever output is configured in KConfig */
int gcov_dump_info(void);

#endif /* __UKGCOV_H__ */
