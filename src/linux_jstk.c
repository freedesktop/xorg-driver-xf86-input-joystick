/*
 * Copyright 2007      by Sascha Hlusiak. <saschahlusiak@freedesktop.org>     
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Sascha   Hlusiak  not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Sascha   Hlusiak   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * SASCHA  HLUSIAK  DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL SASCHA  HLUSIAK  BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <linux/joystick.h>
#include <fcntl.h>
#include <errno.h>

#include "linux_jstk.h"
#include "xf86Jstk.h"


/***********************************************************************
 *
 * xf86JoystickOn --
 *
 * Open and initialize a joystick device
 *
 ***********************************************************************
 */

int
xf86JoystickOn(JoystickDevPtr joystick,int init)
{
  char joy_name[128];

  if ((joystick->fd = open(joystick->device, O_RDWR | O_NDELAY, 0)) < 0) {
    xf86Msg(X_ERROR, "Cannot open joystick '%s' (%s)\n", joystick->device,
            strerror(errno));
    return -1;
  }

/*  ioctl(fd, JSIOCGVERSION, &version);*/
  if (ioctl(joystick->fd, JSIOCGAXES, &joystick->axes)==-1) {
    xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed (%s)\n", joystick->device,
            strerror(errno));
    return -1;
  }
  if (ioctl(joystick->fd, JSIOCGBUTTONS, &joystick->buttons)==-1) {
    xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed (%s)\n", joystick->device,
            strerror(errno));
    return -1;
  }
  if (init != 0) {
    if (ioctl(joystick->fd, JSIOCGNAME(128), joy_name)==-1) {
      xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed (%s)\n", joystick->device,
              strerror(errno));
      return -1;
    }

    xf86Msg(X_INFO, "Joystick: %s. %d buttons, %d axes\n", 
      joy_name, joystick->axes, joystick->buttons);
  }

  return joystick->fd;
}

/***********************************************************************
 *
 * xf86JoystickOff --
 *
 * close the handle.
 *
 ***********************************************************************
 */

void
xf86JoystickOff(JoystickDevPtr joystick)
{
  if ((joystick->fd >= 0)) {
    close(joystick->fd);
    joystick->fd = -1;
  }
}


/***********************************************************************
 *
 * xf86ReadJoystickData --
 *
 * Reads data from fd and stores it in the JoystickDevRec struct
 * return 1 if success, 0 otherwise
 *
 ***********************************************************************
 */

int
xf86ReadJoystickData(JoystickDevPtr joystick)
{
  struct js_event js;
  if (xf86ReadSerial(joystick->fd, &js, sizeof(struct js_event)) != sizeof(struct js_event))
    return 0;

  switch(js.type & ~JS_EVENT_INIT) {
    case JS_EVENT_BUTTON:
      if (js.number<32)
        joystick->button[js.number].pressed=js.value;
      break;
    case JS_EVENT_AXIS:
      if (js.number<32) {
        joystick->axis[js.number].value=js.value;
        if (abs(js.value)<joystick->axis[js.number].deadzone) {
          joystick->axis[js.number].value=0;
        }
      }
      break;
  }

  return 1;
}

/***********************************************************************
 *
 * xf86JoystickGetState --
 *
 * return the state of buttons and the position of the joystick.
 *
 ***********************************************************************
 */

// int
// xf86JoystickGetState(int fd, int *x, int *y, int *buttons)
// {
//   struct js_status      js;
//   int                   status;
//   
//   status = read(fd, &js, JS_RETURN);
//  
//   if (status != JS_RETURN)
//     {
//       Error("Joystick read");      
//       return 0;
//     }
//   
//   *x = js.x;
//   *y = js.y;
//   *buttons = js.buttons;
//   
//   return 1;
// }

