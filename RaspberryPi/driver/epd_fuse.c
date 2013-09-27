// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.


#define VERSION 1

#define STR1(x) #x
#define STR(x) STR1(x)

#define FUSE_USE_VERSION 26

#include <stdint.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <err.h>

#include "gpio.h"
#include "spi.h"
#include "epd.h"


static const char version_buffer[] = {STR(VERSION) "\n"};

#define VERSION_SIZE (sizeof(version_buffer) - sizeof((char)'\0'))


// GPIO setup
#define panel_on_pin  GPIO_P1_16
#define border_pin    GPIO_P1_08
#define discharge_pin GPIO_P1_10
#define pwm_pin       GPIO_P1_12
#define reset_pin     GPIO_P1_18
#define busy_pin      GPIO_P1_22


static const char *version_path = "/version";   // the program version string
static const char *panel_path = "/panel";       // type of panel connected
static const char *current_path = "/current";   // the current screen image
static const char *display_path = "/display";   // the next image to display
static const char *command_path = "/command";   // any write transfers display -> EPD and updates current

static const char *spi_device = "/dev/spidev0.0";  // default Rasberry PI SPI device path

static int temperature = 25;                    // for external temperature compensation (not yet!)

static const struct panel_struct {
	const char *key;
	const char *description;
	const EPD_size size;
	const int width;
	const int height;
	const int byte_count;
} panels[] = {
	{"1.44", "EPD 1.44 128x96\n", EPD_1_44, 128, 96, 128 * 98 / 8},
	{"2.0", "EPD 2.0 200x96\n", EPD_2_0, 200, 96, 200 * 96 / 8},
	{"2.7", "EPD 2.7 264x176\n", EPD_2_7, 264, 176, 264 * 176 / 8},
	{NULL, NULL, 0, 0, 0, 0}  // must be last entry
};

// need to sync size with above (max of all sizes)
// this will be the next display
static char display_buffer[264 * 176 / 8];

// this is the current display
static char current_buffer[sizeof(display_buffer)];

static const struct panel_struct *panel = NULL;
static EPD_type *epd = NULL;
static SPI_type *spi = NULL;


// function prototypes
static void run_command(const char c);


// fuse callbacks
// ==============

static int display_access(const char *path, int mode) {
	return 0;  // every thing allowed!
}


static int display_getattr(const char *path, struct stat *stbuf) {

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 01777;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, version_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = VERSION_SIZE;
	} else if (strcmp(path, panel_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(panel->description);
	} else if (strcmp(path, command_path) == 0) {
		stbuf->st_mode = S_IFREG | 0222;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1;
	} else if (strcmp(path, current_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = panel->byte_count;
	} else if (strcmp(path, display_path) == 0) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = panel->byte_count;
		//stbuf->st_atim.tv_sec = 100000;
		//stbuf->st_mtim.tv_sec = 200000;
		//stbuf->st_ctim.tv_sec = 300000;
	} else {
		return -ENOENT;
	}
	return 0;
}

static int display_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0) {
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, current_path + 1, NULL, 0);
	filler(buf, display_path + 1, NULL, 0);
	filler(buf, panel_path + 1, NULL, 0);
	filler(buf, command_path + 1, NULL, 0);
	filler(buf, version_path + 1, NULL, 0);

	return 0;
}

static int display_open(const char *path, struct fuse_file_info *fi) {
	// read-write items
	if (strcmp(path, display_path) == 0 ||
	    strcmp(path, command_path) == 0) {
		switch (fi->flags & (O_RDONLY | O_WRONLY | O_APPEND | O_TRUNC)) {
		case O_RDONLY:
		case O_WRONLY:
		case O_WRONLY | O_TRUNC:
		case O_WRONLY | O_APPEND:
		case O_RDWR:
			return 0;
		default:
			return -EACCES;
		}
		return 0;
	}

	// read-only items
	if (strcmp(path, panel_path) != 0 &&
	    strcmp(path, current_path) != 0 &&
	    strcmp(path, version_path) != 0) {
		return -ENOENT;
	}
	if ((fi->flags & 3) != O_RDONLY) {
		return -EACCES;
	}
	return 0;
}

static int display_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	(void) mode;
	(void) fi;

	if (strcmp(path, display_path) == 0 ||
	    strcmp(path, command_path) == 0) {
		return 0;
	}
	return -EACCES;
}


static int display_truncate(const char *path, off_t offset) {
	(void) offset;
	if (strcmp(path, display_path) == 0 ||
	    strcmp(path, command_path) == 0) {
		return 0;
	}
	return -EACCES;
}


static int display_read(const char *path, char *buffer, size_t size, off_t offset,
			struct fuse_file_info *fi) {
	(void) fi;
	size_t length;
	const char *source;

	if (strcmp(path, version_path) == 0) {
		source = version_buffer;
		length = VERSION_SIZE;
	} else if (strcmp(path, panel_path) == 0) {
		source = panel->description;
		length = strlen(source);
	} else if (strcmp(path, current_path) == 0) {
		source = current_buffer;
		length = panel->byte_count;
	} else if (strcmp(path, display_path) == 0) {
		source = display_buffer;
		length = panel->byte_count;
	} else {
		return -ENOENT;
	}
	// common read code
	if (offset < length) {
		if (offset + size > length) {
			size = length - offset;
		}
		memcpy(buffer, source + offset, size);
	} else {
		size = 0;
	}
	return size;
}

static int display_write(const char *path, const char *buffer, size_t size, off_t offset,
			 struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	if (strcmp(path, command_path) == 0) {
		if (size > 0) {
			run_command(buffer[0]);
		}
		return size;
	}
	if (strcmp(path, display_path) != 0) {
		return -ENOENT;
	}

	len = sizeof(display_buffer);
	if (offset < len) {
		if (offset + size > len) {
			size = len - offset;
		}
		memcpy(display_buffer + offset, buffer, size);
	} else {
		size = 0;
	}
	return size;
}


static void *display_init(struct fuse_conn_info *conn) {

	if (!GPIO_setup()) {
		warn("GPIO_setup failed");
		goto done;
	}

	spi = SPI_create(spi_device);
	if (NULL == spi) {
		warn("SPI_setup failed");
		goto done_gpio;
	}

	GPIO_mode(panel_on_pin, GPIO_OUTPUT);
	GPIO_mode(border_pin, GPIO_OUTPUT);
	GPIO_mode(discharge_pin, GPIO_OUTPUT);
	GPIO_mode(pwm_pin, GPIO_PWM);
	GPIO_mode(reset_pin, GPIO_OUTPUT);
	GPIO_mode(busy_pin, GPIO_INPUT);

	epd = EPD_create(panel->size,
			 panel_on_pin,
			 border_pin,
			 discharge_pin,
			 pwm_pin,
			 reset_pin,
			 busy_pin,
			 spi);

	if (NULL == epd) {
		warn("EPD_setup failed");
		goto done_spi;
	}

	return (void *)epd;

	// release resources
//done_epd:
//        EPD_destroy(epd);
done_spi:
	SPI_destroy(spi);
done_gpio:
	GPIO_teardown();
done:
	return NULL;
}


static void display_destroy(void *param) {
	if (NULL != param) {
		EPD_destroy(epd);
		SPI_destroy(spi);
		GPIO_teardown();
	}
}


static struct fuse_operations display_operations = {
	.access   = display_access,
	.getattr  = display_getattr,
	.readdir  = display_readdir,
	.truncate = display_truncate,
	.open     = display_open,
	.create   = display_create,
	.read     = display_read,
	.write    = display_write,
	.init     = display_init,
	.destroy  = display_destroy
};


// run a command
static void run_command(const char c) {
	switch(c) {
	case 'C':  // clear the display
		EPD_set_temperature(epd, temperature);
		EPD_begin(epd);
		EPD_clear(epd);
		EPD_end(epd);

		memset(current_buffer, 0, sizeof(current_buffer));
		break;

	case 'U':  // update with contents of display
		EPD_set_temperature(epd, temperature);
		EPD_begin(epd);
		EPD_image(epd, (const uint8_t *)current_buffer, (const uint8_t *)display_buffer);
		EPD_end(epd);

		memcpy(current_buffer, display_buffer, sizeof(display_buffer));
		break;

	default:
		break;
	}
}


// values for setting options
enum {
     KEY_HELP,
     KEY_VERSION,
     KEY_PANEL,
     KEY_SPI
};


static struct fuse_opt display_options[] = {
	FUSE_OPT_KEY("--panel=%s",  KEY_PANEL),
	FUSE_OPT_KEY("panel=%s",    KEY_PANEL),

	FUSE_OPT_KEY("--spi=%s",    KEY_SPI),
	FUSE_OPT_KEY("spi=%s",      KEY_SPI),

	FUSE_OPT_KEY("-V",          KEY_VERSION),
	FUSE_OPT_KEY("--version",   KEY_VERSION),
	FUSE_OPT_KEY("-h",          KEY_HELP),
	FUSE_OPT_KEY("--help",      KEY_HELP),
	FUSE_OPT_END
};


static int option_processor(void *data, const char *arg, int key, struct fuse_args *outargs)
{
     switch (key) {
     case KEY_HELP:
	     fprintf(stderr,
		     "usage: %s mountpoint [options]\n"
		     "\n"
		     "general options:\n"
		     "    -o opt,[opt...]  mount options\n"
		     "    -h   --help      print help\n"
		     "    -V   --version   print version\n"
		     "\n"
		     "Myfs options:\n"
		     "    -o panel=SIZE     set panel size\n"
		     "    -o spi=DEVICE     override default SPI device [%s]\n"
		     "    --panel=NUM       same as '-opanel=SIZE'\n"
		     "    --spi=DEVICE      same as '-ospi=DEVICE'\n"
		     , outargs->argv[0], spi_device);
	     fuse_opt_add_arg(outargs, "-ho");
	     fuse_main(outargs->argc, outargs->argv, &display_operations, NULL);
	     exit(1);

     case KEY_VERSION:
	     fprintf(stderr, "%s version %d\n", outargs->argv[0], VERSION);
	     fuse_opt_add_arg(outargs, "--version");
	     fuse_main(outargs->argc, outargs->argv, &display_operations, NULL);
	     exit(0);

     case KEY_PANEL: {
	     const char *p = strchr(arg, '=');
	     ++p;
	     panel = panels;
	     for (panel = panels; NULL != panel->key; ++panel) {
		     if (strcmp(panel->key, p) == 0) {
			     return 0;
		     }
	     }
	     return 1;
     }

     case KEY_SPI: {
	     const char *p = strchr(arg, '=');
	     spi_device = strdup(p);
	     if (1) {    // test for spi path exists
		     return 0;
	     }
	     return 1;
     }
     }
     return 1;
}


int main(int argc, char *argv[])
{
     struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

     memset(current_buffer, 0, sizeof(current_buffer));
     memset(display_buffer, 0, sizeof(display_buffer));

     fuse_opt_parse(&args, NULL, display_options, option_processor);

     // run fuse
     return fuse_main(args.argc, args.argv, &display_operations, NULL);
}
