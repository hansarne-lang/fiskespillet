# Regression tests for FiskRunAllLib.ps1 (Run-All result classification).
# Run: powershell -NoProfile -ExecutionPolicy Bypass -File Tools\Testing\Test-RunAllLib.ps1
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'FiskRunAllLib.ps1')

$ns = 'xmlns="http://schemas.microsoft.com/powershell/2004/04"'
$progress = '<Obj S="progress" RefId="0"><TN RefId="0"><T>System.Management.Automation.PSCustomObject</T><T>System.Object</T></TN><MS><I64 N="SourceId">1</I64><PR N="Record"><AV>Preparing modules for first use.</AV><AI>0</AI><Nil /><PI>-1</PI><PC>-1</PC><T>Completed</T><SR>-1</SR><SD> </SD></PR></MS></Obj>'
$progress2 = $progress.Replace('RefId="0"', 'RefId="1"')
$cases = [ordered]@{
    'empty'                     = @('', $false)
    'whitespace'                = @("  `r`n ", $false)
    'progress only'             = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns>$progress</Objs>", $false)
    'two progress records'      = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns>$progress$progress2</Objs>", $false)
    'progress + S Error'        = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns>$progress<S S=""Error"">A real error_x000D__x000A_</S></Objs>", $true)
    'only S Error'              = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns><S S=""Error"">boom</S></Objs>", $true)
    'Obj with other stream'     = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns><Obj S=""warning"" RefId=""0""/></Objs>", $true)
    'Obj without stream'        = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns><Obj RefId=""0""/></Objs>", $true)
    'empty CLIXML document'     = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns></Objs>", $true)
    'header without body'       = @("#< CLIXML", $true)
    'malformed CLIXML'          = @("#< CLIXML`r`n<Objs><Obj S=""progress"">", $true)
    'wrong root element'        = @("#< CLIXML`r`n<Other $ns>$progress</Other>", $true)
    'text after progress'       = @("#< CLIXML`r`n<Objs Version=""1.1.0.1"" $ns>$progress</Objs>`r`nException: failed", $true)
    'plain text error'          = @('Exception calling "Start": failed', $true)
}
$failed = 0
foreach ($case in $cases.GetEnumerator()) {
    $actual = Test-ErrorText $case.Value[0]
    $expected = $case.Value[1]
    if ($actual -ne $expected) { $failed++; "FAIL  $($case.Key): expected $expected, got $actual" } else { "ok    $($case.Key)" }
}
$file = [IO.Path]::GetTempFileName()
try {
    [IO.File]::WriteAllText($file, '')
    if (Test-ErrorOutput $file) { $failed++; 'FAIL  empty file' } else { 'ok    empty file' }
    [IO.File]::WriteAllText($file, $cases['progress + S Error'][0])
    if (-not (Test-ErrorOutput $file)) { $failed++; 'FAIL  mixed file' } else { 'ok    mixed file' }
} finally { Remove-Item -LiteralPath $file -ErrorAction SilentlyContinue }
if (-not (Test-ErrorOutput (Join-Path $PSScriptRoot 'does-not-exist.err'))) { 'ok    missing file' } else { $failed++; 'FAIL  missing file' }
if ($failed) { throw "$failed classification test(s) failed" }
'All Run-All classification tests passed.'
