# Change these variables according to your PC
$filenameETL = "2xx.etl"
$filenameCAB = "2xx.cab"
$filenamePCAPNG = "2xx.pcapng"
$logFileWindows = "D:\3.VSCodeFiles\TI5\"
$durationSeconds = 60 # 1 minutes

# Start capturing network traffic
Write-Host "Capturing network traffic..."
$process = Start-Process -FilePath "Netsh" -ArgumentList "trace start capture=yes tracefile=$logFileWindows$filenameETL maxSize=0 filemode=single" -NoNewWindow #$($logfile)
Start-Sleep -Seconds 5

# Monitor progress
Write-Host "Monitoring network traffic for $durationSeconds seconds..."
$countdown = $durationSeconds
while ($countdown -gt 0) {
    Start-Sleep -Seconds 10
    $countdown -= 10
    $size = (Get-Item "$logFileWindows$filenameETL").Length
    Write-Host "Captured $($size / 1MB) MB of network traffic so far ($countdown seconds remaining.)"
}

# Stop capturing network traffic
Write-Host "Stopping network capture..."
Invoke-Expression "Netsh trace stop"
Start-Sleep -Seconds 5

#Converting ETL to PCAP(NG) for Wireshark compatibility
Write-Host "Starting conversion..."
Remove-Item "$logFileWindows$filenameCAB"
Invoke-Expression ".\etl2pcapng $filenameETL $filenamePCAPNG"

# Ending script
Start-Sleep -Seconds 5
Write-Host "Done."