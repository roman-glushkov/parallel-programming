$exe     = ".\build\blur.exe"
$image   = "test.bmp"
$radius  = 10
$repeats = 5
$outCsv  = "results.csv"

$coreList    = @(1, 2, 3, 4)
$threadRange = 1..16

"cores,threads,radius,hw,ms,run" | Out-File -Encoding ASCII $outCsv

$totalRuns = $coreList.Count * $threadRange.Count * $repeats
$runIdx    = 0

foreach ($cores in $coreList) {
    $env:BLUR_CORES = "$cores"

    foreach ($n in $threadRange) {
        for ($r = 1; $r -le $repeats; $r++) {
            $runIdx++
            Write-Host "[$runIdx/$totalRuns] cores=$cores threads=$n run=$r ..." -NoNewline

            $line = & $exe $image out_tmp.bmp $n $radius 2>$null

            if (-not $line) {
                Write-Host " FAILED"
                continue
            }

            $parts = $line -split ",\s*"

            if ($parts.Count -lt 4) {
                Write-Host " FAILED (bad: $line)"
                continue
            }

            $thr = $parts[0].Trim()
            $rad = $parts[1].Trim()
            $hw  = $parts[2].Trim()
            $ms  = $parts[3].Trim()

            "$cores,$thr,$rad,$hw,$ms,$r" | Out-File -Append -Encoding ASCII $outCsv
            Write-Host " $ms ms"
        }
    }
}

Remove-Item Env:BLUR_CORES -ErrorAction SilentlyContinue
Remove-Item out_tmp.bmp -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Done. Results -> $outCsv"