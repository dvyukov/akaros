/* Copyright (C) 1991, 1995, 1996, 1997, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <ros/syscall.h>
#include "ros_stat.h"

/* Get file information about FILE in BUF.  */
int
__lxstat (int vers, const char *file, struct stat *buf)
{
  if (vers != _STAT_VER || file == NULL || buf == NULL)
  {
    __set_errno (EINVAL);
    return -1;
  }

  struct ros_stat nst;
  int ret = (int)ros_syscall(SYS_lstat, file, strlen(file), &nst, 0, 0);
  __convert_stat(&nst,buf);
  return ret;
}
hidden_def (__lxstat)
weak_alias (__lxstat, _lxstat)
