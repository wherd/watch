/*
 |-----------------------------------------------------------------------------
 | Copyright 2016 (C) Sérgio Leal.
 |-----------------------------------------------------------------------------
 |
 | Use of this source code is governed by a MIT-style license (the "License");
 | you may not use this file except in compliance with the License.
 | You may obtain a copy of the License in the LICENSE file.
 | 
 | Unless required by applicable law or agreed to in writing, software
 | distributed under the License is distributed on an "AS IS" BASIS,
 | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 | See the License for the specific language governing permissions and
 | limitations under the License.
 |
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include <CoreServices/CoreServices.h>

/*
 |-----------------------------------------------------------------------------
 | Command to execute
 |-----------------------------------------------------------------------------
 |
 | (string)
 |
 */
char *command;

/*
 | ----------------------------------------------------------------------------
 | Print usage message.
 | ----------------------------------------------------------------------------
 |
 | Arguments:
 |    (string) the application path
 |
 */
static void
usage(char *arg)
{
	char *app = basename(arg);

	printf("Usage: %s <dir> <cmd> [args ...]\n\n", app);

	printf("   <dir>        path to watch for changes\n");
	printf("   <cmd>        command to execute when it changes\n");
	printf("   [args ...]   command arguments\n\n");
}

/*
 | ----------------------------------------------------------------------------
 | Callback executed whenever a change is detected.
 | ----------------------------------------------------------------------------
 |
 | https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/index.html#//apple_ref/c/tdef/FSEventStreamCallback
 |
 */
static void
callback(
	ConstFSEventStreamRef streamRef,
	void *clientCallBackInfo,
	size_t numEvents,
	void *eventPaths,
	const FSEventStreamEventFlags eventFlags[],
	const FSEventStreamEventId eventIds[]
) {
	pid_t pid = fork();
	switch (pid) {
	case -1:
		perror("fork()");
		exit(1);
	case 0:
		execl("/bin/bash", "/bin/bash", "-c", command, NULL);
		perror("execl()");
		exit(1);
	}

	// wait for exit
	int status;
	waitpid(pid, &status, 0);
}

/*
 |-----------------------------------------------------------------------------
 | Main application
 |-----------------------------------------------------------------------------
 |
 | Arguments:
 |    (int) number of arguments
 |    ([]string) application arguments
 |
 | Returns:
 |    (int) application exit code
 |
 */
int
main(int argc, char *argv[])
{
	// validate arguments
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	// create the command
	int len = 0;
	for (int i = 2; i < argc; i++) {
		len += strlen(argv[i]) + 1;
	}

	command = malloc(len * sizeof(char));
	if (!command) {
		fprintf(stderr, "Error: Allocate memory.\n");
		return 1;
	}

	int n = 0;
	for (int i = 2; i < argc; i++) {
		memcpy(&command[n], argv[i], strlen(argv[i]) * sizeof(char));
		n += strlen(argv[i]);
		command[n++] = ' ';
	}

	command[n-1] = '\0';

	// Define variables and create a CFArray object containing CFString
	// objects containing paths to watch.
	CFStringRef path_to_watch = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
	CFArrayRef paths_to_watch = CFArrayCreate(NULL, (const void **)&path_to_watch, 1, NULL);

	// Create the stream, passing in a callback
	FSEventStreamRef stream = FSEventStreamCreate(
		NULL,
		&callback,
		NULL,				// could put stream-specific data here.
		paths_to_watch,
		kFSEventStreamEventIdSinceNow,	// or a previous event ID
		1.0,				// latency in seconds
		kFSEventStreamCreateFlagNone	// flags explained in reference
	);

	// start the loop
	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	FSEventStreamStart(stream);

	CFRunLoopRun();

	// never reached
	free(command);
	return 0;
}
