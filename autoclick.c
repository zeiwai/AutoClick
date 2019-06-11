#include <stdio.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "autoclick.h"

static int virt_dev_fd;
static int keymap[MAXKEY];

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
   char key_c;
   //printf("*keyboard: %s\n",cmdstr);
   sscanf(cmdstr,"k %c", &key_c);
   emit(virt_dev_fd, EV_KEY, keymap[(int)key_c], 1);
   emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
   emit(virt_dev_fd, EV_KEY, keymap[(int)key_c], 0);
   emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
   usleep(15000);
}

void ac_mouse(char *cmdstr)
{
      int x_i,y_i;
      char btn_c;
      //printf("*mouse: %s\n",cmdstr);
      sscanf(cmdstr,"m %d,%d %c",&x_i,&y_i,&btn_c);
      emit(virt_dev_fd, EV_REL, REL_X, x_i);
      emit(virt_dev_fd, EV_REL, REL_Y, y_i);
      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
      switch(btn_c)  {
	case 'l' :
	      emit(virt_dev_fd, EV_KEY, BTN_LEFT, 1);
	      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
	      emit(virt_dev_fd, EV_KEY, BTN_LEFT, 0);
	      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
	      break;
	case 'r' :
	      emit(virt_dev_fd, EV_KEY, BTN_RIGHT, 1);
	      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
	      emit(virt_dev_fd, EV_KEY, BTN_RIGHT, 0);
	      emit(virt_dev_fd, EV_SYN, SYN_REPORT, 0);
	      break;
	default :
	      break;
      }
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

   ioctl(virt_dev_fd, UI_SET_EVBIT, EV_KEY);

   /* enable mouse button events */
   for (int keycode=0x110; keycode <= 0x117; keycode++)
	   ioctl(virt_dev_fd, UI_SET_KEYBIT, keycode);

   /* enable keyboard key events */
   for (int keycode=1; keycode <= 248; keycode++)
	   ioctl(virt_dev_fd, UI_SET_KEYBIT, keycode);

   ioctl(virt_dev_fd, UI_SET_EVBIT, EV_REL);
   for (int keycode=0x00; keycode <= 0x09; keycode++)
	   ioctl(virt_dev_fd, UI_SET_RELBIT, keycode);

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; 
   usetup.id.product = 0x5678; 
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

   // initial keymap for search.
   for (int i=0;i<72;i+=2) {
	   keymap[(int)KEYMAP[i]]=(int)KEYMAP[i+1];
   }

   ac_exec_acs_file("./test.acs");

   clean_virt_dev();
   return 0;
}

