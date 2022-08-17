#include <libbelladonna.h>

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#include <payloads/hyoscine_ramdisk.h>


int belladonna_boot_hyoscine(belladonna_ctx_t ctx) {
	return belladonna_ramdisk_boot(ctx, hyoscine_ramdisk, hyoscine_ramdisk_length);
}

static int read_until(FILE* f, char until, belladonna_ctx_t ctx) {
	/* Allows us to read until the next prompt so that we know we're ready to send the next command */
	int byte_read = '\0';
	while(1) {
		byte_read = fgetc(f);
		if(byte_read < 0) {
			return -1;
		}
		if(isprint(byte_read) || isspace(byte_read)) {
			ctx->debug_log("%c", byte_read);
		}
		if(byte_read == until) {
			return 0;
		}
	}
}

static int send_ramdisk_command(FILE* f, char* cmd, belladonna_ctx_t ctx) {
	/* Send the command then read until '>' so that the prompt is ready */
	ctx->debug_log("%s\n", cmd);
	fprintf(f, "%s\n", cmd);
	return read_until(f, '>', ctx);
}

int belladonna_jailbreak(belladonna_ctx_t ctx, int should_hacktivate) {
	int ret = 0;
	usbmuxd_device_info_t muxdev;
	usbmuxd_device_info_t* dev;

	/* The ramdisk will take time to boot. */
	ctx->progress_log(0);
	ctx->debug_log("Waiting for ramdisk\n");
	while(1) {
		ret = usbmuxd_get_device("hyoscine jailbreakd", &muxdev, DEVICE_LOOKUP_USBMUX);
		if(ret > 0) {
			break;
		}
		sleep(1);
		ret = irecv_open_with_ecid(&ctx->client, 0);
		if(ret == 0) {
			ctx->error_log("Failed to boot ramdisk. Please restore then try again\n");
			return -1;
		}
	}
	dev = &muxdev;

	ctx->progress_log(50);

	ctx->debug_log("Connecting to ramdisk\n");

	/* jailbreakd takes some time to come up after USB has been initialised. */

	int sfd = 0;

	for(int i = 0; i < 10; i += 1) {

		/* jailbreakd runs on port 1337 */
		sfd = usbmuxd_connect(dev->handle, 1337);
		if(sfd > 0) {
			break;
		}
		sleep(1);
	}
	if(sfd < 0) {
		ctx->error_log("Failed to open ramdisk port 1337\n");
		return -1;
	}
	ctx->progress_log(100);
	int flags = fcntl(sfd, F_GETFL);
	fcntl(sfd, F_SETFL, flags & ~O_NONBLOCK);

	FILE* rd = fdopen(sfd, "wb+");
	if(!rd) {
		ctx->error_log("Failed to open ramdisk file handle\n");
		return -1;
	}

	ctx->progress_log(0);
	/* This is ugly, but effectively runs the jailbreak */
	ret = read_until(rd, '>', ctx);
	if(ret != 0) {
		ctx->error_log("Failed to read from the ramdisk\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Enabling non default apps", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to enable non default apps\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "exec /files/non_default_apps", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to enable non default apps\n");
		return -1;
	}
	ctx->progress_log(10);
	ret = send_ramdisk_command(rd, "fbprog 10", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to enable non default apps\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Installing n1ghtshade app", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install the n1ghtshade app\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "untar /files/n1ghtshade.tar /Applications/", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install the n1ghtshade app\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "chmod /Applications/n1ghtshade.app/n1ghtshade 6755", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install the n1ghtshade app\n");
		return -1;
	}
	ctx->progress_log(20);
	ret = send_ramdisk_command(rd, "fbprog 20", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install the n1ghtshade app\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Running uicache", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to run uicache\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "exec /files/uicache /Applications/n1ghtshade.app", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to run uicache\n");
		return -1;
	}
	ctx->progress_log(30);
	ret = send_ramdisk_command(rd, "fbprog 30", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to run uicache\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Disabling system updates", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable system updates\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "exec /files/sysupdate", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable system updates\n");
		return -1;
	}
	ctx->progress_log(40);
	ret = send_ramdisk_command(rd, "fbprog 40", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable system updates\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Installing launchctl", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install launchctl\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "copyno /files/launchctl /bin/launchctl", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install launchctl\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "chmod /bin/launchctl 755", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install launchctl\n");
		return -1;
	}
	ctx->progress_log(50);
	ret = send_ramdisk_command(rd, "fbprog 50", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install launchctl\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Installing jb_init", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "copy /files/jb_init /usr/libexec/rtbuddyd", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "copy /files/jb_init /usr/libexec/dirhelper", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "chmod /usr/libexec/rtbuddyd 6755", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "chmod /usr/libexec/dirhelper 6755", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}
	ctx->progress_log(60);
	ret = send_ramdisk_command(rd, "fbprog 60", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to install jb_init\n");
		return -1;
	}

	ret = send_ramdisk_command(rd, "fbecho Disabling Cydia stashing", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable stashing\n");
		return -1;
	}
	ret = send_ramdisk_command(rd, "touch /mnt/.cydia_no_stash", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable stashing\n");
		return -1;
	}
	ctx->progress_log(70);
	ret = send_ramdisk_command(rd, "fbprog 70", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to disable stashing\n");
		return -1;
	}

	if(should_hacktivate) {
		ret = send_ramdisk_command(rd, "fbecho Hacktivating...", ctx);
		if(ret != 0) {
			ctx->error_log("Failed to hacktivate\n");
			return -1;
		}
		ret = send_ramdisk_command(rd, "exec /files/hacktivate_installer", ctx);
		if(ret != 0) {
			ctx->error_log("Failed to hacktivate\n");
			return -1;
		}
		ctx->progress_log(80);
		ret = send_ramdisk_command(rd, "fbprog 80", ctx);
		if(ret != 0) {
			ctx->error_log("Failed to hacktivate\n");
			return -1;
		}
	}


	ret = send_ramdisk_command(rd, "fbecho Done!", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to send done message\n");
		return -1;
	}
	ctx->progress_log(100);
	ret = send_ramdisk_command(rd, "fbprog 100", ctx);
	if(ret != 0) {
		ctx->error_log("Failed to send done message\n");
		return -1;
	}

	send_ramdisk_command(rd, "reboot", ctx);

	fclose(rd);
	return 0;
}