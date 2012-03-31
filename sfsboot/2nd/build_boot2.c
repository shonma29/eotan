#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MODE32_SIZE 0x0ffc 

#define INIT_SIZE  4096

/****************************************************************************
 *	main ---
 *
 *	引数	ac
 *		av[0] .... コマンド名
 *		av[1] .... init(初期化部分)
 *		av[2] .... mode32(32bit動作部分)
 *              av[3] .... 2ndboot 
 */

int
main (int argc, char **argv)
{
  int fd_init;
  char init_buf[INIT_SIZE];
  int init_size;
  struct stat stat_init;

  int fd_mode32;
  int	mode32_size;
  struct stat stat_mode32;

  int count;
  int fd_2ndboot;
  
  if(argc != 4) {
    fprintf(stderr, "usage : %s init  mode32 2ndboot\n", argv[0]);
    exit(1);
  }

  fd_2ndboot = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd_2ndboot == -1) {
    fprintf(stderr, "cannot create %s for 2ndboot\n", argv[3]);
    exit(1);
  }

  /*******************************************************************
   *  init(start16+start32)を4096byteに整形し、2ndbootに書き出す
   */

  fd_init = open(argv[1], O_RDONLY);
  if(fd_init == -1) {
    fprintf (stderr, "cannot open %s for init\n", argv[1]);
    exit(1);
  }

  fstat(fd_init, &stat_init);
  if(stat_init.st_size > INIT_SIZE) {
    fprintf(stderr, "init_file is too large. %dByte\n", stat_init.st_size);
    exit(1);
  }

  bzero(init_buf, INIT_SIZE);
  init_size = read(fd_init, init_buf, INIT_SIZE); 
  if(init_size == -1) {
    fprintf(stderr, "cannot read %s for init\n", argv[1]);
    exit(1);
  }
  
  write(fd_2ndboot, init_buf, INIT_SIZE);
  close(fd_init);
  
  /*******************************************************************
   *  mode32を2ndbootに書き出す
   */

  fd_mode32 = open(argv[2], O_RDONLY);
  if(fd_mode32 == -1) {
    fprintf (stderr, "can't open %s for mode32\n", argv[2]);
    exit(1);
  }

  fstat(fd_mode32, &stat_mode32);
  mode32_size = stat_mode32.st_size;

  while((count = read (fd_mode32, init_buf, INIT_SIZE)) > 0) {
    write(fd_2ndboot, init_buf, count);
  }
  close(fd_mode32);

  /*******************************************************************
   *  mode32のサイズを、2ndbootに書き込む
   */

  lseek(fd_2ndboot, MODE32_SIZE, SEEK_SET);
  write(fd_2ndboot, &mode32_size, sizeof(mode32_size));
  close(fd_2ndboot);

  fprintf(stderr, "done\n");
  return(0);
}
