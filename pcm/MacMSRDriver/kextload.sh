#!/usr/bin/env bash

cp -R build/Release/PcmMsrDriver.kext /Library/Extensions/.
chown -R root:wheel /Library/Extensions/PcmMsrDriver.kext
kextload /Library/Extensions/PcmMsrDriver.kext
