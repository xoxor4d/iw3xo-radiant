. "$PSScriptRoot/common.ps1"

cp -Force "$PSScriptRoot\..\assets\CoD4Radiant.exe" "$PSScriptRoot\iw3r.tmp"

# Remove certificate & build timestamp if any
& "$PSScriptRoot/PETool.exe" "$PSScriptRoot\iw3r.tmp"  /destamp

# Apply new version info
echo "Updating version information..."
& "$PSScriptRoot/verpatch.exe" "$PSScriptRoot\iw3r.tmp" /rpdb /langid 0x409 /va 6.9.6.9 /pv 6.9.6.9 /s copyright "xoxor4d.github.io" /s desc "Radiant modification built for IW3xo" /s title "x" /s product "IW3xRadiant" /s OriginalFilename "IW3xRadiant.exe" /s pb 1

# Update the icon
echo "Updating the icon..."
winresourcer --operation=add "--exeFile=$PSScriptRoot\iw3r.tmp" --resourceType=Icongroup --resourceName=1 --lang=1033 "--resourceFile=$PSScriptRoot\..\assets\ico_3xr.ico"

# Let's place it into the root directory
mv -Force "$PSScriptRoot\iw3r.tmp" "$PSScriptRoot\..\IW3xRadiant.exe"