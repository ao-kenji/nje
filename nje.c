/* 
 * $Id: nje.c,v 1.3 2007/05/23 14:56:02 aoyama Exp $
 * 
 * Copyright (c) 2007 Kenji AOYAMA <aoyama [at] nk-home [dot] net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* prototypes */
void usage(void);
int main(int, char *[]);

#if defined(__NetBSD__)
#define DEFAULT_DEVICE "/dev/dty00"	/* For NetBSD/i386 */
#elif defined(__OpenBSD__) 
#define DEFAULT_DEVICE "/dev/tty00"	/* For OpenBSD/i386 */
#endif

#define NJE_BAUD	B9600		/* defined in termios.h */

extern char *__progname;
extern char *optarg;
extern int optind;

int vflag = 0;
char del[3] = {0x0d, 0x0a, 0x00};	/* delimiter, as a string */

int
main(int argc, char *argv[])
{
	struct termios tio;
	time_t now;
	int ch, fc, fd, i, ret;
	char devname[MAXPATHLEN];
	char buf[160], stamp[10];

	strlcpy(devname, DEFAULT_DEVICE, sizeof(devname));

	while ((ch = getopt(argc, argv, "f:v")) != -1) {
		switch(ch) {
		case 'f':
			strlcpy(devname, optarg, sizeof(devname));
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/*
	 * Open the device.  NJE-10[56] does not send any data
	 * to us, so open with O_WRONLY.
	 *
	 * To disable modem controls, use O_NONBLOCK at open.
	 */
	if ((fd = open(devname, O_WRONLY | O_NONBLOCK)) < 0) {
		perror(devname);
		exit(1);
	}

	/*
	 * Then, clear O_NONBLOCK flag.
	 */
	if ((fc = fcntl(fd, F_GETFL, 0)) < 0) {
		perror("F_GETFL");
		exit(1);
	}

	if (fcntl(fd, F_SETFL, fc & ~O_NONBLOCK) < 0) {
		perror("F_SETFL");
		exit(1);
	}
		

	/*
	 * 9600bps, 8bit, non-parity, 1 stop bit, local connection,
	 * and no flow control.
	 */
	if (tcgetattr(fd, &tio) < 0) {
		perror("can not get tty attribute");
		exit(1);
	}

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CLOCAL;
	tio.c_lflag = 0;
	cfsetospeed(&tio, NJE_BAUD);

	if (tcsetattr(fd, TCSANOW, &tio) < 0) {
		perror("can not set tty attribute");
		exit(1);
	}

	/*
	 * Prepare sending data
	 *
	 * Message format:
	 *   {delimiter} {timestamp} {contents} {delimiter}
	 *     delimiter: 2 bytes
	 *       \r \n (CR LF)
	 *     timestamp: 8 bytes
	 *       MMDDhhmm in ASCII characters
	 *     contents: up to 128 bytes
	 *       ASCII/Shift-JIS string
	 *       it may contain some attributes and control commands
	 * See http://http://www.hydra.cx/msgbd_code.html
	 */
	time(&now);
	strftime(stamp, sizeof(stamp), "%m%d%H%M", localtime(&now));

	strlcpy(buf, del, sizeof(buf));
	strlcat(buf, stamp, sizeof(buf));

	for (i = 0; i < argc; i++)
		strlcat(buf, argv[i], sizeof(buf));
	if (strlen(buf) > 138) { /* delimiter + timestamp + 128 */
		fprintf(stderr, "too long message\n");
		exit(1);
	}

	strlcat(buf, del, sizeof(buf));

	if (vflag)
		printf("%s", buf);
	/*
	 * Write the message
	 */
	if ((ret = write(fd, buf, strlen(buf))) == -1) {
		fprintf(stderr, "write error\n");
		exit(1);
	}

	close(fd);
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] [-f device]\n", __progname);
	exit(1);
}
