# Generates the register map from PrecRtdSim.xlsm without the sheet buttons (ActiveX - inert where Office disables
# ActiveX): the same VBA functions as "Generate register map", "Generate Modbus" and "Generate LeBin json", run through
# Excel COM, then the copies the buttons do - into this repo instead of the paths in the sheet.
#     pwsh Documents\generate_regmap.ps1 [-Tools <jac-sw-dhw21x-tools path>]
# -Tools also copies the Modbus JSON to the tools repo (RtdEmulPrecRegs.py there has a hand-added class - merge by hand).
param([string]$Tools)
$ErrorActionPreference = 'Stop'
$docs = $PSScriptRoot
$repo = Split-Path $docs
$xl = New-Object -ComObject Excel.Application
try {
    $xl.DisplayAlerts = $false
    $xl.AutomationSecurity = 1   # macros of this workbook enabled for this automation instance only
    $wb = $xl.Workbooks.Open((Join-Path $docs 'PrecRtdSim.xlsm'), 0, $true)
    $wb.Activate()
    $ws = $wb.Worksheets.Item('Registers')
    $common = $wb.Worksheets.Item('Common')
    $xl.Calculate()
    $name = $xl.Run('GetWorkName')
    $prefix = $ws.Range('F5').Value2
    [void]$xl.Run('CreateDefineFile', $ws.Range('K18:M2018'), $prefix, "${name}_Define.txt")
    [void]$xl.Run('CreateStorageFile', $ws.Range('F18:N2028'), $ws.Range('F4').Value2, $common.Range('C11:D36'), "${name}_Storage.txt")
    [void]$xl.Run('CreateFactoryFile', $ws.Range('L18:L2058'), $ws.Range('N18:N2058'), $ws.Range('J18:J2058'), $ws.Range('G18:G2058'), $prefix, "${name}_Factory.txt")
    [void]$xl.Run('RegMap_Generate', 'reg_map_temp', 'reg_map', "${name}_Define.txt", "${name}_Storage.txt", "${name}_Factory.txt")
    [void]$xl.Run('CreateJsonFileModbus', $ws.Range('E18:T2018'), "${name}_Modbus.json", 'mb_rtu_app')
    [void]$xl.Run('CreatePythonClass', $ws.Range('E18:N2018'), "${name}Regs")
    [void]$xl.Run('CreateJsonFileCSharp', $ws.Range('E18:T2018'), "${name}_registers.json", 1.01)
    $wb.Close($false)
} finally {
    $xl.Quit()
    [void][Runtime.InteropServices.Marshal]::ReleaseComObject($xl)
}
$txts = Join-Path $docs 'txts'
$app = Join-Path $repo 'Firmware\PrecRtdSim-APP\Core'
$tests = Join-Path $repo 'Tests\prec-rtd-sim'
Copy-Item (Join-Path $txts 'reg_map.c'), (Join-Path $txts 'reg_map.h') (Join-Path $app 'Common')
Copy-Item (Join-Path $txts 'mb_rtu_app.c'), (Join-Path $txts 'mb_rtu_app.h') (Join-Path $app 'Serial')
Copy-Item (Join-Path $txts "${name}_Modbus.json"), (Join-Path $txts "${name}Regs.py"), (Join-Path $txts "${name}_registers.json") $tests
if ($Tools) {
    Copy-Item (Join-Path $txts "${name}_Modbus.json") (Join-Path $Tools 'VeriFrame\Tools\RTDEmul\Prec\RtdEmulPrec_Modbus.json')
}
"generated: reg_map.*, mb_rtu_app.*, ${name}_Modbus.json, ${name}Regs.py, ${name}_registers.json"
