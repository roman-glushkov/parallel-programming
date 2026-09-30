# experiment.ps1
# Пробный прогон: 4 конфигурации ядер × 3 значения потоков × 2 повтора = 24 запуска.

$exe    = ".\build\blur.exe"
$image  = "test.bmp"
$radius = 10
$repeats = 5

$coreList = @(1, 2, 3, 4)

$threadRange = @{
    1 = 1..16
    2 = 1..16
    3 = 1..16
    4 = 1..16
}

$outCsv = "results.csv"
"cores,threads,radius,hw,ms,run" | Out-File -Encoding ASCII $outCsv

$totalRuns = 0
foreach ($cores in $coreList) { $totalRuns += $threadRange[$cores].Count * $repeats }
$runIdx = 0

foreach ($cores in $coreList) {
    $env:BLUR_CORES = "$cores"
    foreach ($n in $threadRange[$cores]) {
        for ($r = 1; $r -le $repeats; $r++) {
            $runIdx++
            Write-Host "[$runIdx/$totalRuns] cores=$cores threads=$n run=$r ..." -NoNewline

            $line = & $exe $image out_tmp.bmp $n $radius 2>$null
            if (-not $line) { Write-Host " FAILED"; continue }

            $parts = $line -split ",\s*"
            if ($parts.Count -lt 4) { Write-Host " FAILED (bad: $line)"; continue }

            "$cores,$($parts[0].Trim()),$($parts[1].Trim()),$($parts[2].Trim()),$($parts[3].Trim()),$r" |
                Out-File -Append -Encoding ASCII $outCsv
            Write-Host " $($parts[3].Trim()) ms"
        }
    }
}

Remove-Item Env:BLUR_CORES -ErrorAction SilentlyContinue
Remove-Item out_tmp.bmp -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Done. Results -> $outCsv"