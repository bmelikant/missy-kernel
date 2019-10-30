#include <kernel/version.h>

static const char *_banner_text = \
"     __  __________ ______ ________  __                                       \n" \
"    /  \\/   /_   _// ____// ____/\\ \\/ / ************************************\n" \
"   / /\\__/ /  / /  \\___ \\ \\__  \\  \\  /  x86 Microcomputer Operating System *\n" \
"  / /   / /__/ /_ ____/ /____/ /  / /  version 0.0.1_alpha                 *\n" \
" /_/   /_/______//_____//_____/  /_/ ***************************************\n\n";

const char *banner() {
	return _banner_text;
}