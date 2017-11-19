:: clean vs template folders and temp files

del /q /f vs2013\ZInfoTech.sdf vs2013\ZInfoTech.v12.suo
rmdir /s /q vs2013\Debug vs2013\Release vs2013\libzit\Debug vs2013\libzit\Release
rmdir /s /q vs2013\zit_test\Debug vs2013\zit_test\Release vs2013\zpp_test\Debug vs2013\zpp_test\Release

del /q /f vs2017\ZInfoTech.sdf vs2017\ZInfoTech.v12.suo 
del /q /f vs2017\zit_test\zit_test.vcxproj.user
rmdir /s /q vs2017\Debug vs2017\Release vs2017\libzit\Debug vs2017\libzit\Release
rmdir /s /q vs2017\zit_test\Debug vs2017\zit_test\Release vs2017\zpp_test\Debug vs2017\zpp_test\Release
rmdir /s /q vs2017\.vs