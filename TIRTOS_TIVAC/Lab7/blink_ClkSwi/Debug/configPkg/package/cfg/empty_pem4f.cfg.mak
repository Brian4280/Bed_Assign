# invoke SourceDir generated makefile for empty.pem4f
empty.pem4f: .libraries,empty.pem4f
.libraries,empty.pem4f: package/cfg/empty_pem4f.xdl
	$(MAKE) -f C:\Brian\Fall2018\CPE403\Workspace\TIVAC_RTOS\blink_ClkSwi/src/makefile.libs

clean::
	$(MAKE) -f C:\Brian\Fall2018\CPE403\Workspace\TIVAC_RTOS\blink_ClkSwi/src/makefile.libs clean

