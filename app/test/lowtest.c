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
#include <stdlib.h>
#include <unistd.h>

int testfile() {
	int fd;
	int result;
	struct stat st;
	char buf[1];

	fd = open("/init.fm", O_RDONLY);
	if (fd == -1) {
		printf("open error=%d\n", errno);
		return -1;
	}

	result = fstat(fd, &st);
	if (result == -1) {
		printf("fstat error=%d\n", errno);
		return -1;
	}

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
	printf("fstat: atime=%d\n", st.st_atime);
	printf("fstat: mtime=%d\n", st.st_mtime);
	printf("fstat: ctime=%d\n", st.st_ctime);

	if (read(fd, buf, 1) != 1) {
		printf("read error=%d\n", errno);
	}
	else {
		printf("read %x\n", buf[0]);
	}

	close(fd);

	return 0;
}

int main(int argc, char **argv)
{
	char buf[1024];
	struct utsname name;
	int pid;
	int result;
	time_t t;

	testfile();

	printf("dir = %s\n", getcwd(buf, sizeof(buf)));
	chdir("/system");
	printf("dir = %s\n", getcwd(buf, sizeof(buf)));

	pid = fork();
	if (pid == 0) {
		printf("child's pid = %d, ppid = %d\n",
				getpid(), getppid());
		_exit(0);
	}
	else {
		printf("parent's pid = %d, ppid = %d\n",
				getpid(), getppid());
		waitpid(-1, &pid, 0);
	}

	result = uname(&name);
	if (result == 0) {
		printf("uname = %s\n", name.sysname);
		printf("uname = %s\n", name.nodename);
		printf("uname = %s\n", name.release);
		printf("uname = %s\n", name.version);
		printf("uname = %s\n", name.machine);
	}
	else {
		printf("uname errno = %d\n", errno);
	}

	printf("time = %d\n", time(&t));
	printf("time = %d\n", t);

	printf("uid = %d\n", getuid());
	printf("euid = %d\n", geteuid());
	printf("gid = %d\n", getgid());
	printf("egid = %d\n", getegid());
	setuid(682);
	setgid(345);
	printf("uid = %d\n", getuid());
	printf("euid = %d\n", geteuid());
	printf("gid = %d\n", getgid());
	printf("egid = %d\n", getegid());

	return 0;
}
