/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/utsname.h>
#include "cunit.h"

extern void *malloc(size_t);
extern void free(void*);


char *testfile()
{
	int fd;
	struct stat st;
	struct stat st2;
	char buf[1];
	char buf2[1];
	struct utimbuf sb;

	fd = open("/initrd.mk", O_RDWR);
	assert_ne("open[0]", -1, fd);

	assert_eq("fstat", 0, fstat(fd, &st));

	printf("fstat: dev=%x\n", st.st_dev);
	printf("fstat: ino=%d\n", st.st_ino);
	printf("fstat: mode=%x\n", st.st_mode);
	printf("fstat: nlink=%d\n", st.st_nlink);
	printf("fstat: uid=%d\n", st.st_uid);
	printf("fstat: gid=%d\n", st.st_gid);
	printf("fstat: rdev=%x\n", st.st_rdev);
	printf("fstat: size=%d\n", st.st_size);
	printf("fstat: blksize=%d\n", st.st_blksize);
	printf("fstat: blocks=%d\n", st.st_blocks);
	printf("fstat: atime=%d, %d\n",
		(int)((long long int)st.st_atime >> 32),
		(int)((long long int)st.st_atime & 0xffffffff));
	printf("fstat: mtime=%d, %d\n",
		(int)((long long int)st.st_mtime >> 32),
		(int)((long long int)st.st_mtime & 0xffffffff));
	printf("fstat: ctime=%d, %d\n",
		(int)((long long int)st.st_ctime >> 32),
		(int)((long long int)st.st_ctime & 0xffffffff));

	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 0x23, buf[0]);

	assert_eq("read[1]", 1, read(fd, buf, 1));
	assert_eq("read[1] buf[0]", 0x20, buf[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	assert_eq("read[1-2]", 1, read(fd, buf2, 1));
	assert_eq("read[1-2] buf[0]", buf[0], buf2[0]);
	assert_eq("read[1-2] cmp", buf[0], buf2[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	buf[0] = 0x41;
	assert_eq("write[1]", 1, write(fd, buf, 1));

	assert_eq("close[0]", 0, close(fd));

	assert_eq("chown[0]", 0, chown("/initrd.mk", 147, 258));
	assert_eq("chmod[0]", 0, chmod("/initrd.mk", S_IXGRP | S_IROTH | S_IWOTH));

	sb.actime = 25;
	sb.modtime = 7081;
	assert_eq("utime[0]", 0, utime("/initrd.mk", &sb));

	fd = open("/initrd.mk", O_RDONLY);
	assert_ne("open[1]", -1, fd);

	assert_eq("fstat[1]", 0, fstat(fd, &st2));
	assert_eq("chown[0-1]", 147, st2.st_uid);
	assert_eq("chown[0-2]", 258, st2.st_gid);

	printf("fstat: mode=%x\n", st2.st_mode);
	assert_eq("chmod[0-1]",
			S_IFREG | S_IXGRP | S_IROTH | S_IWOTH,
			st2.st_mode);
	assert_eq("utime[0-1]", 25, st2.st_atime);
	assert_eq("utime[0-2]", 7081, st2.st_mtime);

	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 0x23, buf[0]);

	assert_eq("read[1]", read(fd, buf, 1), 1);
	assert_eq("read[1] buf[0]", 0x41, buf[0]);

	assert_eq("close[0]", 0, close(fd));

	assert_eq("access[0]", 0,
			access("/initrd.mk", R_OK | W_OK));

	assert_eq("umask[0]", 022, umask(006));
	fd = open("/mtest", O_CREAT | O_WRONLY, 0666);
	assert_ne("umask[0-1]", -1, fd);
	assert_eq("umask[0-2]", 0, fstat(fd, &st));
	printf("fstat: mode=%x\n", st.st_mode);
	assert_eq("umask[0-3]",
			S_IFREG | S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR,
			st.st_mode);
	assert_eq("umask[0-4]", 0, close(fd));

	assert_eq("fcntl[0]", 0, fcntl(1, 0x10000, 1));

	return NULL;
}

char *testdup()
{
	int fd;
	char buf[1];

	fd = dup(1);
	assert_ne("dup[0]", -1, fd);
	buf[0] = 'a';
	write(fd, buf, 1);
	assert_eq("dup-close[0]", 0, close(fd));

	fd = dup2(1, 2);
	assert_eq("dup2[0]", 2, fd);
	buf[0] = 'b';
	write(fd, buf, 1);
	assert_eq("dup2-close[0]", 0, close(fd));

	return NULL;
}

char *testdir()
{
/*
	int fd;
	struct stat st;
*/
	char buf[1024];

	assert_eq("getcwd[0]", 0, strcmp("/", getcwd(buf, sizeof(buf))));

	assert_eq("chdir", 0, chdir("dev"));
	assert_eq("getcwd[1]", 0, strcmp("/dev", getcwd(buf, sizeof(buf))));
/*
	assert_eq("mkdir[0]", 0,
			mkdir("hoge", S_IRGRP | S_IROTH | S_IWOTH));
	fd = open("hoge", O_RDONLY);
	assert_ne("mkdir-open[0]", -1, fd);
	assert_eq("mkdir-fstat[0]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[0-1]",
			S_IFDIR | S_IRGRP | S_IROTH | S_IWOTH,
			st.st_mode);
	assert_eq("mkdir-close[0]", 0, close(fd));
*/
	return NULL;
}

char *testmm()
{
	int pid;
	time_t t1;
	time_t t2;
	struct timespec ts;
	char *buf;
	int i;

	pid = fork();
	if (pid == 0) {
		printf("child's pid = %d, ppid = %d\n",
				getpid(), getppid());
		exit(EXIT_SUCCESS);
	}
	else {
		printf("parent's pid = %d, ppid = %d\n",
				getpid(), getppid());
		waitpid(-1, &pid, 0);
	}

	t2 = time(&t1);
	printf("time = %d, %d\n",
		(int)((long long int)t1 >> 32),
		(int)((long long int)t1 & 0xffffffff));
	printf("time = %d, %d\n",
		(int)((long long int)t2 >> 32),
		(int)((long long int)t2 & 0xffffffff));
	assert_eq("clock_gettime[0]", 0, clock_gettime(CLOCK_REALTIME, &ts));
	printf("time = %d, %d, %x\n",
		(int)((long long int)(ts.tv_sec) >> 32),
		(int)((long long int)(ts.tv_sec) & 0xffffffff),
		(int)(ts.tv_nsec));
	assert_eq("clock_gettime[1]", -1, clock_gettime(CLOCK_MONOTONIC, &ts));

	sleep(2);
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("time = %d, %d, %x\n",
		(int)((long long int)(ts.tv_sec) >> 32),
		(int)((long long int)(ts.tv_sec) & 0xffffffff),
		(int)(ts.tv_nsec));

	printf("uid = %d\n", getuid());
	printf("euid = %d\n", geteuid());
	printf("gid = %d\n", getgid());
	printf("egid = %d\n", getegid());

	assert_eq("setgid[0]", 0, setgid(345));
	assert_eq("getegid[0]", 345, getegid());

	assert_eq("setuid[0]", 0, setuid(682));
	assert_eq("geteuid[0]", 682, geteuid());

	buf = malloc(1024 * 1024);
	assert_ne("brk", 0, buf);
	for (i = 0; i < 1024 * 1024; i++)	buf[i] = '0';
	free(buf);

	return NULL;
}

char *testothers()
{
	int i;

	for (i = 0; i < 5; i++)
		printf("%d\n", rand());

	srand(1);
	for (i = 0; i < 5; i++)
		printf("%d\n", rand());

	return NULL;
}

int main(int argc, char **argv)
{
	test(testfile);
	test(testdup);
	test(testdir);
	test(testmm);
	test(testothers);

	return 0;
}
