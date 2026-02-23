rm -rf ./bin/switch_auto_core
rm -rf SwitchAutoCore_Mac.zip
rm -rf ./libs
cp ../cmake-build-debug/switch_auto_core SwitchAutoCore_Mac/bin
dylibbundler -od -b -x SwitchAutoCore_Mac/bin/switch_auto_core -d SwitchAutoCore_Mac/libs/
