/*
 * \file: slam_utils.c
 * \brief: Created by hushouguo at 16:51:19 Jul 25 2019
 */

#include "slam.h"

/* check that a floating point number is integer */
bool slam_is_integer(double value) {
	return value == (int64_t) value;
}

