# ================================================================
#  EdgeWake - TinyML Training Data Downloader
#  Downloads ESC-50 dataset and organizes into 3 classes:
#    fire, chainsaw, background
# ================================================================

$ErrorActionPreference = "Stop"
$BASE_DIR = "c:\Users\parth\Downloads\forest_gaurd\training_data"
$ESC50_ZIP = "c:\Users\parth\Downloads\forest_gaurd\esc50.zip"
$ESC50_DIR = "c:\Users\parth\Downloads\forest_gaurd\ESC-50-master"
$ESC50_URL = "https://github.com/karolpiczak/ESC-50/archive/refs/heads/master.zip"

# ---- Create output directories ----
Write-Host ""
Write-Host "========================================"
Write-Host "  EdgeWake Training Data Setup"
Write-Host "========================================"
Write-Host ""

$dirs = @("$BASE_DIR\fire", "$BASE_DIR\chainsaw", "$BASE_DIR\background")
foreach ($d in $dirs) {
    if (!(Test-Path $d)) {
        New-Item -ItemType Directory -Path $d -Force | Out-Null
        Write-Host "[OK] Created: $d"
    }
}

# ---- Step 1: Download ESC-50 dataset ----
Write-Host ""
Write-Host "[1/4] Downloading ESC-50 dataset (~600 MB)..."
Write-Host "      Source: $ESC50_URL"

if (!(Test-Path $ESC50_ZIP)) {
    try {
        Start-BitsTransfer -Source $ESC50_URL -Destination $ESC50_ZIP -DisplayName "ESC-50 Dataset"
        Write-Host "[OK] Download complete!"
    } catch {
        Write-Host "[WARN] BITS failed, trying Invoke-WebRequest..."
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -Uri $ESC50_URL -OutFile $ESC50_ZIP -UseBasicParsing
        Write-Host "[OK] Download complete!"
    }
} else {
    Write-Host "[SKIP] ZIP already exists, skipping download."
}

# ---- Step 2: Extract ZIP ----
Write-Host ""
Write-Host "[2/4] Extracting ESC-50..."

if (!(Test-Path $ESC50_DIR)) {
    Write-Host "      Using tar to extract (this is much faster than Expand-Archive)..."
    tar -xf $ESC50_ZIP -C "c:\Users\parth\Downloads\forest_gaurd\"
    Write-Host "[OK] Extracted to $ESC50_DIR"
} else {
    Write-Host "[SKIP] Already extracted."
}

# ---- Step 3: Read metadata and sort files by class ----
Write-Host ""
Write-Host "[3/4] Sorting audio files by class..."

$csvPath = "$ESC50_DIR\meta\esc50.csv"
if (!(Test-Path $csvPath)) {
    Write-Host "[ERROR] Cannot find esc50.csv at $csvPath"
    exit 1
}

$metadata = Import-Csv $csvPath

# ESC-50 class mappings:
# Class 12: crackling_fire   -> FIRE
# Class 38: chainsaw         -> CHAINSAW
# Background (nature sounds):
# Class  7: insects (crickets)
# Class  9: crow
# Class 10: rain
# Class 11: sea_waves
# Class 13: water_drops
# Class 14: wind
# Class  4: frog

$fireClasses     = @(12)
$chainsawClasses = @(38)
$backgroundClasses = @(7, 9, 10, 11, 13, 14, 4)

$audioDir = "$ESC50_DIR\audio"

$fireCopied = 0
$chainsawCopied = 0
$bgCopied = 0

foreach ($row in $metadata) {
    $filename = $row.filename
    $classId = [int]$row.target
    $srcFile = "$audioDir\$filename"

    if (!(Test-Path $srcFile)) { continue }

    if ($fireClasses -contains $classId) {
        Copy-Item $srcFile "$BASE_DIR\fire\" -Force
        $fireCopied++
    }
    elseif ($chainsawClasses -contains $classId) {
        Copy-Item $srcFile "$BASE_DIR\chainsaw\" -Force
        $chainsawCopied++
    }
    elseif ($backgroundClasses -contains $classId) {
        Copy-Item $srcFile "$BASE_DIR\background\" -Force
        $bgCopied++
    }
}

$fireSeconds = $fireCopied * 5
$chainsawSeconds = $chainsawCopied * 5
$bgSeconds = $bgCopied * 5

Write-Host "[OK] Sorted files:"
Write-Host "     fire:       $fireCopied files ($fireSeconds seconds)"
Write-Host "     chainsaw:   $chainsawCopied files ($chainsawSeconds seconds)"
Write-Host "     background: $bgCopied files ($bgSeconds seconds)"

# ---- Step 4: Resample all files to 16kHz Mono using ffmpeg (if available) ----
Write-Host ""
Write-Host "[4/4] Checking for ffmpeg to resample to 16 kHz Mono..."

$ffmpeg = $null
$ffmpegPaths = @(
    "ffmpeg",
    "C:\ffmpeg\bin\ffmpeg.exe",
    "$env:USERPROFILE\scoop\apps\ffmpeg\current\bin\ffmpeg.exe",
    "C:\ProgramData\chocolatey\bin\ffmpeg.exe"
)

foreach ($fp in $ffmpegPaths) {
    try {
        $null = & $fp -version 2>&1
        $ffmpeg = $fp
        break
    } catch { continue }
}

if ($ffmpeg) {
    Write-Host "[OK] Found ffmpeg: $ffmpeg"
    
    $resampledDir = "$BASE_DIR\resampled"
    $resampledDirs = @("$resampledDir\fire", "$resampledDir\chainsaw", "$resampledDir\background")
    foreach ($d in $resampledDirs) {
        if (!(Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
    }

    $classes = @("fire", "chainsaw", "background")
    foreach ($class in $classes) {
        $files = Get-ChildItem "$BASE_DIR\$class\*.wav"
        $count = 0
        foreach ($f in $files) {
            $outFile = "$resampledDir\$class\$($f.Name)"
            if (!(Test-Path $outFile)) {
                & $ffmpeg -y -i $f.FullName -ar 16000 -ac 1 -sample_fmt s16 $outFile 2>$null
                $count++
            }
        }
        Write-Host "     Resampled $count $class files to 16 kHz mono"
    }
    
    Write-Host ""
    Write-Host "[OK] Resampled files are in: $resampledDir"
    Write-Host "     Upload the 'resampled' folder contents to Edge Impulse."
} else {
    Write-Host "[WARN] ffmpeg not found. Files are 44.1 kHz (ESC-50 original)."
    Write-Host "       Edge Impulse can resample automatically during upload."
    Write-Host "       For best results, install ffmpeg first:"
    Write-Host "         winget install Gyan.FFmpeg"
    Write-Host "       Then re-run this script."
    Write-Host ""
    Write-Host "[OK] Upload files directly from: $BASE_DIR"
}

# ---- Summary ----
Write-Host ""
Write-Host "========================================"
Write-Host "  DONE - SUMMARY"
Write-Host "========================================"
Write-Host ""
Write-Host "  Training data ready at:"
Write-Host "    $BASE_DIR"
Write-Host ""
Write-Host "  Classes:"
Write-Host "    fire/       - $fireCopied clips ($fireSeconds sec)"
Write-Host "    chainsaw/   - $chainsawCopied clips ($chainsawSeconds sec)"
Write-Host "    background/ - $bgCopied clips ($bgSeconds sec)"
Write-Host ""
Write-Host "  Next step:"
Write-Host "    1. Go to Edge Impulse Studio -> Data acquisition -> Add data"
Write-Host "    2. Upload fire/*.wav with label 'fire'"
Write-Host "    3. Upload chainsaw/*.wav with label 'chainsaw'"
Write-Host "    4. Upload background/*.wav with label 'background'"
Write-Host "    5. Set split: 80% training / 20% testing"
Write-Host ""
