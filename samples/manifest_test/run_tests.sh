#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`

#Generate from template
sed s@{ABS_PATH}@$SCRIPT_PATH/@g manifest_template/1manifest.template  > 1manifest_test.manifest

echo some input text > some.input

echo -------------------------------run test 1
echo manifest1 test
rm 1.errlog -f
echo ../../zvm/zerovm -M${SCRIPT_PATH}/1manifest_test.manifest
../../zvm/zerovm -M${SCRIPT_PATH}/1manifest_test.manifest
echo "stderr output >>>>>>>>>>"
cat 1.errlog




