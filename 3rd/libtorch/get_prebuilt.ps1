# The following script downloads the libtorch prebuilt binaries from the
# official website and extracts the contents into the libtorch cgv project
# folder. The desired library configuration can be changed by using another
# URL from the website.

# This is the official URL to get the prebuilt binaries as debug API with CUDA.
# If a release or updated version should be used change this variable.
# Appropriate URLs can be found on https://pytorch.org/get-started/locally/
$url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-debug-1.5.0.zip"
$zip_file = $PSScriptRoot + "\" + $url.Substring($url.LastIndexOf("/") + 1)

# The script will check if an already downloaded archive is present and skip
# over or use the Windows inbuilt BITS to download the file from the given url.
# The download will continue as an background task. If canceling of the 
# download is necessary use the following commands in PowerShell:
#   Get-BitsTransfer | Remove-BitsTransfer

if (-not (Test-Path $zip_file)) {
    Import-Module BitsTransfer
    $Job = Start-BitsTransfer -Source $url -Destination $PSScriptRoot -Asynchronous

    While ( ($Job.JobState.ToString() -eq 'Transferring') -or ($Job.JobState.ToString() -eq 'Connecting') ) {
        $pct = [int](($Job.BytesTransferred * 100) / $Job.BytesTotal)
        Write-Progress -Activity "Downloading $url" -CurrentOperation "$pct% complete"
        Start-Sleep 1 # Prevent the process from stalling the system
    }
    Complete-BitsTransfer -BitsJob $Job
} else {
    Write-Output("Found file {0} already present" -f $zip_file)
}

# To avoid confusion of project source files and distribution files extract
# them into a separat folder.
$local_target = $PSScriptRoot + "/dist"

Expand-Archive -Force -LiteralPath $zip_file -DestinationPath $local_target
