/*
 *  Testing AlsaHw Iotcl from userspace
 *  Author: fulup@iot.bzh

 * Usage:
 *  Compile: gcc -g alsa-ctl-rename2.c -lasound -o alsa-ctl-rename2
 *  Syntaxe: ./alsa-ctl-rename2 /dev/sndcontrol? NUMDID "New Ctls Name"
 *  Example: ./alsa-ctl-rename2 /dev/snd/controlC4 4 "My Master Volume"
 *  Test   : amixer -D "hw:4" cget numid=4

 * Note:
 *   this test try access directly snd/controlC by lowlevel IOCT
 *   in order to execute a replace that is normally not accessible
 *   libasound applications.

 * References:
 *  Alsa-lib
 *   ./include/sound/asound.h
 *   ./src/control/control_hw.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

#define SNDRV_CTL_IOCTL_ELEM_INFO(len)    _IOC((_IOC_READ|_IOC_WRITE), 'U', 0x11, len)
#define SNDRV_CTL_IOCTL_ELEM_ADD(len)     _IOC((_IOC_READ|_IOC_WRITE), 'U', 0x17, len)
#define SNDRV_CTL_IOCTL_ELEM_REPLACE(len) _IOC((_IOC_READ|_IOC_WRITE), 'U', 0x18, len)


main (int argc, char **argv) {
   int infolen = snd_ctl_elem_info_sizeof(); 	
   snd_ctl_elem_info_t *ctlInfo;
   int numid, fd, count, err;

   if (argc != 4) {
      fprintf (stderr, "syntaxe %s /dev/snd/control? numid NewCtlName\n", argv[0]);
      goto OnErrorExit;
   }   

   fd= open(argv[1],0);
   if (fd < 0) {
      fprintf (stderr, "ERROR fail to open control file=[%s]\n", argv[1]);
      goto OnErrorExit;     
   }


   count = sscanf (argv[2], "%d", &numid);
   if (count != 1) {
      fprintf (stderr, "ERROR numid is be a valid integer=[%s]\n", argv[2]);
      goto OnErrorExit;
   }

   // allocate infostructure on stack
   snd_ctl_elem_info_alloca(&ctlInfo);
   snd_ctl_elem_info_set_numid(ctlInfo, numid);

   err = ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO(infolen), ctlInfo);
   if (err < 0) {
     fprintf (stderr, "ERROR numid=[%d] Not Found err=%d\n", numid, err);
     goto OnErrorExit;
   }
   fprintf (stdout, "numid=[%d] oldname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));
   
   snd_ctl_elem_info_set_name(ctlInfo, argv[3]);
   err = ioctl(fd, SNDRV_CTL_IOCTL_ELEM_REPLACE(infolen), ctlInfo);
   if (err < 0) {
    fprintf (stderr, "ERROR fail to to replace sound control err=%d\n", numid, err);
    goto OnErrorExit;
   }
   fprintf (stdout, "numid=[%d] askname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));

   if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO(infolen), ctlInfo) < 0) goto OnErrorExit;
   fprintf (stdout, "numid=[%d] setname=[%s]\n", snd_ctl_elem_info_get_numid(ctlInfo), snd_ctl_elem_info_get_name(ctlInfo));

   // happy exit
   exit(0);
   
   OnErrorExit:
      fprintf (stderr, "OnErrorExit error=[%s]\n", snd_strerror(errno));      
      exit(-1);
}


