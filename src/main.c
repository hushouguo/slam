/*
 * \file: main.c
 * \brief: Created by hushouguo at 15:30:03 Jul 26 2019
 */

#include "slam.h"

int main() {
	slam_runnable_t* runnable = slam_runnable_new("./main.lua");
	printf("runnable: %p\n", runnable);
	if (runnable) {
		slam_runnable_delete(runnable);
	}
	return 0;
}
