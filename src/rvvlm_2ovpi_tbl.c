// SPDX-FileCopyrightText: 2023 Rivos Inc.
//
// SPDX-License-Identifier: Apache-2.0

// This table is used my different functions of the log and exponential family
#include <stdint.h>

// This is 2^500 * (2/pi) and the lsb of dbl_2ovpi_tbl[j] is 2^(500-(j+1)*52),
// j=0,1,...,27
const extern double dbl_2ovpi_tbl[28] = {
    0x1.45f306dc9c882p+499,  0x1.4a7f09d5f47d4p+446,  0x1.a6ee06db14ad0p+393,
    -0x1.b0ef1bef806bcp+342, 0x1.8eaf7aef1586cp+290,  0x1.c91b8e909374cp+238,
    -0x1.ff9b6d115f630p+184, 0x1.921cfe1deb1d0p+132,  -0x1.3b5963045df74p+82,
    0x1.7d4baed1213a8p+30,   -0x1.8e3f652e82070p-22,  0x1.3991d63983530p-76,
    0x1.cfa4e422fc5e0p-127,  -0x1.036be27003b40p-179, -0x1.0fd33f8086800p-239,
    -0x1.dce94beb25c20p-285, 0x1.b4d9fb3c9f2c4p-334,  -0x1.922c2e7026588p-386,
    0x1.7fa8b5d49eeb0p-438,  0x1.faf97c5ecf41cp-490,  0x1.cfbc529497538p-543,
    -0x1.012813b81ca8cp-594, 0x1.0ac06608df900p-649,  -0x1.251503cc10f7cp-698,
    -0x1.942f27895871cp-750, 0x1.615ee61b08660p-804,  -0x1.99ea83ad7e5f0p-854,
    0x1.1bffb1009ae60p-909};
