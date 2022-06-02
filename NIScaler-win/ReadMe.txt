
Purpose:
+ Rescale SpikeGLX NI data acquired before version 20220101:
+ Create voltage calibration files for NI devices.
+ Apply corrections to existing SpikeGLX NI bin/meta files.
Run messages are appended to NIScaler.log in the current working directory.

Output:
+ Calibration files are placed into 'cal_dir'.
+ Corrected NI data files are placed (with same name) into 'dst_dir' folder.

Usage:
>NIScaler < parameters >

Parameters:
-create_cal     ;scan NI devices and create calibration files
-apply          ;use calibration files to correct SpikeGLX NI data
-cal_dir=path   ;where to put/get calibration files
-src_dir=path   ;if applying, directory with nidq.bin/meta files to fix
-dst_dir=path   ;if applying, where to put fixed nidq.bin/meta files
-dev1=new_name  ;optional new name of dev1 if moved or renamed since run
-dev2=new_name  ;optional new name of dev2 if moved or renamed since run

Notes:
- An NIScaler-run can 'create_cal' alone or 'apply' alone, or do both in one run.
- Command line params can be listed in any order; 'create_cal' is done before 'apply'.
- Apply finds and acts upon all nidq.bin/meta files in src_dir.
- Apply adds a metadata item 'NIScaler=date'.
- Apply skips files if {version >= 20220101, no cal data for that product, meta item 'NIScaler' present}.
- Folder dst_dir must already exist.

- You can call NIScaler from a script.
- You can try it by editing the included 'runit.bat' file. Edit the file to set your own parameters. Then double-click the bat file to run it.
- Options must not have spaces.
- In *.bat files, continue long lines using <space><caret>. Like this ^.
- Remove all white space at line ends, especially after a caret (^).


Change Log
----------
Version 1.0
- Initial release.


