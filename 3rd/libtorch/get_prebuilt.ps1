# The following script downloads the libtorch prebuilt binaries from the
# official website and extracts the contents into the libtorch cgv project
# folder. The desired library configuration can be changed by using another
# URL from the website.

# This is the official URL to get the prebuilt binaries as debug API with CUDA.
# If a release or updated version should be used change this variable.
# Appropriate URLs can be found on https://pytorch.org/get-started/locally/
$debug_url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-debug-1.9.0%2Bcpu.zip"
$release_url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.9.0%2Bcpu.zip"

$debug_zip_file = $PSScriptRoot + "\" + $debug_url.Substring($debug_url.LastIndexOf("/") + 1)
$release_zip_file = $PSScriptRoot + "\" + $release_url.Substring($release_url.LastIndexOf("/") + 1)

# The script will check if an already downloaded archive is present and skip
# over or use the Windows inbuilt BITS to download the file from the given url.
# The download will continue as an background task. If canceling of the 
# download is necessary use the following commands in PowerShell:
#   Get-BitsTransfer | Remove-BitsTransfer

if (-not (Test-Path $debug_zip_file)) {
    Import-Module BitsTransfer
    $Job = Start-BitsTransfer -Source $debug_url -Destination $PSScriptRoot -Asynchronous

    While ( ($Job.JobState.ToString() -eq 'Transferring') -or ($Job.JobState.ToString() -eq 'Connecting') ) {
        $pct = [int](($Job.BytesTransferred * 100) / $Job.BytesTotal)
        Write-Progress -Activity "Downloading $debug_url" -CurrentOperation "$pct% complete"
        Start-Sleep 1 # Prevent the process from stalling the system
    }
    Complete-BitsTransfer -BitsJob $Job
} else {
    Write-Output("Found file {0} already present" -f $debug_zip_file)
}

if (-not (Test-Path $release_zip_file)) {
    Import-Module BitsTransfer
    $Job = Start-BitsTransfer -Source $release_url -Destination $PSScriptRoot -Asynchronous

    While ( ($Job.JobState.ToString() -eq 'Transferring') -or ($Job.JobState.ToString() -eq 'Connecting') ) {
        $pct = [int](($Job.BytesTransferred * 100) / $Job.BytesTotal)
        Write-Progress -Activity "Downloading $release_url" -CurrentOperation "$pct% complete"
        Start-Sleep 1 # Prevent the process from stalling the system
    }
    Complete-BitsTransfer -BitsJob $Job
} else {
    Write-Output("Found file {0} already present" -f $release_zip_file)
}

# To avoid confusion of project source files and distribution files extract
# them into a separat folder.
$debug_local_target = $PSScriptRoot + "/dist"
$release_local_target = $PSScriptRoot + "/dist_release"

Write-Output("Removing old files")
Remove-Item -LiteralPath $debug_local_target -Recurse
Remove-Item -LiteralPath $release_local_target -Recurse

Expand-Archive -Force -LiteralPath $debug_zip_file -DestinationPath $debug_local_target
Expand-Archive -Force -LiteralPath $release_zip_file -DestinationPath $release_local_target

$delete = $true
$confirmation = Read-Host "Do you want to delete the download archives [y/n]?"
while($confirmation -notmatch "[yY]")
{
    if ($confirmation -match "[nN]") {
        $delete = $false
        break
    }
    $confirmation = Read-Host "Do you want to delete the download archives [y/n]?"
}

if($delete) {
    Write-Output("Removing download archive {0}" -f $debug_zip_file)
    Remove-Item -LiteralPath $debug_zip_file

    Write-Output("Removing download archive {0}" -f $release_zip_file)
    Remove-Item -LiteralPath $release_zip_file
}

$debug_libs = $debug_local_target + "\libtorch\lib"
$release_libs = $release_local_target + "\libtorch\lib"

$debug_libs_target = $debug_local_target + "\libtorch\lib\debug\"
$release_libs_target = $debug_local_target + "\libtorch\lib\release"

Write-Output("Copying libs to debug and release folders")

If(!(Test-Path $debug_libs_target)) {
    New-Item -Path $debug_libs -Name "debug" -ItemType "directory"
}
Get-ChildItem -LiteralPath $debug_libs -File -Recurse | Move-Item -Destination $debug_libs_target

If(!(Test-Path $release_libs_target)) {
    New-Item -Path $debug_libs -Name "release" -ItemType "directory"
}
Get-ChildItem -LiteralPath $release_libs -File -Recurse | Move-Item -Destination $release_libs_target

Write-Output("Removing files no longer needed from {0}" -f $release_local_target)
Remove-Item -LiteralPath $release_local_target -Recurse
