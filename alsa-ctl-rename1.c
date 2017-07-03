/*
 *  Testing AlsaHw Iotcl from userspace
 *  Author: fulup@iot.bzh

 * Usage:
 *  Compile: gcc -g alsa-ctl-rename1.c -lasound -o alsa-ctl-rename1
 *  Syntaxe: ./alsa-ctl-rename1 hw:? NUMDID "New Ctls Name"
 *  Example: ./alsa-ctl-rename1 hw:4 4 "My Master Volume"
 *  Test   : amixer -D "hw:4" cget numid=4

 * Note:
 *   this test try access private asound structure to execute
 *   low level control function and access rename.

 * References:
 *  Alsa-lib
 *   ./include/sound/asound.h
 *   ./src/control/control_hw.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

typedef struct _snd_ctl_ops {
        int (*close)(snd_ctl_t *handle);
        int (*nonblock)(snd_ctl_t *handle, int nonblock);
        int (*async)(snd_ctl_t *handle, int sig, pid_t pid);
        int (*subscribe_events)(snd_ctl_t *handle, int subscribe);
        int (*card_info)(snd_ctl_t *handle, snd_ctl_card_info_t *info);
        int (*element_list)(snd_ctl_t *handle, snd_ctl_elem_list_t *list);
        int (*element_info)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
        int (*element_add)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
        int (*element_replace)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
        int (*element_remove)(snd_ctl_t *handle, snd_ctl_elem_id_t *id);
        int (*element_read)(snd_ctl_t *handle, snd_ctl_elem_value_t *control);
        int (*element_write)(snd_ctl_t *handle, snd_ctl_elem_value_t *control);
        int (*element_lock)(snd_ctl_t *handle, snd_ctl_elem_id_t *lock);
        int (*element_unlock)(snd_ctl_t *handle, snd_ctl_elem_id_t *unlock);
        int (*element_tlv)(snd_ctl_t *handle, int op_flag, unsigned int numid,
                           unsigned int *tlv, unsigned int tlv_size);
        int (*hwdep_next_device)(snd_ctl_t *handle, int *device);
        int (*hwdep_info)(snd_ctl_t *handle, snd_hwdep_info_t * info);
        int (*pcm_next_device)(snd_ctl_t *handle, int *device);
        int (*pcm_info)(snd_ctl_t *handle, snd_pcm_info_t * info);
        int (*pcm_prefer_subdevice)(snd_ctl_t *handle, int subdev);
        int (*rawmidi_next_device)(snd_ctl_t *handle, int *device);
        int (*rawmidi_info)(snd_ctl_t *handle, snd_rawmidi_info_t * info);
        int (*rawmidi_prefer_subdevice)(snd_ctl_t *handle, int subdev);
        int (*set_power_state)(snd_ctl_t *handle, unsigned int state);
        int (*get_power_state)(snd_ctl_t *handle, unsigned int *state);
        int (*read)(snd_ctl_t *handle, snd_ctl_event_t *event);
        int (*poll_descriptors_count)(snd_ctl_t *handle);
        int (*poll_descriptors)(snd_ctl_t *handle, struct pollfd *pfds, unsigned int space);
        int (*poll_revents)(snd_ctl_t *handle, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
} snd_ctl_ops_t;

typedef struct private_snd_ctl {
        void *open_func;
        char *name;
        snd_ctl_type_t type;
        const snd_ctl_ops_t *ops;
        void *private_data;
        int nonblock;
        int poll_fd;
        void *async_handlers;
} private_snd_ctl_t;

typedef struct {
        int card;
        int fd;
        unsigned int protocol;
} snd_ctl_hw_t;

main (int argc, char **argv) {
   int infolen = snd_ctl_elem_info_sizeof(); 	
   snd_ctl_elem_info_t *ctlInfo;
   int numid, count, err;
   snd_ctl_t  *ctlDev;

   if (argc != 4) {
      fprintf (stderr, "syntaxe %s hw:? numid NewCtlName\n", argv[0]);
      goto OnErrorExit;
    }   

    // open control interface for devid
    err = snd_ctl_open(&ctlDev, argv[1], 0);
    if (err < 0) {
      fprintf (stderr, "ERROR fail to open control file=[%s]\n", argv[1]);
      goto OnErrorExit;
    }

    // force access to private cound control handle
    private_snd_ctl_t *handle= (private_snd_ctl_t*) ctlDev;
    snd_ctl_hw_t *hw = handle->private_data;

    count = sscanf (argv[2], "%d", &numid);
    if (count != 1) {
      fprintf (stderr, "ERROR numid is be a valid integer=[%s]\n", argv[2]);
      goto OnErrorExit;
    }

    // allocate infostructure on stack
    snd_ctl_elem_info_alloca(&ctlInfo);
    snd_ctl_elem_info_set_numid(ctlInfo, numid);

    err= handle->ops->element_info (ctlDev, ctlInfo);   // equivalent to err = snd_ctl_elem_info(ctlDev, ctlInfo);
    if (err < 0) {
      fprintf (stderr, "ERROR numid=[%d] not found err=%d\n", numid, err);
      goto OnErrorExit;
    }
    fprintf (stdout, "numid=[%d] oldname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));
   
    snd_ctl_elem_info_set_name(ctlInfo, argv[3]);
    err= handle->ops->element_replace (ctlDev, ctlInfo);
    if (err < 0) {
      fprintf (stderr, "ERROR fail to to replace sound control err=%d\n", numid, err);
      goto OnErrorExit;
    }
    fprintf (stdout, "numid=[%d] askname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));

    err= handle->ops->element_info (ctlDev, ctlInfo); 
    fprintf (stdout, "numid=[%d] setname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));

    // happy exit
    exit(0);
   
  OnErrorExit:
    fprintf (stderr, "OnErrorExit error=[%s]\n", strerror (errno));      
    exit(-1);
}


