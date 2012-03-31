#include "../init.h"
#include "lowlib.h"

W
posix_ls (W ac, B **av)
{
  W			fd;
  static struct stat	st;


  fd = open (av[1], O_RDONLY, 0);
  if (fd < 0)
    {
      printf ("cannot file open.\n");
      return (fd);
    }

  if (fstat (fd, &st))
    {
      printf ("cannot stat for file.\n");
      return (fd);
    }

  if (st.st_size < 0)
    {
      printf ("file size is 0.\n");
      return (0);
    }

  if (st.st_mode & FS_FMT_REG)
    {
      printf ("%s %d/%d %d bytes\n", av[1], st.st_uid, st.st_gid, st.st_size);
    }
  else if (st.st_mode & FS_FMT_DIR)
    {
      
    }

  close (fd);

  return E_OK;
}


ER
posix_cat (W ac, B **av)
{
  W			fd;
  static struct stat	st;
  static B		buf[1024];
  W			length, rlength;
  W			i;
  W			offset = 0;


  fd = open (av[1], O_RDONLY, 0);
  if (fd < 0)
    {
      printf ("cannot file open.\n");
      return (fd);
    }

  if (fstat (fd, &st))
    {
      printf ("cannot stat for file.\n");
      return (fd);
    }

  if (st.st_size < 0)
    {
      printf ("file size is 0.\n");
      return (0);
    }

  if (ac > 2)
    {
      offset = atoi (av[2]);
      if (offset > st.st_size)
	{
	  printf ("offset overflow\n");
	  return (E_PAR);
	}

      printf ("lseek: offset = %d, mode = %d\n", offset, 0);
      if (lseek (fd, offset, 0))
	{
	  printf ("cannot lseek.\n");
	  return (E_SYS);
	}
      printf ("lseek success.\n");
    }

  printf ("file size is = %d\n", st.st_size);
  for (rlength = st.st_size - offset; rlength > 0; rlength -= length)
    {
      printf ("read: %d, 0x%x, %d\n", fd, buf,sizeof (buf));
      if ((length = read (fd, buf, sizeof (buf))) <= 0)
	{
	  break;
	}

      printf ("read size = %d\n", length);
      for (i = 0; i < length; i++)
	{
	  putc (buf[i], stdout);
	}
    }

  printf ("\nread done\n");
  close (fd);

  return E_OK;
}

ER
posix_writefile (W ac, B **av)
{
  W	fd;

  fd = open (av[1], O_CREAT|O_RDWR, 0666);
  if (fd < 0)
    {
      printf ("cannot open file. %s\n", av[1]);
      return E_SYS;
    }

  if (write (fd, av[2], strlen(av[2])) < 0)
    {
      return E_SYS;
    }
  
  close(fd);
  return E_OK;
}
