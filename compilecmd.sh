#!/bin/sh
#
# run compiledb for each project in the kernel tree to configure clangd server

MAKE_CLEAN=0
for i in "$@"; do
    case "$i" in
        -c|--clean)
            MAKE_CLEAN=1
            shift
            ;;
        *)
            echo "Unrecognized option"
            shift
            ;;
    esac
done

# make sure compiledb is installed
COMPILEDB=$(which compiledb)
[ $? != 0 ] && {
    echo "Command 'compiledb' was not found";
    exit 1;
}

COMPILEDB="$COMPILEDB -n"
set -e
. ./config.sh

if [ $MAKE_CLEAN -eq 0 ]; then
    for PROJECT in $PROJECTS; do
        (cd $PROJECT && $COMPILEDB $MAKE)
    done
    echo "Generated compile_commands.json for projects";
fi

if [ $MAKE_CLEAN -eq 1 ]; then
    for PROJECT in $PROJECTS; do
        (cd $PROJECT && rm -f compile_commands.json)
    done
    echo "Removed compile_commands.json configuration files"
fi

exit 0;
