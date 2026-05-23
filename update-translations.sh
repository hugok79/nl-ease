#!/bin/bash
#
# update-translation:
#  This script updates nl-ease's localization files.
#  It updates the .pot file and derives .po files from it.

set -e

PROJECT_NAME="nl-ease"
SRC_DIR="src"
PO_DIR="po"
POT_FILE="${PO_DIR}/${PROJECT_NAME}.pot"
BUGS_ADDRESS="https://github.com/diekrz2/nl-ease/issues"

echo
echo "Locales update:"
echo

# .pot
xgettext --language=C --keyword=_ --keyword=N_ --from-code=UTF-8 \
         --package-name="${PROJECT_NAME}" --package-version="1.0" \
         --copyright-holder="diekrz2" \
         --msgid-bugs-address="${BUGS_ADDRESS}" \
         --output="${POT_FILE}" ${SRC_DIR}/*.c

echo "${POT_FILE} updated"

# .po
for po_file in ${PO_DIR}/*.po; do
    if [ -f "$po_file" ]; then
        lang=$(basename "$po_file" .po)
        echo
        echo "${lang}.po update..."
        echo
        msgmerge --update --backup=off "$po_file" "${POT_FILE}"
    fi
done

echo
echo "'Report-Msgid-Bugs-To' set to: "
echo "${BUGS_ADDRESS} "
echo
echo "All done."
echo
