#include <stdio.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "autoclick.h"

static int virt_dev_fd;

void emit(int fd, int type, int code, int val) {
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

void ac_keyboard(char *cmdstr)
{
   printf("keyboard\n");
   emit(virt_dev_fd, EV_KEY, KEY_SPACE, 1);
   emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
   emit(virt_dev_fd, EV_KEY, KEY_SPACE, 0);
   emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
   usleep(15000);
}

void ac_mouse(char *cmdstr)
{
      int x_i,y_i;
      char btn_i;
      printf("%s",cmdstr);
      sscanf(cmdstr,"m %d,%d %c",&x_i,&y_i,&btn_i);
      printf("mouse\n");
      emit(virt_dev_fd, EV_REL, REL_X, x_i);
      emit(virt_dev_fd, EV_REL, REL_Y, y_i);
      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
      usleep(15000);
}

void ac_comment(char *cmdstr)
{
}

void ac_undefined(char *cmdstr)
{
}

AcCmdType ac_get_cmd_type(char *cmd) 
{
	if(cmd[0]=='k') return AC_KEYBOARD;
	else if(cmd[0]=='m') return AC_MOUSE;
	else if(cmd[0]=='#') return AC_COMMENT;
	else return AC_UNDEFINED;
}

void ac_exec_acs_file(char *acsfile)
{
	char *cmdstr = (char *)malloc(MAXCMDLINE);
	int cmdlen = 0;
	FILE *acs = fopen(acsfile,"r");
	if(acs == NULL) 
	{
		printf("%s not exist\n", acsfile);
		return -1;
	}
	while(getline(&cmdstr,&cmdlen,acs) != -1)
	{
		printf("%s",cmdstr);
		switch(ac_get_cmd_type((char *)cmdstr)) {
			case AC_KEYBOARD:	ac_keyboard(cmdstr); break;
			case AC_MOUSE:		ac_mouse(cmdstr); break;
			case AC_COMMENT:	ac_comment(cmdstr); break;
			case AC_UNDEFINED:	ac_undefined(cmdstr);
		}
	}
	free(cmdstr);
	fclose(acs);
}

   
void init_virt_dev()
{
   struct uinput_setup usetup;
   int i = 50;

   virt_dev_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

   /* enable mouse button left and relative events */
   ioctl(virt_dev_fd, UI_SET_EVBIT, EV_KEY);
   ioctl(virt_dev_fd, UI_SET_KEYBIT, BTN_LEFT);

   ioctl(virt_dev_fd, UI_SET_EVBIT, EV_KEY);
   ioctl(virt_dev_fd, UI_SET_KEYBIT, KEY_SPACE);

   ioctl(virt_dev_fd, UI_SET_EVBIT, EV_REL);
   ioctl(virt_dev_fd, UI_SET_RELBIT, REL_X);
   ioctl(virt_dev_fd, UI_SET_RELBIT, REL_Y);

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; /* sample vendor */
   usetup.id.product = 0x5678; /* sample product */
   strcpy(usetup.name, "Example device");

   ioctl(virt_dev_fd, UI_DEV_SETUP, &usetup);
   ioctl(virt_dev_fd, UI_DEV_CREATE);
   sleep(1);
}

void clean_virt_dev()
{
   ioctl(virt_dev_fd, UI_DEV_DESTROY);
   close(virt_dev_fd);
}

int main(void)
{
   init_virt_dev();
   ac_exec_acs_file("./test.acs");

   clean_virt_dev();
   return 0;
}

