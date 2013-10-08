/*
 * fstab_observer.h
 *
 *  Created on: 03.12.2012
 *      Author: yaroslav
 */

#ifndef FSTAB_OBSERVER_H_
#define FSTAB_OBSERVER_H_

#include "nvram_observer.h"

#define FSTAB_SECTION_NAME         "fstab"
#define FSTAB_PARAM_CHANNEL_KEY    "channel"
#define FSTAB_PARAM_MOUNTPOINT_KEY "mountpoint"
#define FSTAB_PARAM_ACCESS_KEY     "access"
#define FSTAB_PARAM_WARMUP         "warmup"

#define FSTAB_VAL_ACCESS_READ      "ro"  /*for injecting files into FS*/
#define FSTAB_VAL_ACCESS_WRITE     "wo"  /*for copying files into image*/

#define FSTAB_VAL_WARMUP_YES       "yes"
#define FSTAB_VAL_WARMUP_NO        "no"

#define IS_NEED_TO_HANDLE_FSTAB_RECORD(warmup_stage_complete, warmup_record) \
    !(warmup_stage_complete) && (warmup_record)? 1:			\
    (warmup_stage_complete) && !(warmup_record)? 1 : 0

/*get static interface object not intended to destroy after using*/
struct MNvramObserver* get_fstab_observer();
void handle_tar_export();

#endif /* FSTAB_OBSERVER_H_ */
