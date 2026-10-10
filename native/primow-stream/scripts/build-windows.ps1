param([string]$Work = "$PSScriptRoot/../../../generated-artifacts/windows")
$ErrorActionPreference = "Stop"
function Run([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed ($LASTEXITCODE)" }
}
$Work = [IO.Path]::GetFullPath($Work)
$App = [IO.Path]::GetFullPath("$PSScriptRoot/..").Replace('\','/')
$Obs = "$Work/obs"
$Stage = "$Work/stage"
New-Item -Force -ItemType Directory $Work | Out-Null
if (!(Test-Path "$Obs/.git")) {
    Run git @("clone", "--branch", "31.0.3", "--depth", "1", "--recursive",
        "https://github.com/obsproject/obs-studio.git", $Obs)
}
$Commit = (& git -C $Obs rev-parse HEAD).Trim()
if ($Commit -ne "fcd1910bf5116b69404a6ecdda6efedd1d00ebdf") { throw "Unexpected OBS revision" }
$Top = "$Obs/CMakeLists.txt"
$Original = (& git -C $Obs show HEAD:CMakeLists.txt) -join "`n"
# Build PRIMOW inside OBS's configured dependency graph, without the OBS UI.
Set-Content $Top ($Original + "`nadd_subdirectory(`"$App`" primow-stream)`n")
Push-Location $Obs
try {
    Run cmake @("--preset", "windows-x64", "-DENABLE_UI=OFF", "-DENABLE_BROWSER=OFF",
        "-DENABLE_VLC=OFF", "-DENABLE_SCRIPTING=OFF", "-DENABLE_AJA=OFF",
        "-DENABLE_DECKLINK=OFF", "-DENABLE_WEBRTC=OFF", "-DENABLE_VIRTUALCAM=OFF")
    Run cmake @("--build", "build_x64", "--config", "Release", "--parallel")
    Run cmake @("--install", "build_x64", "--config", "Release", "--prefix", $Stage)
} finally { Pop-Location }
$Deploy = Get-ChildItem "$Obs/.deps" -Recurse -Filter windeployqt.exe | Select-Object -First 1
if (!$Deploy) { throw "windeployqt not found in pinned OBS dependencies" }
Run $Deploy.FullName @("--release", "--no-translations", "--no-opengl-sw",
    "$Stage/bin/64bit/primow-stream.exe")
$Required = @("bin/64bit/primow-stream.exe", "bin/64bit/obs.dll",
    "bin/64bit/libobs-d3d11.dll", "bin/64bit/platforms/qwindows.dll",
    "obs-plugins/64bit/win-capture.dll", "obs-plugins/64bit/win-wasapi.dll",
    "obs-plugins/64bit/obs-nvenc.dll", "obs-plugins/64bit/obs-ffmpeg.dll",
    "obs-plugins/64bit/obs-outputs.dll", "obs-plugins/64bit/rtmp-services.dll")
foreach ($File in $Required) {
    if (!(Test-Path "$Stage/$File")) { throw "Missing runtime file: $File" }
}
New-Item -Force -ItemType Directory "$Stage/THIRD_PARTY_LICENSES" | Out-Null
Copy-Item "$Obs/COPYING" "$Stage/THIRD_PARTY_LICENSES/OBS-GPL.txt"
Copy-Item "$App/THIRD_PARTY_LICENSES/README.md" "$Stage/THIRD_PARTY_LICENSES/"
Copy-Item "$App/README.md" "$Stage/README.md"
@"
@echo off
cd /d "%~dp0bin\64bit"
start "" primow-stream.exe
"@ | Set-Content "$Stage/PRIMOW-STREAM.cmd"
Compress-Archive -Force "$Stage/*" "$Work/PRIMOW-STREAM-Portable.zip"
$ISCC = "${env:ProgramFiles(x86)}/Inno Setup 6/ISCC.exe"
if (!(Test-Path $ISCC)) { throw "Install Inno Setup 6 to generate Setup.exe" }
Run $ISCC @("/DStage=$Stage", "/DOutput=$Work", "$App/packaging/setup.iss")
Write-Host "Build and file inventory complete. Hardware/streaming/clean-machine validation still required."
