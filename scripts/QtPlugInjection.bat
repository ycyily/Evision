@echo off
echo  :inject QtDesingner Plugin into place.

if exist $(QTDIR)\plugins\designer\$(AssemblyName).dll (
    del $(QTDIR)\plugins\designer\$(AssemblyName).dll
    echo delete old version of $(AssemblyName).dll done
)
if exist $(QTDIR)\plugins\designer\$(AssemblyName).lib (
    del $(QTDIR)\plugins\designer\$(AssemblyName).lib
    echo delete old version of $(AssemblyName).lib done
)

copy $(OutDir)$(AssemblyName).dll $(QTDIR)\plugins\designer\$(AssemblyName).dll
echo copy $(AssemblyName).dll done

copy $(OutDir)$(AssemblyName).lib $(QTDIR)\plugins\designer\$(AssemblyName).lib
echo copy $(AssemblyName).lib done