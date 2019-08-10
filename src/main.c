/*
 * \file: main.c
 * \brief: Created by hushouguo at 15:30:03 Jul 26 2019
 */

#include "slam.h"

int main(int argc, char** argv) {
	if (!slam_main_init(argc, argv)) {
	    return SLAM_PANIC;
	}
	slam_main_run();
	slam_main_delete();
	Debug("Bye!");
	return 0;
}

