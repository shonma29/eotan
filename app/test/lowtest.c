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
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
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
	off_t offset;

	fd = open("/motd", O_RDWR);
	assert_ne("open[0]", -1, fd);

	assert_eq("fstat", 0, fstat(fd, &st));

	printf("fstat: dev=%x\n", st.st_dev);
	printf("fstat: ino=%d\n", st.st_ino);
	printf("fstat: mode=%x\n", st.st_mode);
	printf("fstat: nlink=%d\n", st.st_nlink);
	printf("fstat: uid=%d\n", st.st_uid);
	printf("fstat: gid=%d\n", st.st_gid);
	printf("fstat: rdev=%x\n", st.st_rdev);
	printf("fstat: size=%d\n", (int)st.st_size);
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
	assert_eq("read[0] buf[0]", 'k', buf[0]);

	assert_eq("read[1]", 1, read(fd, buf, 1));
	assert_eq("read[1] buf[0]", 'e', buf[0]);

	offset = lseek(fd, 1, SEEK_SET);
	assert_t("lseek[1]", offset == 1LL);
	assert_eq("read[1-2]", 1, read(fd, buf2, 1));
	assert_eq("read[1-2] buf[0]", buf[0], buf2[0]);
	assert_eq("read[1-2] cmp", buf[0], buf2[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	buf[0] = '@';
	assert_eq("write[1]", 1, write(fd, buf, 1));

	assert_eq("close[0]", 0, close(fd));

	assert_eq("chmod[0]", 0, chmod("/motd", S_IXGRP | S_IROTH | S_IWOTH));

	fd = open("/motd", O_RDONLY);
	assert_ne("open[1]", -1, fd);

	assert_eq("fstat[1]", 0, fstat(fd, &st2));

	printf("fstat: mode=%x\n", st2.st_mode);
	assert_eq("chmod[0-1]",
			S_IFREG | S_IXGRP | S_IROTH | S_IWOTH,
			st2.st_mode);

	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 'k', buf[0]);

	assert_eq("read[1]", read(fd, buf, 1), 1);
	assert_eq("read[1] buf[0]", '@', buf[0]);

	assert_eq("close[0]", 0, close(fd));

	assert_eq("access[0]", 0,
			access("/motd", R_OK | W_OK));

	return NULL;
}

char *testdup()
{
	int fd;
	char buf[1];

	fd = dup2(1, 2);
	assert_eq("dup2[0]", 2, fd);
	buf[0] = 'b';
	write(fd, buf, 1);
	assert_eq("dup2-close[0]", 0, close(fd));

	return NULL;
}

char *testdir()
{
	int fd;
	struct stat st;
	char buf[1024];

	assert_eq("getcwd[0]", 0, strcmp("/", getcwd(buf, sizeof(buf))));

	assert_eq("chdir", 0, chdir("bin"));
	assert_eq("getcwd[1]", 0, strcmp("/bin", getcwd(buf, sizeof(buf))));

	assert_eq("mkdir[0]", 0,
			mkdir("hoge", S_IRGRP | S_IROTH | S_IWOTH));
	fd = open("hoge", O_RDONLY);
	assert_ne("mkdir-open[0]", -1, fd);
	assert_eq("mkdir-fstat[0]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[0-1]",
			S_IFDIR | S_IRGRP | S_IROTH,
			st.st_mode);
	assert_eq("mkdir-close[0]", 0, close(fd));

	/* umask */
	assert_eq("chmod", 0, chmod("hoge", 0705));
	fd = open("hoge/mtestfile", O_CREAT | O_WRONLY, 0666);
	assert_ne("umask[0-1]", -1, fd);
	assert_eq("umask[0-2]", 0, fstat(fd, &st));
	printf("fstat: mode=%x\n", st.st_mode);
	assert_eq("umask[0-3]",
			S_IFREG | S_IRUSR | S_IWUSR | S_IROTH,
			st.st_mode);
	assert_eq("umask[0-4]", 0, close(fd));

	assert_eq("mkdir[1]", 0,
			mkdir("hoge/mtestdir", 0777));
	fd = open("hoge/mtestdir", O_RDONLY);
	assert_ne("mkdir-open[1]", -1, fd);
	assert_eq("mkdir-fstat[1]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[1-1]",
			S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IXOTH,
			st.st_mode);
	assert_eq("mkdir-close[1]", 0, close(fd));

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
		sleep(1);
		printf("parent's pid = %d, ppid = %d\n",
				getpid(), getppid());
		wait(&pid);
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
	printf("gid = %d\n", getgid());

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
